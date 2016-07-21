#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "tokenizer.h"
#include "mystring.h"
#include "helper.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

node* execute(node* jobList, char* largs[], char* rargs[], char* input, char* in, char* out, int pipestatus, int back, int jc, int error){
	int pidl;
	int pidr = 0;
	int pg;
	int status;
	int count = 0;
	int new_outno;
	int new_inno;
	int fd[2];
	char* job;
	node* current;
	node* recentJob;

	//error checking block
	if (error < 0) {
		if (error == -1) {
			write(1, "Error: Invalid I/O Input\n", 26);
			return jobList;
		}

		if (error == -2) {
			write(1, "Error: Invalid Pipe Input\n", 27);
			return jobList;
		}

		if (error == -3) {
			write(1, "Error: Invalid Background Input\n", 33);
			return jobList;
		}

		if (error == -4) {
			write(1, "Error: Invalid fg or bg input\n", 31);
			return jobList;
		}
	}

	//fg
	if (jc == 1) {
		//find a stopped job
		recentJob = findStopped(jobList);
		if (recentJob == NULL) {
			recentJob = jobList; // get the top of the stack
		}

		//this means that there are 0 background jobs
		if (recentJob == NULL) {
			printf("Error: No background jobs\n");
		}

		//send cont signal, give tc to the group
		else {
			if (killpg(recentJob->pgid, SIGCONT) < 0) perror("kill");
			if (tcsetpgrp(STDIN_FILENO, recentJob->pgid) < 0) perror("tcsetpgrp fg");

			//get the pid and pg
			pidl = recentJob->pid;
			pg = recentJob->pgid;
			pipestatus = recentJob->pipe;
			job = malloc(slen(recentJob->command) + 1);
			scopy(recentJob->command, job);

			//if pipe, get the other pid too
			if(pipestatus) {
				jobList = delete(jobList, recentJob->pid);
				recentJob = getg(jobList, pg);
				pidr = recentJob->pid;
			}

			//delete the job off the bg list
			jobList = delete(jobList, recentJob->pid);

			//print job on the shell
			printf("%s\n", job);

			//wait for both processes to finish
			waitpid(pidl, &status, WUNTRACED);
			waitpid(pidr, &status, WUNTRACED);

			// if foreground was stopped, place this job onto the list, notify it was stopped
			if (WIFSTOPPED(status)) {
				printf("\nStopped: %s\n", job);
				jobList = push(jobList, job, pidl, getpgid(pidl), pipestatus, 1);
				if (pipestatus) jobList = push(jobList, job, pidr, getpgid(pidl), pipestatus, 1);
			}

			//free job (i wish)
			free(job);

			//take back control of tc
			if (tcsetpgrp(STDIN_FILENO, getpgid(0)) < 0) perror("tcsetpgrp parent");

			jobList = bgWait(jobList);
		}
		return jobList;
	}

	//bg
	if (jc == 2) {
		// find a stopped job
		recentJob = findStopped(jobList);

		//if there are no stopped jobs, move on
		if (recentJob == NULL) {
			printf("Error: No stopped background jobs\n");
		}

		//otherwise, give signal to that job to continue
		else {
			if (killpg(recentJob->pgid, SIGCONT) < 0) perror("kill");
			pg = recentJob->pgid;
			jobList = changeStopped(jobList, pg);
			printf("Running : %s\n", recentJob->command);
		}

		// poll for background processes
		jobList = bgWait(jobList);

		//return
		return jobList;
	}

	//set up pipe fd's
	if (pipestatus) {
		if (pipe(fd) < 0) perror("Error");
	}

	// fork - twice if there's a pipe
	if ((pidl = fork()) < 0) {
		perror("Error");
	}

	//left command child branch
	if(pidl == 0) {
		//set all signals to default
		signal(SIGTTOU, SIG_DFL);
		signal(SIGTTIN, SIG_DFL);
		signal(SIGTERM, SIG_DFL);
		signal(SIGTSTP, SIG_DFL);
		signal(SIGINT, SIG_DFL);

		// set pgid
		if (setpgid(getpid(), getpid()) < 0) perror("setpgid");

		// give tc if not background (both in child and parent for rc)
		if (!back) {
			if (tcsetpgrp(STDIN_FILENO, getpgid(0)) < 0) perror("tcsetpgrp child");
		}

		//close input + dup2 out to the pipe
		if (pipestatus) {
			if (close(fd[0]) < 0) perror("Error");
			if(dup2(fd[1], 1) < 0) perror("Error");
		}

		// set I/O redirections in child process - stdout
		if (out != NULL && (!pipestatus)) {
			new_outno = open(out, O_WRONLY | O_TRUNC | O_CREAT, 0644); //to do - error check
			if (dup2(new_outno, STDOUT_FILENO) < 0) {
				perror("Error");
				exit(1);
			}
		}

		//stdin
		if (in != NULL) {
			new_inno = open(in, O_RDONLY); //to do - error check
			if (dup2(new_inno, STDIN_FILENO) < 0) {
				perror("Error");
				exit(1);
			}
		}

		// exec
		if (execvp(largs[0], largs) < 0) {
			perror("Error");
			exit(1);
		}
	}

	//PARENT
	else {
		// SECOND PROCESS BRANCH
		if (pipestatus) {
			if((pidr = fork()) < 0) {
				perror("Error");
			}

			// second child branch
			else if(pidr == 0) {

				//set all signals to default
				signal(SIGTTOU, SIG_DFL);
				signal(SIGTTIN, SIG_DFL);
				signal(SIGTERM, SIG_DFL);
				signal(SIGTSTP, SIG_DFL);
				signal(SIGINT, SIG_DFL);

				// set group id if foreground
				if (setpgid(getpid(), pidl) < 0) perror("Error");

				// give tc if not background (both in child and parent for rc)
				if (!back) {
					if (tcsetpgrp(STDIN_FILENO, getpgid(0)) < 0) perror("tcsetpgrp child");
				}

				//close output + dup2 input from pipe
				if (close(fd[1]) < 0) perror("Error");
				if (dup2(fd[0], 0) < 0) perror("Error");

				// set I/O redirections in child process - stdout
				if (out != NULL) {
					new_outno = open(out, O_WRONLY | O_TRUNC | O_CREAT, 0644); //to do - error check
					if (dup2(new_outno, STDOUT_FILENO) < 0) {
						perror("Error");
						exit(1);
					}
				}

				// exec
				if (execvp(rargs[0], rargs) < 0) {
					perror("Error");
					exit(1);
				}
			}
		}

		//PARENT BRANCH:
		//close both pipes in the parent
		if (pipestatus) {
			if (close(fd[0]) < 0) perror("Error");
			if (close(fd[1]) < 0) perror("Error");
		} 

		//set groups
		if (setpgid(pidl, pidl) < 0) perror("setpgid"); 
		if (pipestatus) {
			if (setpgid(pidr, pidl) < 0) perror("setpgid");
		}

		//if background, put job into the list
		if (back) {
			jobList = push(jobList, input, pidl, getpgid(pidl), pipestatus, 0);
			// also put the right side on the list if pipe
			if (pipestatus) jobList = push(jobList, input, pidr, getpgid(pidr), pipestatus, 0);
		}

		//for foreground
		if (!back) {
			//set tcpgrp for child/children
			if (tcsetpgrp(STDIN_FILENO, pidl) < 0) perror("tcsetpgrp child");

			//wait for both child processes to finish if not bg
			waitpid(pidl, &status, WUNTRACED);
			waitpid(pidr, &status, WUNTRACED);

			// if foreground was stopped, place this job onto the list, notify it was stopped
			if (WIFSTOPPED(status)) {
				printf("\nStopped: %s\n", input);
				jobList = push(jobList, input, pidl, getpgid(pidl), pipestatus, 1);
				if (pipestatus) jobList = push(jobList, input, pidr, getpgid(pidl), pipestatus, 1);
			}

			//take back control of tc
			if (tcsetpgrp(STDIN_FILENO, getpgid(0)) < 0) perror("tcsetpgrp parent");
		}

		// wait for background
		jobList = bgWait(jobList);
		return jobList;
	}
	//should never get here but to fend off warnings
	return jobList;
}

node* bgWait(node* jobList) {
	int pid, pg;
	char* job;
	int status;
	node* current;

	// for background processes that might still be running
	while (1) {
		pid = waitpid(-1, &status, WNOHANG | WUNTRACED);

		//if 0 or < 0, either no change or no jobs to wait for
		if (pid <= 0) break;

		// job was stopped - report
		else if (WIFSTOPPED(status)) {
			current = get(jobList, pid);
			job = current->command;
			current->stopped = 0;
			printf("Stopped: %s\n", job);
		}

		// background job is finished - report
		else if (pid > 0) {
			current = get(jobList, pid);

			//pipe case
			if (current->pipe) {
				// get the group of the first process that finished, delete it
				pg = current->pgid;
				jobList = delete(jobList, pid);

				//get the node of the same group, change its pipe to 0
				jobList = changePipe(jobList, pg);
			}
			//non-pipe case - simpler
			else {
				job = current->command;
				printf ("Finished: %s\n", job);
				jobList = delete (jobList, pid);
			}
		}
	}
	return jobList;
}
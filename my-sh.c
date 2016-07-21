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

int main() {
	// initialize variables
	TOKENIZER* tokenizer;
	char input[1024] = "";
	char* tok;
	char* largs[342];
  char* rargs[342];
	char* new_out = NULL;
	char* new_in = NULL;
  char* next = NULL;
  node* jobList = NULL;
	int br;
	int counter = 0;
	int out = 0;
	int in = 0;
	int error = 0;
  int pipe = 0;
  int back = 0;
  int jc = 0;

	input[1023] = '\0';
	write(1, "$ ", 2);

  // ignore signals
  signal(SIGTTOU, SIG_IGN);
  signal(SIGTTIN, SIG_IGN);
  signal(SIGTERM, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGINT, SIG_IGN);

	// wait for input
	while ((br = read(STDIN_FILENO, input, 1024)) > 0) {
		if (br <= 1) continue;
		// remove \n and replace with null
		input[br - 1] = '\0';

		//tokenizing and putting tokens into the right variables
		tokenizer = init_tokenizer(input);
    	while( (tok = get_next_token( tokenizer )) != NULL ) {
        //check for fg
        if (scmp(tok, "fg") == 1) {
          if (counter == 0) jc = 1;
          if ((next = get_next_token(tokenizer)) != NULL) error = -4;
        }

        //check for bg
        if (scmp(tok, "bg") == 1) {
          if (counter == 0) jc = 2;
          if ((next = get_next_token(tokenizer)) != NULL) error = -4;
        }

        //check for ampersand
        if (scmp(tok, "&") == 1) {
          if ((next = get_next_token(tokenizer)) != NULL) error = -3;
          back = 1;
        }

        //check for pipe
        else if (scmp(tok, "|") == 1) {
          //check if pipe has already been used or if there is nothing to the left of it
          if (pipe || counter == 0) error = -2;            

          // check if there is already an out in the left of the pipe
          if (out == 1) error = -1;

          //reset counter for right side + null end the left side
          pipe = 1;
          largs[counter] = NULL;
          counter = 0;
        }

    		//check for I/O Redirection - stdout
    		else if (scmp(tok, ">") == 1) {
    			if (out || counter == 0) {
    				error = -1;
    			}

          //set flag for out and get the filename
    			out = 1;
    			new_out = get_next_token(tokenizer);

          // check if there was anything after - if not error
    			if (new_out == NULL || scmp(new_out, ">") == 1) error = -1;
    		}

    		//stdin
    		else if (scmp(tok, "<") == 1) {
          // check if there's already a < or if there's nothing to the left of it
    			if(in || counter == 0) {
    				error = -1;
    			}

          //check if < appears on the right side of pipe
          if (pipe) {
            error = -1;
          }

          //set flag for in and get the filename
    			in = 1;
    			new_in = get_next_token(tokenizer);

          // check if there was anything after- if not error
    			if (new_in == NULL || scmp(new_in, "<") == 1) error = -1;
    		}

		    //if not a delimiter, add tokens into the arguments array
    		else {
          // right side of pipe
          if (pipe) {
        		rargs[counter] = malloc(slen(tok) + 1);

        		//malloc check
        		if (rargs[counter] == NULL) {
        			printf("Out of memory!\n");
        			exit(1);
        		}

            //copy token into args array
            scopy(tok, rargs[counter]);
            counter++;
          }

          // leftside
          else {
            largs[counter] = malloc(slen(tok) + 1);

            //malloc check
            if (largs[counter] == NULL) {
              printf("Out of memory!\n");
              exit(1);
            }
      	    
            //copy token into args array
  			    scopy(tok, largs[counter]);
  			    counter++;
          }
        }
        //free memory
        free(tok);
      }

    	// NULL end the args arrays - care for pipe
    	if (pipe) rargs[counter] = NULL;
      else {
        largs[counter] = NULL;
        rargs[0] = NULL;
      }

      //to show running, take off ampersand first
      if (back && error == 0) {
        input[br - 2]= '\0';
        printf("Running : %s\n", input);
      }

      //execute with args + I/O files
    	jobList = execute(jobList, largs, rargs, input, new_in, new_out, pipe, back, jc, error);

    	//free memory
    	free_tokenizer(tokenizer);
      counter = 0;
    	while(largs[counter] != NULL) {
    		free(largs[counter]);
    		counter++;
    	}
      counter = 0;
      while(rargs[counter] != NULL) {
        free(rargs[counter]);
        counter++;
      }
    	if(out) free(new_out);
    	if(in) free(new_in);
      if(jc || back) free(next);

    	//reset variables
    	counter = 0;
    	in = 0;
    	out = 0;
    	error = 0;
      pipe = 0;
    	new_out = NULL;
    	new_in = NULL;
      back = 0;
      jc = 0;
    	write(1, "$ ", 2);
	}
  //free stuff from jobList if things were sleeping when exited
  while (jobList != NULL) {
    jobList = pop(jobList);
  }
}
README

Name: Jay Jung
Eniac Username: jayjung

List of files:
my-sh.c
helper.h/helper.c
node.h/node.c
mystring.h/mystring.c
tokenizer.h/tokenizer.c

Compilation: make with all of the source files in the folder

Overview: All parts have been finished

Description:
my-sh.c: the main file where the main loop is located. Mainly takes in user input, handles it, and passes it into the execute function in the helper file. 
Catches I/O redirection files, pipes, background processes, and fg/bg commands.

helper.c: the helper file where the main forking/execution is done. the execute method takes in the parsed arguments from my-sh, forks a new process and 
then executes the appropriate command with the arguments passed in. Handles all of the other functionalities as well - 
I/O redirects, pipes, polling for background processes, and bringing processes to the foreground.

mystring.c: mostly my own string commands that I needed for the project - contains my own versions of strlen, strcpy, and strcmp.

node.c: linkedlist (actually more like an iterable stack) used to keep track of background processes - 
stores the process's pid, pgid, whether it is part of a pipe, and if it is stopped or not.

tokenizer.c: given

Mainly, the code will start at my-sh and wait for user input - once it is given, the tokenizer will parse the input and put each token into appropriate places - 
the two parts of the pipe and the in/out filenames. It also takes note of whether there is an ampersand, a pipe, or if the command was fg or bg. 
Then, it passes all of this information into the execute function.

The execute function handles the basic forking/exec-ing. It also takes into consideration all of the considerations like I/O, pipe, and background. 
It will then wait for the current foreground process passed in (if there was one), and then wait for any background processes to harvest before exiting.

In the case of I/O, it will open/dup2 the appropriate files

In the case of pipe, it will fork a second child process for the right side of the pipe along with any I/O that needs to be done

In the case of an ampersand, it will bypass the foreground waiting and simply add the process to the list to keep track of.

In the case of bg - it will scan the list of background processes for the first one stopped. 
Because it is a stack, the first one that it comes across is the one that was most recently stopped and inserted into the list. 
It then gives that process SIGCONT

In the case of fg - it will also scan the list for the first bg process stopped - 
if there isn't one, it will just take the first process on the list since it is the most recent job. 
Whichever of the two happens, it will pass on the signal, give it terminal control, and then wait for that job to finish. 

Extra Credit:
Because I put the background processes in a stack, bg/fg works on all processes in the list, multiple times, 
not just on the most recent one. However, it still cannot bg/fg ANY process on the stack by calling its name.




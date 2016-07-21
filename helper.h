#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include "tokenizer.h"
#include "mystring.h"
#include "node.h"

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

node* execute(node* jobList, char* largs[], char*rargs[], char* input, char* in, char* out, int pipe, int back, int jc, int error);

node* bgWait(node* jobList);
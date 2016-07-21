#include <stdlib.h>
#include <stdio.h>
#include "mystring.h"

typedef struct node {
	int pid;
	int pgid;
	int pipe;
	int stopped;
	char* command;
	struct node *next;
} node;

/*push a node onto a stack with pgid and its job command*/
node* push (node *head, char* s, int p, int pg, int pipestatus, int stopped);

/* pop a node from the stack with pgid and its job command*/
node* pop (node *head);

/* get the the node with input pid*/
node* get (node *head, int p);

/* get the node with input pgid */
node* getg (node *head, int pg);

/* delete the node with the given pid */
node* delete(node *head, int p);

/* change the pipestatus of the other process in the list given pgid */
node* changePipe(node *head, int pg);

/* change the stoppedstatus of all nodes with pgid to 0 */
node* changeStopped(node *head, int pg);

/* finds the stopped process*/
node* findStopped(node *head);

/*print pgids of nodes - testing purposes */
void printList(node *head);
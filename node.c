#include "node.h"
#include <stdlib.h>
#include <stdio.h>
#include "mystring.h"

// insert a node into the head of the list
node* push (node *head, char* s, int p, int pg, int pipestatus, int stoppedStatus) {
	// make new node, set new node's fields
	node *new_node = malloc (sizeof (node));
		
	// check if malloc succeeded
	if (new_node == NULL) {
		printf("ERROR: no space on heap");
		return NULL;
	}
	
	// if stack is already empty, make the head the new_node
	if (head == NULL) {
		head = new_node;
		
		head->command = malloc(slen(s) + 1);
		scopy(s, head->command);
		 
		head->pipe = pipestatus;
		head->pgid = pg;
		head->pid = p;
		head->stopped = stoppedStatus;
		head->next = NULL;
		return head;
	}
	
	// initialize fields
	new_node->command = malloc(slen(s) + 1);
	scopy(s, new_node->command);
	new_node->pid = p;
	new_node->pgid = pg;
	new_node->pipe = pipestatus;
	new_node->stopped = stoppedStatus;
	new_node->next = head;
	
	// set the head to new node
	head = new_node;
	return head;
}

node* pop (node* head) {
	if (head == NULL) {
		printf("stack of jobs is empty\n");
		return NULL;
	}

	//assign temp head node to free
	node* temp = head; 

	//assign head to be the next
	head = head->next;

	//free old head
	free(temp->command);
	free(temp);
	return head;
}

node* get (node *head, int p) {
	// null check...does stack have any elements?
	if (head == NULL) {
		return NULL;
	}

	// initialize temp pointers
	node *current;
	current = head;
	
	// iterate through the stack
	while (1) {
		if (current == NULL) break;
		if (current->pid == p) break;
		current = current -> next;
	}
	if (current == NULL) return NULL;
	return current;
}

node* getg (node *head, int pg) {
	// null check...does stack have any elements?
	if (head == NULL) {
		return NULL;
	}

	// initialize temp pointers
	node *current;
	current = head;
	
	// iterate through the stack
	while (1) {
		if (current == NULL) break;
		if (current->pid == pg) break;
		current = current -> next;
	}
	if (current == NULL) return NULL;
	return current;
}

node* delete(node *head, int p) {
	//null check
	if (head == NULL) {
		return NULL;
	}

	// initialize temp pointers
	node* current;
	node* temp;
	current = head;

	// first check if head is the element we're looking for
	if (current->pid == p) {
		return pop(head);
	}

	//iterate through the stack
	while(1) {
		if (current->next == NULL) break;
		if (current->next->pid == p) break;
		current = current -> next;
	}
	
	//if current->next is null, then we didn't find the element, return same list
	if (current->next == NULL) return head;

	// otherwise, current->next is target
	temp = current->next;
	current->next = temp->next;
	free(temp->command);
	free(temp);
	return head;
}	

node* changePipe(node *head, int pg) {
	// null check...does stack have any elements?
	if (head == NULL) {
		return NULL;
	}

	// initialize temp pointers
	node *current;
	current = head;
	
	// iterate through the stack
	while (1) {
		if (current == NULL) break;
		if (current->pgid == pg) break;
		current = current -> next;
	}
	if (current == NULL) return NULL;
	current->pipe = 0;
	return head;
}

node* findStopped(node *head) {
	// initialize temp pointers
	node *current;
	current = head;

	// iterate through the stack
	while (1) {
		if (current == NULL) break;
		if (current->stopped) break;
		current = current->next;
	}

	return current;
}

node* changeStopped(node *head, int pg) {
	if (head == NULL) return head;
	// initialize temp pointers
	node *current;
	current = head;

	//iterate through the stack
	while(current != NULL) {
		if (current == NULL) break;
		if (current->pgid == pg) {
			current->stopped = 0;
		}
		current = current->next;
	}

	return head;
}

void printList(node* head) {
	// initialize temp pointers
	node *current;
	current = head;
	
	printf("[TOP]\n");
	
	// iterate through the stack, printing each one
	while (1) {
		if (current == NULL) break;
		int numb = current -> pid;
		printf("%i\n", numb);
		current = current -> next;
	}
	
	printf("[BOTTOM]\n");
}
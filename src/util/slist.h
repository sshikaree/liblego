#ifndef SLIST_H
#define SLIST_H

#include <stddef.h>
#include <stdbool.h>

#include <tidy/tidy.h>

typedef struct NodeListElement {
	struct NodeListElement* next;
	TidyNode				tnode;
} NodeListElement;


typedef struct	NodeList {
	NodeListElement*	first;
	NodeListElement*	last;
	size_t				size;
} NodeList;



NodeList* NodeList_new(void);
void NodeList_destroy(NodeList* lst);
//void NodeList_destroyFull(NodeList* lst);

NodeListElement* NodeList_append(NodeList* lst, TidyNode tnode);
NodeListElement* NodeList_prepend(NodeList* lst, TidyNode tnode);
NodeList* NodeList_reverse(NodeList* lst);
NodeList* NodeList_remove(NodeList* lst, TidyNode tnode, bool remove_all);







#endif // SLIST_H

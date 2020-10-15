#include "slist.h"

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>

/// Creates new list of #TidyNode.
NodeList* NodeList_new() {
	NodeList* lst = (NodeList*)malloc(sizeof(NodeList));
	if (!lst) {
		return NULL;
	}
	lst->first = NULL;
	lst->last  = NULL;
	lst->size  = 0;

	return lst;
}


/// Deletes list. Each TidyNode should be deleted separately.
void NodeList_destroy(NodeList* lst) {
	if (!lst) {
		return;
	}
	for (NodeListElement* elm = lst->first; elm; ) {
		NodeListElement* tmp = elm;
		elm = elm->next;
		free(tmp);
	}
	free(lst);
}

//void NodeList_destroyFull(NodeList* lst) {
//	if (!lst) {
//		return;
//	}
//	NodeListElement* tmp = NULL;
//	for (NodeListElement* elm = lst->first; elm; ) {
//		tmp = elm;
//		elm = elm->next;
//		free(tmp->tnode);
//		free(tmp);
//	}
//	free(lst);
//}

/// Adds a new element to the end of the list.
/// It creates new SList if @lst == NULL.
/// @returns pointer to the new element of NULL if malloc() fails.
NodeListElement* NodeList_append(NodeList* lst, TidyNode tnode) {
	NodeListElement* elm = (NodeListElement*)malloc(sizeof(NodeListElement));
	if (!elm) {
		return NULL;
	}
	elm->next = NULL;
	elm->tnode = tnode;

	if (!lst) {
		// Should we create new list here or just return NULL?
		lst = NodeList_new();
		if (!lst) {
			free(elm);
			return NULL;
		}
		lst->first	= elm;
		lst->last	= elm;
	} else {
		// list is empty
		if (lst->size == 0) {
			lst->first	= elm;
			lst->last	= elm;
		} else {
			lst->last->next = elm;
			lst->last = elm;
		}
	}
	lst->size++;
	return elm;
}


/// Reverse list order.
NodeList* NodeList_reverse(NodeList* lst) {
	if (!lst || !lst->first) {
		return lst;
	}
	NodeListElement* prev	= NULL;
	NodeListElement* next 		= NULL;
	for (NodeListElement* elm = lst->first; elm; ) {
		next = elm->next;
		elm->next = prev;
		prev = elm;
		elm = next;
	}
	prev = lst->first;
	lst->first = lst->last;
	lst->last = prev;
	return lst;
}


/// Adds new element to the beginning of the list.
/// It creates new list if @lst == NULL.
/// @returns pointer to the new element or NULL if malloc() failed.
NodeListElement* NodeList_prepend(NodeList* lst, TidyNode tnode) {
	NodeListElement* elm = (NodeListElement*)malloc(sizeof(NodeListElement));
	if (!elm) {
		return NULL;
	}
	elm->tnode = tnode;
	// Should we create new list here or just return NULL?
	if (!lst) {
		lst = NodeList_new();
		if (!lst) {
			free(elm);
			return NULL;
		}
		elm->next	= NULL;
		lst->first	= elm;
		lst->last	= elm;
		lst->size	= 1;
	} else {
		elm->next	= lst->first;
		lst->first	= elm;
		lst->size++;
	}

	return elm;
}

/// Removes found elements with corresponding data from the list.
/// If @remove_all is false removes only first matching element.
NodeList* NodeList_remove(NodeList* lst, TidyNode tnode, bool remove_all) {
	if (!lst || !lst->first) {
		return lst;
	}
	NodeListElement* tmp = NULL;
	// check first element
	if (lst->first->tnode == tnode) {
		tmp = lst->first;
		lst->first = lst->first->next;
		free(tmp);
		lst->size--;
		if (remove_all == false) {
			return lst;
		}
	}

	for (NodeListElement* elm = lst->first; elm && elm->next; elm = elm->next) {
		if (elm->next->tnode == tnode) {
			tmp = elm->next;
			elm->next = elm->next->next;
			free(tmp);
			lst->size--;
			if (remove_all == false) {
				return lst;
			}
		}
	}

	return lst;
}

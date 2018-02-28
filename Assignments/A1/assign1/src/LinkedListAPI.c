#include <stdlib.h>
#include <assert.h>
#include "LinkedListAPI.h"

List initializeList(char *(*printFunction)(void *toBePrinted), void (*deleteFunction)(void *toBeDeleted), int (*compareFunction)(const void *first, const void *second)) {
	List list;

	assert(deleteFunction != NULL);
	assert(compareFunction != NULL);
	assert(printFunction != NULL);

	list.head = NULL;
	list.tail = NULL;
	list.length = 0;
	list.deleteData = deleteFunction;
	list.compareData = compareFunction;
	list.printData = printFunction;

	return list;
}

Node *initializeNode(void *data) {
	Node *node = malloc(sizeof(Node));

	if (node == NULL) {
		return NULL;
	}

	node->next = NULL;
	node->previous = NULL;
	node->data = data;

	return node;
}

void insertFront(List *list, void *toBeAdded) {
	if (list == NULL || toBeAdded == NULL) {
		return;
	}

	Node *node = initializeNode(toBeAdded);

	if (list->head == NULL && list->tail == NULL) {
		list->head = node;
		list->tail = list->head;
	} else {
		node->next = list->head;
		list->head->previous = node;
		list->head = node;
	}

	list->length++;
}

void insertBack(List *list, void *toBeAdded) {
	if (list == NULL || toBeAdded == NULL) {
		return;
	}

	Node *node = initializeNode(toBeAdded);

	if (list->head == NULL && list->tail == NULL) {
		list->head = node;
		list->tail = list->head;
	} else {
		node->previous = list->tail;
		list->tail->next = node;
		list->tail = node;
	}

	list->length++;
}

void clearList(List *list) {
	if (list == NULL) {
		return;
	}

	Node *node;

	while (list->head != NULL) {
		list->deleteData(list->head->data);

		node = list->head;
		list->head = list->head->next;

		free(node);
	}

	list->head = NULL;
	list->tail = NULL;
}

void insertSorted(List *list, void *toBeAdded) {
	if (list == NULL || toBeAdded == NULL) {
		return;
	}

	if (list->head == NULL) {
		insertBack(list, toBeAdded);
		return;
	}

	if (list->compareData(toBeAdded, list->head->data) <= 0) {
		insertFront(list, toBeAdded);
		return;
	}

	if (list->compareData(toBeAdded, list->tail->data) > 0) {
		insertBack(list, toBeAdded);
		return;
	}

	Node *current = list->head;

	while (current != NULL) {
		if (list->compareData(toBeAdded, current->data) <= 0) {
			Node *new = initializeNode(toBeAdded);

			new->next = current;
			new->previous = current->previous;
			current->previous->next = new;
			current->previous = new;

			list->length++;

			return;
		}

		current = current->next;
	}
}

void *deleteDataFromList(List *list, void *toBeDeleted) {
	if (list == NULL || toBeDeleted == NULL) {
		return NULL;
	}

	Node *temp = list->head;

	while (temp != NULL) {
		if (list->compareData(toBeDeleted, temp->data) == 0) {
			Node *delete = temp;

			if (temp->previous != NULL) {
				temp->previous->next = delete->next;
			} else {
				list->head = delete->next;
			}

			if (temp->next != NULL) {
				temp->next->previous = delete->previous;
			} else {
				list->tail = delete->previous;
			}

			void *data = delete->data;

			free(delete);

			list->length--;

			return data;
		}

		temp = temp->next;
	}

	return NULL;
}

void *getFromFront(List list) {
	return list.head == NULL ? NULL : list.head->data;
}

void *getFromBack(List list) {
	return list.tail == NULL ? NULL : list.tail->data;
}

char *toString(List list) {
	ListIterator iter = createIterator(list);
	char *str = malloc(sizeof(char));
	void *elem;

	strcpy(str, "");

	while ((elem = nextElement(&iter)) != NULL) {
		char *desc = list.printData(elem);
		int len = strlen(str) + 50 + strlen(desc);
		str = realloc(str, len);
		strcat(str, "\n");
		strcat(str, desc);

		free(desc);
	}

	return str;
}

ListIterator createIterator(List list) {
	ListIterator iter;

	iter.current = list.head;

	return iter;
}

void *nextElement(ListIterator *iter) {
	Node *node = iter->current;

	if (node != NULL) {
		iter->current = iter->current->next;
		return node->data;
	}

	return NULL;
}

int getLength(List list) {
	return list.length;
}

void *findElement(List list, bool (*customCompare)(const void *first, const void *second), const void *searchRecord) {
	ListIterator iter = createIterator(list);
	void *data;

	assert(customCompare != NULL);

	while ((data = nextElement(&iter)) != NULL) {
		if (customCompare(data, searchRecord)) {
			return data;
		}
	}

	return NULL;
}

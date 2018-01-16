#include "tLib.h"

void NodeInit(tNode *node)
{
	node->nextNode = node;
	node->preNode = node;
}

#define firstNode headNode.nextNode
#define lastNode headNode.preNode

void ListInit(tList *list)
{
	list->firstNode = &(list->headNode);
	list->lastNode = &(list->headNode);
	list->nodeCount = 0;
}

u32 ListCount(tList *list)
{
	return list->nodeCount;
}

tNode *ListFirst(tList *list)
{
	tNode *node = (tNode *)0;

	if (list->nodeCount != 0) {
		node = list->firstNode;
	}

	return node;
}

tNode *ListLast(tList *list)
{
	tNode *node = (tNode *)0;

	if (list->nodeCount != 0) {
		node = list->lastNode;
	}
	
	return node;
}

tNode *ListPre(tList *list, tNode *node)
{
	if (node->preNode == node) {
		return (tNode *)0;
	} else {
		return node->preNode;
	}
}

tNode *ListNext(tList *list, tNode *node)
{
	if (node->nextNode == node) {
		return (tNode *)0;
	} else {
		return node->nextNode;
	}
}

void ListRemoveAll(tList *list)
{
	u32 count;
	tNode *nextNode;

	nextNode = list->firstNode;
	for (count = list->nodeCount; count != 0; count--) {
		tNode *currentNode = nextNode;
		nextNode = nextNode->nextNode;

		currentNode->nextNode = currentNode;
		currentNode->preNode = currentNode;
	}

	list->firstNode = &(list->headNode);
	list->lastNode = &(list->headNode);
	list->nodeCount = 0;
}

void ListAddFirst(tList *list, tNode *node)
{
	node->preNode = list->firstNode->preNode;
	node->nextNode = list->firstNode;

	list->firstNode->preNode = node;
	list->firstNode = node;
	list->nodeCount++;
}

void ListAddLast(tList *list, tNode *node)
{
	node->preNode = list->lastNode;
	node->nextNode = list->firstNode->preNode;

	list->lastNode->nextNode = node;
	list->lastNode = node;
	list->nodeCount++;
}

tNode *ListRemoveFirst(tList *list)
{
	tNode *node = (tNode *)0;

	if (list->nodeCount != 0) {
		node = list->firstNode;

		node->nextNode->preNode = node->preNode;
		list->firstNode = node->nextNode;
		list->nodeCount--;
	}
	return node;
}

void ListInsertAfter(tList *list, tNode *nodeAfter, tNode *nodeToInsert)
{
	nodeToInsert->preNode = nodeAfter;
	nodeToInsert->nextNode = nodeAfter->nextNode;

	nodeAfter->nextNode->preNode = nodeToInsert;
	nodeAfter->nextNode = nodeToInsert;
	
	list->nodeCount++;
}

void ListRemove(tList *list, tNode *node)
{
	node->preNode->nextNode = node->nextNode;
	node->nextNode->preNode = node->preNode;
	list->nodeCount--;
}


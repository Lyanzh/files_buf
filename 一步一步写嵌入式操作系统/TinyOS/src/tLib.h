#ifndef TLIB_H
#define TLIB_H

#include "global.h"

typedef struct
{
	u32 bitmap;
} tBitmap;

typedef struct _tNode
{
	struct _tNode *preNode;
	struct _tNode *nextNode;
} tNode;

typedef struct _tList
{
	tNode headNode;
	u32 nodeCount;
} tList;

#define NodeParent(node, parent, name) (parent *)((u32)node - (u32)&((parent *)0)->name)

void NodeInit(tNode *node);
void ListInit(tList *list);
u32 ListCount(tList *list);
tNode *ListFirst(tList *list);
tNode *ListLast(tList *list);
tNode *ListPre(tList *list, tNode *node);
tNode *ListNext(tList *list, tNode *node);
void ListRemoveAll(tList *list);
void ListAddFirst(tList *list, tNode *node);
void ListAddLast(tList *list, tNode *node);
tNode *ListRemoveFirst(tList *list);
void ListInsertAfter(tList *list, tNode *nodeAfter, tNode *nodeToInsert);
void ListRemove(tList *list, tNode *node);

void BitmapInit(tBitmap *bitmap);
u32 BitmapPosCount(void);
void BitmapSet(tBitmap *bitmap, u32 pos);
void BitmapClear(tBitmap *bitmap, u32 pos);
u32 BitmapGetFirstSet(tBitmap *bitmap);

#endif

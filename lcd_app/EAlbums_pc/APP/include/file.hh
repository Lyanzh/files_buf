#ifndef _FILE_H
#define _FILE_H

#include <dirent.h>

typedef struct File_Node
{
	char *pcName;
	File_Node *pNext;
} T_FileNode, *PT_FileNode;

typedef PT_FileNode PT_FileList;

#endif /* _FILE_H */
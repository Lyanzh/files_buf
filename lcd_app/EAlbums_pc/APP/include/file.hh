#ifndef _FILE_H
#define _FILE_H

#include <dirent.h>

typedef struct File_Node
{
	char *pcName;
	struct File_Node *ptPre;
	struct File_Node *ptNext;
} T_FileNode, *PT_FileNode;

typedef PT_FileNode PT_FileList;

extern PT_FileList g_ptFileListHead;

extern PT_FileList g_ptFileListCurShow;

extern int Read_File_List(char *pcBasePath);
extern void Show_File_List(void);

#endif /* _FILE_H */
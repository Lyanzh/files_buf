#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

PT_FileList g_ptFileListHead;
PT_FileList g_ptFileListCurShow;

void File_List_Add(char *pcFileName)//双向循环链表
{
	PT_FileNode pt_File;
	PT_FileNode pt_FileTmp;

	pt_File = (PT_FileNode)malloc(sizeof(T_FileNode));
	pt_File->pcName = (char *)malloc(strlen(pcFileName) + 1);
	memcpy(pt_File->pcName, pcFileName, (strlen(pcFileName) + 1));

	pt_File->ptPre = pt_File;
	pt_File->ptNext = pt_File;
	
	if (!g_ptFileListHead) {
		g_ptFileListHead = pt_File;
	} else {
		pt_FileTmp = g_ptFileListHead->ptPre;/* 取出尾部 */
		/* 从尾部加入 */
		pt_FileTmp->ptNext->ptPre = pt_File;
		pt_File->ptNext = pt_FileTmp->ptNext;
		pt_FileTmp->ptNext = pt_File;
		pt_File->ptPre = pt_FileTmp;
	}
}

void Show_File_List(void)
{
	PT_FileNode pt_FileTmp;
	pt_FileTmp = g_ptFileListHead;
	if (!pt_FileTmp) {
		return;
	} else {
		printf("file_name:%s\n", pt_FileTmp->pcName);
		while (pt_FileTmp->ptNext != g_ptFileListHead) {
			pt_FileTmp = pt_FileTmp->ptNext;
			printf("file_name:%s\n", pt_FileTmp->pcName);
		}
	}
}

/* d_type:
DT_BLK      This is a block device.

DT_CHR      This is a character device.

DT_DIR      This is a directory.

DT_FIFO     This is a named pipe (FIFO).

DT_LNK      This is a symbolic link.

DT_REG      This is a regular file.

DT_SOCK     This is a UNIX domain socket.

DT_UNKNOWN  The file type is unknown.
*/

#if 0
struct stat {
   dev_t     st_dev;     /* ID of device containing file */
   ino_t     st_ino;     /* inode number */
   mode_t    st_mode;    /* protection */
   nlink_t   st_nlink;   /* number of hard links */
   uid_t     st_uid;     /* user ID of owner */
   gid_t     st_gid;     /* group ID of owner */
   dev_t     st_rdev;    /* device ID (if special file) */
   off_t     st_size;    /* total size, in bytes */
   blksize_t st_blksize; /* blocksize for file system I/O */
   blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
   time_t    st_atime;   /* time of last access */
   time_t    st_mtime;   /* time of last modification */
   time_t    st_ctime;   /* time of last status change */
};
#endif

int Read_File_List(char *pcBasePath)
{
    DIR *pDir;
    struct dirent *ptPtr;
    char *pcStr;
    int iSrcStrLen;

    //struct stat tFizeStat;

    if ((pDir = opendir(pcBasePath)) == NULL) {
        printf("Error:open dir %s error", pcBasePath);
        exit(1);
    }

    while ((ptPtr = readdir(pDir)) != NULL) {
    #if 0
        if(strcmp(ptPtr->d_name, ".") == 0 || strcmp(ptPtr->d_name, "..") == 0)	{	//current dir OR parrent dir
            continue;
        } else if(ptPtr->d_type == 8) {	//file
            //printf("d_name:%s/%s\n", basePath, ptr->d_name);
            stat(ptPtr->d_name, &tFizeStat);
            File_List_Add(ptPtr->d_name);
        } else if(ptPtr->d_type == 10) {	//link file
            printf("d_name:%s/%s\n", pcBasePath, ptPtr->d_name);
        } else if(ptPtr->d_type == 4) {	//dir
            memset(base, '\0', sizeof(base));
            strcpy(base, pcBasePath);
            strcat(base, "/");
            strcat(base, ptPtr->d_name);
            //readFileList(base);
        }
	#endif
		if(ptPtr->d_type == 8) {	//file
            //printf("d_name:%s/%s\n", pcBasePath, ptPtr->d_name);
            //stat(ptPtr->d_name, &tFizeStat);
            iSrcStrLen = strlen(ptPtr->d_name);
            if (iSrcStrLen <= 4) {
            	continue;
            }
            pcStr = ptPtr->d_name + iSrcStrLen - 4;
            if (strcmp(pcStr, ".jpg") == 0 || strcmp(pcStr, ".JPG") == 0) {
            	printf("jpg : %s\n", ptPtr->d_name);
            	File_List_Add(ptPtr->d_name);
            }
        }
    }
    closedir(pDir);
    return 1;
}


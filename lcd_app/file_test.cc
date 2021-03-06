#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>


typedef struct File_Node
{
	char *pcName;
	struct File_Node *pNext;
} T_FileNode, *PT_FileNode;

typedef PT_FileNode PT_FileList;

PT_FileList pt_FileListHead;

void File_List_Add(char *pcFileName)
{
	PT_FileNode pt_File;
	PT_FileNode pt_FileTmp;

	pt_File = (PT_FileNode)malloc(sizeof(T_FileNode));
	pt_File->pcName = (char *)malloc(strlen(pcFileName) + 1);
	memcpy(pt_File->pcName, pcFileName, (strlen(pcFileName) + 1));
	
	if (!pt_FileListHead) {
		pt_FileListHead = pt_File;
	} else {
		pt_FileTmp = pt_FileListHead;
		while (pt_FileTmp->pNext) {
			pt_FileTmp = pt_FileTmp->pNext;
		}
		pt_FileTmp->pNext = pt_File;
		pt_File->pNext = NULL;
	}
}

void Show_File_List(void)
{
	PT_FileNode pt_FileTmp;
	pt_FileTmp = pt_FileListHead;
	if (!pt_FileTmp) {
		return;
	} else {
		while (pt_FileTmp) {
			printf("d_name:%s\n", pt_FileTmp->pcName);
			pt_FileTmp = pt_FileTmp->pNext;
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

int readFileList(char *basePath)
{
    DIR *dir;
    struct dirent *ptr;
    char base[1000];

    struct stat fizestat;

    if ((dir = opendir(basePath)) == NULL) {
        perror("Open dir error...");
        exit(1);
    }

    printf("Path:%s\n", basePath);
    
    chdir((const char*)dir);//changes the current working directory of the  calling  process

    while ((ptr = readdir(dir)) != NULL) {
        if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)	{	//current dir OR parrent dir
            continue;
        } else if(ptr->d_type == 8) {	//file
            //printf("d_name:%s/%s\n", basePath, ptr->d_name);
            stat(ptr->d_name, &fizestat);
            File_List_Add(ptr->d_name);
        } else if(ptr->d_type == 10) {	//link file
            printf("d_name:%s/%s\n", basePath, ptr->d_name);
        } else if(ptr->d_type == 4) {	//dir
            memset(base, '\0', sizeof(base));
            strcpy(base, basePath);
            strcat(base, "/");
            strcat(base, ptr->d_name);
            //readFileList(base);
        }
    }
    closedir(dir);
    return 1;
}

int main(void)
{
    DIR *dir;
    char basePath[100];

    //get the current absoulte path
    memset(basePath, '\0', sizeof(basePath));
    getcwd(basePath, 99);
    printf("the current dir is : %s\n", basePath);

    //get the file list
    readFileList(basePath);

    Show_File_List();
    return 0;
}

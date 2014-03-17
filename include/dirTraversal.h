#ifndef __DIR_TRAVERSAL_H__
#define __DIR_TRAVERSAL_H__
/*
struct _fileList
{
	struct _fileList *
};
typedef _fileList * fileList;
*/

#ifndef WIN32 // for linux
int isDir(const char* path);
#else // for windows
#endif // enf for Linux or Windows

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*file_callback)(void * usr ,const char* fileName,int isDir);

int dirTraversal(const char *path, int recursive,file_callback xCallback,void * usr);
//int dirTraversal(const char *path, int recursive,int (*callback)(void * usr ,const char* fileName,int isDir),void * usr);

#ifdef __cplusplus
}
#endif

#endif //__DIR_TRAVERSAL_H__

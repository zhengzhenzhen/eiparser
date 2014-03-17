/**
 * The MIT License (MIT)
 *
 *
 * Copyright (C) 2013 Yu Jing (yujing5b5d@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute,sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED,INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
#include "dirTraversal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32 // for linux
#undef __STRICT_ANSI__
#define D_GNU_SOURCE
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#else // for windows
#include <io.h>
#endif //WIN32




#ifndef WIN32 // for linux

inline int isDir(const char* path)
{
         struct stat st;
         lstat(path, &st);
         return S_ISDIR(st.st_mode);
}
//
int doTraversal(const char *path, int recursive,file_callback xCallback,void * usr)
{
	DIR *pdir;
	struct dirent *pdirent;
	char tmp[1024];
	pdir = opendir(path);
	if(pdir)
	{
		while((pdirent = readdir(pdir)) != 0)
		{
			//ignore "." && ".."
			if(!strcmp(pdirent->d_name, ".")||
				!strcmp(pdirent->d_name, "..")) continue;
			sprintf(tmp, "%s/%s", path, pdirent->d_name);
			xCallback(usr,tmp,isDir(tmp));
			//if is Dir and recursive is true , into recursive
			if(isDir(tmp) && recursive)
			{
				doTraversal(tmp, recursive,xCallback,usr);
			}
		}
	}else
	{
		fprintf(stderr,"opendir error:%s\n", path);
	}
	closedir(pdir);
	return 1;
}

//interface
int dirTraversal(const char *path, int recursive,file_callback xCallback,void * usr)
{
	int len;
	char tmp[256];
	len = strlen(path);
	strcpy(tmp, path);
	if(tmp[len - 1] == '/') tmp[len -1] = '\0';

	if(isDir(tmp))
	{
		doTraversal(tmp, recursive,xCallback,usr);
	}
	else
	{
		//printf("%s\n", path);
		xCallback(usr,path,isDir(path));
	}
	return 1;
}


#else //for windows
/**
 *
 */

//int dirTraversal(const char *path, int recursive,file_callback xCallback)
int dirTraversal(const char *path, int recursive,file_callback xCallback,void * usr)
{
	int len = strlen(path)+3;
	long handle;
	char mypath[1024];
	char searchpath[1024];
	char tmppath[1024];
	char nxtpath[1024];
	char sp = '/';
	int i;
	struct _finddata_t fileinfo;
	sprintf(mypath,"%s",path);
	switch(mypath[len-1])
	{
		case '\\':
			sp = '\\';
			len -= 1;
			mypath[len-1] = '\0';
			break;
		case '/':
			len -= 1;
			mypath[len-1] = '\0';
		case '.':
			sp = '/';
			break;
		default :
			for(i=0;i<len;i++)
			{
				if(mypath[i]=='\\'||mypath[i]=='/')
				{
					sp = mypath[i];
					break;
				}
			}
	}
	sprintf(tmppath,"%s",mypath);
	sprintf(searchpath,"%s%c%s",mypath,sp,"*");

	for(handle=_findfirst(searchpath,&fileinfo);!_findnext(handle,&fileinfo);)
	{
		if(-1==handle) return -1;
		//
		sprintf(nxtpath,"%s%c%s",tmppath,sp,fileinfo.name);

		// call back
		if((0 != strcmp(fileinfo.name,"."))
				&& (0 != strcmp(fileinfo.name,"..")))
			xCallback(usr,nxtpath,((fileinfo.attrib & _A_SUBDIR)!=0));

		if(((fileinfo.attrib & _A_SUBDIR)!=0)
				&& recursive
				&& 0 != strcmp(fileinfo.name,".")
				&& 0 != strcmp(fileinfo.name,".."))
			dirTraversal(nxtpath,recursive,xCallback,usr);

	}
	_findclose(handle);
	return 1;
}

#endif //end of linux/windows


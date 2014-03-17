#include "ini.h"
#include <iostream>  
#include <string>
#include <vector>

#include <cstdio>
#include <cstdlib>  
#include <cstring>

#include <pthread.h>

#include "dirTraversal.h"
#include "info_getter.h"
#include "ini.h"
#include "cfg_def.h"

using std::string;
using std::vector;

typedef struct dpctl_args
{
	vector<string>* _dirList;
	vector<string>::size_type ix_s;
	vector<string>::size_type ix_e;
	FILE * fp;
	int pid;
}dpctl_args,* _dpctl_args;


int ALL_COUNT = 0;
int FINISHED_COUNT = 0;

inline string & string_replace(string & strBig, const string & strsrc, const string &strdst)
{
    string::size_type pos=0;
    string::size_type srclen=strsrc.size();
    string::size_type dstlen=strdst.size();
    while( (pos=strBig.find(strsrc, pos)) != string::npos)
    {
        strBig.replace(pos, srclen, strdst);
        pos += dstlen;
    }
    return strBig;
}

inline void fappend(FILE * fp,string & data) 
{
	string_clean(data);
	fprintf(fp,"\"%s\",",string_replace(data,"\"","\"\"").c_str());
}

int xml_handler(void * args,const char* fileName,int isDir)
{
	if(isDir) return 0;
	FILE * fp = ((dpctl_args * ) args) -> fp;
	{
		pubItem item ;
		//bool eiparse(const char * filename,pubItem & p_item);
		if(eiparse(fileName,item))
		{
			if(item.authors.size() > 0 )
			{ // full detail
				int pos = 1; 
				for(vector<authorItem>::iterator it =item.authors.begin() ; it != item.authors.end() ; it ++ )
				{
					fappend(fp,string(it->givename).append(" ").append(it->surname));
					std::map<string,affItem>::iterator affit = item.affs.find(it->affid);
					if( affit != item.affs.end())
					{
						//affit->second
						fappend(fp,affit->second.aff);
						fappend(fp,(!it->email.empty())?it->email: affit->second.email);
					}else
					{
						fprintf(fp,",%s,",(!it->email.empty())?it->email.c_str():"");
					}
					fprintf(fp,"%d,",pos);pos++;
					fappend(fp,item.title);
					fappend(fp,item.doi);
					fappend(fp,item.issn);
					fappend(fp,item.pubname);
					fappend(fp,item.puber);
					fappend(fp,item.pubyear);
					fappend(fp,item.vol);
					fappend(fp,item.no);
					fappend(fp,item.page_start);
					fappend(fp,item.page_end);
					fappend(fp,item.keywords);
					fappend(fp,item.abstract);
					fprintf(fp,"\n");
				}
			}else if(item.s_authors.size() > 0)
			{ // simple author
				int pos = 1;
				for(vector<string>::iterator it = item.s_authors.begin() ; it != item.s_authors.end(); it ++ )
				{
					fappend(fp,(*it));
					fprintf(fp,",,"); // affiliations , email
					fprintf(fp,"%d,",pos);pos++;
					fappend(fp,item.title);
					fappend(fp,item.doi);
					fappend(fp,item.issn);
					fappend(fp,item.pubname);
					fappend(fp,item.puber);
					fappend(fp,item.pubyear);
					fappend(fp,item.vol);
					fappend(fp,item.no);
					fappend(fp,item.page_start);
					fappend(fp,item.page_end);
					fappend(fp,item.keywords);
					fappend(fp,item.abstract);
					fprintf(fp,"\n");
				}
			}
		}
		//fprintf(fp,"%s\n",((*_dirList)[ix]).c_str());
	}
	return 0;
}


void * data_parse_ctl_thread(void * args)
{
	vector<string> * _dirList = ((dpctl_args * ) args) -> _dirList;
	vector<string>::size_type ix_s = ((dpctl_args * ) args) -> ix_s;
	vector<string>::size_type ix_e = ((dpctl_args * ) args) -> ix_e;
	FILE * fp = ((dpctl_args * ) args) -> fp;
	int pid = ((dpctl_args * ) args) -> pid;
	printf("[%d] process start .. \n",pid);
	ALL_COUNT ++ ;
	unsigned long cp_size = ix_e - ix_s ; // current process size
	for(vector<string>::size_type ix = ix_s ; ix < ix_e ; ix ++)
	{
		printf("[%d] proc : %lu of %lu (ALL: %d of %d)\n",pid,ix-ix_s,cp_size,FINISHED_COUNT,ALL_COUNT);
		dirTraversal((*_dirList)[ix].c_str(),1,xml_handler,args);
	}	
	fclose(fp);
	FINISHED_COUNT ++ ;
	printf("[%d] process end .. (ALL : %d of %d))\n",pid,FINISHED_COUNT,ALL_COUNT);
	return NULL;
}

int load_root_dir(void * usr,const char* fileName,int isDir)
{
	vector<string> * _rootdir = (vector<string> *) usr;
		//for(char x = '0' ; x <= '9' ; x ++ )
		//	for(char y = '0' ; y <= '9' ; y ++)
		//		for(char z = '0' ; z <= '9' ; z++)
		//			_rootdir->push_back(string(fileName)+"#" + x + y + z);
	_rootdir->push_back(string(fileName));
	return 0;
}

int config_handler(void * data, const char * section , const char * name , const char * value)
{
	_config_info _cfg_info = (_config_info) data;
	if(strcmp(section,"base") == 0 && strcmp(name,"xml_dir") == 0)
	{
		_cfg_info->src_dir.assign(value);
		if(_cfg_info->src_dir[_cfg_info->src_dir.size() - 1] != '/')
		{
			_cfg_info->src_dir += '/';
		}
	}else if(strcmp(section,"base") == 0 && strcmp(name,"dest_dir") == 0)
	{
		_cfg_info->dest_dir.assign(value);
		if(_cfg_info->dest_dir[_cfg_info->dest_dir.size() - 1] != '/')
		{
			_cfg_info->dest_dir += '/';
		}
	}else if(strcmp(section,"base") == 0 && strcmp(name,"thread_num") == 0)
	{
		_cfg_info->thread_num = atoi(value);
	}
	return 0;
}

  
int main(int argc, char *argv[])  
{
	config_info cfg_info;
	cfg_info.src_dir.assign("../data/");
	cfg_info.dest_dir.assign("data/");
	cfg_info.thread_num = 50;
	int error_code = 0;
	if((error_code = ini_parse("config.ini",config_handler,(void*)(&cfg_info))) < 0)
        {
        	switch (error_code) // -1 on file open error, or -2 on memory allocation
        	{
        	case -1:
        		fprintf(stderr,"ERROR on %s:%d , file open error \n",__FILE__,__LINE__);
        		break;
        	case -2:
        		fprintf(stderr,"ERROR on %s:%d , memory allocation error \n",__FILE__,__LINE__);
        		break;
        	default:
        		fprintf(stderr,"ERROR on %s:%d , unknown error [%d]\n",__FILE__,__LINE__,error_code);
        	}
        	return -1;
        }
        printf("starting ... \nxml  dir\t: %s \ndest dir\t: %s \nthreads\t\t: %d\n",
        		cfg_info.src_dir.c_str(),cfg_info.dest_dir.c_str(),cfg_info.thread_num);
        
	// definition :
	int thread_size = cfg_info.thread_num;
	/*    int num = 1234;  
	std::string str = "tujiaw";  
	std::thread t(my_thread, num, str);  
	t.detach();
	getchar();
	*/
	//int dirTraversal(const char *path, int recursive,int (*callback)(void * usr ,const char* fileName,int isDir),void * usr);
	//int dirTraversal(const char *path, int recursive,file_callback xCallback,void * usr)
	vector<string> rootdir ; 
	dirTraversal(cfg_info.src_dir.c_str(),0,load_root_dir,(void *)&rootdir);
	int each_thread_size = rootdir.size() / thread_size + 1;
	printf("each thread size : %d \n",each_thread_size);
	int count = 0;
	FILE * fp[thread_size];
	pthread_t thl[thread_size];
	dpctl_args  * _args = new dpctl_args[thread_size];
	for(vector<string>::size_type ix = 0 ; ix < rootdir.size() ; ix += each_thread_size)
	{
		fp[count] = fopen((cfg_info.dest_dir+std::to_string(count)+".txt").c_str(),"w");
		//printf("cnt : %d \n",count+1);
		_args[count]._dirList = & rootdir;
		_args[count].ix_s = ix;
		_args[count].ix_e = ix + each_thread_size < rootdir.size() ? ix + each_thread_size : rootdir.size();
		_args[count].fp = fp[count];
		_args[count].pid = count;
		int rc = pthread_create(&thl[count], NULL, data_parse_ctl_thread, (void *)&_args[count]);
		if (rc)
		{
		    printf("ERROR; return code is %d\n", rc);
		    return EXIT_FAILURE;
		}
		count ++ ;
	}
	
	for(int i = 0 ; i < count ; i ++)
	{
		pthread_join(thl[i], NULL);
	}
	
	printf("all done\n");
	/*
	for(vector<string>::const_iterator it = rootdir.begin(); it != rootdir.end() ; it ++)
	{
		printf("%s\n",it->c_str());
	}
	*/
	return 0;  
}  

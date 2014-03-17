#ifndef __CFG_READ_H__
#define __CFG_READ_H__
#include <string>

typedef struct config_info
{
	std::string src_dir;
	std::string dest_dir;
	int thread_num;
}config_info, * _config_info;



#endif // __CFG_READ_H__

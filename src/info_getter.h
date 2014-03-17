#ifndef __INFO_GETTER_H__
#define __INFO_GETTER_H__

#include <string>
#include <vector>
#include <map>


using std::string;
using std::vector;
using std::map;


typedef struct
{
	string givename;
	string surname;
	string affid;
	string email;
}authorItem;

typedef struct
{
	string str;
	string aff;
	string email;	
}affItem;

typedef struct
{
	string title;
	vector<string> s_authors;
	vector<authorItem> authors;
	map<string,affItem> affs;
	string doi;
	string issn;
	string pubname;
	string puber;
	string pubyear;
	string vol; // volumn
	string no; // number , issue
	string page_start;
	string page_end;
	string abstract;
	string keywords;
}pubItem;


bool eiparse(const char * filename,pubItem & p_item);


inline int white_space_level(char ch)
{
	if(ch == ' ') return 0;
	if(ch == '\t' || ch == '\n' || ch == '\r') return 1;
	return 2;
}

inline void string_clean(string & str)
{
    string::size_type pos=0;
    bool firstFlag = true;
    bool whiteFlag = false;
    while( pos < str.size())
    {
    	int lv = white_space_level(str[pos]);
    	if(lv == 1)
    	{
    		str.erase(pos,1);
    		continue;
    	}
    	if((firstFlag || whiteFlag) && lv == 0)
    	{
    		str.erase(pos,1);
    		continue;
    	}
    	firstFlag = false;
    	whiteFlag = (lv == 0);
    	pos ++ ;
        //strBig.erase(pos, srclen);
        //strBig.insert(pos, strdst);
        //pos += dstlen;
    }
}


#endif // __INFO_GETTER_H__

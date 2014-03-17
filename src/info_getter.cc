#include "info_getter.h"
#include <fstream>
#include <iostream>
#include <stdlib.h>

#include <string>
#include <vector>
#include <map>

#include "gumbo.h"

#include <cstring>

using std::string;
using std::vector;
using std::map;


// node string
inline bool nsequal(const char * s1, const char * s2,int lenlim)
{
	int s2len = strlen(s2);
	return (s2len < lenlim) ? (strncmp(s1+1,s2,strlen(s2)-1) == 0 ): false;
}

#define NODE_NAME_IS(node,name) (nsequal(node->v.element.original_tag.data,name,(int)node->v.element.original_tag.length))

bool cleantext(GumboNode* node,std::string & contents) {
	if (node->type == GUMBO_NODE_TEXT) {
		contents.assign(node->v.text.text);
		return true;
	} else if (node->type == GUMBO_NODE_ELEMENT &&
		node->v.element.tag != GUMBO_TAG_SCRIPT &&
		node->v.element.tag != GUMBO_TAG_STYLE) {
		GumboVector* children = &node->v.element.children;
		contents = "";
		for (unsigned int i = 0; i < children->length; ++i) {
			std::string text;
			cleantext((GumboNode*) children->data[i],text);
			if (i != 0 && !text.empty()) {
				contents.append(" ");
			}
			contents.append(text);
		}
		return true;
	} else {
		return true;
	}
}


void s_author_getter(GumboNode* node,vector<string>& s_authors)
{
	if (node->type == GUMBO_NODE_ELEMENT &&
		node->v.element.tag != GUMBO_TAG_SCRIPT &&
		node->v.element.tag != GUMBO_TAG_STYLE) 
	{
		if(NODE_NAME_IS(node,"rdf:li"))
		{
			string s("");
			cleantext(node,s);
			s_authors.push_back(s);
		}else
		{
			GumboVector* children = &node->v.element.children;
			for (unsigned int i = 0; i < children->length; ++i)
			{
				s_author_getter((GumboNode*) children->data[i] , s_authors);
			}
		}
	}
}

void keywords_getter(GumboNode* node,string& keywords)
{
	if (node->type == GUMBO_NODE_ELEMENT &&
		node->v.element.tag != GUMBO_TAG_SCRIPT &&
		node->v.element.tag != GUMBO_TAG_STYLE) 
	{
		if(NODE_NAME_IS(node,"ce:text"))
		{
			string s("");
			cleantext(node,s);
			keywords.append(s).append(",");
		}else
		{
			GumboVector* children = &node->v.element.children;
			for (unsigned int i = 0; i < children->length; ++i)
			{
				keywords_getter((GumboNode*) children->data[i] , keywords);
			}
		}
	}
}


void adv_author_getter(GumboNode* node,authorItem & aunode)
{
	if (node->type == GUMBO_NODE_ELEMENT &&
		node->v.element.tag != GUMBO_TAG_SCRIPT &&
		node->v.element.tag != GUMBO_TAG_STYLE) 
	{
		if(NODE_NAME_IS(node,"ce:given-name"))
		{
			string s("");
			cleantext(node,s);
			aunode.givename.assign(s.begin(),s.end());
		}else if(NODE_NAME_IS(node,"ce:surname"))
		{
			string s("");
			cleantext(node,s);
			aunode.surname.assign(s.begin(),s.end());
		}else if(NODE_NAME_IS(node,"ce:cross-ref"))
		{
			GumboAttribute* affid;
			if ((affid = gumbo_get_attribute(&node->v.element.attributes, "refid")) != 0 ) {
				aunode.affid.assign(affid->value);
			}
		}else if(NODE_NAME_IS(node,"ce:e-address"))
		{
			string s("");
			cleantext(node,s);
			aunode.email.assign(s.begin(),s.end());
		}
	}
}

void affs_getter(GumboNode* node,affItem & affnode)
{
	if (node->type == GUMBO_NODE_ELEMENT &&
		node->v.element.tag != GUMBO_TAG_SCRIPT &&
		node->v.element.tag != GUMBO_TAG_STYLE) 
	{
		if(NODE_NAME_IS(node,"ce:textfn"))
		{
			GumboVector* children = &node->v.element.children;
			for (unsigned int i = 0; i < children->length; ++i)
			{
				GumboNode* s_node = (GumboNode*) children->data[i];
				if(s_node->type ==  GUMBO_NODE_TEXT)
				{
					affnode.aff.assign(s_node->v.text.text);
					affnode.aff.erase(affnode.aff.find_last_not_of(" \n\r\t")+1);
				}else if((s_node->type == GUMBO_NODE_ELEMENT) && NODE_NAME_IS(s_node,"ce:footnote"))
				{
					GumboVector* s_children = &s_node->v.element.children;
					for (unsigned int i = 0; i < s_children->length; ++i)
					{
						//
						GumboNode* t_node = (GumboNode*) s_children->data[i];
						if((t_node->type == GUMBO_NODE_ELEMENT)&& NODE_NAME_IS(t_node,"ce:note-para"))
						{
							cleantext(t_node,affnode.email);
							if(affnode.email.compare(0,7,"E-mail:",7) == 0)
							{
								affnode.email.assign(affnode.email,7,string::npos);
							}
						}
					}
				}
			}
			//cleantext(node,affnode.str);
			//affnode.str.assign(node->v.text.text);
			//string::size_type pos = affnode.str.find("E-mail:");
			//if(pos != string::npos){
			//	affnode.email.assign(affnode.str.substr(pos));
			//}
		}
	}
}

//TODO
void pcheck(GumboNode* node,pubItem & p_item) // person check
{
	if (node->type == GUMBO_NODE_ELEMENT &&
		node->v.element.tag != GUMBO_TAG_SCRIPT &&
		node->v.element.tag != GUMBO_TAG_STYLE) 
	{
		if(NODE_NAME_IS(node,"ce:author"))
		{
			GumboVector* children = &node->v.element.children;
			authorItem aunode;
			for (unsigned int i = 0; i < children->length; ++i)
			{
				adv_author_getter((GumboNode*) children->data[i] ,aunode);
			}
			p_item.authors.push_back(aunode);
			
		}if(NODE_NAME_IS(node,"ce:affiliation"))
		{
			string s_affid;
			affItem  affnode;
			GumboAttribute* affid;
			if ((affid = gumbo_get_attribute(&node->v.element.attributes, "id")) != 0 ) {
				s_affid.assign(affid->value);
			}
			GumboVector* children = &node->v.element.children;
			for (unsigned int i = 0; i < children->length; ++i)
			{
				affs_getter((GumboNode*) children->data[i] ,affnode);
			}
			//p_item.affs.push_back(affnode);
			p_item.affs.insert ( std::pair<string,affItem>(s_affid,affnode));
		}
	}
}




void mcheck(GumboNode* node,pubItem& p_item) // main check loop
{
	if (node->type == GUMBO_NODE_ELEMENT &&
		node->v.element.tag != GUMBO_TAG_SCRIPT &&
		node->v.element.tag != GUMBO_TAG_STYLE) 
	{

		GumboVector* children = &node->v.element.children;
		if(NODE_NAME_IS(node,"dc:creator")) // simple name
		{
			for (unsigned int i = 0; i < children->length; ++i)
			{
				s_author_getter((GumboNode*) children->data[i] , p_item.s_authors); // simple author name check
			}
		}else if(NODE_NAME_IS(node,"ce:author-group"))
		{
			for (unsigned int i = 0; i < children->length; ++i)
			{
				pcheck((GumboNode*) children->data[i] , p_item); // person check
			}
		}else if(NODE_NAME_IS(node,"dc:title"))
		{
			cleantext(node,p_item.title);
		}else if(NODE_NAME_IS(node,"dc:publisher"))
		{
			cleantext(node,p_item.puber);
		}else if(NODE_NAME_IS(node,"prism:publicationName"))
		{
			cleantext(node,p_item.pubname);
			//prism:coverDisplayDate
		}else if(NODE_NAME_IS(node,"prism:coverDisplayDate"))
		{
			cleantext(node,p_item.pubyear);
		}else if(NODE_NAME_IS(node,"prism:issn"))//prism:issn
		{
			cleantext(node,p_item.issn);
		}else if(NODE_NAME_IS(node,"prism:volume"))//prism:issn
		{
			cleantext(node,p_item.vol);
		}else if(NODE_NAME_IS(node,"prism:number"))//prism:issn
		{
			cleantext(node,p_item.no);
		}else if(NODE_NAME_IS(node,"prism:doi"))
		{
			cleantext(node,p_item.doi);
			/*
			if(p_item.doi.compare(0,4,"doi:",4) == 0)
			{
				p_item.doi.assign(p_item.doi,4,string::npos);
			}
			*/
		}else if(NODE_NAME_IS(node,"prism:startingPage"))//prism:issn
		{
			cleantext(node,p_item.page_start);
		}else if(NODE_NAME_IS(node,"prism:endingPage"))//prism:issn
		{
			cleantext(node,p_item.page_end);
			//ce:abstract-sec
		}else if(NODE_NAME_IS(node,"ce:abstract-sec"))//prism:issn
		{
			cleantext(node,p_item.abstract);
			//std::string::iterator end_pos = std::remove(p_item.abstract.begin(), p_item.abstract.end(),'\n');
			//p_item.abstract.erase(end_pos, p_item.abstract.end());
			//p_item.abstract.erase(p_item.abstract.find_last_not_of(" \n\r\t")+1);
			
			//ce:keywords
			string_clean(p_item.abstract);
		}else if(NODE_NAME_IS(node,"ce:keywords"))//prism:issn
		{
			keywords_getter(node,p_item.keywords);
		}else 
		{
			
			for (unsigned int i = 0; i < children->length; ++i)
			{
				mcheck((GumboNode*) children->data[i] , p_item);
			}
		}
	}
}

bool eiparse(const char * filename,pubItem & p_item)
{
	FILE * fp = fopen(filename,"rb");
	if(fp == NULL) return false;
	fseek(fp,0L,SEEK_END);
	size_t sz = ftell(fp);
	rewind(fp);
	char * info = (char * ) malloc(sz * sizeof(char));
	fread(info,sz,1,fp);
	fclose(fp);
	
	GumboOutput* output = gumbo_parse(info);
	mcheck(output->root,p_item);
	gumbo_destroy_output(&kGumboDefaultOptions, output);
	free(info);
	/*
	printf("title\t: %s\n",p_item.title.c_str());
	printf("doi\t: %s\n",p_item.doi.c_str());
	printf("publisher\t: %s\n",p_item.puber.c_str());
	printf("pub name\t: %s\n",p_item.pubname.c_str());
	printf("pub year\t: %s\n",p_item.pubyear.c_str());
	printf("page\t: %s-%s\n",p_item.page_start.c_str(),p_item.page_end.c_str());
	printf("ISSN\t: %s\n",p_item.issn.c_str());
	printf("Vol. %s , No. %s \n",p_item.vol.c_str(),p_item.no.c_str());
	
	printf("abstract\t: %s\n",p_item.abstract.c_str());
	printf("keywords\t: %s\n",p_item.keywords.c_str());
	for(vector<string>::size_type ix  = 0 ; ix != p_item.s_authors.size(); ix ++)
	{
		printf("s_authors : %s [%d]\n",p_item.s_authors[ix].c_str(),(int)(ix+1));
	}
	for(vector<authorItem>::size_type ix  = 0 ; ix != p_item.authors.size(); ix ++)
	{
		printf("authors : %s %s %s \n",p_item.authors[ix].givename.c_str()
						,p_item.authors[ix].surname.c_str()
						,p_item.authors[ix].affid.c_str());
	}
	
	for(map<string,affItem>::iterator it  = p_item.affs.begin() ; it != p_item.affs.end(); it ++)
	{
		printf("affs : [%s] #%s #%s \n",it->first.c_str()
						,it->second.aff.c_str()
						,it->second.email.c_str());
	}
	printf("all done \n");
	*/
	return true;
}
/*
int main(int argc, char** argv) {
	if (argc != 2) {
		std::cout << "Usage: clean_text <html filename>\n";
		exit(EXIT_FAILURE);
	}
	pubItem p_item;
	eiparse(argv[1],p_item);

	return 0;
}
*/


//gumbo_normalized_tagname
/*
for(int i = 0 ; i < (int)node->v.element.original_tag.length; i ++)
{
	putchar(node->v.element.original_tag.data[i]);
}
printf("\n");
*/
//printf("## %s\n",node->v.element.original_tag.data);


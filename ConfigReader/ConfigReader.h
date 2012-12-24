/*
 * ConfigReader.h
 *
 *  Created on: 2012-12-13
 *      Author: LiuYongJin
 */


#ifndef _CONFIG_READER_H_
#define _CONFIG_READER_H_

#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
using std::map;
using std::string;

//参数行最大长度
#define LINE_LEN  256

typedef map<string, string> StrMap;
class ConfigReader
{
public:
	ConfigReader(const char *config_path):m_config_path(config_path){Init();}
	void Init();
	void ShowKeyValue();

	string GetValueString(const char *key, string default_value="");
	int GetValueInt(const char *key, int default_value=0);
private:
	string m_config_path;
	StrMap m_parameter;
};

inline
void ConfigReader::Init()
{
	FILE *fp = fopen(m_config_path.c_str(), "r");
	if(fp == NULL)
		return;

	char buf[LINE_LEN];
	while(fgets(buf,LINE_LEN,fp) != NULL)  //read one line
	{
		char *temp = buf;
		//1. parse key
		while(*temp!='\0' && (*temp==' ' || *temp=='\t')) ++temp;  //跳过行首空白
		if(*temp=='\0' || *temp=='\r' || *temp=='\n' || *temp=='#' || *temp=='/') //空白行或者注释行
			continue;
		char *key = temp++;
		while(*temp!='\0' && *temp!=' ' && *temp!='\t' && *temp!='=')++temp;
		if(*temp == '\0')continue;  //找不到空白符或者"="号

		//2. parse "="
		while(*temp==' ' || *temp=='\t') *temp++='\0';
		if(*temp != '=')continue;  //没有找到"="号
		*temp++='\0';

		//3. parse value
		while(*temp!='\0' && (*temp==' ' || *temp=='\t')) ++temp;  //跳过空白
		if(*temp=='\0' || *temp=='\r' || *temp=='\n' || *temp=='#' || *temp=='/') //不完整行
			continue;
		char *value = temp++;
		while(*temp!='\0' && *temp!=' ' && *temp!='\t' && *temp!='#' && *temp!='/' && *temp!='\r' && *temp!='\n') ++temp;
		*temp = '\0';

		//4. save key-value
		StrMap::iterator it = m_parameter.find(key);
		if(it == m_parameter.end())
			m_parameter.insert(std::make_pair(key, value));
		else
			it->second = value;
	}

	fclose(fp);
}

inline
string ConfigReader::GetValueString(const char *key, string default_value)
{
	StrMap::iterator it = m_parameter.find(key);
	if(it != m_parameter.end())
		return it->second;
	return default_value;
}

inline
int ConfigReader::GetValueInt(const char *key, int default_value)
{
	StrMap::iterator it = m_parameter.find(key);
	if(it != m_parameter.end())
	{
		int temp;
		if(sscanf(it->second.c_str(), "%d", &temp) >0 )
			return temp;
	}
	return default_value;
}

inline
void ConfigReader::ShowKeyValue()
{
	StrMap::iterator it;
	for(it=m_parameter.begin(); it!=m_parameter.end(); ++it)
		printf("%s = %s\n", it->first.c_str(), it->second.c_str());
}

#endif //_CONFIG_READER_H_

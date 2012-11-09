/*
	读取配置文件，
	# ; ! 后面的也内容为注释
	支持[section]章节
	如:
	[gmcc] # gmcc section
	[unicom] # unicom section
*/

#if !defined _SERVER_CONFIG_H_
#define _SERVER_CONFIG_H_

#include <fstream>
#include <map>
#include <string>

using namespace std;

class CServerConf {
public:
	CServerConf();
	CServerConf(const char* szFileName);
	virtual ~CServerConf();


public:
	string& operator[](const char* szName);
	string& operator()(const char* szSection, const char* szName);

	int ParseFile(const char* szConfigFile);

private:
	static int StrimString(char* szLine);
	int ParseFile();

private:
	ifstream m_ConfigFile;
	map<string, string> m_ConfigMap;
};


#endif


#pragma once

#include <ctools/ConfigAbstract.h>

#define TSLOC(MainClass,Locat) TranslationSystem::MainClass.Locat

#include <string>

class FinalFontPane_Translation : public conf::ConfigAbstract
{
public:
	const char* ViewMode = "View Mode";

	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	// return true for continue xml parsing of childs in this node or false for interupt the child exploration
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");
};

#include <unordered_map>

class MainClassLocator
{
protected:
	std::unordered_map<std::string, std::string> m_DataBase;

public:
	
};

class TranslationSystem : public conf::ConfigAbstract
{
public:
	static FinalFontPane_Translation FFP;

public:
	std::string getXml(const std::string& vOffset, const std::string& vUserDatas = "");
	// return true for continue xml parsing of childs in this node or false for interupt the child exploration
	bool setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas = "");
};

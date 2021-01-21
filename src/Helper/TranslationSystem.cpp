#include "TranslationSystem.h"

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

std::string FinalFontPane_Translation::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vOffset);
	UNUSED(vUserDatas);

	std::string str;

	//str += vOffset + "<totocat>\n";
	//str += vOffset + "\t<toto>" + ct::toStr(toto_value) + "</toto>\n";
	//str += vOffset + "</totocat>\n";

	return str;
}

// return true for continue xml parsing of childs in this node or false for interupt the child exploration
bool FinalFontPane_Translation::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	/*if (strParentName == "totocat")
	{
		if (strName == "toto")
			m_Toto = ct::ivariant(strValue).GetI();
	}*/

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////
//// STATIC DECLARATIONS //////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

FinalFontPane_Translation TranslationSystem::FFP;

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

std::string TranslationSystem::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vOffset);
	UNUSED(vUserDatas);

	std::string str;

	//str += vOffset + "<totocat>\n";
	//str += vOffset + "\t<toto>" + ct::toStr(toto_value) + "</toto>\n";
	//str += vOffset + "</totocat>\n";

	return str;
}

// return true for continue xml parsing of childs in this node or false for interupt the child exploration
bool TranslationSystem::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	/*if (strParentName == "totocat")
	{
		if (strName == "toto")
			m_Toto = ct::ivariant(strValue).GetI();
	}*/

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

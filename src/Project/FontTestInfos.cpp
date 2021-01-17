// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
 * Copyright 2020 Stephane Cuillerdier (aka Aiekick)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "FontTestInfos.h"

#include <Project/ProjectFile.h>
#include <ctools/Logger.h>

FontTestInfos::FontTestInfos() = default;
FontTestInfos::~FontTestInfos()
{
	
}

void FontTestInfos::Clear()
{
	m_PreviewFontSize = 100.0f;
	m_ShowBaseLine = true;
	m_TestFontName.clear();
	m_GlyphToInsert.clear();
	m_GlyphToInsert_ToLoad.clear();
	m_TestString.clear();
	m_InputBuffer[0] = '\0';
	m_TestFont.reset();
}

void FontTestInfos::Load(ProjectFile* vProjectFile)
{
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		m_GlyphToInsert.clear();
		if (!m_GlyphToInsert_ToLoad.empty())
		{
			for (auto ficdp_toload : m_GlyphToInsert_ToLoad)
			{
				auto font = vProjectFile->GetFontWithFontName(ficdp_toload.second.second);
				if (font)
				{
					FontInfosCodePoint ficdp;
					ficdp.first = ficdp_toload.second.first;
					ficdp.second = font;
					m_GlyphToInsert[ficdp_toload.first] = ficdp;
				}
			}

			m_GlyphToInsert_ToLoad.clear();
		}

		m_TestFont = vProjectFile->GetFontWithFontName(m_TestFontName);
	}
}

void FontTestInfos::ResizeInsertedGlyphs(uint32_t vPos, bool vExpandOrReduce)
{
	std::map<uint32_t, FontInfosCodePoint> res;

	for (auto fi : m_GlyphToInsert)
	{
		if (fi.first < vPos)
		{
			if (vExpandOrReduce)
				res[fi.first] = fi.second;
			else
				res[fi.first - 1] = fi.second;
		}
		else if (fi.first > vPos)
		{
			if (vExpandOrReduce)
				res[fi.first + 1] = fi.second;
			else
				res[fi.first] = fi.second;
		}
	}

	m_GlyphToInsert = res;
}

//////////////////////////////////////////////////////////////////////////////
//// CONFIG FILE /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

std::string FontTestInfos::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string res;

	res += vOffset + "<fonttest>\n";
	res += vOffset + "\t<fontsize>" + ct::toStr(m_PreviewFontSize) + "</fontsize>\n";
	res += vOffset + "\t<showbaseline>" + (m_ShowBaseLine ? "true" : "false") + "</showbaseline>\n";
	res += vOffset + "\t<fontname>" + m_TestFontName + "</fontname>\n";
	res += vOffset + "\t<teststring>" + m_TestString + "</teststring>\n";
	res += vOffset + "\t<insertedglyphs>\n";
	for (auto fi : m_GlyphToInsert)
	{
		if (fi.second.second)
		{
			res += vOffset + "\t\t<insert pos=\"" + ct::toStr(fi.first) + "\" orgId=\"" +
				ct::toStr(fi.second.first) + "\" font=\"" + fi.second.second->m_FontFileName + "\"/>\n";
		}
	}
	res += vOffset + "\t</insertedglyphs>\n";
	res += vOffset + "</fonttest>\n";

	return res;
}

bool FontTestInfos::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strName == "fonttest")
	{
		for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			RecursParsingConfig(child->ToElement(), vElem);
		}
	}

	if (strParentName == "fonttest")
	{
		if (strName == "fontsize")
			m_PreviewFontSize = ct::fvariant(strValue).GetF();
		else if (strName == "showbaseline")
			m_ShowBaseLine = ct::ivariant(strValue).GetB();
		else if (strName == "fontname")
			m_TestFontName = strValue;
		else if (strName == "teststring")
		{
			m_TestString = strValue;
			snprintf(m_InputBuffer, 499, "%s", m_TestString.c_str());
		}
	}

	if (strParentName == "insertedglyphs")
	{
		if (strName == "insert")
		{
			uint32_t pos = 0;
			FontInfosCodePoint_ToLoad fi;

			for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
			{
				std::string attName = attr->Name();
				std::string attValue = attr->Value();

				if (attName == "pos")
					pos = ct::ivariant(attValue).GetI();
				else if (attName == "orgId")
					fi.first = ct::fvariant(attValue).GetI();
				else if (attName == "font")
					fi.second = attValue;
			}

			m_GlyphToInsert_ToLoad[pos] = fi;
		}
	}

	return true;
}

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
#include "FontInfos.h"

#include <FileHelper.h>
#include "Project/ProjectFile.h"
#include "Gui/ImGuiWidgets.h"
#include "Helper/Messaging.h"
#include <Logger.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "Helper/stb_truetype.h"

#include <glad/glad.h>

FontInfos::FontInfos() = default;
FontInfos::~FontInfos() = default;

// Extract the UpperCase char's of a string and return as a Prefix
// if the font name is like "FontAwesome" (UpperCase char), this func will give the prefix "FA"
// if all the name is lowercase, retunr nothing..
static std::string GetPrefixFromFontFileName(const std::string& vFileName)
{
    std::string res;

    if (!vFileName.empty())
    {
        for (auto c : vFileName)
        {
            if (std::isupper(c))
            {
                res += c;
            }
        }
    }

    return res;
}

bool FontInfos::LoadFont(ProjectFile *vProjectFile, const std::string& vFontFilePathName)
{
	bool res = false;

	if (!vProjectFile || !vProjectFile->IsLoaded())
		return res;

	std::string fontFilePathName = FileHelper::Instance()->CorrectFilePathName(vFontFilePathName);
	
	if (!FileHelper::Instance()->IsAbsolutePath(fontFilePathName))
	{
		fontFilePathName = vProjectFile->GetAbsolutePath(fontFilePathName);
	}
	
	if (FileHelper::Instance()->IsFileExist(fontFilePathName))
	{
		static const ImWchar ranges[] =
		{
			0x0020,
			0xFFFF, // Full Range
			0,
		};
		m_FontConfig.GlyphRanges = &ranges[0];
		m_FontConfig.OversampleH = m_Oversample;
		m_FontConfig.OversampleV = m_Oversample;
		m_ImFontAtlas.Clear();
		m_ImFontAtlas.Flags = m_ImFontAtlas.Flags | ImFontAtlasFlags_NoMouseCursors;

		auto ps = FileHelper::Instance()->ParsePathFileName(fontFilePathName);
		if (ps.isOk)
		{
			m_FontFileName = ps.name + "." + ps.ext;

			ImFont *font = m_ImFontAtlas.AddFontFromFileTTF(
				fontFilePathName.c_str(),
				(float)m_FontSize,
				&m_FontConfig);
			if (font)
			{
				if (m_ImFontAtlas.Build())
				{
					if (!m_ImFontAtlas.Fonts.empty())
					{
						if (m_FontPrefix.empty())
							m_FontPrefix = GetPrefixFromFontFileName(ps.name);

						m_FontFilePathName = vProjectFile->GetRelativePath(fontFilePathName);

						DestroyFontTexture();
						CreateFontTexture();

						FillGlyphNames();
						GenerateCodePointToGlypNamesDB();
						GetInfos();

						// update glyph ptrs
						for (auto &it : m_SelectedGlyphs)
						{
							uint32_t codePoint = it.first;

							auto glyph = font->FindGlyphNoFallback((ImWchar)codePoint);
							if (glyph)
							{
								it.second.glyph = *glyph;
								it.second.oldHeaderName = GetGlyphName(codePoint);
								it.second.glyphIndex = m_GlyphCodePointToGlyphIndex[codePoint];
							}
						}

						m_NeedFilePathResolve = false;

						res = true;
					}
				}
				else
				{
					Messaging::Instance()->AddError(true, nullptr, nullptr,
					        "The  File %s.%s seem to be bad. Can't load", ps.name.c_str(), ps.ext.c_str());
				}
			}
			else
			{
				Messaging::Instance()->AddError(true, nullptr, nullptr,
				        "The  File %s.%s seem to be bad. Can't load", ps.name.c_str(), ps.ext.c_str());
			}
		}
	}
	else
	{
		Messaging::Instance()->AddError(true, nullptr, nullptr, "font %s not found", fontFilePathName.c_str());
		m_NeedFilePathResolve = true;
	}

	vProjectFile->SetProjectChange();

	return res;
}

void FontInfos::Clear()
{
	DestroyFontTexture();
	m_ImFontAtlas.Clear();
	m_GlyphNames.clear();
	m_GlyphCodePointToName.clear();
	m_SelectedGlyphs.clear();
	m_Filters.clear();
}

static const char *standardMacNames[258] = { ".notdef", ".null", "nonmarkingreturn", "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", "ampersand", "quotesingle", "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", "slash", "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "colon", "semicolon", "less", "equal", "greater", "question", "at", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft", "backslash", "bracketright", "asciicircum", "underscore", "grave", "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft", "bar", "braceright", "asciitilde", "Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis", "aacute", "agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla", "eacute", "egrave", "ecircumflex", "edieresis", "iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute", "ograve", "ocircumflex", "odieresis", "otilde", "uacute", "ugrave", "ucircumflex", "udieresis", "dagger", "degree", "cent", "sterling", "section", "bullet", "paragraph", "germandbls", "registered", "copyright", "trademark", "acute", "dieresis", "notequal", "AE", "Oslash", "infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation", "product", "pi", "integral", "ordfeminine", "ordmasculine", "Omega", "ae", "oslash", "questiondown", "exclamdown", "logicalnot", "radical", "florin", "approxequal", "Delta", "guillemotleft", "guillemotright", "ellipsis", "nonbreakingspace", "Agrave", "Atilde", "Otilde", "OE", "oe", "endash", "emdash", "quotedblleft", "quotedblright", "quoteleft", "quoteright", "divide", "lozenge", "ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright", "fi", "fl", "daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand", "Acircumflex", "Ecircumflex", "Aacute", "Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex", "apple", "Ograve", "Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde", "macron", "breve", "dotaccent", "ring", "cedilla", "hungarumlaut", "ogonek", "caron", "Lslash", "lslash", "Scaron", "scaron", "Zcaron", "zcaron", "brokenbar", "Eth", "eth", "Yacute", "yacute", "Thorn", "thorn", "minus", "multiply", "onesuperior", "twosuperior", "threesuperior", "onehalf", "onequarter", "threequarters", "franc", "Gbreve", "gbreve", "Idotaccent", "Scedilla", "scedilla", "Cacute", "cacute", "Ccaron", "ccaron", "dcroat" };
void FontInfos::FillGlyphNames()
{
	if (!m_ImFontAtlas.ConfigData.empty())
	{
		m_GlyphNames.clear();

		stbtt_fontinfo fontInfo;
		const int font_offset = stbtt_GetFontOffsetForIndex(
			(unsigned char*)m_ImFontAtlas.ConfigData[0].FontData,
			m_ImFontAtlas.ConfigData[0].FontNo);
		if (!stbtt_InitFont(&fontInfo,
			(unsigned char*)m_ImFontAtlas.ConfigData[0].FontData, font_offset))
			return;

		// get table offet and length
		// https://developer.apple.com/fonts/TrueType-Reference-Manual/RM06/Chap6.html => Table Directory
		stbtt_int32 num_tables = ttUSHORT(fontInfo.data + fontInfo.fontstart + 4);
		stbtt_uint32 tabledir = fontInfo.fontstart + 12;
		stbtt_uint32 tablePos = 0;
		stbtt_uint32 tableLen = 0;
		for (int i = 0; i < num_tables; ++i)
		{
			stbtt_uint32 loc = tabledir + 16 * i;
			if (stbtt_tag(fontInfo.data + loc + 0, "post"))
			{
				tablePos = ttULONG(fontInfo.data + loc + 8);
				tableLen = ttULONG(fontInfo.data + loc + 12);
				break;
			}
		}
		if (!tablePos) return;

		// fill map of names
		stbtt_uint8 *data = fontInfo.data + tablePos;
		stbtt_int32 version = ttUSHORT(data);

		//stbtt_uint32 italicAngle = ttUSHORT(data + 4);
		//uint16_t underlinePosition = ttUSHORT(data + 8);
		//uint16_t underlineThickness = ttUSHORT(data + 10);
		//uint32_t isFixedPitch = ttUSHORT(data + 12);
		//uint32_t minMemType42 = ttUSHORT(data + 16);
		//uint32_t maxMemType42 = ttUSHORT(data + 20);
		//uint32_t minMemType1 = ttUSHORT(data + 24);
		//uint32_t maxMemType1 = ttUSHORT(data + 28);
		//uint16_t numGlyphs = ttUSHORT(data + 32);

		if (version == 2)
		{
			std::vector<std::string> pendingNames;
			stbtt_uint16 numberGlyphs = ttUSHORT(data + 32);
			stbtt_uint32 offset = 34 + 2 * numberGlyphs;
			while (offset < tableLen)
			{
				uint8_t len = data[offset];
				std::string s;
				if (len > 0)
				{
					s = std::string((const char *)data + offset + 1, len);
				}
				offset += len + 1;
				pendingNames.push_back(s);
			}
			for (stbtt_uint16 j = 0; j < numberGlyphs; j++)
			{
				stbtt_uint16 mapIdx = ttUSHORT(data + 34 + 2 * j);
				if (mapIdx >= 258)
				{
					stbtt_uint16 idx = mapIdx - 258;
					if (idx < pendingNames.size())
						m_GlyphNames.push_back(pendingNames[idx]);
				}
				else
				{
					m_GlyphNames.emplace_back(standardMacNames[mapIdx]);
				}
			}
		}
	}
}

std::string FontInfos::GetGlyphName(uint32_t vCodePoint)
{
	if (m_GlyphCodePointToName.find(vCodePoint) != m_GlyphCodePointToName.end())
	{
		return m_GlyphCodePointToName[vCodePoint];
	}
	return "Symbol Name";
}

void FontInfos::DrawInfos()
{
	if (!m_ImFontAtlas.Fonts.empty())
	{
		if (ImGui::BeginFramedGroup("Current Font Infos"))
		{
			if (!m_FontFilePathName.empty())
			{
				ImGui::Text("[Font path]");
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("%s", m_FontFilePathName.c_str());
			}

			ImGui::Text("Count Glyphs : %i", m_ImFontAtlas.Fonts[0]->Glyphs.size());
			ImGui::Text("Count Selected Glyphs : %u", m_SelectedGlyphs.size());
			ImGui::Text("Texture Size : %i x %i", m_ImFontAtlas.TexWidth, m_ImFontAtlas.TexHeight);
			ImGui::Text("Ascent / Descent : %i / %i", m_Ascent, m_Descent);
			//ImGui::Text("Line gap : %i", m_LineGap); // dont know what is it haha
			//ImGui::Text("Scale pixel height : %.4f", m_Point); // same.., its used internally by ImGui but dont know what is it
			ImGui::Text("Glyph Bounding Box :\n\tinf x : %i, inf y : %i\n\tsup x : %i, sup y : %i", 
				m_BoundingBox.x, m_BoundingBox.y, m_BoundingBox.z, m_BoundingBox.w);

			ImGui::EndFramedGroup(true);
		}
	}
}

void FontInfos::GetInfos()
{
	stbtt_fontinfo fontInfo;
	const int font_offset = stbtt_GetFontOffsetForIndex(
		(unsigned char*)m_ImFontAtlas.ConfigData[0].FontData,
		m_ImFontAtlas.ConfigData[0].FontNo);
	if (stbtt_InitFont(&fontInfo, (unsigned char*)m_ImFontAtlas.ConfigData[0].FontData, font_offset))
	{
		stbtt_GetFontVMetrics(&fontInfo, &m_Ascent, &m_Descent, &m_LineGap);
		stbtt_GetFontBoundingBox(&fontInfo, &m_BoundingBox.x, &m_BoundingBox.y, &m_BoundingBox.z, &m_BoundingBox.w);
		m_Point = stbtt_ScaleForPixelHeight(&fontInfo, (float)m_FontSize);
	}
}

void FontInfos::GenerateCodePointToGlypNamesDB()
{
	if (!m_ImFontAtlas.ConfigData.empty())
	{
		m_GlyphCodePointToName.clear();

		stbtt_fontinfo fontInfo;
		const int font_offset = stbtt_GetFontOffsetForIndex(
			(unsigned char*)m_ImFontAtlas.ConfigData[0].FontData,
			m_ImFontAtlas.ConfigData[0].FontNo);
		if (stbtt_InitFont(&fontInfo, (unsigned char*)m_ImFontAtlas.ConfigData[0].FontData, font_offset))
		{
			if (m_ImFontAtlas.IsBuilt())
			{
				ImFont* font = m_ImFontAtlas.Fonts[0];
				if (font)
				{
					for (auto glyph : font->Glyphs)
					{
						int glyphIndex = stbtt_FindGlyphIndex(&fontInfo, (uint32_t)glyph.Codepoint);
						m_GlyphCodePointToGlyphIndex[(uint32_t)glyph.Codepoint] = glyphIndex;
						if (glyphIndex < (int)m_GlyphNames.size())
						{
							std::string name = m_GlyphNames[glyphIndex];
							m_GlyphCodePointToName[glyph.Codepoint] = name;
						}
						else
						{
							m_GlyphCodePointToName[glyph.Codepoint] = "";
						}
					}
				}
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//// FONT TEXTURE ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void FontInfos::CreateFontTexture()
{
	if (!m_ImFontAtlas.Fonts.empty())
	{
		unsigned char* pixels;
		int width, height;
		m_ImFontAtlas.GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

		GLuint id = 0;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		m_ImFontAtlas.TexID = (ImTextureID)id;

		glBindTexture(GL_TEXTURE_2D, last_texture);
	}
}

void FontInfos::DestroyFontTexture()
{
    // use (size_t) instead of (Gluint) for avoid error : 
	// << Cast from pointer to smaller type 'int' loses information >> on MAcOs mojave..
    // it weird than this trick work...
    GLuint id = (size_t)m_ImFontAtlas.TexID;
	if (id)
	{
		glDeleteTextures(1, &id);
		m_ImFontAtlas.TexID = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////////
//// CONFIG FILE /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

std::string FontInfos::getXml(const std::string& vOffset)
{
	std::string res;

	res += vOffset + "<font name=\"" + m_FontFileName + "\">\n";

	if (!m_SelectedGlyphs.empty())
	{
		res += vOffset + "\t<glyphs>\n";
		for (auto &it : m_SelectedGlyphs)
		{
			res += vOffset + "\t\t<glyph orgId=\"" + ct::toStr(it.second.glyph.Codepoint) +
				"\" newId=\"" + ct::toStr(it.second.newCodePoint) + 
				"\" orgName=\"" + it.second.oldHeaderName + 
				"\" newName=\"" + it.second.newHeaderName + "\"/>\n";
		}
		res += vOffset + "\t</glyphs>\n";
	}

	res += vOffset + "\t<prefix>" + m_FontPrefix + "</prefix>\n";
	res += vOffset + "\t<pathfilename>" + m_FontFilePathName + "</pathfilename>\n";
	res += vOffset + "\t<oversample>" + ct::toStr(m_Oversample) + "</oversample>\n";
	res += vOffset + "\t<fontsize>" + ct::toStr(m_FontSize) + "</fontsize>\n";
	
	if (!m_Filters.empty())
	{
		res += vOffset + "\t<filters>\n";
		for (auto &it : m_Filters)
		{
			res += vOffset + "\t\t<filter name=\"" + it + "\"/>\n";
		}
		res += vOffset + "\t</filters>\n";
	}

	res += vOffset + "</font>\n";

	return res;
}

void FontInfos::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent)
{
	// The value of this child identifies the name of this element
	std::string strName;
	std::string strValue;
	std::string strParentName;

	strName = vElem->Value();
	if (vElem->GetText())
		strValue = vElem->GetText();
	if (vParent != nullptr)
		strParentName = vParent->Value();

	if (strName == "font")
	{
		auto att = vElem->FirstAttribute();
		if (att && std::string(att->Name()) == "name")
		{
			m_FontFileName = att->Value();
		}

		for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
		{
			RecursParsingConfig(child->ToElement(), vElem);
		}
	}
	else if (strParentName == "font")
	{
		if (strName == "prefix")
			m_FontPrefix = strValue;
		else if (strName == "pathfilename")
			m_FontFilePathName = strValue;
		else if (strName == "oversample")
			m_Oversample = ct::ivariant(strValue).getI();
		else if (strName == "fontsize")
			m_FontSize = ct::ivariant(strValue).getI();
		else if (strName == "glyphs" || strName == "filters")
		{
			for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
			{
				RecursParsingConfig(child->ToElement(), vElem);
			}
		}
	}
	else if (strParentName == "glyphs" &&  strName == "glyph")
	{
		uint32_t oldcodepoint = 0;
        uint32_t newcodepoint = 0;
		std::string oldName;
		std::string newName;

		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "orgId" || 
				attName == "id") // for compatibility with first format, will be removed in few versions
				oldcodepoint = (uint32_t)ct::ivariant(attValue).getI();
			else if (attName == "newId" || 
				attName == "nid")  // for compatibility with first format, will be removed in few versions
				newcodepoint = (uint32_t)ct::ivariant(attValue).getI();
			else if (attName == "orgName") oldName = attValue;
			else if (attName == "newName" || 
				attName == "name")  // for compatibility with first format, will be removed in few versions
				newName = attValue;
		}

		ImFontGlyph g{};
		g.Codepoint = oldcodepoint;
		m_SelectedGlyphs[oldcodepoint] = GlyphInfos(g, oldName, newName, newcodepoint);
	}
	else if (strParentName == "filters" &&  strName == "filter")
	{
		ct::ResetBuffer(m_SearchBuffer);
		auto att = vElem->FirstAttribute();
		if (att && std::string(att->Name()) == "name")
		{
			std::string attValue = att->Value();
			m_Filters.insert(attValue);
			if (m_Filters.size() > 1)
				ct::AppendToBuffer(m_SearchBuffer, 1023, ",");
			ct::AppendToBuffer(m_SearchBuffer, 1023, attValue);
		}
	}
}

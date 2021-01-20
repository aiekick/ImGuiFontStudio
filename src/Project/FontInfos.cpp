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

#include <ctools/FileHelper.h>
#include <Project/ProjectFile.h>
#include <Gui/ImGuiWidgets.h>
#include <Helper/Messaging.h>
#include <ctools/Logger.h>
#include <Panes/ParamsPane.h>

#include <ImguiImpl/freetype/imgui_freetype.h>

#define STB_TRUETYPE_IMPLEMENTATION  
#include <imgui/imstb_truetype.h>

#include <glad/glad.h>

#include <array>

///////////////////////////////////////////////////////////////////////////////////
// https://stackoverflow.com/questions/31161284/how-can-i-get-the-corresponding-error-string-from-an-ft-error-code
#include <ft2build.h>
#include FT_FREETYPE_H
const char* getErrorMessage(FT_Error err)
{
#undef FTERRORS_H_
#define FT_ERRORDEF( e, v, s )  case e: return s;
#define FT_ERROR_START_LIST     switch (err) {
#define FT_ERROR_END_LIST       }
#include FT_ERRORS_H
	return "(Unknown error)";
}
///////////////////////////////////////////////////////////////////////////////////
static ProjectFile defaultProjectValues;
static FontInfos defaultFontInfosValues;
///////////////////////////////////////////////////////////////////////////////////

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

std::shared_ptr<FontInfos> FontInfos::Create()
{
	auto res = std::make_shared<FontInfos>();
	res->m_This = res;
	return res;
}

FontInfos::FontInfos() = default;
FontInfos::~FontInfos()
{
	m_ImFontAtlas.Clear();
}

bool FontInfos::LoadFont(ProjectFile *vProjectFile, const std::string& vFontFilePathName)
{
	bool res = false;

	if (!vProjectFile || !vProjectFile->IsLoaded())
		return res;

	std::string fontFilePathName = FileHelper::Instance()->CorrectSlashTypeForFilePathName(vFontFilePathName);
	
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
		m_ImFontAtlas.Flags |= 
			ImFontAtlasFlags_NoMouseCursors | // hte mouse cursors
			ImFontAtlasFlags_NoBakedLines; // the big triangle

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
				bool success = false;

				m_ImFontAtlas.TexGlyphPadding = fontPadding;

				for (int n = 0; n < m_ImFontAtlas.ConfigData.Size; n++)
				{
					freeTypeFlag = (rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE) ? freeTypeFlag : 0x00;

					ImFontConfig* font_config = (ImFontConfig*)&m_ImFontAtlas.ConfigData[n];
					font_config->RasterizerMultiply = fontMultiply;
					font_config->RasterizerFlags = freeTypeFlag;
					font_config->OversampleH = m_Oversample;
					font_config->OversampleV = m_Oversample;
				}
				
				FT_Error freetypeError = 0;
				if (rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE)
				{
					success = ImGuiFreeType::BuildFontAtlas(&m_ImFontAtlas, freeTypeFlag, &freetypeError);
				}
				else if (rasterizerMode == RasterizerEnum::RASTERIZER_STB)
				{
					success = m_ImFontAtlas.Build();
				}

				if (success)
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
						UpdateInfos();
						UpdateFiltering();

						// update glyph ptrs
						for (auto &it : m_SelectedGlyphs)
						{
							uint32_t codePoint = it.first;

							auto glyph = font->FindGlyphNoFallback((ImWchar)codePoint);
							if (glyph)
							{
								if (it.second)
								{
									it.second->glyph = *glyph;
									it.second->oldHeaderName = GetGlyphName(codePoint);
									it.second->glyphIndex = m_GlyphCodePointToGlyphIndex[codePoint];
								}
								
							}
						}

						m_NeedFilePathResolve = false;

						res = true;
					}
				}
				else
				{
					if (rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE)
					{
						auto reason = getErrorMessage(freetypeError);
						Messaging::Instance()->AddError(true, nullptr, nullptr,
							"Feetype fail to load font file %s.%s. Reason : %s", ps.name.c_str(), ps.ext.c_str(), reason);
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
	rasterizerMode = RasterizerEnum::RASTERIZER_STB;
	freeTypeFlag = 0;
	fontMultiply = 1.0f;
	fontPadding = 1;
}

static const char *standardMacNames[258] = 
{ ".notdef", ".null", "nonmarkingreturn", "space", "exclam", "quotedbl", "numbersign", "dollar", "percent", 
"ampersand", "quotesingle", "parenleft", "parenright", "asterisk", "plus", "comma", "hyphen", "period", 
"slash", "zero", "one", "two", "three", "four", "five", "six", "seven", "eight", "nine", "colon", 
"semicolon", "less", "equal", "greater", "question", "at", "A", "B", "C", "D", "E", "F", "G", "H", 
"I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "bracketleft", 
"backslash", "bracketright", "asciicircum", "underscore", "grave", "a", "b", "c", "d", "e", "f", "g", "h", 
"i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "braceleft", "bar", 
"braceright", "asciitilde", "Adieresis", "Aring", "Ccedilla", "Eacute", "Ntilde", "Odieresis", "Udieresis", 
"aacute", "agrave", "acircumflex", "adieresis", "atilde", "aring", "ccedilla", "eacute", "egrave", "ecircumflex", 
"edieresis", "iacute", "igrave", "icircumflex", "idieresis", "ntilde", "oacute", "ograve", "ocircumflex", "odieresis", 
"otilde", "uacute", "ugrave", "ucircumflex", "udieresis", "dagger", "degree", "cent", "sterling", "section", "bullet", 
"paragraph", "germandbls", "registered", "copyright", "trademark", "acute", "dieresis", "notequal", "AE", "Oslash", 
"infinity", "plusminus", "lessequal", "greaterequal", "yen", "mu", "partialdiff", "summation", "product", "pi", 
"integral", "ordfeminine", "ordmasculine", "Omega", "ae", "oslash", "questiondown", "exclamdown", "logicalnot", 
"radical", "florin", "approxequal", "Delta", "guillemotleft", "guillemotright", "ellipsis", "nonbreakingspace", 
"Agrave", "Atilde", "Otilde", "OE", "oe", "endash", "emdash", "quotedblleft", "quotedblright", "quoteleft", 
"quoteright", "divide", "lozenge", "ydieresis", "Ydieresis", "fraction", "currency", "guilsinglleft", "guilsinglright", 
"fi", "fl", "daggerdbl", "periodcentered", "quotesinglbase", "quotedblbase", "perthousand", "Acircumflex", "Ecircumflex", 
"Aacute", "Edieresis", "Egrave", "Iacute", "Icircumflex", "Idieresis", "Igrave", "Oacute", "Ocircumflex", "apple", "Ograve", 
"Uacute", "Ucircumflex", "Ugrave", "dotlessi", "circumflex", "tilde", "macron", "breve", "dotaccent", "ring", "cedilla", 
"hungarumlaut", "ogonek", "caron", "Lslash", "lslash", "Scaron", "scaron", "Zcaron", "zcaron", "brokenbar", "Eth", "eth", 
"Yacute", "yacute", "Thorn", "thorn", "minus", "multiply", "onesuperior", "twosuperior", "threesuperior", "onehalf", 
"onequarter", "threequarters", "franc", "Gbreve", "gbreve", "Idotaccent", "Scedilla", "scedilla", "Cacute", "cacute", 
"Ccaron", "ccaron", "dcroat" };

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

void FontInfos::DrawInfos(ProjectFile* vProjectFile)
{
	if (!m_ImFontAtlas.Fonts.empty() && !m_InfosToDisplay.empty())
	{
		bool needFontReGen = false;
		
		float aw = 0.0f;

		if (ImGui::BeginFramedGroup("Selected Font"))
		{
			aw = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2.0f;

			if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_Bullet))
			{
				static ImGuiTableFlags flags =
					ImGuiTableFlags_SizingFixedFit |
					ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
					ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
					ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_Borders;
				if (ImGui::BeginTable("##fontinfosTable", 2, flags, ImVec2(aw, ImGui::GetTextLineHeightWithSpacing() * 7)))
				{
					ImGui::TableSetupScrollFreeze(0, 1); // Make header always visible
					ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthFixed, -1, 0);
					ImGui::TableSetupColumn("Infos", ImGuiTableColumnFlags_WidthStretch, -1, 1);
					ImGui::TableHeadersRow(); // draw headers

					m_InfosToDisplayClipper.Begin((int)m_InfosToDisplay.size(), ImGui::GetTextLineHeightWithSpacing());
					while (m_InfosToDisplayClipper.Step())
					{
						for (int i = m_InfosToDisplayClipper.DisplayStart; i < m_InfosToDisplayClipper.DisplayEnd; i++)
						{
							if (i < 0) continue;

							const auto& infos = m_InfosToDisplay[i];

							ImGui::TableNextRow();

							if (ImGui::TableSetColumnIndex(0)) // first column
							{
								ImGui::Selectable(infos.first.c_str(), false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap);
							}
							if (ImGui::TableSetColumnIndex(1)) // second column
							{
								float sw = ImGui::CalcTextSize(infos.second.c_str()).x;
								ImGui::Text("%s", infos.second.c_str());
								if (sw > aw - ImGui::GetCursorPosX())
								{
									if (ImGui::IsItemHovered())
										ImGui::SetTooltip(infos.second.c_str());
								}
							}
						}
					}
					ImGui::EndTable();
				}
			}
			
			ImGui::FramedGroupSeparator();

			ImGui::Text("Selected glyphs : %u", (uint32_t)m_SelectedGlyphs.size());

			if (ImGui::Button("Clear Glyph Transforms (All Glyphs)", ImVec2(-1,0)))
			{
				ClearTransforms(vProjectFile);
			}
			
			ImGui::FramedGroupSeparator();

			aw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

			ImGui::PushItemWidth(aw);
			if (ImGui::RadioButtonLabeled("FreeType (Default)", "Use FreeType Raterizer", FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE))
			{
				needFontReGen = true;
				FontInfos::rasterizerMode = RasterizerEnum::RASTERIZER_FREETYPE;
			}
			
			ImGui::SameLine();

			if (ImGui::RadioButtonLabeled("Stb", "Use Stb Raterizer", FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_STB))
			{
				needFontReGen = true;
				FontInfos::rasterizerMode = RasterizerEnum::RASTERIZER_STB;
			}
			ImGui::PopItemWidth();
			
			ImGui::FramedGroupSeparator();

			needFontReGen |= ImGui::SliderIntDefaultCompact(-1.0f, "Font Size", &vProjectFile->m_SelectedFont->m_FontSize, 7, 50, defaultFontInfosValues.m_FontSize);
			
			if (FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_STB)
			{
				needFontReGen |= ImGui::SliderIntDefaultCompact(-1.0f, "Font Anti-aliasing", &vProjectFile->m_SelectedFont->m_Oversample, 1, 5, defaultFontInfosValues.m_Oversample);
			}
			else if (FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE)
			{
				needFontReGen |= ImGui::SliderFloatDefaultCompact(-1.0f, "Multiply", &fontMultiply, 0.0f, 2.0f, 1.0f);
			}

			needFontReGen |= ImGui::SliderIntDefaultCompact(-1.0f, "Padding", &fontPadding, 0, 16, 1);

			if (FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE)
			{
				if (ImGui::CollapsingHeader("Freetype Settings", ImGuiTreeNodeFlags_Bullet))
				{
					needFontReGen |= ImGui::CheckboxFlags("NoHinting", &freeTypeFlag, ImGuiFreeType::NoHinting);
					needFontReGen |= ImGui::CheckboxFlags("NoAutoHint", &freeTypeFlag, ImGuiFreeType::NoAutoHint);
					needFontReGen |= ImGui::CheckboxFlags("ForceAutoHint", &freeTypeFlag, ImGuiFreeType::ForceAutoHint);
					needFontReGen |= ImGui::CheckboxFlags("LightHinting", &freeTypeFlag, ImGuiFreeType::LightHinting);
					needFontReGen |= ImGui::CheckboxFlags("MonoHinting", &freeTypeFlag, ImGuiFreeType::MonoHinting);
					needFontReGen |= ImGui::CheckboxFlags("Bold", &freeTypeFlag, ImGuiFreeType::Bold);
					needFontReGen |= ImGui::CheckboxFlags("Oblique", &freeTypeFlag, ImGuiFreeType::Oblique);
					needFontReGen |= ImGui::CheckboxFlags("Monochrome", &freeTypeFlag, ImGuiFreeType::Monochrome);
					//needFontReGen |= ImGui::CheckboxFlags("LoadColor", &freeTypeFlag, ImGuiFreeType::LoadColor);
				}
			}

			ImGui::EndFramedGroup(true);
		}

		if (needFontReGen)
		{
			vProjectFile->m_SelectedFont->m_FontSize = ct::clamp(vProjectFile->m_SelectedFont->m_FontSize, 7, 50);
			vProjectFile->m_SelectedFont->m_Oversample = ct::clamp(vProjectFile->m_SelectedFont->m_Oversample, 1, 5);
			ParamsPane::Instance()->OpenFont(vProjectFile, vProjectFile->m_SelectedFont->m_FontFilePathName, false);
			vProjectFile->SetProjectChange();
		}
	}
}

void FontInfos::UpdateInfos()
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

	//--------------------------------------------------------
	m_InfosToDisplay.clear();
	m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Font", m_FontFilePathName));
	if (GetImFont())
	{
		m_InfosToDisplay.push_back(std::pair<std::string, std::string>("N Glyphs :", ct::toStr(GetImFont()->Glyphs.size())));
	}
	//m_InfosToDisplay.push_back(std::pair<std::string, std::string>("N Sel Glyphs :", ct::toStr(m_SelectedGlyphs.size())));
	m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Texture Size :", ct::toStr("%i x %i", m_ImFontAtlas.TexWidth, m_ImFontAtlas.TexHeight)));
	m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Ascent / Descent :", ct::toStr("%i / %i", m_Ascent, m_Descent)));
	m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Glyph BBox :", ct::toStr("min : %i x %i/max : %i x %i",
		m_BoundingBox.x, m_BoundingBox.y, m_BoundingBox.z, m_BoundingBox.w)));
#ifdef _DEBUG
	//m_InfosToDisplay.push_back(std::pair<std::string, std::string>("SizeInPixels :", ct::toStr("%.2f", m_FontConfig.SizePixels)));
	//m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Line gap :", ct::toStr("%i", m_LineGap))); // dont know what is it haha
	m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Scale pixel height :", ct::toStr("%.4f", m_Point))); // same.., its used internally by ImGui but dont know what is it
#endif
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
				ImFont* font = GetImFont();
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

void FontInfos::UpdateFiltering()
{
	m_FilteredGlyphs.clear();

	ImFont* font = GetImFont();
	if (font)
	{
		uint32_t countGlyphs = (uint32_t)font->Glyphs.size();
		for (uint32_t idx = 0; idx < countGlyphs; idx++)
		{
			auto glyph = *(font->Glyphs.begin() + idx);

			if (!m_Filters.empty())
			{
				std::string name = m_GlyphCodePointToName[glyph.Codepoint];

				if (!name.empty())
				{
					for (const auto& it : m_Filters)
					{
						if (name.find(it) != std::string::npos) // found
						{
							m_FilteredGlyphs.push_back(glyph);
						}
					}
				}
			}
			else
			{
				m_FilteredGlyphs.push_back(glyph);
			}
		}
	}
}

void FontInfos::ClearTransforms(ProjectFile* vProjectFile)
{
	if (vProjectFile)
	{
		for (auto glyph : m_SelectedGlyphs)
		{
			if (glyph.second)
			{
				glyph.second->m_Translation = 0.0f;
				glyph.second->m_Scale = 1.0f;
				glyph.second->simpleGlyph.ClearTransform();
			}
		}

		vProjectFile->SetProjectChange();
	}
}

ImFont* FontInfos::GetImFont()
{
	if (!m_ImFontAtlas.Fonts.empty())
	{
		return m_ImFontAtlas.Fonts[0];
	}
	return nullptr;
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

		// size_t is 4 bytes sized for x32 and 8 bytes sizes for x64.
		// TexID is ImTextureID is a void so same size as size_t
		// id is a uint so 4 bytes on x32 and x64
		// so conversion first on size_t (uint32/64) and after on ImTextureID give no warnings
		m_ImFontAtlas.TexID = (ImTextureID)(size_t)id; 

		glBindTexture(GL_TEXTURE_2D, last_texture);
	}
}

void FontInfos::DestroyFontTexture()
{
	// size_t is 4 bytes sized for x32 and 8 bytes sizes for x64.
	// TexID is ImTextureID is a void so same size as size_t
	// id is a uint so 4 bytes on x32 and x64
	// so conversion first on size_t (uint32/64) and after on GLuint give no warnings
    GLuint id = (GLuint)(size_t)m_ImFontAtlas.TexID;
	if (id)
	{
		glDeleteTextures(1, &id);
		m_ImFontAtlas.TexID = nullptr;
	}
}

//////////////////////////////////////////////////////////////////////////////
//// CONFIG FILE /////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

std::string FontInfos::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string res;

	res += vOffset + "<font name=\"" + m_FontFileName + "\">\n";

	if (!m_SelectedGlyphs.empty())
	{
		res += vOffset + "\t<glyphs>\n";
		for (auto &it : m_SelectedGlyphs)
		{
			if (it.second)
			{
				res += vOffset + "\t\t<glyph orgId=\"" + ct::toStr(it.second->glyph.Codepoint) +
					"\" newId=\"" + ct::toStr(it.second->newCodePoint) +
					"\" orgName=\"" + it.second->oldHeaderName +
					"\" newName=\"" + it.second->newHeaderName +
					"\" trans=\"" + ct::fvec2(it.second->m_Translation).string() + "\"/>\n";
			}
		}
		res += vOffset + "\t</glyphs>\n";
	}

	res += vOffset + "\t<prefix>" + m_FontPrefix + "</prefix>\n";
	res += vOffset + "\t<pathfilename>" + m_FontFilePathName + "</pathfilename>\n";
	res += vOffset + "\t<oversample>" + ct::toStr(m_Oversample) + "</oversample>\n";
	res += vOffset + "\t<fontsize>" + ct::toStr(m_FontSize) + "</fontsize>\n";

	res += vOffset + "\t<rasterizer>" + ct::toStr(rasterizerMode) + "</rasterizer>\n";
	res += vOffset + "\t<freetypeflag>" + ct::toStr(freeTypeFlag) + "</freetypeflag>\n";
	res += vOffset + "\t<freetypemultiply>" + ct::toStr(fontMultiply) + "</freetypemultiply>\n";
	res += vOffset + "\t<padding>" + ct::toStr(fontPadding) + "</padding>\n";

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

bool FontInfos::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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
			m_Oversample = ct::ivariant(strValue).GetI();
		else if (strName == "fontsize")
			m_FontSize = ct::ivariant(strValue).GetI();
		else if (strName == "rasterizer")
			rasterizerMode = (RasterizerEnum)ct::ivariant(strValue).GetI();
		else if (strName == "freetypeflag")
			freeTypeFlag = ct::ivariant(strValue).GetI();
		else if (strName == "freetypemultiply")
			fontMultiply = ct::fvariant(strValue).GetF();
		else if (strName == "padding")
			fontPadding = ct::ivariant(strValue).GetI();
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
		ImVec2 translation;

		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "orgId" ||
				attName == "id") // for compatibility with first format, will be removed in few versions
				oldcodepoint = (uint32_t)ct::ivariant(attValue).GetI();
			else if (attName == "newId" ||
				attName == "nid")  // for compatibility with first format, will be removed in few versions
				newcodepoint = (uint32_t)ct::ivariant(attValue).GetI();
			else if (attName == "orgName") oldName = attValue;
			else if (attName == "newName" ||
				attName == "name")  // for compatibility with first format, will be removed in few versions
				newName = attValue;
			else if (attName == "trans")
				translation = ct::toImVec2(ct::fvariant(attValue).GetV2());
		}

		ImFontGlyph g{};
		g.Codepoint = oldcodepoint;
		m_SelectedGlyphs[oldcodepoint] = GlyphInfos::Create(m_This, g, oldName, newName, newcodepoint, translation);
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

	return true;
}

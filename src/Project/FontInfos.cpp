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
#include <Res/CustomFont.h>
#include <MainFrame.h>

#include <Helper/Profiler.h>
#include <ImguiImpl/freetype/imgui_freetype_unleashed.h>

#define STB_TRUETYPE_IMPLEMENTATION  
#include <imgui/imstb_truetype.h>

#include <glad/glad.h>

#include <array>

#define USE_THREADED_FONT_LOADING

///////////////////////////////////////////////////////////////////////////////////
static ProjectFile defaultProjectValues;
static FontInfos defaultFontInfosValues;
///////////////////////////////////////////////////////////////////////////////////
uint32_t FontInfos::maxThreadInParallel = (std::thread::hardware_concurrency() / 2U);
std::atomic<uint32_t> FontInfos::countThreadInParallel = 0U;
///////////////////////////////////////////////////////////////////////////////////

// Extract the UpperCase char's of a string and return as a Prefix
// if the font name is like "FontAwesome" (UpperCase char), this func will give the prefix "FA"
// if all the name is lowercase, retunr nothing..
static std::string GetPrefixFromFontFileName(const std::string& vFileName)
{
	ZoneScoped;

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
	ZoneScoped;

	auto res = std::make_shared<FontInfos>();
	res->m_This = res;
	return res;
}

FontInfos::FontInfos() = default;
FontInfos::~FontInfos()
{
	ZoneScoped;

	m_ImFontAtlas.Clear();
	Clear();
}

void FontInfos::Clear()
{
	ZoneScoped;

	DestroyFontTexture();
	m_ImFontAtlas.Clear();
	m_GlyphNames.clear();
	m_Glyphs.clear();
	m_SelectedGlyphs.clear();
	m_Filters.clear();
	rasterizerMode = RasterizerEnum::RASTERIZER_FREETYPE;
	freeTypeFlag = ImGuiFreeType_unleashed::FreeType_Default;
	fontMultiply = 1.0f;
	fontPadding = 1;
	textureFiltering = GL_NEAREST;
}

bool FontInfos::LoadFont(
	ProjectFile *vProjectFile, 
	const std::string& vFontFilePathName,
	std::function<void()> vLoadingThreadFinishFunc)
{
	ZoneScoped;

	m_LoadingThreadFinishFunc = vLoadingThreadFinishFunc;
	m_FontFilePathNameToLoad = vFontFilePathName;
	m_WaitingToBeLoaded = true;

	return false;
}

//////////////////////////////////////////////////////////////////////////////
//// THREAD //////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

bool FontInfos::LoadFontThreaded(
	ProjectFile* vProjectFile,
	const std::string& vFontFilePathName,
	std::atomic<bool>& vWorking,
	std::atomic<float>& vProgress,
	std::atomic<uint32_t>& vObjectsCount)
{
	ZoneScoped;

	tracy::SetThreadName("Load Font");

	if (!vWorking) return false;

	if (!vProjectFile || !vProjectFile->IsLoaded())
		return false;

	std::string fontFilePathName = FileHelper::Instance()->CorrectSlashTypeForFilePathName(vFontFilePathName);

	if (!FileHelper::Instance()->IsAbsolutePath(fontFilePathName))
	{
		fontFilePathName = vProjectFile->GetAbsolutePath(fontFilePathName);
	}

	bool res = false;

	if (FileHelper::Instance()->IsFileExist(fontFilePathName))
	{
		m_ImFontAtlas.Clear();
		m_ImFontAtlas.Flags |=
			ImFontAtlasFlags_NoMouseCursors | // hte mouse cursors
			ImFontAtlasFlags_NoBakedLines; // the big triangle
		auto ps = FileHelper::Instance()->ParsePathFileName(fontFilePathName);
		if (ps.isOk)
		{
			m_FontFileName = ps.name + "." + ps.ext;
			if (!vWorking) return false;

			{
				ZoneScopedN("AddFontFromFileTTF");
				res = m_ImFontAtlas.AddFontFromFileTTF(fontFilePathName.c_str(), (float)m_FontSize);
			}

			if (res)
			{
				if (!vWorking) return false;
				m_FontFilePathName = vProjectFile->GetRelativePath(fontFilePathName);
				m_ImFontAtlas.TexGlyphPadding = fontPadding;
				ImGuiFreeType_unleashed::FT_Error freetypeError = 0;
				if (!vWorking) return false;
				if (ImGuiFreeType_unleashed::BuildFontAtlas(&m_ImFontAtlas, freeTypeFlag, &freetypeError))
				{
					if (m_ImFontAtlas.IsBuilt())
					{
						if (!m_ImFontAtlas.Fonts.empty())
						{
							if (!vWorking) return false;
							if (m_FontPrefix.empty())
								m_FontPrefix = GetPrefixFromFontFileName(ps.name);
							Build_Glyph_DataBase();
							if (!vWorking) return false;
							UpdateFiltering();
							if (!vWorking) return false;
							UpdateSelectedGlyphs();
							if (!vWorking) return false;
							UpdateInfos();
							if (!vWorking) return false;
							m_NeedFilePathResolve = false;
							res = true;
						}
					}
					else
					{
						res = false;
						Messaging::Instance()->AddError(true, nullptr, nullptr, "Atlas not Built..");
					}
				}
				else
				{
					res = false;
					if (rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE)
					{
						Messaging::Instance()->AddError(true, nullptr, nullptr,
							"Feetype fail to load font file %s.%s. Reason : %s",
							ps.name.c_str(), ps.ext.c_str(), ImGuiFreeType_unleashed::GetErrorMessage(freetypeError));
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
				res = false;
				Messaging::Instance()->AddError(true, nullptr, nullptr,
					"The  File %s.%s seem to be bad. Can't load", ps.name.c_str(), ps.ext.c_str());
			}
		}
	}
	else
	{
		res = false;
		Messaging::Instance()->AddError(true, nullptr, nullptr, "font %s not found", fontFilePathName.c_str());
		m_NeedFilePathResolve = true;
	}

	vProjectFile->SetProjectChange();

	// stop the thread
	vWorking = false;

	return res;
}

void FontInfos::StartLoadingThread(
	ProjectFile* vProjectFile,
	const std::string& vFontFilePathName,
	std::function<void()> vLoadingThreadFinishFunc)
{
	ZoneScoped;

	StopLoadingThread();

	m_LoadingThreadFinishFunc = vLoadingThreadFinishFunc;
	m_LoadingWorking = true;
	m_LoadingProgress = 0.0f;
	m_LoadingObjectsCount = 0U;
	countThreadInParallel++;
	m_LoadingThread = std::thread(
		&FontInfos::LoadFontThreaded, this,
		vProjectFile,
		vFontFilePathName,
		std::ref(m_LoadingWorking),
		std::ref(m_LoadingProgress),
		std::ref(m_LoadingObjectsCount));
}

void FontInfos::DrawLoadingProgressBar(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (IsJoinable())
	{
		const float ratio = m_LoadingProgress;
		const size_t count = m_LoadingObjectsCount;
		
		static char buffer[100] = "";
		snprintf(buffer, 100, "[%s] Objects (%zu)", m_FontFileName.c_str(), count);
		if (ImGui::Button(ICON_IGFS_ICON_VIEW))
		{
			m_LoadingWorking = false;
		}
		ImGui::SameLine();
		ImGui::ProgressBar(ratio, ImVec2(-1, 0), buffer);
	}
	PendingThreadAction(vProjectFile);
}

bool FontInfos::StopLoadingThread()
{
	ZoneScoped;

	bool res = false;

	res = m_LoadingThread.joinable();
	if (res)
	{
		m_LoadingWorking = false;
		Join();
	}

	return res;
}

bool FontInfos::IsJoinable()
{
	ZoneScoped;

	return m_LoadingThread.joinable();
}

void FontInfos::Join()
{
	ZoneScoped;

	m_LoadingThread.join();
	countThreadInParallel--;
}

void FontInfos::PendingThreadAction(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (m_LoadingThread.joinable()) // need terminate ?
	{
		if (!m_LoadingWorking)
		{
			Join();
			m_WaitingToBeLoaded = false;

			if (m_ImFontAtlas.IsBuilt())
			{
				// we are outside of the thread
				// we can now create the texture
				CreateOrUpdateFontTexture();

				if (m_LoadingThreadFinishFunc)
				{
					m_LoadingThreadFinishFunc();
				}

				m_IsLoaded = true;
			}
			else
			{
				m_IsLoaded = false;
			}
		}
	}
	else if (m_WaitingToBeLoaded) // need loading ?
	{
		if (countThreadInParallel < maxThreadInParallel)
		{
			StartLoadingThread(vProjectFile, m_FontFilePathNameToLoad, m_LoadingThreadFinishFunc);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////
//// FONT TEXTURE ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void FontInfos::Build_Glyph_DataBase()
{
	ZoneScoped;

	m_Glyphs.clear();
	m_GlyphFilteringStats = GlyphFilteringStats();

	if (!m_ImFontAtlas.Fonts.empty())
	{
		bool res = false;

		{
			ZoneScopedN("m_TTFRRW.OpenFontStream");

			res = m_TTFRRW.OpenFontStream(
				(uint8_t*)m_ImFontAtlas.ConfigData[0].FontData,
				(size_t)m_ImFontAtlas.ConfigData[0].FontDataSize,
				TTFRRW::TTFRRW_PROCESSING_FLAG_NONE
				//| TTFRRW::TTFRRW_PROCESSING_FLAG_NO_GLYPH_PARSING
				| TTFRRW::TTFRRW_PROCESSING_FLAG_VERBOSE_PROFILER
				| TTFRRW::TTFRRW_PROCESSING_FLAG_VERBOSE_ONLY_ERRORS
				//| TTFRRW::TTFRRW_PROCESSING_FLAG_NO_ERRORS
				, m_FontFileName.c_str(),
				&m_LoadingWorking,
				&m_LoadingProgress,
				&m_LoadingObjectsCount);
		}

		if (res)
		{
			ImFont* font = m_ImFontAtlas.Fonts[0];
			if (font)
			{
				for (const auto& imGlyph : font->Glyphs)
				{
					ZoneScopedN("Fill Glyphs");

					BaseGlyph glyph;
					glyph.glyphIndex = imGlyph.Codepoint;
					glyph.codePoint = 0U;
					glyph.AdvanceX = imGlyph.AdvanceX;

					glyph.X0 = imGlyph.X0;
					glyph.X1 = imGlyph.X1;
					glyph.Y0 = imGlyph.Y0;
					glyph.Y1 = imGlyph.Y1;

					glyph.U0 = imGlyph.U0;
					glyph.U1 = imGlyph.U1;
					glyph.V0 = imGlyph.V0;
					glyph.V1 = imGlyph.V1;

					glyph.color;

					GlyphCategoryFlags flags = 0;
					auto g = m_TTFRRW.GetGlyphWithGlyphIndex(glyph.glyphIndex);
					if (g)
					{
						//glyph.AdvanceX = g->m_AdvanceX;

						if (g->m_IsSimple)
							flags |= GLYPH_CATEGORY_FLAG_SIMPLE;
						else
							flags |= GLYPH_CATEGORY_FLAG_COMPOSITE;

						if (!g->m_Name.empty())
						{
							glyph.name = g->m_Name;
							flags |= GLYPH_CATEGORY_FLAG_NAMED;
						}
							
						if (g->m_IsLayer)
						{
							flags |= GLYPH_CATEGORY_FLAG_LAYER;
							for (const auto& gc : g->m_Color)
							{
								glyph.color[gc.first] = ImVec4(gc.second.x, gc.second.y, gc.second.z, gc.second.w);
							}
							for (const auto& gpi : g->m_PaletteIndex)
							{
								glyph.paletteIndex[gpi.first] = gpi.second;
							}
							for (const auto& gp : g->m_Parents)
							{
								glyph.parents.emplace(gp);
							}
						}

						glyph.contours = g->m_Contours;
						if (!g->m_Layers.empty())
						{
							flags |= GLYPH_CATEGORY_FLAG_COLORED;
							for (auto layerID : g->m_Layers)
							{
								glyph.layers.push_back(layerID);
							}
						}

						glyph.color[glyph.glyphIndex] = ImVec4(1, 1, 1, 1);

						glyph.bbox.x = g->m_LocalBBox.lowerBound.x;
						glyph.bbox.y = g->m_LocalBBox.lowerBound.y;
						glyph.bbox.z = g->m_LocalBBox.upperBound.x;
						glyph.bbox.w = g->m_LocalBBox.upperBound.y;
					}

					auto arr = m_TTFRRW.GetCodePointsFromGlyphIndex(glyph.glyphIndex);
					if (arr && !arr->empty())
					{
						if (arr->size() == 1)
						{
							flags |= GLYPH_CATEGORY_FLAG_MAPPED;
							glyph.codePoint = *arr->begin();
							m_CodePointToGlyphIndex[glyph.codePoint] = glyph.glyphIndex;
						}
						else
							flags |= GLYPH_CATEGORY_FLAG_UNMAPPED;
					}
					else
						flags |= GLYPH_CATEGORY_FLAG_UNMAPPED;

					m_GlyphFilteringStats.m_CountGlyphs++;
					if (flags & GLYPH_CATEGORY_FLAG_SIMPLE)
						m_GlyphFilteringStats.m_CountSimpleGlyphs++;
					if (flags & GLYPH_CATEGORY_FLAG_COMPOSITE)
						m_GlyphFilteringStats.m_CountCompositeGlyphs++;
					if (flags & GLYPH_CATEGORY_FLAG_COLORED)
						m_GlyphFilteringStats.m_CountColoredGlyphs++;
					if (flags & GLYPH_CATEGORY_FLAG_LAYER)
						m_GlyphFilteringStats.m_CountLayerGlyphs++;
					if (flags & GLYPH_CATEGORY_FLAG_MAPPED)
						m_GlyphFilteringStats.m_CountMappedGlyphs++;
					if (flags & GLYPH_CATEGORY_FLAG_UNMAPPED)
						m_GlyphFilteringStats.m_CountUnMappedGlyphs++;
					if (flags & GLYPH_CATEGORY_FLAG_NAMED)
						m_GlyphFilteringStats.m_CountMamedGlyphs++;

					glyph.category = flags;

					glyph.fontInfos = m_This;

					m_Glyphs[glyph.glyphIndex] = glyph;
				}
			}
		}
	}
}

std::string FontInfos::GetGlyphName(GlyphIndex vGlyphIndex)
{
	ZoneScoped;

	std::string res;
	if (m_Glyphs.find(vGlyphIndex) != m_Glyphs.end())
	{
		res = m_Glyphs[vGlyphIndex].name;
	}
	if (res.empty())
		res = ct::toStr("Symbol_%u", vGlyphIndex);
	return res;
}

void FontInfos::DrawInfos(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (!m_ImFontAtlas.Fonts.empty())
	{
		bool needFontReGen = false;
		bool needTextureReGen = false;

		float aw = 0.0f;

		if (ImGui::BeginFramedGroup("Selected Font"))
		{
			aw = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x;

			if (!m_InfosToDisplay.empty())
			{
				if (ImGui::CollapsingHeader("Infos", ImGuiTreeNodeFlags_Bullet))
				{
					static ImGuiTableFlags flags =
						ImGuiTableFlags_SizingFixedFit |
						ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable |
						ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
						ImGuiTableFlags_NoHostExtendY | ImGuiTableFlags_Borders;
					if (ImGui::BeginTable("##fontinfosTable", 2, flags, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetTextLineHeightWithSpacing() * 7)))
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
			}
			
			ImGui::FramedGroupSeparator();

			ImGui::Text("Selected glyphs : %u", (uint32_t)m_SelectedGlyphs.size());

			ImGui::FramedGroupText("Clear for all Glyphs");

			aw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) * 0.3333f;

			if (ImGui::Button("Translations", ImVec2(aw,0)))
			{
				ClearTranslations(vProjectFile);
			}

			ImGui::SameLine();

			if (ImGui::Button("Scales", ImVec2(aw, 0)))
			{
				ClearScales(vProjectFile);
			}

			ImGui::SameLine();

			if (ImGui::Button("Both", ImVec2(aw, 0)))
			{
				ClearTransforms(vProjectFile);
			}

			ImGui::FramedGroupSeparator();

			aw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

			ImGui::PushItemWidth(aw);

			/*if (ImGui::RadioButtonLabeled("FreeType", "Use FreeType Raterizer", FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE))
			{
				needFontReGen = true;
				FontInfos::rasterizerMode = RasterizerEnum::RASTERIZER_FREETYPE;
			}*/

#ifdef _DEBUG
			if (ImGui::RadioButtonLabeled(0.0f, "Linear", "Use Linear Texture Filtering", textureFiltering == GL_LINEAR))
			{
				needTextureReGen = true;
				textureFiltering = GL_LINEAR;
			}

			ImGui::SameLine();

			if (ImGui::RadioButtonLabeled(0.0f, "Nearest", "Use Nearest Texture Filtering", textureFiltering == GL_NEAREST))
			{
				needTextureReGen = true;
				textureFiltering = GL_NEAREST;
			}
#endif
			ImGui::PopItemWidth();
			
			ImGui::FramedGroupSeparator();

			needFontReGen |= ImGui::SliderIntDefaultCompact(-1.0f, "Font Size", &m_FontSize, 7, 50, defaultFontInfosValues.m_FontSize);
			
			if (FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE)
			{
				needFontReGen |= ImGui::SliderFloatDefaultCompact(-1.0f, "Multiply", &fontMultiply, 0.0f, 2.0f, 1.0f);
			}
			else if (FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_TTFRRW)
			{
				FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE; // pour les anciens fichiers
			}

			needFontReGen |= ImGui::SliderIntDefaultCompact(-1.0f, "Padding", &fontPadding, 0, 16, 1);

			if (FontInfos::rasterizerMode == RasterizerEnum::RASTERIZER_FREETYPE)
			{
				if (ImGui::CollapsingHeader("Freetype Settings", ImGuiTreeNodeFlags_Bullet))
				{
					needFontReGen |= ImGui::CheckboxFlags("NoHinting", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_NoHinting);
					needFontReGen |= ImGui::CheckboxFlags("NoAutoHint", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_NoAutoHint);
					needFontReGen |= ImGui::CheckboxFlags("ForceAutoHint", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_ForceAutoHint);
					needFontReGen |= ImGui::CheckboxFlags("LightHinting", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_LightHinting);
					needFontReGen |= ImGui::CheckboxFlags("MonoHinting", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_MonoHinting);
					needFontReGen |= ImGui::CheckboxFlags("Bold", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_Bold);
					needFontReGen |= ImGui::CheckboxFlags("Oblique", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_Oblique);
					needFontReGen |= ImGui::CheckboxFlags("Monochrome", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_Monochrome);
					needFontReGen |= ImGui::CheckboxFlags("LoadColor", &freeTypeFlag, ImGuiFreeType_unleashed::FreeType_LoadColor);
				}
			}

			ImGui::EndFramedGroup(true);
		}

		if (needFontReGen)
		{
			m_FontSize = ct::clamp(m_FontSize, 7, 50);
			m_Oversample = ct::clamp(m_Oversample, 1, 5);
			ParamsPane::Instance()->OpenFont(vProjectFile, m_FontFilePathName, false);
			vProjectFile->SetProjectChange();
		}

		if (needTextureReGen)
		{
			CreateOrUpdateFontTexture();
			vProjectFile->SetProjectChange();
		}
	}
}

void FontInfos::DrawFilteringWidgets(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (ImGui::BeginFramedGroup("Filtering"))
	{
		bool change = false;

		float mrw2 = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 2.0f) * 0.5f;

		ImGui::FramedGroupText("Filtereds (%u)", m_GlyphFilteringStats.m_CountFilteredGlyphs);

		if (ImGui::RadioButtonLabeled(mrw2, "OR", "OR", m_GlyphFilteringOpOR == true))
		{
			change = true;
			m_GlyphFilteringOpOR = !m_GlyphFilteringOpOR;
		}
		ImGui::SameLine();
		if (ImGui::RadioButtonLabeled(mrw2, "AND", "AND", m_GlyphFilteringOpOR == false))
		{
			change = true;
			m_GlyphFilteringOpOR = !m_GlyphFilteringOpOR;
		}
		
		const bool useColorFiltering = true;// !((m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_ALL) == GLYPH_CATEGORY_FLAG_ALL);
		float mrw = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x;
		if (useColorFiltering)
			mrw -= ImGui::GetStyle().ItemSpacing.x + ImGui::GetFrameHeight();
		if (ImGui::RadioButtonLabeled(mrw, "All", "All", (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_ALL) == GLYPH_CATEGORY_FLAG_ALL))
		{
			change = true;
			if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_ALL)
				m_GlyphDisplayCategoryFlags = GLYPH_CATEGORY_FLAG_NONE;
			else
				m_GlyphDisplayCategoryFlags = GLYPH_CATEGORY_FLAG_ALL;
		}
		if (useColorFiltering)
		{
			ImGui::SameLine();
			ImGui::Checkbox("##UseColor", &m_GlyphFilteringStats.m_UseFilterColoring);
			if (ImGui::IsItemHovered())
			{
				ImGui::SetTooltip("Use Color for Filtering");
			}
		}

		static ImGuiColorEditFlags colorFlags =
			ImGuiColorEditFlags_Float |
			ImGuiColorEditFlags_NoInputs |
			ImGuiColorEditFlags_NoTooltip |
			ImGuiColorEditFlags_NoLabel;

		change |= ImGui::RadioButtonLabeled_BitWize<GlyphCategoryFlags>(mrw, ct::toCStr("Simple (%u)", m_GlyphFilteringStats.m_CountSimpleGlyphs),
			"Simple", &m_GlyphDisplayCategoryFlags, GLYPH_CATEGORY_FLAG_SIMPLE);
		if (useColorFiltering)
		{
			ImGui::SameLine();
			ImGui::ColorEdit4("Simple", &m_GlyphFilteringStats.SimpleColor.x, colorFlags);
		}
		change |= ImGui::RadioButtonLabeled_BitWize<GlyphCategoryFlags>(mrw, ct::toCStr("Composite (%u)", m_GlyphFilteringStats.m_CountCompositeGlyphs),
			"Composite", &m_GlyphDisplayCategoryFlags, GLYPH_CATEGORY_FLAG_COMPOSITE);
		if (useColorFiltering)
		{
			ImGui::SameLine();
			ImGui::ColorEdit4("Composite", &m_GlyphFilteringStats.CompositeColor.x, colorFlags);
		}
		change |= ImGui::RadioButtonLabeled_BitWize<GlyphCategoryFlags>(mrw, ct::toCStr("Mapped (%u)", m_GlyphFilteringStats.m_CountMappedGlyphs),
			"Mapped", &m_GlyphDisplayCategoryFlags, GLYPH_CATEGORY_FLAG_MAPPED);
		if (useColorFiltering)
		{
			ImGui::SameLine();
			ImGui::ColorEdit4("Mapped", &m_GlyphFilteringStats.MappedColor.x, colorFlags);
		}
		change |= ImGui::RadioButtonLabeled_BitWize<GlyphCategoryFlags>(mrw, ct::toCStr("Un Mapped (%u)", m_GlyphFilteringStats.m_CountUnMappedGlyphs),
			"Un Mapped", &m_GlyphDisplayCategoryFlags, GLYPH_CATEGORY_FLAG_UNMAPPED);
		if (useColorFiltering)
		{
			ImGui::SameLine();
			ImGui::ColorEdit4("Un Mapped", &m_GlyphFilteringStats.UnMappedColor.x, colorFlags);
		}
		change |= ImGui::RadioButtonLabeled_BitWize<GlyphCategoryFlags>(mrw, ct::toCStr("Colored (%u)", m_GlyphFilteringStats.m_CountColoredGlyphs),
			"Colored", &m_GlyphDisplayCategoryFlags, GLYPH_CATEGORY_FLAG_COLORED);
		if (useColorFiltering)
		{
			ImGui::SameLine();
			ImGui::ColorEdit4("Colored", &m_GlyphFilteringStats.ColoredColor.x, colorFlags);
		}
		change |= ImGui::RadioButtonLabeled_BitWize<GlyphCategoryFlags>(mrw, ct::toCStr("Layer (%u)", m_GlyphFilteringStats.m_CountLayerGlyphs),
			"Layer", &m_GlyphDisplayCategoryFlags, GLYPH_CATEGORY_FLAG_LAYER);
		if (useColorFiltering)
		{
			ImGui::SameLine();
			ImGui::ColorEdit4("Layer", &m_GlyphFilteringStats.LayerColor.x, colorFlags);
		}
		change |= ImGui::RadioButtonLabeled_BitWize<GlyphCategoryFlags>(mrw, ct::toCStr("Named (%u)", m_GlyphFilteringStats.m_CountMamedGlyphs),
			"Named", &m_GlyphDisplayCategoryFlags, GLYPH_CATEGORY_FLAG_NAMED);
		if (useColorFiltering)
		{
			ImGui::SameLine();
			ImGui::ColorEdit4("Named", &m_GlyphFilteringStats.NamedColor.x, colorFlags);
		}

		if (change)
		{
			UpdateFiltering();
			vProjectFile->SetProjectChange();
		}

		ImGui::EndFramedGroup(true);
	}
}

void FontInfos::UpdateInfos()
{
	ZoneScoped;

	if (m_TTFRRW.IsValidFotGlyppTreatment())
	{
		auto infos = m_TTFRRW.GetFontInfos();
		m_Ascent = infos.m_Ascent;
		m_Descent = infos.m_Descent;
		m_LineGap = infos.m_LineGap;
		m_BoundingBox.x = infos.m_GlobalBBox.lowerBound.x;
		m_BoundingBox.y = infos.m_GlobalBBox.lowerBound.y;
		m_BoundingBox.z = infos.m_GlobalBBox.upperBound.x;
		m_BoundingBox.w = infos.m_GlobalBBox.upperBound.y;
		//m_Point = (m_Ascent - m_Descent) / (float)m_FontSize;
		m_Point = (m_BoundingBox.w - m_BoundingBox.y) / (float)m_FontSize;
		m_FontDiffScale = (float)(m_BoundingBox.w - m_BoundingBox.y) / (float)(m_Ascent - m_Descent);
		m_AscentDiffScale = (float)m_Ascent / (float)m_BoundingBox.w;

		//--------------------------------------------------------

		m_InfosToDisplay.clear();
		m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Font", m_FontFilePathName));
		m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Glyph Count", ct::toStr("%u", infos.m_GlyphCount)));
		m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Ascent / Descent / Line gap :", ct::toStr("%i / %i / %i", m_Ascent, m_Descent, m_LineGap)));
		m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Glyph BBox :", ct::toStr("min : %i x %i/max : %i x %i", m_BoundingBox.x, m_BoundingBox.y, m_BoundingBox.z, m_BoundingBox.w)));
		m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Size (px) :", ct::toStr("%.2f", m_FontSize)));
		m_InfosToDisplay.push_back(std::pair<std::string, std::string>("H Scale (px) :", ct::toStr("%.4f", m_Point))); // same.., its used internally by ImGui but dont know what is it
		m_InfosToDisplay.push_back(std::pair<std::string, std::string>("Texture Size :", ct::toStr("%i x %i", m_ImFontAtlas.TexWidth, m_ImFontAtlas.TexHeight)));
	}
}

void FontInfos::UpdateSelectedGlyphs()
{
	ZoneScoped;

	// update glyph ptrs
	for (const auto& selGlyph : m_SelectedGlyphs)
	{
		auto baseGlyph = GetGlyphByGlyphIndex(selGlyph.first);
		if (baseGlyph)
		{
			if (selGlyph.second)
			{
				selGlyph.second->glyph = *baseGlyph;
			}
		}
	}
}

void FontInfos::UpdateFiltering()
{
	ZoneScoped;

	std::set<GlyphIndex> preFilter;

	int countNeeded = 0;
	if (m_GlyphFilteringOpOR == false)
	{
		if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_SIMPLE)
			countNeeded++;
		if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_COMPOSITE)
			countNeeded++;
		if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_COLORED)
			countNeeded++;
		if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_LAYER)
			countNeeded++;
		if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_MAPPED)
			countNeeded++;
		if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_UNMAPPED)
			countNeeded++;
		if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_NAMED)
			countNeeded++;
	}
	for (const auto& glyph : m_Glyphs)
	{
		int countFound = 0;
		if (glyph.second.category & GLYPH_CATEGORY_FLAG_SIMPLE)
		{
			if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_SIMPLE)
				countFound++;
		}
		if (glyph.second.category & GLYPH_CATEGORY_FLAG_COMPOSITE)
		{
			if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_COMPOSITE)
				countFound++;
		}
		if (glyph.second.category & GLYPH_CATEGORY_FLAG_COLORED)
		{
			if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_COLORED)
				countFound++;
		}
		if (glyph.second.category & GLYPH_CATEGORY_FLAG_LAYER)
		{
			if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_LAYER)
				countFound++;
		}
		if (glyph.second.category & GLYPH_CATEGORY_FLAG_MAPPED)
		{
			if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_MAPPED)
				countFound++;
		}
		if (glyph.second.category & GLYPH_CATEGORY_FLAG_UNMAPPED)
		{
			if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_UNMAPPED)
				countFound++;
		}
		if (glyph.second.category & GLYPH_CATEGORY_FLAG_NAMED)
		{
			if (m_GlyphDisplayCategoryFlags & GLYPH_CATEGORY_FLAG_NAMED)
				countFound++;
		}
		
		bool found = false;

		if (m_GlyphFilteringOpOR)
		{
			if (countFound)
				found = true;
		}
		else // and
		{
			if (countFound == countNeeded)
				found = true;
		}

		if (found)
		{
			if (!m_Filters.empty())
			{
				if (!glyph.second.name.empty())
				{
					for (const auto& it : m_Filters)
					{
						if (glyph.second.name.find(it) != std::string::npos) // found
						{
							preFilter.emplace(glyph.first);
						}
					}
				}
			}
			else
			{
				preFilter.emplace(glyph.first);
			}
		}
	}

	m_FilteredGlyphs.clear();
	for (auto filter : preFilter)
	{
		m_FilteredGlyphs.push_back(filter);
	}

	m_GlyphFilteringStats.m_CountFilteredGlyphs = (uint32_t)m_FilteredGlyphs.size();
}

void FontInfos::ClearTransforms(ProjectFile* vProjectFile)
{
	ZoneScoped;

	ClearTranslations(vProjectFile);
	ClearScales(vProjectFile);
}

void FontInfos::ClearScales(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (vProjectFile)
	{
		for (auto glyph : m_SelectedGlyphs)
		{
			if (glyph.second)
			{
				glyph.second->m_Scale = 1.0f;
				glyph.second->simpleGlyph.ClearScale();
			}
		}

		vProjectFile->SetProjectChange();
	}
}

void FontInfos::ClearTranslations(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (vProjectFile)
	{
		for (auto glyph : m_SelectedGlyphs)
		{
			if (glyph.second)
			{
				glyph.second->m_Translation = 0.0f;
				glyph.second->simpleGlyph.ClearTranslation();
			}
		}

		vProjectFile->SetProjectChange();
	}
}

BaseGlyph* FontInfos::GetGlyphByGlyphIndex(GlyphIndex vGlyphIndex)
{
	ZoneScoped;

	if (m_Glyphs.find(vGlyphIndex) != m_Glyphs.end())
	{
		return &m_Glyphs[vGlyphIndex];
	}

	return nullptr;
}

BaseGlyph* FontInfos::GetFirstGlyphByCodePoint(CodePoint vCodePoint)
{
	ZoneScoped;

	if (m_CodePointToGlyphIndex.find(vCodePoint) != m_CodePointToGlyphIndex.end()) // found
	{
		auto glyphIndex = m_CodePointToGlyphIndex[vCodePoint];
		return GetGlyphByGlyphIndex(glyphIndex);
	}

	return nullptr;
}

ImFont* FontInfos::GetImFontPtr()
{
	ZoneScoped;

	if (!m_ImFontAtlas.Fonts.empty())
	{
		return m_ImFontAtlas.Fonts[0];
	}
	return nullptr;
}

bool FontInfos::IsUsable()
{
	return !(IsJoinable() || m_WaitingToBeLoaded);
}

//////////////////////////////////////////////////////////////////////////////
//// FONT TEXTURE ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void FontInfos::CreateOrUpdateFontTexture()
{
	ZoneScoped;
	TracyGpuZone("FontInfos CreateFontTexture");

	if (!m_ImFontAtlas.Fonts.empty())
	{
		DestroyFontTexture();

		unsigned char* pixels;
		int width, height;
		m_ImFontAtlas.GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

		GLuint id = 0;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, textureFiltering);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, textureFiltering);
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
	ZoneScoped;
	TracyGpuZone("FontInfos DestroyFontTexture");

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
	ZoneScoped;

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
				res += vOffset + "\t\t<glyph orgId=\"" + ct::toStr(it.second->glyph.glyphIndex) +
					"\" newId=\"" + ct::toStr(it.second->newCodePoint) +
					"\" orgName=\"" + it.second->glyph.name +
					"\" newName=\"" + it.second->newHeaderName +
					"\" trans=\"" + ct::fvec2(it.second->m_Translation).string() + 
					"\" scale=\"" + ct::fvec2(it.second->m_Scale).string() +
					"\"/>\n";
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
	res += vOffset + "\t<texturefiltering>" + ct::toStr(textureFiltering) + "</texturefiltering>\n";

	res += vOffset + "\t<glyphfiltering>\n";
	res += vOffset + "\t<flags>" + ct::toStr(m_GlyphDisplayCategoryFlags) + "</flags>\n";
	res += vOffset + "\t<op>" + (m_GlyphFilteringOpOR ? "true" : "false") + "</op>\n";
	res += vOffset + "\t<use_color>" + (m_GlyphFilteringStats.m_UseFilterColoring ? "true" : "false") + "</use_color>\n";
	res += vOffset + "\t<simple_color>" + ct::toStr(ImGui::GetColorU32(m_GlyphFilteringStats.SimpleColor)) + "</simple_color>\n";
	res += vOffset + "\t<composite_color>" + ct::toStr(ImGui::GetColorU32(m_GlyphFilteringStats.CompositeColor)) + "</composite_color>\n";
	res += vOffset + "\t<mapped_color>" + ct::toStr(ImGui::GetColorU32(m_GlyphFilteringStats.MappedColor)) + "</mapped_color>\n";
	res += vOffset + "\t<unmapped_color>" + ct::toStr(ImGui::GetColorU32(m_GlyphFilteringStats.UnMappedColor)) + "</unmapped_color>\n";
	res += vOffset + "\t<colored_color>" + ct::toStr(ImGui::GetColorU32(m_GlyphFilteringStats.ColoredColor)) + "</colored_color>\n";
	res += vOffset + "\t<layer_color>" + ct::toStr(ImGui::GetColorU32(m_GlyphFilteringStats.LayerColor)) + "</layer_color>\n";
	res += vOffset + "\t<named_color>" + ct::toStr(ImGui::GetColorU32(m_GlyphFilteringStats.NamedColor)) + "</named_color>\n";
	res += vOffset + "\t</glyphfiltering>\n";

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
	ZoneScoped;

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
		else if (strName == "texturefiltering")
			textureFiltering = (GLenum)ct::ivariant(strValue).GetI();
		else if (strName == "glyphs" || strName == "filters")
		{
			for (tinyxml2::XMLElement* child = vElem->FirstChildElement(); child != nullptr; child = child->NextSiblingElement())
			{
				RecursParsingConfig(child->ToElement(), vElem);
			}
		}
	}
	else if (strParentName == "glyphfiltering")
	{
		if (strName == "flags")
			m_GlyphDisplayCategoryFlags = (GlyphCategoryFlags)ct::ivariant(strValue).GetI();
		else if (strName == "op")
			m_GlyphFilteringOpOR = ct::ivariant(strValue).GetB();
		else if (strName == "use_color")
			m_GlyphFilteringStats.m_UseFilterColoring = ct::ivariant(strValue).GetB();
		else if (strName == "simple_color")
			m_GlyphFilteringStats.SimpleColor = ImGui::ColorConvertU32ToFloat4((ImU32)ct::ivariant(strValue).GetI());
		else if (strName == "composite_color")
			m_GlyphFilteringStats.CompositeColor = ImGui::ColorConvertU32ToFloat4((ImU32)ct::ivariant(strValue).GetI());
		else if (strName == "mapped_color")
			m_GlyphFilteringStats.MappedColor = ImGui::ColorConvertU32ToFloat4((ImU32)ct::ivariant(strValue).GetI());
		else if (strName == "unmapped_color")
			m_GlyphFilteringStats.UnMappedColor = ImGui::ColorConvertU32ToFloat4((ImU32)ct::ivariant(strValue).GetI());
		else if (strName == "colored_color")
			m_GlyphFilteringStats.ColoredColor = ImGui::ColorConvertU32ToFloat4((ImU32)ct::ivariant(strValue).GetI());
		else if (strName == "layer_color")
			m_GlyphFilteringStats.LayerColor = ImGui::ColorConvertU32ToFloat4((ImU32)ct::ivariant(strValue).GetI());
		else if (strName == "named_color")
			m_GlyphFilteringStats.NamedColor = ImGui::ColorConvertU32ToFloat4((ImU32)ct::ivariant(strValue).GetI());
	}
	else if (strParentName == "glyphs" &&  strName == "glyph")
	{
		uint32_t oldGlyphIndex = 0;
        uint32_t newcodepoint = 0;
		std::string oldName;
		std::string newName;
		ImVec2 translation;
		ImVec2 scale;

		for (const tinyxml2::XMLAttribute* attr = vElem->FirstAttribute(); attr != nullptr; attr = attr->Next())
		{
			std::string attName = attr->Name();
			std::string attValue = attr->Value();

			if (attName == "orgId" ||
				attName == "id") // for compatibility with first format, will be removed in few versions
				oldGlyphIndex = (uint32_t)ct::ivariant(attValue).GetI();
			else if (attName == "newId" ||
				attName == "nid")  // for compatibility with first format, will be removed in few versions
				newcodepoint = (uint32_t)ct::ivariant(attValue).GetI();
			else if (attName == "orgName") oldName = attValue;
			else if (attName == "newName" ||
				attName == "name")  // for compatibility with first format, will be removed in few versions
				newName = attValue;
			else if (attName == "trans")
				translation = ct::toImVec2(ct::fvariant(attValue).GetV2());
			else if (attName == "scale")
				scale = ct::toImVec2(ct::fvariant(attValue).GetV2());
		}

		BaseGlyph baseGlyph = {};
		baseGlyph.glyphIndex = oldGlyphIndex;
		m_SelectedGlyphs[oldGlyphIndex] = GlyphInfos::Create(m_This, baseGlyph, oldName, newName, newcodepoint, translation, scale);
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

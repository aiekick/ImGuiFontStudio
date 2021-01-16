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

#include "FontPreviewPane.h"

#include <Helper/FontHelper.h>
#include <MainFrame.h>
#include <Panes/Manager/LayoutManager.h>
#include <Gui/ImGuiWidgets.h>
#ifdef _DEBUG
#include <Panes/DebugPane.h>
#endif

#include <Helper/FontHelper.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <sfntly/font_factory.h>
#include <Gui/ImGuiWidgets.h>
#include <Helper/SelectionHelper.h>

FontPreviewPane::FontPreviewPane() = default;
FontPreviewPane::~FontPreviewPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FontPreviewPane::Init()
{
	
}

void FontPreviewPane::Unit()
{

}

int FontPreviewPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	paneWidgetId = vWidgetId;

	DrawFontPreviewPane(vProjectFile);

	return paneWidgetId;
}

void FontPreviewPane::DrawDialogsAndPopups(ProjectFile* /*vProjectFile*/)
{

}

int FontPreviewPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	UNUSED(vProjectFile);
	UNUSED(vUserDatas);

	return vWidgetId;
}

/*
la font des glyoh n'aura pas la meme BBox que la font text avec laquelle elle devra etre affichée
donc il faudrait voir le resultat et ajuster ci-besoin, donc on doit :
- charger une font texte de test
- composer un texte avec cette font
- slectionner les glyph a voir, positionner dans le text
*/

/*
on va taper un texte.
can va nous afficher les box qui vont contenir les characteres
et on va pouvoir choisir un glyph dans ces boites, ce qui nous affichera en dessous le resultat
et il faudra pouvoir scale/translate le glyph
*/

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

static uint32_t _TextCursorPos = 0;
static int InputTextCallback(ImGuiInputTextCallbackData* vData)
{
	if (vData)
	{
		_TextCursorPos = vData->CursorPos;
	}
	
	return 0;
}

void FontPreviewPane::DrawFontPreviewPane(ProjectFile *vProjectFile)
{
	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_FONT_PREVIEW)
	{
		if (ImGui::Begin<PaneFlags>(FONT_PREVIEW_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_FONT_PREVIEW,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				if (ImGui::BeginMenuBar())
				{
					ImGui::EndMenuBar();
				}

				ImGui::Text("Current Selection");

				bool change = false;
				ImVec2 cell_size, glyph_size;
				uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);
				if (glyphCountX)
				{
					auto sel = SelectionHelper::Instance()->GetSelection();
					size_t idx = 0;
					for (auto glyph : *sel)
					{
						if (glyph.second)
						{
							uint32_t x = idx % glyphCountX;

							if (x) ImGui::SameLine();

							if (glyph.second->m_SelectedGlyphs.find(glyph.first) != glyph.second->m_SelectedGlyphs.end())
							{
								auto glyphInfos = glyph.second->m_SelectedGlyphs[glyph.first];

								ImVec2 hostTextureSize = ImVec2(
									(float)glyph.second->m_ImFontAtlas.TexWidth,
									(float)glyph.second->m_ImFontAtlas.TexHeight);

								ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
								bool check = GlyphInfos::DrawGlyphButton(vProjectFile, glyph.second,
									0, glyph_size, glyphInfos.glyph, hostTextureSize);
								ImGui::PopStyleVar();

								if (check)
								{
									m_GlyphToInsert[_TextCursorPos] = glyph;
									change = true;
								}
							}

							idx++;
						}
					}
				}

				ImGui::Separator();
				
				static char buffer[500] = "test\0";

				ImGui::Text("Text :");
				float aw = ImGui::GetContentRegionAvail().x;
				ImGui::PushItemWidth(aw);
				ImGui::PushID(paneWidgetId++);
				change |= ImGui::InputText("##ImGuiFontStudio", buffer, 499,
					ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackCharFilter,
					&InputTextCallback);
				ImGui::PopID();
				ImGui::PopItemWidth();

				if (change)
				{
					m_TestSentense = buffer;
				}

				DrawMixedFontResult(vProjectFile);
			}
		}

		ImGui::End();
	}
}

void FontPreviewPane::DrawMixedFontResult(ProjectFile* vProjectFile)
{
	ImGui::Text("%s", m_TestSentense.c_str());

	// ici il va falloir afficher les glyphs au bon endroit dans la string m_TestSentense

	uint32_t idx = 0;
	for (auto c : m_TestSentense)
	{
		if (idx)
		{
			ImGui::SameLine();
		}

		if (m_GlyphToInsert.find(idx) != m_GlyphToInsert.end())
		{
			// on dessin le glyph
			auto glyph = &m_GlyphToInsert[idx];
			if (glyph->second)
			{
				if (glyph->second->m_SelectedGlyphs.find(glyph->first) != glyph->second->m_SelectedGlyphs.end())
				{
					ImVec2 cell_size, glyph_size;
					GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);

					auto glyphInfos = glyph->second->m_SelectedGlyphs[glyph->first];

					ImVec2 hostTextureSize = ImVec2(
						(float)glyph->second->m_ImFontAtlas.TexWidth,
						(float)glyph->second->m_ImFontAtlas.TexHeight);

					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
					GlyphInfos::DrawGlyphButton(vProjectFile, glyph->second,
						0, glyph_size, glyphInfos.glyph, hostTextureSize);
					ImGui::PopStyleVar();
				}
			}
		}
		else
		{
			// on dessine la lettre
			ImGui::Text("%c", c);
		}

		idx++;
	}
}
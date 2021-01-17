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
#include <Panes/ParamsPane.h>
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
//// STATIC ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

static char buffer[500] = "ImGuiFontStudio\0";
static uint32_t _TextCursorPos = 0;
static int InputTextCallback(ImGuiInputTextCallbackData * vData)
{
	if (vData)
	{
		_TextCursorPos = vData->CursorPos;
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FontPreviewPane::Init()
{
	m_TestSentense = buffer;
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

								ImGui::PushID(paneWidgetId++);
								ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
								bool check = GlyphInfos::DrawGlyphButton(vProjectFile, glyph.second,
									0, glyph_size, glyphInfos.glyph, hostTextureSize);
								ImGui::PopStyleVar();
								ImGui::PopID();

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
				
				if (ImGui::Button("Clear##glyphselection"))
				{
					m_GlyphToInsert.clear();
				}
				
				ImGui::Text("Test :");
				if (ImGui::Button("Clear##testfont"))
				{
					m_TestFont.reset();
				}
				ImGui::SameLine();
				if (ImGui::Button("Use the selected font"))
				{
					m_TestFont = vProjectFile->m_SelectedFont;
				}
				
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
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

				DrawMixerWidget(vProjectFile);

				DrawMixedFontResult(vProjectFile);
			}
		}

		ImGui::End();
	}
}

void FontPreviewPane::DrawMixerWidget(ProjectFile* vProjectFile)
{
	ImFont* font = ImGui::GetFont();
	if (!m_TestFont.expired())
	{
		auto fontPtr = m_TestFont.lock();
		if (fontPtr)
		{
			font = fontPtr->m_ImFontAtlas.Fonts[0];
		}
	}

	if (font)
	{
		ImVec2 cell_size, glyph_size;
		uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);

		uint32_t idx = 0;
		for (auto c : m_TestSentense)
		{
			bool used = false;
			if (m_GlyphToInsert.find(idx) != m_GlyphToInsert.end())
			{
				// on dessin le glyph
				auto glyphInfos = &m_GlyphToInsert[idx];
				if (glyphInfos->second)
				{
					if (glyphInfos->second->m_SelectedGlyphs.find(glyphInfos->first) != glyphInfos->second->m_SelectedGlyphs.end())
					{
						auto glyph = glyphInfos->second->m_SelectedGlyphs[glyphInfos->first].glyph;

						if (idx)
						{
							ImGui::SameLine();
						}

						bool selected = false;
						auto glyphFont = glyphInfos->second->m_ImFontAtlas.Fonts[0];
						ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
						DrawGlyphButton(vProjectFile, glyphFont, glyphFont->FontSize, &selected, glyph_size, glyph);
						ImGui::PopStyleVar();

						used = true;
					}
				}
			}

			auto glyph = font->FindGlyph(c);
			if (glyph)
			{
				if (idx || used)
				{
					ImGui::SameLine();
				}

				bool selected = false;
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
				DrawGlyphButton(vProjectFile, font, font->FontSize, &selected, glyph_size, *glyph);
				ImGui::PopStyleVar();
			}

			idx++;
		}
	}
}

void FontPreviewPane::DrawMixedFontResult(ProjectFile* vProjectFile)
{
	ImFont* font = ImGui::GetFont();
	
	if (!m_TestFont.expired())
	{
		auto fontPtr = m_TestFont.lock();
		if (fontPtr)
		{
			font = fontPtr->m_ImFontAtlas.Fonts[0];
		}
	}

	if (font)
	{
		static bool _showBaseLine = true;
		ImGui::RadioButtonLabeled("Base Line", "Show/Hide base line", &_showBaseLine);
		ImGui::SameLine();
		ImGui::SliderFloatDefaultCompact(ImGui::GetContentRegionAvail().x, "Preview Size", &m_FontSizePreview, 1, 300, font->FontSize);

		float testFontScale = m_FontSizePreview / font->FontSize;
		float testFontAscent = font->Ascent * testFontScale;

		// ici il va falloir afficher les glyphs au bon endroit dans la string m_TestSentense
		float offsetX = 0.0f;

		float aw = ImGui::GetContentRegionAvail().x;
		ImU32 colFont = ImGui::GetColorU32(ImGuiCol_Text);

		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		const ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		ImGuiID id = window->GetID("#MixedFontDisplay");

		float baseFontRatioX = 1.0f;
		if (font->ContainerAtlas->TexHeight > 0)
			baseFontRatioX = (float)font->ContainerAtlas->TexWidth / (float)font->ContainerAtlas->TexHeight;

		ImVec2 pos = window->DC.CursorPos;
		ImVec2 size = ImVec2(aw, m_FontSizePreview);
		const ImRect bb(pos, pos + size);
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, id))
			return;

		uint32_t idx = 0;
		for (auto c : m_TestSentense)
		{
			if (m_GlyphToInsert.find(idx) != m_GlyphToInsert.end())
			{
				// on dessin le glyph
				auto glyphInfos = &m_GlyphToInsert[idx];
				if (glyphInfos->second)
				{
					if (glyphInfos->second->m_SelectedGlyphs.find(glyphInfos->first) != glyphInfos->second->m_SelectedGlyphs.end())
					{
						ImFont* glyphFont = glyphInfos->second->m_ImFontAtlas.Fonts[0];
						if (glyphFont)
						{
							float scale = (font->FontSize / glyphInfos->second->m_FontSize) * (m_FontSizePreview / font->FontSize);
							float glyphFontAscent = glyphFont->Ascent * scale;
							float ascOffset = glyphFontAscent - testFontAscent;
							
							auto glyph = glyphInfos->second->m_SelectedGlyphs[glyphInfos->first].glyph;
							ImVec2 pMin = ImVec2(pos.x + offsetX, pos.y - ascOffset);
							ImTextureID texId = (ImTextureID)glyphFont->ContainerAtlas->TexID;
							window->DrawList->PushTextureID(texId);
							glyphFont->RenderChar(window->DrawList, m_FontSizePreview, pMin, colFont, (ImWchar)glyphInfos->first);
							window->DrawList->PopTextureID();
							offsetX += glyph.AdvanceX * scale;
						}
					}
				}
			}

			auto glyph = font->FindGlyph(c);
			if (glyph)
			{
				ImVec2 pMin = ImVec2(pos.x + offsetX, pos.y);
				ImTextureID texId = (ImTextureID)font->ContainerAtlas->TexID;
				window->DrawList->PushTextureID(texId);
				font->RenderChar(window->DrawList, m_FontSizePreview, pMin, colFont, (ImWchar)c);
				window->DrawList->PopTextureID();
				offsetX += glyph->AdvanceX * testFontScale;
			}
			
			if (_showBaseLine)
			{
				// Base Line
				float asc = font->Ascent * testFontScale;
				window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + asc), ImVec2(bb.Max.x, bb.Min.y + asc), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 1.0f); // base line
			}

			idx++;
		}
	}
}

bool FontPreviewPane::DrawGlyphButton(
	ProjectFile* vProjectFile, ImFont *vFont, float vFontSize,
	bool* vSelected, ImVec2 vGlyphSize, ImFontGlyph vGlyph)
{
	bool res = false;

	if (vProjectFile && vFont && vGlyphSize.y > 0.0f && vGlyphSize.x > 0.0f)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImGui::PushID((void*)(intptr_t)vFont->ContainerAtlas->TexID);
		const ImGuiID id = window->GetID("#image");
		ImGui::PopID();

		ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vGlyphSize + style.FramePadding * 2);
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		if (pressed && vSelected)
			*vSelected = !*vSelected;

		res = pressed;

		// Render
		const ImU32 col = ImGui::GetColorU32(((held && hovered) || (vSelected && *vSelected)) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderNavHighlight(bb, id);
		float rounding = ImClamp((float)ImMin(style.FramePadding.x, style.FramePadding.y), 0.0f, style.FrameRounding);
		ImGui::RenderFrame(bb.Min, bb.Max, col, true, rounding);

		ImGui::PushClipRect(bb.Min, bb.Max, true);

		bb.Min += style.FramePadding;
		bb.Max -= style.FramePadding;
		
		ImVec2 pScale = vGlyphSize / vFontSize;
		float adv = vGlyph.AdvanceX * pScale.y;
		float offsetX = bb.GetSize().x * 0.5f - adv * 0.5;
		/*auto cf = ImGui::GetFont();
		if (cf)
		{
			float fAsc = vFont->Ascent * pScale.y;
			float cfAsc = cf->Ascent * vGlyphSize.y / cf->FontSize;
			//startPos.y += cfAsc - fAsc;
		}*/
		
		if (vProjectFile->m_ShowBaseLine)// draw base line
		{
			float asc = vFont->Ascent * pScale.y;
			window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + asc), ImVec2(bb.Max.x, bb.Min.y + asc), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 2.0f); // base line
		}

		if (vProjectFile->m_ShowOriginX) // draw origin x
		{
			window->DrawList->AddLine(ImVec2(bb.Min.x + offsetX, bb.Min.y), ImVec2(bb.Min.x + offsetX, bb.Max.y), ImGui::GetColorU32(ImGuiCol_PlotLinesHovered), 2.0f); // base line
		}

		if (vProjectFile->m_ShowAdvanceX) // draw advance X
		{
			float adv = vGlyph.AdvanceX * pScale.y + offsetX;
			window->DrawList->AddLine(ImVec2(bb.Min.x + adv, bb.Min.y), ImVec2(bb.Min.x + adv, bb.Max.y), ImGui::GetColorU32(ImGuiCol_PlotLines), 2.0f); // base line
		}

		window->DrawList->PushTextureID(vFont->ContainerAtlas->TexID);
		vFont->RenderChar(window->DrawList, vGlyphSize.y, ImVec2(bb.Min.x + offsetX, bb.Min.y), ImGui::GetColorU32(ImGuiCol_Text), (ImWchar)vGlyph.Codepoint);
		window->DrawList->PopTextureID();

		ImGui::PopClipRect();
	}

	return res;
}
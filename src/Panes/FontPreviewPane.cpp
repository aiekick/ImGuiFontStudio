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
				
				if (ImGui::Button("Clear"))
				{
					m_GlyphToInsert.clear();
				}
				ImGui::SameLine();
				ImGui::SliderFloatDefaultCompact(ImGui::GetContentRegionAvail().x, "Font Size", &m_FontSizePreview, 1, 100, 15.0f);

				ImGui::Text("Text :");
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
	ImVec2 cell_size, glyph_size;
	uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);
	
	ImFont* font = ImGui::GetFont();
	if (font)
	{
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
						auto glyph = glyphInfos->second->m_SelectedGlyphs[glyphInfos->first].glyph;

						if (idx)
						{
							ImGui::SameLine();
						}

						bool selected = false;
						DrawGlyphButton(vProjectFile, glyphInfos->second->m_ImFontAtlas.Fonts[0], m_FontSizePreview, &selected, glyph_size, glyph);
					}
				}
			}

			auto glyph = font->FindGlyph(c);
			if (glyph)
			{
				if (idx)
				{
					ImGui::SameLine();
				}

				bool selected = false;
				DrawGlyphButton(vProjectFile, font->ContainerAtlas->Fonts[0], font->FontSize, &selected, glyph_size, *glyph);
			}

			idx++;
		}
	}
}

void FontPreviewPane::DrawMixedFontResult(ProjectFile* vProjectFile)
{
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
	
	ImFont* font = ImGui::GetFont();
	if (font)
	{
		float baseFontRatioX = 1.0f;
		if (font->ContainerAtlas->TexHeight > 0)
			baseFontRatioX = font->ContainerAtlas->TexWidth / font->ContainerAtlas->TexHeight;

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
						auto glyph = glyphInfos->second->m_SelectedGlyphs[glyphInfos->first].glyph;

						float scale = (font->FontSize / glyphInfos->second->m_FontSize) * (m_FontSizePreview / font->FontSize);
						ImTextureID texId = (ImTextureID)glyphInfos->second->m_ImFontAtlas.TexID;

						ImVec2 uv0 = ImVec2(glyph.U0, glyph.V0);
						ImVec2 uv1 = ImVec2(glyph.U1, glyph.V1);
						
						float hostRatioX = 1.0f;
						if (glyphInfos->second->m_ImFontAtlas.TexHeight > 0)
							hostRatioX = glyphInfos->second->m_ImFontAtlas.TexWidth / glyphInfos->second->m_ImFontAtlas.TexHeight;
						ImVec2 uvSize = uv1 - uv0;
						float ratioX = uvSize.x * hostRatioX / uvSize.y;

						ImVec2 xy0 = ImVec2(glyph.X0, glyph.Y0) * scale;
						ImVec2 xy1 = ImVec2(glyph.X1, glyph.Y1) * scale;
						ImVec2 pMin = ImVec2(pos.x + offsetX + xy0.x, pos.y + xy0.y);
						ImVec2 pMax = pMin + xy1;

						ImRect glyphRect = ImRect(pMin, pMax);
						ImVec2 glyphSize = glyphRect.GetSize();

						// redim with ratio
						float newX = glyphSize.y * ratioX;
						glyphSize = ImVec2(glyphSize.x, glyphSize.x / ratioX) * 0.5f;
						if (newX < glyphSize.x)
							glyphSize = ImVec2(newX, glyphSize.y) * 0.5f;
						ImVec2 center = glyphRect.GetCenter();

						window->DrawList->AddImage(texId, center - glyphSize, center + glyphSize, uv0, uv1, colFont);
						offsetX += glyph.AdvanceX * scale;
					}
				}
			}

			auto glyph = font->FindGlyph(c);
			if (glyph)
			{
				float scale = m_FontSizePreview / font->FontSize;
				ImTextureID texId = (ImTextureID)font->ContainerAtlas->TexID;
				ImVec2 uv0 = ImVec2(glyph->U0, glyph->V0);
				ImVec2 uv1 = ImVec2(glyph->U1, glyph->V1);

				ImVec2 uvSize = uv1 - uv0;
				float ratioX = uvSize.x * baseFontRatioX / uvSize.y;

				ImVec2 xy0 = ImVec2(glyph->X0, glyph->Y0) * scale;
				ImVec2 xy1 = ImVec2(glyph->X1, glyph->Y1) * scale;
				ImVec2 pMin = ImVec2(pos.x + offsetX + xy0.x, pos.y + xy0.y);
				ImVec2 pMax = pMin + xy1;

				ImRect glyphRect = ImRect(pMin, pMax);
				ImVec2 glyphSize = glyphRect.GetSize();

				// redim with ratio
				//float newX = glyphSize.y * ratioX;
				//glyphSize = ImVec2(glyphSize.x, glyphSize.x / ratioX) * 0.5f;
				//if (newX < glyphSize.x)
				//	glyphSize = ImVec2(newX, glyphSize.y) * 0.5f;
				//ImVec2 center = glyphRect.GetCenter();

				window->DrawList->AddImage(texId, pMin, pMax, uv0, uv1, colFont);
				offsetX += glyph->AdvanceX * scale;
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

		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vGlyphSize + style.FramePadding * 2);
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

		ImVec2 startPos = bb.Min + style.FramePadding;
		ImVec2 endPos = startPos + vGlyphSize;

		ImVec2 uv0 = ImVec2(vGlyph.U0, vGlyph.V0);
		ImVec2 uv1 = ImVec2(vGlyph.U1, vGlyph.V1);
		ImVec2 center = ImVec2(0, 0);
		ImVec2 glyphSize = ImVec2(0, 0);

		float hostRatioX = 1.0f;
		if (vFont->ContainerAtlas->TexHeight > 0)
			hostRatioX = vFont->ContainerAtlas->TexWidth / vFont->ContainerAtlas->TexHeight;
		ImVec2 uvSize = uv1 - uv0;
		float ratioX = uvSize.x * hostRatioX / uvSize.y;

		ImGui::PushClipRect(bb.Min, bb.Max, true);

		ImVec2 pScale = vGlyphSize / vFontSize;

		ImVec2 xy0 = ImVec2(vGlyph.X0, vGlyph.Y0) * pScale;
		ImVec2 xy1 = ImVec2(vGlyph.X1, vGlyph.Y1) * pScale;
		ImRect realGlyphRect = ImRect(startPos + xy0, startPos + xy1);
		ImVec2 realGlyphSize = realGlyphRect.GetSize();

		// redim with ratio
		float newX = realGlyphSize.y * ratioX;
		glyphSize = ImVec2(realGlyphSize.x, realGlyphSize.x / ratioX) * 0.5f;
		if (newX < realGlyphSize.x)
			glyphSize = ImVec2(newX, realGlyphSize.y) * 0.5f;
		center = realGlyphRect.GetCenter();

		float offsetX = vGlyphSize.x * 0.5f - realGlyphSize.x * 0.5;
		center.x += offsetX; // center the glyph

		/*if (vProjectFile->m_ShowBaseLine)// draw base line
		{
			float asc = vFontInfos->m_Ascent * vFontInfos->m_Point * pScale.y;
			window->DrawList->AddLine(ImVec2(bb.Min.x, startPos.y + asc), ImVec2(bb.Max.x, startPos.y + asc), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 2.0f); // base line
		}

		if (vProjectFile->m_ShowOriginX) // draw origin x
		{
			window->DrawList->AddLine(ImVec2(startPos.x + offsetX, bb.Min.y), ImVec2(startPos.x + offsetX, bb.Max.y), ImGui::GetColorU32(ImGuiCol_PlotLinesHovered), 2.0f); // base line
		}

		if (vProjectFile->m_ShowAdvanceX) // draw advance X
		{
			float adv = vGlyph.AdvanceX * pScale.y + offsetX;
			window->DrawList->AddLine(ImVec2(startPos.x + adv, bb.Min.y), ImVec2(startPos.x + adv, bb.Max.y), ImGui::GetColorU32(ImGuiCol_PlotLines), 2.0f); // base line
		}*/

		window->DrawList->AddImage(vFont->ContainerAtlas->TexID, center - glyphSize, center + glyphSize, uv0, uv1, ImGui::GetColorU32(ImGuiCol_Text)); // glyph

		ImGui::PopClipRect();
	}

	return res;
}
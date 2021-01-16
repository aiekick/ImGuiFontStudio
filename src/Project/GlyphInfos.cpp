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
#include "GlyphInfos.h"

#include <Project/ProjectFile.h>

#include <utility>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <Gui/ImGuiWidgets.h>
#include <Helper/ImGuiThemeHelper.h>
#include <Helper/AssetManager.h>

 ///////////////////////////////////////////////////////////////////////////////////
 //// PUBLIC : STATIC //////////////////////////////////////////////////////////////
 ///////////////////////////////////////////////////////////////////////////////////

int GlyphDisplayHelper::CalcGlyphsCountAndSize(
	ProjectFile* vProjectFile,				/* project file for save some vars  */
	ImVec2* vCellSize,						/* cell size						*/
	ImVec2* vGlyphSize,						/* glyph size (cell - paddings)		*/
	bool vGlyphEdited,						/* in edition						*/
	bool vForceEditMode,					/* edition forced					*/
	bool vForceEditModeOneColumn			/* edition in one column			*/)
{
	if (vProjectFile && vCellSize && vGlyphSize)
	{
		float aw = ImGui::GetContentRegionAvail().x;
		
		int glyphCount = vProjectFile->m_Preview_Glyph_CountX;
		float glyphWidth = (float)vProjectFile->m_Preview_Glyph_Width;
		
		// GlyphSize est Menant, puis glyphCount est appliqué
		if (vProjectFile->m_GlyphDisplayTuningMode & GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_SIZE)
		{
			glyphCount = (int)(aw / ct::maxi(glyphWidth, 1.0f));
			glyphWidth = aw / (float)ct::maxi(glyphCount, 1);
			if (vProjectFile->m_GlyphSizePolicyChangeFromWidgetUse)
			{
				vProjectFile->m_Preview_Glyph_CountX = glyphCount;
			}
		}
		// GlyphCount est Menant, dont m_Preview_Glyph_CountX n'est jamais réécrit en dehors du user
		else if (vProjectFile->m_GlyphDisplayTuningMode & GlyphDisplayTuningModeFlags::GLYPH_DISPLAY_TUNING_MODE_GLYPH_COUNT)
		{
			glyphWidth = aw / (float)ct::maxi(glyphCount, 1);
			if (vProjectFile->m_GlyphSizePolicyChangeFromWidgetUse)
			{
				vProjectFile->m_Preview_Glyph_Width = glyphWidth;
			}
		}
			
		if (glyphCount > 0)
		{
			*vCellSize = ImVec2(glyphWidth, glyphWidth);
			*vGlyphSize = *vCellSize - ImGui::GetStyle().ItemSpacing;

			if (vGlyphEdited || vForceEditMode)
			{
				// we will forgot vGlyphCountX
				// cell_size_y will be item height x 2 + padding_y
				// cell_size_x will be cell_size_y + padding_x + 100
				const ImVec2 label_size = ImGui::CalcTextSize("gt", nullptr, true);
				const ImVec2 frame_size = ImGui::CalcItemSize(ImVec2(0, 0), ImGui::CalcItemWidth(),
					label_size.y + ImGui::GetStyle().FramePadding.y * 2.0f);
				float cell_size_y = frame_size.y * 2.0f + ImGui::GetStyle().FramePadding.y;
				float cell_size_x = cell_size_y + ImGui::GetStyle().FramePadding.x + GLYPH_EDIT_CONTROl_WIDTH;
				glyphCount = ct::maxi(1, (int)ct::floor(aw / cell_size_x));
				*vGlyphSize = ImVec2(cell_size_y, cell_size_y);
			}

			if (vForceEditModeOneColumn && vForceEditMode)
			{
				glyphCount = 1;
			}

			*vGlyphSize -= ImGui::GetStyle().FramePadding * 2.0f;
		}

		return glyphCount;
	}

	assert(0);
	return 0;
}

 //////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////
 //////////////////////////////////////////////////////////

void SimpleGlyph_Solo::clear()
{
	coords.clear();
	onCurve.clear();
	isValid = false;
	rc = 0;
}

void SimpleGlyph_Solo::LoadSimpleGlyph(sfntly::GlyphTable::SimpleGlyph *vGlyph)
{
	if (vGlyph)
	{
		vGlyph->Initialize();
		clear();
		int cmax = vGlyph->NumberOfContours();
		for (int c = 0; c < cmax; c++)
		{
			onCurve.emplace_back();
			coords.emplace_back();
			int pmax = vGlyph->numberOfPoints(c);
			for (int p = 0; p < pmax; p++)
			{
				int32_t x = vGlyph->xCoordinate(c, p);
				int32_t y = vGlyph->yCoordinate(c, p);
				coords[c].push_back(ct::ivec2(x, y));
				bool oc = vGlyph->onCurve(c, p);
				onCurve[c].push_back(oc);
			}
		}
		isValid = !coords.empty();
		rc.x = vGlyph->XMin();
		rc.y = vGlyph->YMin();
		rc.z = vGlyph->XMax();
		rc.w = vGlyph->YMax();
	}
}

int SimpleGlyph_Solo::GetCountContours() const
{
	return (int)coords.size();
}

ct::ivec2 SimpleGlyph_Solo::GetCoords(int32_t vContour, int32_t vPoint)
{
	int count = (int)coords[vContour].size();

	ct::ivec2 p = coords[vContour][vPoint % count];

	// apply transformation
	p += m_Translation;
	p.x = (int)(p.x * m_Scale.x);
	p.y = (int)(p.y * m_Scale.y);
	
	return p;
}

bool SimpleGlyph_Solo::IsOnCurve(int32_t vContour, int32_t vPoint)
{
	int count = (int)coords[vContour].size();
	return onCurve[vContour][vPoint % count];
}

ct::ivec2 SimpleGlyph_Solo::Scale(ct::ivec2 p, double scale) const
{
	return {
		(int)round(scale * ((double)p.x - (double)rc.x)),
		(int)round(scale * ((double)rc.w - (double)p.y))};
}

ct::ivec2 SimpleGlyph_Solo::GetCoords(int32_t vContour, int32_t vPoint, double scale)
{
	return Scale(GetCoords(vContour, vPoint), scale);
}

//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////

GlyphInfos::GlyphInfos()
{
	glyph.AdvanceX = 0;
	glyph.Codepoint = 0;
	glyph.U0 = 0;
	glyph.U1 = 0;
	glyph.V0 = 0;
	glyph.V1 = 0;
	glyph.Visible = false;
	glyph.X0 = 0;
	glyph.X1 = 0;
	glyph.Y0 = 0;
	glyph.Y1 = 0;
}

GlyphInfos::GlyphInfos(
	ImFontGlyph vGlyph, std::string vOldName, 
	std::string vNewName, uint32_t vNewCodePoint)
{
	glyph = vGlyph;
	oldHeaderName = vOldName;
	newHeaderName = vNewName;
	newCodePoint = vNewCodePoint;
	if (newCodePoint == 0)
		newCodePoint = glyph.Codepoint;
}

GlyphInfos::~GlyphInfos() = default;

std::shared_ptr<FontInfos> GlyphInfos::GetFontInfos()
{
	if (!fontInfos.expired())
	{
		return fontInfos.lock();
	}

	return 0;
}

void GlyphInfos::SetFontInfos(std::shared_ptr<FontInfos> vFontInfos)
{
	fontInfos = vFontInfos;
	if (!vFontInfos)
		fontInfos.reset();
}

bool GlyphInfos::DrawGlyphButton(
	ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos, 
	bool* vSelected, ImVec2 vGlyphSize, ImFontGlyph vGlyph, ImVec2 vHostTextureSize,
	int frame_padding, float vRectThickNess, ImVec4 vRectColor)
{
	bool res = false;

	if (vFontInfos)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImGui::PushID((void*)(intptr_t)vFontInfos->m_ImFontAtlas.TexID);
		const ImGuiID id = window->GetID("#image");
		ImGui::PopID();
		
		const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vGlyphSize + padding * 2);
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
		float rounding = ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding);
#ifndef USE_GRADIENT
		// normal
		ImGui::RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
#else

#if 1
		// inner shadow
		ImVec4 cb = ImColor(col).Value; // color base : cb
		float sha = ImGuiThemeHelper::Instance()->m_ShadowStrength;
		ImVec4 cbd = ImVec4(cb.x * sha, cb.y * sha, cb.z * sha, cb.w * 0.9f); // color base darker : cbd
		ImGui::RenderInnerShadowFrame(bb.Min, bb.Max, col, ImGui::GetColorU32(cbd), ImGui::GetColorU32(ImGuiCol_WindowBg), true, rounding);
#else
		ImTextureID texId = (ImTextureID)AssetManager::Instance()->m_Textures["btn"].glTex;
		window->DrawList->AddImage(texId, bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
#endif

#endif
		ImGui::AddInvertedRectFilled(window->DrawList, bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_WindowBg), rounding, ImDrawCornerFlags_All);
		if (vRectThickNess > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vRectColor), 0.0, 15, vRectThickNess);
		}

		ImVec2 startPos = bb.Min + padding;
		ImVec2 endPos = startPos + vGlyphSize;

		ImVec2 uv0 = ImVec2(vGlyph.U0, vGlyph.V0);
		ImVec2 uv1 = ImVec2(vGlyph.U1, vGlyph.V1);
		ImVec2 center = ImVec2(0, 0);
		ImVec2 glyphSize = ImVec2(0, 0);

		float hostRatioX = 1.0f;
		if (vHostTextureSize.y > 0)
			hostRatioX = vHostTextureSize.x / vHostTextureSize.y;
		ImVec2 uvSize = uv1 - uv0;
		float ratioX = uvSize.x * hostRatioX / uvSize.y;

		ImGui::PushClipRect(bb.Min, bb.Max, true);

		if (vProjectFile->m_ZoomGlyphs)
		{
			float newX = vGlyphSize.y * ratioX;
			glyphSize = ImVec2(vGlyphSize.x, vGlyphSize.x / ratioX) * 0.5f;
			if (newX < vGlyphSize.x)
				glyphSize = ImVec2(newX, vGlyphSize.y) * 0.5f;
			center = bb.GetCenter();
		}
		else
		{
			ImVec2 pScale = vGlyphSize / (float)vFontInfos->m_FontSize;

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

			if (vProjectFile->m_ShowBaseLine)// draw base line
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
			}
		}

		window->DrawList->AddImage(vFontInfos->m_ImFontAtlas.TexID, center - glyphSize, center + glyphSize, uv0, uv1, ImGui::GetColorU32(ImGuiCol_Text)); // glyph

		ImGui::PopClipRect();
	}

	return res;
}
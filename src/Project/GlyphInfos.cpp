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
#include <Panes/DebugPane.h>

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

void SimpleGlyph_Solo::Clear()
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
		Clear();
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
	p.x = (int)(p.x * m_Scale.x);
	p.y = (int)(p.y * m_Scale.y);
	p += ct::ivec2(m_Translation);

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

void SimpleGlyph_Solo::ClearTransform()
{
	m_Translation = 0.0f;
	m_Scale = 1.0f;
}

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer
void SimpleGlyph_Solo::DrawCurves(float vGlobalScale, int vFontAscent, int vFontDescent, int vMaxContour, int vQuadBezierCountSegments, bool vShowControlLines)
{
	if (isValid)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;
		auto drawList = window->DrawList;

		int cmax = (int)coords.size();

		ImVec2 contentStart = ImGui::GetCursorScreenPos();
		ImVec2 contentSize = ImGui::GetContentRegionAvail();
		ImVec2 contentCenter = contentStart + contentSize * 0.5f;

		drawList->AddRectFilled(contentStart, contentStart + contentSize, ImGui::GetColorU32(ImGuiCol_FrameBg));

		ImRect glypRect = ImRect(
			((float)rc.x),
			((float)rc.y),
			((float)(rc.z - rc.x)),
			((float)(rc.w - rc.y)));

		ImVec2 glyphCenter = glypRect.GetCenter();
		ImVec2 glyphSize = glypRect.GetSize();
		glyphSize.y = ImMax(glyphSize.y, (float)vFontAscent - (float)vFontDescent);

		ImVec2 pScale = contentSize / glyphSize;
		float newScale = ImMin(pScale.x, pScale.y) * vGlobalScale;
		glypRect.Min *= newScale;
		glypRect.Max *= newScale;
		glyphSize = glypRect.GetSize();

		ImVec2 pos = contentCenter - glyphSize * 0.5f;

		// x 0 + blue
		drawList->AddLine(
			ct::toImVec2(Scale(ct::ivec2(0, (int32_t)ct::floor(rc.y)), newScale)) + pos,
			ct::toImVec2(Scale(ct::ivec2(0, (int32_t)ct::floor(rc.w)), newScale)) + pos,
			ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), 2.0f);

		// Ascent
		drawList->AddLine(
			ct::toImVec2(Scale(ct::ivec2((int32_t)ct::floor(rc.x), vFontAscent), newScale)) + pos,
			ct::toImVec2(Scale(ct::ivec2((int32_t)ct::floor(rc.z), vFontAscent), newScale)) + pos,
			ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 2.0f);

		// y 0
		drawList->AddLine(
			ct::toImVec2(Scale(ct::ivec2((int32_t)ct::floor(rc.x), 0), newScale)) + pos,
			ct::toImVec2(Scale(ct::ivec2((int32_t)ct::floor(rc.z), 0), newScale)) + pos,
			ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 1.0f);

		// Descent
		drawList->AddLine(
			ct::toImVec2(Scale(ct::ivec2((int32_t)ct::floor(rc.x), vFontDescent), newScale)) + pos,
			ct::toImVec2(Scale(ct::ivec2((int32_t)ct::floor(rc.z), vFontDescent), newScale)) + pos,
			ImGui::GetColorU32(ImVec4(1, 0, 0, 1)), 2.0f);

		for (int c = 0; c < cmax; c++)
		{
			if (c >= vMaxContour) break;

			int pmax = (int)coords[c].size();

			int firstOn = 0;
			for (int p = 0; p < pmax; p++)
			{
				if (IsOnCurve(c, p))
				{
					firstOn = p;
					break;
				}
			}

			// curve

			drawList->PathLineTo(ct::toImVec2(GetCoords(c, firstOn, newScale)) + pos);

			for (int i = 0; i < pmax; i++)
			{
				int icurr = firstOn + i + 1;
				int inext = firstOn + i + 2;
				ct::ivec2 cur = GetCoords(c, icurr, newScale);

				if (IsOnCurve(c, icurr))
				{
					drawList->PathLineTo(ct::toImVec2(cur) + pos);
				}
				else
				{
					ct::ivec2 nex = GetCoords(c, inext, newScale);
					if (!IsOnCurve(c, inext))
					{
						nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
						nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
					}
					drawList->PathBezierQuadraticCurveTo(
						ct::toImVec2(cur) + pos,
						ct::toImVec2(nex) + pos, vQuadBezierCountSegments);
				}
			}

			drawList->PathStroke(ImGui::GetColorU32(ImGuiCol_Text), true);

#ifdef _DEBUG
			DebugPane::Instance()->DrawGlyphCurrentPoint(newScale, pos, drawList);
#endif

			if (vShowControlLines) // control lines
			{
				drawList->PathLineTo(ct::toImVec2(GetCoords(c, firstOn, newScale)) + pos);

				for (int i = 0; i < pmax; i++)
				{
					int icurr = firstOn + i + 1;
					int inext = firstOn + i + 2;
					ct::ivec2 cur = GetCoords(c, icurr, newScale);
					if (IsOnCurve(c, icurr))
					{
						drawList->PathLineTo(ct::toImVec2(cur) + pos);
					}
					else
					{
						ct::ivec2 nex = GetCoords(c, inext, newScale);
						if (!IsOnCurve(c, inext))
						{
							nex.x = (int)(((double)nex.x + (double)cur.x) * 0.5);
							nex.y = (int)(((double)nex.y + (double)cur.y) * 0.5);
						}
						drawList->PathLineTo(ct::toImVec2(cur) + pos);
						drawList->PathLineTo(ct::toImVec2(nex) + pos);
					}
				}

				drawList->PathStroke(ImGui::GetColorU32(ImVec4(0, 0, 1, 1)), true);
			}
		}
	}
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
	std::string vNewName, uint32_t vNewCodePoint,
	ImVec2 vTranslation)
{
	glyph = vGlyph;
	oldHeaderName = vOldName;
	newHeaderName = vNewName;
	newCodePoint = vNewCodePoint;
	if (newCodePoint == 0)
		newCodePoint = glyph.Codepoint;
	m_Translation = vTranslation;
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

int GlyphInfos::DrawGlyphButton(
	int &vWidgetPushId, // by adress because we want modify it
	ProjectFile* vProjectFile, ImFont* vFont,
	bool* vSelected, ImVec2 vGlyphSize, const ImFontGlyph *vGlyph, 
	ImVec2 vTranslation, ImVec2 vScale,
	int frame_padding, float vRectThickNess, ImVec4 vRectColor)
{
	int res = 0;

	if (vFont && vGlyph)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;

		ImGui::PushID(++vWidgetPushId);
		ImGui::PushID((void*)(intptr_t)vFont->ContainerAtlas->TexID);
		const ImGuiID id = window->GetID("#image");
		ImGui::PopID();
		ImGui::PopID();

		const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
		ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vGlyphSize + padding * 2);
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
		if (pressed)
		{
			if (vSelected)
			{
				*vSelected = !*vSelected;
			}

			if (g.ActiveIdMouseButton == 0) // left click
				res = 1;
			if (g.ActiveIdMouseButton == 1) // right click
				res = 2;
		}

		// Render
		const ImU32 col = ImGui::GetColorU32(((held && hovered) || (vSelected && *vSelected)) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderNavHighlight(bb, id);

		const float rounding = ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding);
#ifdef USE_SHADOW
		if (!ImGuiThemeHelper::m_UseShadow)
		{
#endif
			// normal
			ImGui::RenderFrame(bb.Min, bb.Max, col, true, rounding);
#ifdef USE_SHADOW
		}
		else
		{
			if (ImGuiThemeHelper::m_UseTextureForShadow)
			{
				ImTextureID texId = (ImTextureID)AssetManager::Instance()->m_Textures["btn"].glTex;
				window->DrawList->AddImage(texId, bb.Min, bb.Max, ImVec2(0, 0), ImVec2(1, 1), col);
			}
			else
			{
				// inner shadow
				ImVec4 cb = ImColor(col).Value; // color base : cb
				float sha = ImGuiThemeHelper::Instance()->m_ShadowStrength;
				ImVec4 cbd = ImVec4(cb.x * sha, cb.y * sha, cb.z * sha, cb.w * 0.9f); // color base darker : cbd
				ImGui::RenderInnerShadowFrame(bb.Min, bb.Max, col, ImGui::GetColorU32(cbd), ImGui::GetColorU32(ImGuiCol_WindowBg), true, rounding);
			}
		}
		ImGui::AddInvertedRectFilled(window->DrawList, bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_WindowBg), rounding, ImDrawCornerFlags_All);
#endif

		// double codepoint / name rect display
		if (vRectThickNess > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vRectColor), 0.0, 15, vRectThickNess);
		}

		ImGui::PushClipRect(bb.Min, bb.Max, true);
		
		bb.Min += style.FramePadding;
		bb.Max -= style.FramePadding;

		ImVec2 pScale = vGlyphSize / vFont->FontSize;
		float adv = vGlyph->AdvanceX * pScale.y;
		float offsetX = bb.GetSize().x * 0.5f - adv * 0.5f; // horizontal centering of the glyph
		ImVec2 trans = vTranslation * pScale;
		ImVec2 scale = vScale;

		if (!vProjectFile->m_ZoomGlyphs)
		{
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
				window->DrawList->AddLine(ImVec2(bb.Min.x + adv + offsetX, bb.Min.y), ImVec2(bb.Min.x + adv + offsetX, bb.Max.y), ImGui::GetColorU32(ImGuiCol_PlotLines), 2.0f); // base line
			}
		}

		RenderGlyph(vFont, window->DrawList,
			vGlyphSize.y, 
			bb.Min, bb.Max, ImVec2(offsetX,0),
			ImGui::GetColorU32(ImGuiCol_Text), 
			(ImWchar)vGlyph->Codepoint, 
			trans, scale,
			vProjectFile->m_ZoomGlyphs);
		
		ImGui::PopClipRect();
	}

	return res;
}

void GlyphInfos::RenderGlyph(ImFont* vFont, ImDrawList* vDrawList, float vGlyphHeight, ImVec2 vMin, ImVec2 vMax, ImVec2 vOffset, ImU32 vCol, ImWchar vGlyphCodePoint, ImVec2 vTranslation, ImVec2 vScale, bool vZoomed)
{
	if (vFont && vFont->ContainerAtlas && vDrawList && vGlyphHeight > 0.0f) // (vGlyphSize > 0.0 for avoid div by zero)
	{
		const ImFontGlyph* glyph = vFont->FindGlyph(vGlyphCodePoint);
		if (!glyph || !glyph->Visible)
			return;

		float scale = (vGlyphHeight >= 0.0f) ? (vGlyphHeight / vFont->FontSize) : 1.0f;
		//vPos.x = IM_FLOOR(vPos.x);
		//vPos.y = IM_FLOOR(vPos.y);

		ImVec2 pMin(0, 0), pMax(0, 0);

		if (vZoomed)
		{
			ImVec2 gSize = ImVec2(glyph->X1 - glyph->X0, glyph->Y1 - glyph->Y0);
			if (IS_FLOAT_EQUAL(gSize.y, 0.0f))
				return;
			float ratioX = gSize.x / gSize.y;
			float newX = vGlyphHeight * ratioX;
			gSize = ImVec2(vGlyphHeight, vGlyphHeight / ratioX) * 0.5f;
			if (newX < vGlyphHeight)
				gSize = ImVec2(newX, vGlyphHeight) * 0.5f;
			ImVec2 center = ImRect(vMin, vMax).GetCenter();

			pMin = center - gSize;
			pMax = center + gSize;
		}
		else
		{
			pMin = ImVec2(vMin.x + vOffset.x + glyph->X0 * scale * vScale.x + vTranslation.x, vMin.y + vOffset.y + glyph->Y0 * scale * vScale.y - vTranslation.y);
			pMax = ImVec2(vMin.x + vOffset.x + glyph->X1 * scale * vScale.x + vTranslation.x, vMin.y + vOffset.y + glyph->Y1 * scale * vScale.y - vTranslation.y);
		}

		ImVec2 uv0 = ImVec2(glyph->U0, glyph->V0);
		ImVec2 uv1 = ImVec2(glyph->U1, glyph->V1);

		vDrawList->PushTextureID(vFont->ContainerAtlas->TexID);
		vDrawList->PrimReserve(6, 4);
		vDrawList->PrimRectUV(pMin, pMax, uv0, uv1, vCol);
		vDrawList->PopTextureID();
	}
}
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
#include "SourceFontPane.h"

#include <ctools/FileHelper.h>
#include <ImGuiFileDialog/ImGuiFileDialog.h>

#include <MainFrame.h>
#include <Helper/SelectionHelper.h>
#include <Panes/FinalFontPane.h>
#include <Panes/Manager/LayoutManager.h>
#include <Project/FontInfos.h>
#include <Project/ProjectFile.h>
#include <Project/GlyphInfos.h>

#include <cinttypes> // printf zu

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

static ProjectFile defaultProjectValues;
static FontInfos defaultFontInfosValues;

SourceFontPane::SourceFontPane() = default;
SourceFontPane::~SourceFontPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SourceFontPane::Init()
{
	
}

void SourceFontPane::Unit()
{

}

int SourceFontPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	paneWidgetId = vWidgetId;

	DrawSourceFontPane(vProjectFile);
	
	return paneWidgetId;
}

void SourceFontPane::DrawDialogsAndPopups(ProjectFile * vProjectFile)
{
	UNUSED(vProjectFile);
}

int SourceFontPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	UNUSED(vUserDatas);

	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (LayoutManager::Instance()->IsSpecificPaneFocused(PaneFlags::PANE_SOURCE))
		{
			const float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 4.0f;
			const float mrw = maxWidth / 2.0f - ImGui::GetStyle().FramePadding.x;

			if (ImGui::BeginFramedGroup("Source Font Pane"))
			{
				ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(
					ICON_IGFS_GLYPHS " Glyphs", "Show Font Glyphs",
					&vProjectFile->m_SourceFontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH, mrw, true);

				ImGui::SameLine();

				ImGui::RadioButtonLabeled_BitWize<SourceFontPaneFlags>(
					ICON_IGFS_TEXTURE " Texture", "Show Font Texture",
					&vProjectFile->m_SourceFontPaneFlags, SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE, mrw, true);

				ImGui::EndFramedGroup(true);
			}
		}
	}

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : PANES //////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SourceFontPane::DrawSourceFontPane(ProjectFile *vProjectFile)
{
	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_SOURCE)
	{
		if (ImGui::Begin<PaneFlags>(SOURCE_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_SOURCE,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile && vProjectFile->IsLoaded())
			{
				if (vProjectFile->m_SelectedFont)
				{
					if (vProjectFile->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_GLYPH)
					{
						if (ImGui::BeginMenuBar())
						{
							if (ImGui::BeginMenu("Infos"))
							{
								if (ImGui::MenuItem("Show Tooltip", "", &vProjectFile->m_SourcePane_ShowGlyphTooltip))
								{
									vProjectFile->SetProjectChange();
								}

								ImGui::EndMenu();
							}

							ImGui::Spacing();

							SelectionHelper::Instance()->DrawSelectionMenu(vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

							ImGui::Spacing();
							
							if (vProjectFile->m_Preview_Glyph_CountX)
							{
								DrawFilterBar(vProjectFile, vProjectFile->m_SelectedFont);
							}

							ImGui::EndMenuBar();
						}
						
						DrawFontAtlas_Virtual(vProjectFile, vProjectFile->m_SelectedFont);
					}
					else if (vProjectFile->m_SourceFontPaneFlags & SourceFontPaneFlags::SOURCE_FONT_PANE_TEXTURE)
					{
						DrawFontTexture(vProjectFile->m_SelectedFont);
					}
				}
			}
		}

		ImGui::End();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE : WIDGETS, VIEW //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void SourceFontPane::DrawFilterBar(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos)
{
	if (vProjectFile && vFontInfos)
	{
		ImGui::PushID(vFontInfos.get());
		ImGui::Text("Filters");
		ImGui::Text("(?)");
		if (ImGui::IsItemHovered())
			ImGui::SetTooltip("will search for any filter separated\n\tby a coma ',' or a space ' '");
		if (ImGui::MenuItem(ICON_IGFS_DESTROY "##clearFilter"))
		{
			ct::ResetBuffer(vFontInfos->m_SearchBuffer);
			vFontInfos->m_Filters.clear();
			vProjectFile->SetProjectChange();
		}
		ImGui::PushItemWidth(400);
		bool filterChanged = ImGui::InputText("##Filter", vFontInfos->m_SearchBuffer, 1023);
		ImGui::PopItemWidth();
		ImGui::PopID();
		if (filterChanged)
		{
			vFontInfos->m_Filters.clear();
			std::string s = vFontInfos->m_SearchBuffer;
			auto arr = ct::splitStringToVector(s, ", ");
			for (const auto &it : arr)
			{
				vFontInfos->m_Filters.insert(it);
			}
			vProjectFile->SetProjectChange();
		}
	}
}

bool SourceFontPane::IfCatchedByFilters(std::shared_ptr<FontInfos> vFontInfos, const std::string& vSymbolName)
{
	if (vFontInfos)
	{
		if (vFontInfos->m_Filters.empty())
		{
			return true;
		}
		else
		{
			for (const auto &it : vFontInfos->m_Filters)
			{
				if (vSymbolName.find(it) != std::string::npos) // found
				{
					return true;
				}
			}
		}
	}

	return false;
}

bool SourceFontPane::DrawGlyphButton(ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos,
	std::string vName, bool* vSelected, ImVec2 vGlyphSize, ImFontGlyph vGlyph, ImVec2 vHostTextureSize, 
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

		ImGui::PushID(NewWidgetId());
		ImGui::PushID((void*)(intptr_t)vFontInfos->m_ImFontAtlas.TexID);
		const ImGuiID id = window->GetID("#image");
		ImGui::PopID();
		ImGui::PopID();

		const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + vGlyphSize + style.FramePadding * 2);
		ImGui::ItemSize(bb);
		if (!ImGui::ItemAdd(bb, id))
			return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held);

		if (pressed && vSelected)
			*vSelected = !*vSelected;

		// Render
		const ImU32 col = ImGui::GetColorU32(((held && hovered) || (vSelected && *vSelected)) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		ImGui::RenderNavHighlight(bb, id);
		ImGui::RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(style.FramePadding.x, style.FramePadding.y), 0.0f, 12.0f));
		if (vRectThickNess > 0.0f)
		{
			window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(vRectColor), 0.0, 15, vRectThickNess);
		}

		ImVec2 startPos = bb.Min + style.FramePadding;
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

void SourceFontPane::DrawFontAtlas_Virtual(ProjectFile *vProjectFile, std::shared_ptr<FontInfos> vFontInfos)
{
	auto win = ImGui::GetCurrentWindowRead();
	if (win)
	{
		win->DrawList->ChannelsSplit(2);

		if (vProjectFile && vProjectFile->IsLoaded() &&
			vFontInfos)
		{
			vProjectFile->m_Preview_Glyph_CountX = ct::maxi(vProjectFile->m_Preview_Glyph_CountX, 1);

			if (vFontInfos->m_ImFontAtlas.IsBuilt())
			{
				ImFont* font = vFontInfos->m_ImFontAtlas.Fonts[0];

				if (vFontInfos->m_ImFontAtlas.TexID)
				{
					ImVec2 hostTextureSize = ImVec2(
						(float)vFontInfos->m_ImFontAtlas.TexWidth,
						(float)vFontInfos->m_ImFontAtlas.TexHeight);
					ImVec2 cell_size, glyph_size;
					uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);
					if (glyphCountX)
					{
						uint32_t idx = 0, lastGlyphCodePoint = 0;
						ImVec4 glyphRangeColoring = ImGui::GetStyleColorVec4(ImGuiCol_Button);
						bool showRangeColoring = vProjectFile->IsRangeColoringShown();
						if (!font->Glyphs.empty())
						{
							uint32_t countGlyphs = (uint32_t)font->Glyphs.size();
							int rowCount = (int)ct::ceil((double)countGlyphs / (double)glyphCountX);
							ImGuiListClipper m_Clipper;

							/*
							ImGui::SetTooltip(
								"cell size : %.2f, %.2f\n\
								glyph size : %.2f, %.2f\n\
								count glyphs : x:%u,y:%i\n\
								line height : %.0f",
								cell_size.x, cell_size.y,
								glyph_size.x, glyph_size.y,
								glyphCountX, rowCount,
								cell_size.y);
							*/

							m_Clipper.Begin(rowCount, cell_size.y);
							while (m_Clipper.Step())
							{
								for (int j = m_Clipper.DisplayStart; j < m_Clipper.DisplayEnd; j++)
								{
									if (j < 0) continue;

									for (uint32_t i = 0; i < glyphCountX; i++)
									{
										uint32_t glyphIdx = i + j * glyphCountX;
										if (glyphIdx < countGlyphs)
										{
											auto glyph = *(font->Glyphs.begin() + glyphIdx);

											std::string name = vFontInfos->m_GlyphCodePointToName[glyph.Codepoint];
											if (IfCatchedByFilters(vFontInfos, name))
											{
												uint32_t x = idx % glyphCountX;

												if (x) ImGui::SameLine();

												if (showRangeColoring)
												{
													if (glyph.Codepoint != lastGlyphCodePoint + 1)
													{
														glyphRangeColoring = vProjectFile->GetColorFromInteger(glyph.Codepoint);
													}

													ImGui::PushStyleColor(ImGuiCol_Button, glyphRangeColoring);
													ImVec4 bh = glyphRangeColoring; bh.w = 0.8f;
													ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bh);
													ImVec4 ba = glyphRangeColoring; ba.w = 1.0f;
													ImGui::PushStyleColor(ImGuiCol_ButtonActive, ba);
												}

												win->DrawList->ChannelsSetCurrent(1);

												bool selected = false;
												SelectionHelper::Instance()->IsGlyphIntersectedAndSelected(
													vFontInfos, glyph_size, glyph.Codepoint, &selected,
													SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);

												win->DrawList->ChannelsSetCurrent(0);

												ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
												bool check = GlyphInfos::DrawGlyphButton(vProjectFile, vFontInfos, &selected, glyph_size, glyph, hostTextureSize);
												ImGui::PopStyleVar();

												if (check)
												{
													SelectionHelper::Instance()->SelectWithToolOrApplyOnGlyph(
														vProjectFile, vFontInfos,
														glyph, idx, selected, true,
														SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
												}

												if (showRangeColoring)
												{
													ImGui::PopStyleColor(3);
												}

												if (vProjectFile->m_SourcePane_ShowGlyphTooltip)
												{
													if (ImGui::IsItemHovered())
													{
														ImGui::SetTooltip("name : %s\ncodepoint : %i", name.c_str(), (int)glyph.Codepoint);
													}
												}

												lastGlyphCodePoint = glyph.Codepoint;
												idx++;
											}
										}
									}
								}
							}
							m_Clipper.End();
						}

						SelectionHelper::Instance()->SelectWithToolOrApply(
							vProjectFile, SelectionContainerEnum::SELECTION_CONTAINER_SOURCE);
					}
				}
			}
		}

		win->DrawList->ChannelsMerge();
	}
}

void SourceFontPane::DrawFontTexture(std::shared_ptr<FontInfos> vFontInfos)
{
	if (vFontInfos)
	{
		if (vFontInfos->m_ImFontAtlas.IsBuilt())
		{
			if (vFontInfos->m_ImFontAtlas.TexID)
			{
				if (ImGui::BeginMenuBar())
				{
					if (ImGui::MenuItem("Save to File"))
					{
						ImGuiFileDialog::Instance()->OpenModal("SaveFontToPictureFile", "Svae Font Testure to File", ".png", 
							".", 0, IGFD::UserDatas(&vFontInfos->m_ImFontAtlas), ImGuiFileDialogFlags_ConfirmOverwrite);
					}

					ImGui::EndMenuBar();
				}

				float w = ImGui::GetContentRegionAvail().x;
				float h = w * (float)vFontInfos->m_ImFontAtlas.TexHeight;
				if (vFontInfos->m_ImFontAtlas.TexWidth > 0)
					h /= (float)vFontInfos->m_ImFontAtlas.TexWidth;
				ImGui::Image(vFontInfos->m_ImFontAtlas.TexID, 
					ImVec2(w, h), ImVec2(0, 0), ImVec2(1, 1), 
					ImGui::GetStyleColorVec4(ImGuiCol_Text));
			}
		}
	}
}

std::string SourceFontPane::getXml(const std::string& vOffset, const std::string& vUserDatas)
{
	UNUSED(vUserDatas);

	std::string str;

	str += vOffset + "<sourcefontpane>\n";

	//str += vOffset + "\t<glyphsizepolicy_count>" + ct::toStr(m_GlyphSize_Policy_Count) + "</glyphsizepolicy_count>\n";
	//str += vOffset + "\t<glyphsizepolicy_width>" + ct::toStr(m_GlyphSize_Policy_Width) + "</glyphsizepolicy_width>\n";

	str += vOffset + "</sourcefontpane>\n";

	return str;
}

bool SourceFontPane::setFromXml(tinyxml2::XMLElement* vElem, tinyxml2::XMLElement* vParent, const std::string& vUserDatas)
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

	if (strParentName == "sourcefontpane")
	{
		/*if (strName == "glyphsizepolicy_count")
			m_GlyphSize_Policy_Count = ct::ivariant(strValue).GetI();
		else if (strName == "glyphsizepolicy_width")
			m_GlyphSize_Policy_Width = ct::fvariant(strValue).GetF();*/
	}

	return true;
}
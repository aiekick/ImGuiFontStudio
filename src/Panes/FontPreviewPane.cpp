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

#include <Generator/FontGenerator.h>
#include <MainFrame.h>
#include <Panes/Manager/LayoutManager.h>
#include <Gui/ImGuiWidgets.h>
#ifdef _DEBUG
#include <Panes/DebugPane.h>
#endif
#include <Panes/ParamsPane.h>
#include <Generator/FontGenerator.h>
#include <Panes/GlyphPane.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <sfntly/font_factory.h>
#include <Gui/ImGuiWidgets.h>
#include <Helper/SelectionHelper.h>
#include <Project/GlyphInfos.h>
#include <Helper/Profiler.h>

FontPreviewPane::FontPreviewPane() = default;
FontPreviewPane::~FontPreviewPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// STATIC ///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

static uint32_t _TextCursorPos = 0;
static int InputTextCallback(ImGuiInputTextCallbackData * vData)
{
	ZoneScoped;

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
	ZoneScoped;
}

void FontPreviewPane::Unit()
{
	ZoneScoped;
}

int FontPreviewPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	ZoneScoped;

	paneWidgetId = vWidgetId;

	DrawFontPreviewPane(vProjectFile);

	return paneWidgetId;
}

void FontPreviewPane::DrawDialogsAndPopups(ProjectFile* /*vProjectFile*/)
{
	ZoneScoped;
}

int FontPreviewPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	ZoneScoped;

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
ca va nous afficher les box qui vont contenir les characteres
et on va pouvoir choisir un glyph dans ces boites, ce qui nous affichera en dessous le resultat
et il faudra pouvoir scale/translate le glyph
*/

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void FontPreviewPane::DrawFontPreviewPane(ProjectFile *vProjectFile)
{
	ZoneScoped;

	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_FONT_PREVIEW)
	{
		if (ImGui::Begin<PaneFlags>(FONT_PREVIEW_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_FONT_PREVIEW,
			//ImGuiWindowFlags_NoTitleBar |
			//ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				ImGui::Text("Select glyphs to test in Final Pane");
				
				ImGui::Text("Current Selection");

				ImGui::Text("Left click for insert the font icon at the pos into the test string");
				
				bool change = false;
				ImVec2 cell_size, glyph_size;
				uint32_t glyphCountX = GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);
				if (glyphCountX)
				{
					auto font = ImGui::GetFont();
					if (font)
					{
						auto sel = SelectionHelper::Instance()->GetSelection();
						size_t idx = 0;
						for (auto glyph : *sel)
						{
							if (glyph.second)
							{
								uint32_t x = idx % glyphCountX;

								if (glyph.second->m_SelectedGlyphs.find(glyph.first) != glyph.second->m_SelectedGlyphs.end())
								{
									auto glyphInfos = glyph.second->m_SelectedGlyphs[glyph.first];
									if (glyphInfos)
									{
										if (x) ImGui::SameLine();

										if (GlyphInfos::DrawGlyphButton(paneWidgetId, vProjectFile, glyph.second, 0, glyph_size, &glyphInfos->glyph) == 1) // left
										{
											vProjectFile->m_FontTestInfos.m_GlyphToInsert[_TextCursorPos] = glyph;
											vProjectFile->SetProjectChange();
											GlyphPane::Instance()->LoadGlyph(vProjectFile, glyph.second, glyphInfos);
											change = true;
										}
									}
								}

								idx++;
							}
						}
					}
				}
				
				ImGui::Separator();
				
				ImGui::Text("Test :");
				if (ImGui::Button("Clear##testfont"))
				{
					vProjectFile->m_FontTestInfos.m_TestFont.reset();
					vProjectFile->m_FontTestInfos.m_TestFontName = "";
					vProjectFile->SetProjectChange();
				}
				ImGui::SameLine();
				if (ImGui::Button("Use the selected font"))
				{
					vProjectFile->m_FontTestInfos.m_TestFont = vProjectFile->m_SelectedFont;
					vProjectFile->m_FontTestInfos.m_TestFontName = vProjectFile->m_SelectedFont->m_FontFileName;
					vProjectFile->SetProjectChange();
				}
				
				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
				ImGui::PushID(paneWidgetId++);
				change |= ImGui::InputText("##ImGuiFontStudio", vProjectFile->m_FontTestInfos.m_InputBuffer, 499,
					ImGuiInputTextFlags_CallbackAlways | ImGuiInputTextFlags_CallbackCharFilter,
					&InputTextCallback);
				ImGui::PopID();
				ImGui::PopItemWidth();

				if (change)
				{
					std::string bufferString = vProjectFile->m_FontTestInfos.m_InputBuffer;
					if (bufferString.size() < vProjectFile->m_FontTestInfos.m_TestString.size())
					{
						vProjectFile->m_FontTestInfos.ResizeInsertedGlyphs(_TextCursorPos, false);
					}
					else if (bufferString.size() > vProjectFile->m_FontTestInfos.m_TestString.size())
					{
						vProjectFile->m_FontTestInfos.ResizeInsertedGlyphs(_TextCursorPos, true);
					}
					vProjectFile->m_FontTestInfos.m_TestString = vProjectFile->m_FontTestInfos.m_InputBuffer;
					vProjectFile->SetProjectChange();
				}

				DrawMixerWidget(vProjectFile);

				DrawMixedFontResult(vProjectFile);
			}
		}

		ImGui::End();
	}
}

// mixer widget
void FontPreviewPane::DrawMixerWidget(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (vProjectFile->m_FontTestInfos.m_TestString.empty())
	{
		ImGui::Text("You need to input a test text first");
		return;
	}

	if (ImGui::Button("Clear Glyphs##glyphselection"))
	{
		vProjectFile->m_FontTestInfos.m_GlyphToInsert.clear();
		vProjectFile->SetProjectChange();
	}

	ImGui::Text("Left/Right click for select box where to insert Icon\nRight click on button for remove the font Icon");
	
	if (!vProjectFile->m_FontTestInfos.m_TestFont.expired())
	{
		auto fontPtr = vProjectFile->m_FontTestInfos.m_TestFont.lock();
		if (fontPtr.use_count())
		{
			ImVec2 cell_size, glyph_size;
			GlyphDisplayHelper::CalcGlyphsCountAndSize(vProjectFile, &cell_size, &glyph_size);

			ImVec2 basePos = ImGui::GetCursorPos();

			ImGuiWindow* window = ImGui::GetCurrentWindow();
			if (window)
			{
				if (window->SkipItems)
					return;

				ImGuiContext& g = *GImGui;
				const ImGuiStyle& style = g.Style;

				auto defaultGlyph = fontPtr->GetFirstGlyphByCodePoint((CodePoint)' '); // putain ! c'est un const
				if (defaultGlyph)
				{
					uint32_t count = (uint32_t)vProjectFile->m_FontTestInfos.m_TestString.size();
					for (uint32_t idx = 0; idx <= count; idx++)
					{
						if (idx)
						{
							ImGui::SameLine();
						}

						ImVec2 off = ImGui::GetCursorScreenPos();
						bool selected = (_TextCursorPos == idx);
						int check = 0;
						bool found = false;
						if (vProjectFile->m_FontTestInfos.m_GlyphToInsert.find(idx) != vProjectFile->m_FontTestInfos.m_GlyphToInsert.end())
						{
							auto glyphInsert = &vProjectFile->m_FontTestInfos.m_GlyphToInsert[idx];
							if (glyphInsert->second.use_count() &&
								glyphInsert->second->m_SelectedGlyphs.find(glyphInsert->first) != glyphInsert->second->m_SelectedGlyphs.end())
							{
								const auto glyphInfos = glyphInsert->second->m_SelectedGlyphs[glyphInsert->first];
								if (glyphInfos.use_count())
								{
									const auto glyph = &glyphInfos->glyph;
									ct::fvec2 trans = glyphInfos->m_Translation * glyphInsert->second->m_Point;
									ct::fvec2 scale = glyphInfos->m_Scale;
									check = GlyphInfos::DrawGlyphButton(paneWidgetId, vProjectFile,
										glyphInsert->second, &selected,
										glyph_size, glyph, -1,
										ImVec2(trans.x, trans.y),
										ImVec2(scale.x, scale.y));
									found = true;
								}
							}
						}

						if (!found)
						{
							check = GlyphInfos::DrawGlyphButton(paneWidgetId, vProjectFile, fontPtr, &selected, glyph_size, defaultGlyph);
						}

						if (check)
						{
							_TextCursorPos = idx;

							if (check == 2)
							{
								vProjectFile->m_FontTestInfos.m_GlyphToInsert.erase(_TextCursorPos);
								vProjectFile->SetProjectChange();
								GlyphPane::Instance()->Clear();
							}
						}

						// draw accolade
						off.y += cell_size.y;
						ImVec2 a = ImVec2(off.x, off.y);
						ImVec2 b = ImVec2(off.x + cell_size.x - style.FramePadding.x, off.y + cell_size.y * 0.25f);
						ImVec2 c = ImVec2(off.x + cell_size.x * 0.5f - style.FramePadding.x * 0.5f, off.y + cell_size.y * 0.5f - style.FramePadding.x * 0.5f);
						window->DrawList->AddBezierCubic(ImVec2(a.x, a.y), ImVec2(a.x, c.y), ImVec2(c.x, a.y), ImVec2(c.x, c.y), ImGui::GetColorU32(ImGuiCol_Text), 2.0f);
						window->DrawList->AddBezierCubic(ImVec2(b.x, a.y), ImVec2(b.x, c.y), ImVec2(c.x, a.y), ImVec2(c.x, c.y), ImGui::GetColorU32(ImGuiCol_Text), 2.0f);
					}

					ImVec2 newPos = ImGui::GetCursorPos();
					newPos += cell_size * 0.5f;
					ImGui::SetCursorPos(newPos);

					for (uint32_t idx = 0; idx < count; idx++)
					{
						ImWchar c = vProjectFile->m_FontTestInfos.m_TestString[idx];
						auto glyph = fontPtr->GetFirstGlyphByCodePoint((CodePoint)c);
						if (glyph)
						{
							if (idx)
							{
								ImGui::SameLine();
							}

							bool selected = false;
							GlyphInfos::DrawGlyphButton(paneWidgetId, vProjectFile, fontPtr, &selected, glyph_size, glyph);
						}
					}
				}
			}
		}
	}
}

// final test
void FontPreviewPane::DrawMixedFontResult(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (!vProjectFile->m_FontTestInfos.m_TestFont.expired())
	{
		auto fontPtr = vProjectFile->m_FontTestInfos.m_TestFont.lock();
		if (fontPtr.use_count())
		{
			ImFont* font = fontPtr->GetImFontPtr();
			if (font)
			{
				bool change = false;
				change |= ImGui::RadioButtonLabeled(0.0f, "Base Line", "Show/Hide base line", &vProjectFile->m_FontTestInfos.m_ShowBaseLine);
				ImGui::SameLine();
				change |= ImGui::SliderFloatDefaultCompact(ImGui::GetContentRegionAvail().x, "Preview Size", &vProjectFile->m_FontTestInfos.m_PreviewFontSize, 1, 300, font->FontSize);

				if (change)
				{
					vProjectFile->SetProjectChange();
				}

				float testFontScale = vProjectFile->m_FontTestInfos.m_PreviewFontSize / font->FontSize;
				float testFontAscent = font->Ascent * testFontScale;

				// ici il va falloir afficher les glyphs au bon endroit dans la string m_TestSentense
				float offsetX = 0.0f;

				float aw = ImGui::GetContentRegionAvail().x;
				ImU32 colFont = ImGui::GetColorU32(ImGuiCol_Text);

				ImGuiWindow* window = ImGui::GetCurrentWindow();
				if (window->SkipItems)
					return;

				ImGuiID id = window->GetID("#MixedFontDisplay");

				if (font->ContainerAtlas)
				{
					float baseFontRatioX = 1.0f;
					if (font->ContainerAtlas->TexHeight > 0)
						baseFontRatioX = (float)font->ContainerAtlas->TexWidth / (float)font->ContainerAtlas->TexHeight;

					ImVec2 pos = window->DC.CursorPos;
					ImVec2 size = ImVec2(aw, vProjectFile->m_FontTestInfos.m_PreviewFontSize);
					const ImRect bb(pos, pos + size);
					ImGui::ItemSize(bb);
					if (!ImGui::ItemAdd(bb, id))
						return;

					uint32_t count = (uint32_t)vProjectFile->m_FontTestInfos.m_TestString.size();
					for (uint32_t idx = 0; idx <= count; idx++)
					{
						if (vProjectFile->m_FontTestInfos.m_GlyphToInsert.find(idx) !=
							vProjectFile->m_FontTestInfos.m_GlyphToInsert.end())
						{
							// on dessin le glyph
							auto glyphInfos = &vProjectFile->m_FontTestInfos.m_GlyphToInsert[idx];
							if (glyphInfos->second)
							{
								if (glyphInfos->second->m_SelectedGlyphs.find(glyphInfos->first) != glyphInfos->second->m_SelectedGlyphs.end())
								{
									ImFont* glyphFont = glyphInfos->second->GetImFontPtr();
									if (glyphFont && glyphFont->ContainerAtlas)
									{
										float scale = (font->FontSize / glyphInfos->second->m_FontSize) * (vProjectFile->m_FontTestInfos.m_PreviewFontSize / font->FontSize);
										float glyphFontAscent = glyphFont->Ascent * scale;
										float ascOffset = glyphFontAscent - testFontAscent;

										if (glyphInfos->second->m_SelectedGlyphs[glyphInfos->first])
										{
											ct::fvec2 trans = glyphInfos->second->m_SelectedGlyphs[glyphInfos->first]->m_Translation * glyphInfos->second->m_Point * scale;

											auto glyph = glyphInfos->second->m_SelectedGlyphs[glyphInfos->first]->glyph;
											ImVec2 pMin = ImVec2(pos.x + offsetX + trans.x, pos.y - ascOffset - trans.y);

											ImTextureID texId = (ImTextureID)glyphFont->ContainerAtlas->TexID;

											window->DrawList->PushTextureID(texId);
											glyphFont->RenderChar(
												window->DrawList, vProjectFile->m_FontTestInfos.m_PreviewFontSize,
												pMin, colFont, (ImWchar)glyphInfos->first);
											window->DrawList->PopTextureID();
											offsetX += glyph.AdvanceX * scale;
										}
									}
								}
							}
						}

						if (idx < count)
						{
							CodePoint c = vProjectFile->m_FontTestInfos.m_TestString[idx];
							auto g = fontPtr->GetFirstGlyphByCodePoint(c);
							if (g)
							{
								c = g->glyphIndex;
								auto glyph = font->FindGlyph(c);
								if (glyph && font->ContainerAtlas)
								{
									ImVec2 pMin = ImVec2(pos.x + offsetX, pos.y);
									ImTextureID texId = (ImTextureID)font->ContainerAtlas->TexID;
									window->DrawList->PushTextureID(texId);
									font->RenderChar(window->DrawList, vProjectFile->m_FontTestInfos.m_PreviewFontSize, pMin, colFont, (ImWchar)c);
									window->DrawList->PopTextureID();
									offsetX += glyph->AdvanceX * testFontScale;
								}
							}
						}

						if (vProjectFile->m_FontTestInfos.m_ShowBaseLine)
						{
							// Base Line
							float asc = font->Ascent * testFontScale;
							window->DrawList->AddLine(ImVec2(bb.Min.x, bb.Min.y + asc), ImVec2(bb.Max.x, bb.Min.y + asc), ImGui::GetColorU32(ImGuiCol_PlotHistogram), 1.0f); // base line
						}
					}
				}
			}
		}
	}
}

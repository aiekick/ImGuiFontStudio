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

#include "GlyphPane.h"

#include <MainFrame.h>

#include <Panes/Manager/LayoutManager.h>
#include <Gui/ImWidgets.h>
#ifdef _DEBUG
#include <Panes/DebugPane.h>
#endif

#include <Helper/MeshDecomposer.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>

#include <ctools/cTools.h>
#include <ctools/FileHelper.h>
#include <Helper/Profiler.h>
#include <sfntly/font_factory.h>

#include <Helper/Messaging.h>

GlyphPane::GlyphPane() = default;
GlyphPane::~GlyphPane() = default;

///////////////////////////////////////////////////////////////////////////////////
//// OVERRIDES ////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

void GlyphPane::Init()
{
	ZoneScoped;
}

void GlyphPane::Unit()
{
	ZoneScoped;
}

int GlyphPane::DrawPanes(ProjectFile * vProjectFile, int vWidgetId)
{
	ZoneScoped;

	paneWidgetId = vWidgetId;

	DrawGlyphPane(vProjectFile);

	return paneWidgetId;
}

void GlyphPane::DrawDialogsAndPopups(ProjectFile* /*vProjectFile*/)
{
	ZoneScoped;
}

int GlyphPane::DrawWidgets(ProjectFile* vProjectFile, int vWidgetId, std::string vUserDatas)
{
	ZoneScoped;

	UNUSED(vUserDatas);
	
	if (vProjectFile && vProjectFile->IsLoaded())
	{
		if (LayoutManager::Instance()->IsSpecificPaneFocused(PaneFlags::PANE_GLYPH))
		{
			if (ImGui::BeginFramedGroup("Glyph Edition"))
			{
				const float maxWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x;
				const float mrw = maxWidth * 0.5f;

				bool change = false;

				if (!m_GlyphToDisplay.expired())
				{
					auto glyphInfosPtr = m_GlyphToDisplay.lock();
					if (glyphInfosPtr.use_count())
					{
						if (!glyphInfosPtr->GetFontInfos().expired())
						{
							auto fontInfosPtr = glyphInfosPtr->GetFontInfos().lock();
							if (fontInfosPtr)
							{
								if (!glyphInfosPtr->simpleGlyph.m_Layers.empty())
								{
									for (auto& layer : glyphInfosPtr->simpleGlyph.m_Layers)
									{
										int res = GlyphInfos::DrawGlyphButton(vWidgetId, vProjectFile, fontInfosPtr, &layer.second, 
											ImVec2(maxWidth, 30), &layer.first, glyphInfosPtr->glyph.glyphIndex);
										if (res == 2) // right click
										{

										}

										if (ImGui::IsItemHovered())
										{
											ImGui::SetTooltip("Count Parents %u", (uint32_t)layer.first.parents.size());
										}
									}
								}
								else
								{
									int res = GlyphInfos::DrawGlyphButton(vWidgetId, vProjectFile, fontInfosPtr, 0, 
										ImVec2(maxWidth, 30), &glyphInfosPtr->simpleGlyph.m_Glyph, glyphInfosPtr->glyph.glyphIndex);
									if (res == 2) // right click
									{

									}

									if (ImGui::IsItemHovered())
									{
										ImGui::SetTooltip("Count Parents %u", (uint32_t)glyphInfosPtr->simpleGlyph.m_Glyph.parents.size());
									}
								}
							}
						}
					}
				}

				ImGui::EndFramedGroup();
			}
		}
	}

	return vWidgetId;
}

///////////////////////////////////////////////////////////////////////////////////
//// PRIVATE //////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

static ProjectFile _DefaultProjectFile;
void GlyphPane::DrawGlyphPane(ProjectFile *vProjectFile)
{
	ZoneScoped;

	if (LayoutManager::m_Pane_Shown & PaneFlags::PANE_GLYPH)
	{
		if (ImGui::Begin<PaneFlags>(GLYPH_PANE,
			&LayoutManager::m_Pane_Shown, PaneFlags::PANE_GLYPH,
			//ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_MenuBar |
			//ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoCollapse |
			//ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoBringToFrontOnFocus))
		{
			if (vProjectFile &&  vProjectFile->IsLoaded())
			{
				static float _ZoomPrecisionRatio = 1.0f / vProjectFile->m_GlyphPreviewZoomPrecision;

				if (ImGui::BeginMenuBar())
				{
					ImGui::Text("(?)");
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip(
"\
You can select a glyph :\n\
 - click on a glyph in Selected Font Pane\n\
 - right click on a glyph in final pane\n\
 - click on a glyph in font preview pane\n\
You can Re Scale with mouse Wheel\
");
					}

					if (ImGui::BeginMenu("Global Scale"))
					{
						if (ImGui::SliderFloatDefaultCompact(300.0f, "Scale Precision", &vProjectFile->m_GlyphPreviewZoomPrecision,
							1.0f, 2000.0f, _DefaultProjectFile.m_GlyphPreviewZoomPrecision))
						{
							vProjectFile->m_GlyphPreviewZoomPrecision = ImMax(vProjectFile->m_GlyphPreviewZoomPrecision, 1.0f);
							_ZoomPrecisionRatio = 1.0f / vProjectFile->m_GlyphPreviewZoomPrecision;
						}

						if (ImGui::SliderFloatDefaultCompact(300.0f, "Scale", &vProjectFile->m_GlyphPreview_Scale,
							0.01f, 2.0f, _DefaultProjectFile.m_GlyphPreview_Scale))
						{
							vProjectFile->SetProjectChange();
						}

						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}

				// Re Scale with mouse wheel
				if (ImGui::IsWindowHovered())
				{
					if (IS_FLOAT_DIFFERENT(ImGui::GetIO().MouseWheel, 0.0f))
					{
						vProjectFile->m_GlyphPreview_Scale += ImGui::GetIO().MouseWheel * _ZoomPrecisionRatio;
						vProjectFile->m_GlyphPreview_Scale = ImMax(vProjectFile->m_GlyphPreview_Scale, 0.00001f);
					}
				}

				DrawSimpleGlyph(vProjectFile);
			}
		}

		ImGui::End();
	}
}

///////////////////////////////////////////////////////////////////////////////////
//// LOAD GLYPH ///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer

static int limitContour = 0;
static int maxContour = 0;
static int limitPoint = 0;
static int maxPoint = 0;

bool GlyphPane::LoadGlyph(ProjectFile* vProjectFile, std::shared_ptr<FontInfos> vFontInfos, std::weak_ptr<GlyphInfos> vGlyphInfos)
{
	ZoneScoped;

	bool res = false;

	if (vFontInfos.use_count() && !vGlyphInfos.expired())
	{
		auto glyphInfosPtr = vGlyphInfos.lock();
		if (glyphInfosPtr.use_count())
		{
			if (glyphInfosPtr->glyph.category & GLYPH_CATEGORY_FLAG_SIMPLE)
			{
				m_GlyphToDisplay = vGlyphInfos;
				glyphInfosPtr->simpleGlyph.LoadGlyph(&glyphInfosPtr->glyph, vProjectFile);
				glyphInfosPtr->simpleGlyph.m_Translation = glyphInfosPtr->m_Translation;
				glyphInfosPtr->simpleGlyph.m_Scale = glyphInfosPtr->m_Scale;
				limitContour = maxContour = glyphInfosPtr->simpleGlyph.GetMaxContours();
				limitPoint = maxPoint = glyphInfosPtr->simpleGlyph.GetMaxPoints();
			}
			else if (glyphInfosPtr->glyph.category & GLYPH_CATEGORY_FLAG_COMPOSITE)
			{
				m_GlyphToDisplay.reset();

				// show and active the glyph pane
				LayoutManager::Instance()->ShowAndFocusSpecificPane(PaneFlags::PANE_GLYPH);

				Messaging::Instance()->AddWarning(true, nullptr, nullptr,
					"Composite glyph drawing is not supported for the moment");
			}

#ifdef _DEBUG
			DebugPane::Instance()->SetGlyphToDebug(m_GlyphToDisplay);
#endif
			// show and active the glyph pane
			LayoutManager::Instance()->ShowAndFocusSpecificPane(PaneFlags::PANE_GLYPH);

			res = true;
		}
	}

	return res;
}

void GlyphPane::Clear()
{
	ZoneScoped;

	if (!m_GlyphToDisplay.expired())
	{
		auto glyphInfosPtr = m_GlyphToDisplay.lock();
		if (glyphInfosPtr.use_count())
		{
			glyphInfosPtr->simpleGlyph.Clear();
		}
	}
	m_GlyphToDisplay.reset();
	limitContour = 0;

#ifdef _DEBUG
	DebugPane::Instance()->Clear();
#endif
}

// https://github.com/rillig/sfntly/tree/master/java/src/com/google/typography/font/tools/fontviewer
bool GlyphPane::DrawSimpleGlyph(ProjectFile* vProjectFile)
{
	ZoneScoped;

	if (!m_GlyphToDisplay.expired())
	{
		auto glyphInfosPtr = m_GlyphToDisplay.lock();
		if (glyphInfosPtr.use_count())
		{
			auto g = &glyphInfosPtr->simpleGlyph;
			if (g->isValid)
			{
				auto fontInfos = glyphInfosPtr->GetFontInfos();
				if (!fontInfos.expired())
				{
					auto fontInfosPtr = fontInfos.lock();
					if (fontInfosPtr.use_count())
					{
						ct::ivec4 rc = g->m_Glyph.bbox;
						bool change = false;
						bool needReMesh = false;
						float aw = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x * 5.0f) * 0.5f;
						
						needReMesh |= ImGui::SliderIntDefaultCompact(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x, "Curve Segments", &vProjectFile->m_GlyphPreview_QuadBezierCountSegments,
							0, 12, _DefaultProjectFile.m_GlyphPreview_QuadBezierCountSegments);
						//ImGui::SameLine();
						//change |= ImGui::SliderIntDefaultCompact(aw, "Count Curves", &limitContour, 1, maxContour, maxContour);
						//limitContour = ct::mini(limitContour, maxContour);

						//change |= ImGui::SliderIntDefaultCompact(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x, "Count Point", &limitPoint, 1, maxPoint, maxPoint);
						//limitPoint = ct::mini(limitPoint, maxPoint);

						change |= ImGui::SliderFloatDefaultCompact(aw, "Trans X", &glyphInfosPtr->m_Translation.x, (float)-rc.z, (float)rc.z, 0.0f);
						ImGui::SameLine();
						change |= ImGui::SliderFloatDefaultCompact(aw, "Trans Y", &glyphInfosPtr->m_Translation.y, (float)-rc.w, (float)rc.w, 0.0f);
#ifdef _DEBUG
						change |= ImGui::SliderFloatDefaultCompact(aw, "Scale X", &glyphInfosPtr->m_Scale.x, 0.01f, 3.0f, 1.0f);
						ImGui::SameLine();
						change |= ImGui::SliderFloatDefaultCompact(aw, "Scale Y", &glyphInfosPtr->m_Scale.y, 0.01f, 3.0f, 1.0f);
#endif

						ImGui::Separator();

						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Grid", "Show/Hide Canvas Grid", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_CANVAS_GRID); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Legends", "Show/Hide Legends", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_LEGENDS); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Axis X", "Show/Hide Font Axis X", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_FONT_AXIS_X); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Axis Y", "Show/Hide Font Axis Y", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_FONT_AXIS_Y); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Origin Point", "Show/Hide Font Zero Point", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_FONT_ORIGIN_XY); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Control Lines", "Show/Hide Glyph Control Lines X", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_GLYPH_CONTROL_LINES);
						
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Font BBox", "Show/Hide Font Bounding Box", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_FONT_BBOX); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Glyph BBox", "Show/Hide Glyph Bounidng Box", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_GLYPH_BBOX); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Ascent", "Show/Hide Font Ascent", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_FONT_ASCENT); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Descent", "Show/Hide Font Descent", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_FONT_DESCENT); ImGui::SameLine();
						change |= ImGui::RadioButtonLabeled_BitWize<GlyphDrawingFlags>(0.0f, "Advance X", "Show/Hide Glyph Advance X", &vProjectFile->m_GlyphDrawingFlags, GLYPH_DRAWING_GLYPH_ADVANCEX);
						
						ImGui::Separator();

						needReMesh |= ImGui::RadioButtonLabeled_BitWize<PartitionAlgoFlags>(0.0f, "EC", "Apply Ear Clipping Poly Partition Algo", &vProjectFile->m_PartitionAlgo, PARTITION_ALGO_EAR_CLIPPING, false, true, PARTITION_ALGO_RADIO); ImGui::SameLine();
						needReMesh |= ImGui::RadioButtonLabeled_BitWize<PartitionAlgoFlags>(0.0f, "Mono", "Apply Monotone Poly Partition Algo", &vProjectFile->m_PartitionAlgo, PARTITION_ALGO_MONO, false, true, PARTITION_ALGO_RADIO); ImGui::SameLine();
						needReMesh |= ImGui::RadioButtonLabeled_BitWize<PartitionAlgoFlags>(0.0f, "HM", "Apply Convex HM Poly Partition Algo", &vProjectFile->m_PartitionAlgo, PARTITION_ALGO_CONVEX_HM, false, true, PARTITION_ALGO_RADIO);

						if (change)
						{
							// will come back with svg or/and glyph edition
							g->m_Translation = glyphInfosPtr->m_Translation; // wip
							g->m_Scale = glyphInfosPtr->m_Scale;// for the moment scale is overwrite by merged system
							vProjectFile->SetProjectChange();
						}

						if (needReMesh)
						{
							g->ComputeWholeGlyphMesh(vProjectFile->m_GlyphPreview_QuadBezierCountSegments, vProjectFile->m_PartitionAlgo);
							vProjectFile->SetProjectChange();
						}

						ct::ivec4 frc = fontInfosPtr->m_BoundingBox;

						ImGui::Text("Font BBox : %i %i %i %i | Glyph BBox : %i %i %i %i", frc.x, frc.y, frc.z, frc.w, rc.x, rc.y, rc.z, rc.w);

						ImGui::Separator();

						g->DrawGlyph(
							vProjectFile->m_GlyphPreview_Scale,
							fontInfosPtr,
							glyphInfosPtr,
							limitPoint, limitContour,
							vProjectFile->m_GlyphPreview_QuadBezierCountSegments,
							vProjectFile->m_GlyphDrawingFlags);
					}
				}
			}
		}
	}

	return true;
}

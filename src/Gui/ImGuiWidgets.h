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
#pragma once

#include <imgui/imgui.h>

#include <ctools/cTools.h>

#include <cstdarg> // variadic
#include <cstdint> // types like uint32_t

struct ImGuiWindow;

namespace ImGui
{
	IMGUI_API void AddInvertedRectFilled(ImDrawList* vDrawList, const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawCornerFlags rounding_corners);
	IMGUI_API void RenderInnerShadowFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, ImU32 fill_col_darker, ImU32 bg_Color, bool border, float rounding);
	IMGUI_API void DrawShadowImage(ImTextureID vShadowImage, const ImVec2& vSize, ImU32 col);

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IMGUI_API bool SelectableWithBtn(const char* label, bool selected = false, const char* btnLabel = 0, bool *btnClicked = 0, bool *btnHovered = 0, ImVec4 vBtnColor = ImVec4(0.8f, 0.5f, 0.2f, 1.0f), ImVec4 vBtnHoveredColor = ImVec4(1.0f, 0.8f, 0.2f, 1.0f), ImGuiSelectableFlags flags = 0, const ImVec2& size_args = ImVec2(0.0f, 0.0f));
	IMGUI_API bool ImageCheckButton(ImTextureID user_texture_id, bool *v, const ImVec2& size, const ImVec2& uv0 = ImVec2(0.0f, 0.0f), const ImVec2& uv1 = ImVec2(1.0f, 1.0f), const ImVec2& vHostTextureSize = ImVec2(0.0f, 0.0f), int frame_padding = -1, float vRectThickNess = 0.0f, ImVec4 vRectColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
	IMGUI_API bool BeginFramedGroup(const char *vLabel, bool vSpacing = false, ImVec4 vCol = ImVec4(0.0f, 0.0f, 0.0f, 0.5f), ImVec4 vHoveredCol = ImVec4(0.15f, 0.15f, 0.15f, 0.5f));
	IMGUI_API void EndFramedGroup(bool vSpacing = true);
	IMGUI_API void FramedGroupSeparator();
	IMGUI_API void FramedGroupText(const char* vFmt, ...);
	IMGUI_API void FramedGroupText(ImVec4 vTextColor, const char* vFmt, ...);
	IMGUI_API bool CollapsingHeader_SmallHeight(const char *vName, float vHeightRatio, float vWidth, bool vDefaulExpanded, bool *vIsOpen = 0);
	IMGUI_API bool RadioButtonLabeled(float vWidth, const char* label, bool active, bool disabled);
	IMGUI_API bool RadioButtonLabeled(float vWidth, const char* label, const char* help, bool active, bool disabled = false);
	IMGUI_API bool RadioButtonLabeled(float vWidth, const char* label, const char* help, bool *active, bool disabled = false);
	/*template<typename T>
	IMGUI_API bool RadioButtonLabeled_BitWize(
		const char *vLabel, const char *vHelp, T *vContainer, T vFlag,
		float vWidth = 0.0f,
		bool vOneOrZeroAtTime = false, //only one selcted at a time
		bool vAlwaysOne = true, // radio behavior, always one selected
		T vFlagsToTakeIntoAccount = (T)0,
		T vFlagForDisableSelection = (T)0) // radio witl use only theses flags
	{
		bool disabled = *vContainer & vFlagForDisableSelection;
		return RadioButtonLabeled_BitWize(
			vLabel, vHelp, vContainer, vFlag, vWidth, 
			vOneOrZeroAtTime, vAlwaysOne, vFlagsToTakeIntoAccount, disabled);
	}*/
	template<typename T> 
	IMGUI_API bool RadioButtonLabeled_BitWize(
		float vWidth,
		const char *vLabel, const char *vHelp, T *vContainer, T vFlag,
		bool vOneOrZeroAtTime = false, //only one selcted at a time
		bool vAlwaysOne = true, // radio behavior, always one selected
		T vFlagsToTakeIntoAccount = (T)0,
		bool vDisableSelection = false) // radio witl use only theses flags
	{
		bool selected = *vContainer & vFlag;
		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		bool res = RadioButtonLabeled(vWidth, vLabel, vHelp, &selected, vDisableSelection);
		//ImGui::PopStyleVar();
		if (res)
		{
			if (selected)
			{
				if (vOneOrZeroAtTime)
				{
					if (vFlagsToTakeIntoAccount)
					{
						if (vFlag & vFlagsToTakeIntoAccount)
						{
							*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
							*vContainer = (T)(*vContainer | vFlag); // add
						}
					}
					else *vContainer = vFlag; // set
				}
				else
				{
					if (vFlagsToTakeIntoAccount)
					{
						if (vFlag & vFlagsToTakeIntoAccount)
						{
							*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
							*vContainer = (T)(*vContainer | vFlag); // add
						}
					}
					else *vContainer = (T)(*vContainer | vFlag); // add
				}
			}
			else
			{
				if (vOneOrZeroAtTime)
				{
					if (!vAlwaysOne) *vContainer = (T)(0); // remove all
				}
				else *vContainer = (T)(*vContainer & ~vFlag); // remove one
			}
		}
		return res;
	}
	template<typename T>
	IMGUI_API bool RadioButtonLabeled_BitWize(
		float vWidth,
		const char* vLabelOK, const char* vLabelNOK, const char* vHelp, T* vContainer, T vFlag,
		bool vOneOrZeroAtTime = false, //only one selcted at a time
		bool vAlwaysOne = true, // radio behavior, always one selected
		T vFlagsToTakeIntoAccount = (T)0,
		bool vDisableSelection = false) // radio witl use only theses flags
	{
		bool selected = *vContainer & vFlag;
		const char* label = (selected ? vLabelOK : vLabelNOK);
		//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		bool res = RadioButtonLabeled(vWidth, label, vHelp, &selected, vDisableSelection);
		//ImGui::PopStyleVar();
		if (res)
		{
			if (selected)
			{
				if (vOneOrZeroAtTime)
				{
					if (vFlagsToTakeIntoAccount)
					{
						if (vFlag & vFlagsToTakeIntoAccount)
						{
							*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
							*vContainer = (T)(*vContainer | vFlag); // add
						}
					}
					else *vContainer = vFlag; // set
				}
				else
				{
					if (vFlagsToTakeIntoAccount)
					{
						if (vFlag & vFlagsToTakeIntoAccount)
						{
							*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
							*vContainer = (T)(*vContainer | vFlag); // add
						}
					}
					else *vContainer = (T)(*vContainer | vFlag); // add
				}
			}
			else
			{
				if (vOneOrZeroAtTime)
				{
					if (!vAlwaysOne) *vContainer = (T)(0); // remove all
				}
				else *vContainer = (T)(*vContainer & ~vFlag); // remove one
			}
		}
		return res;
	}
	template<typename T> 
	IMGUI_API bool MenuItem(
		const char* label, const char *shortcut, T *vContainer, T vFlag, bool vOnlyOneSameTime = false)
	{
		bool selected = *vContainer & vFlag;
		bool res = MenuItem(label, shortcut, &selected, true);
		if (res)
		{
			if (selected)
			{
				if (vOnlyOneSameTime)
				{
					*vContainer = vFlag; // set
				}
				else
				{
					*vContainer = (T)(*vContainer | vFlag);// add
				}
			}
			else
			{
				if (!vOnlyOneSameTime)
				{
					*vContainer = (T)(*vContainer & ~vFlag); // remove
				}
			}
		}
		return res;
	}
	template<typename T>
	IMGUI_API bool Begin(
		const char* name, T *vContainer, T vFlag, ImGuiWindowFlags flags)
	{
		bool check = *vContainer & vFlag;
		bool res = Begin(name, &check, flags);
		//if (res)
		//{
			// si on fait que quand c'est ouvert, alors on peut pas fermer un onglet qui n'est pas focused
			// alors on le sort de la boucle
			if (check)
				*vContainer = (T)(*vContainer | vFlag); // add
			else
				*vContainer = (T)(*vContainer & ~vFlag); // remove
		//}
		return res;
	}
	IMGUI_API bool ClickableTextUrl(const char* label,const char* url,bool vOnlined = true);
	/*IMGUI_API bool ClickableTextUrl(const char* label,
		const char* url, 
		bool vOnlined = true,
		ImVec4 vColor = ImVec4(0.85f, 0.85f, 0.85f, 1.0f),
		ImVec4 vHoveredColor = ImVec4(0.85f, 0.85f, 0.5f, 1.0f),
		ImVec4 vClickColor = ImVec4(0.85f, 0.85f, 0.0f, 1.0f));
	IMGUI_API bool ClickableTextFile(const char* label, 
		const char* file, 
		bool vOnlined = true,
		ImVec4 vColor = ImVec4(0.85f, 0.85f, 0.85f, 1.0f),
		ImVec4 vHoveredColor = ImVec4(0.85f, 0.85f, 0.5f, 1.0f),
		ImVec4 vClickColor = ImVec4(0.85f, 0.85f, 0.0f, 1.0f));*/
	IMGUI_API void Spacing(float vSpace);
	IMGUI_API ImGuiWindow* GetHoveredWindow();
	IMGUI_API bool BeginMainStatusBar();
	IMGUI_API void EndMainStatusBar();
	IMGUI_API bool Button(const char* label, const char* help);
	IMGUI_API bool Selectable_FramedText(const char* fmt, ...);
	IMGUI_API bool InputText_Validation(const char* label, char* buf, size_t buf_size, 
		const bool *vValidation = 0, const char* vValidationHelp = 0,
		ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
	IMGUI_API void HelpMarker(const char* desc);

	////////////////////////////////////////////////////////////////////////////

	IMGUI_API bool CustomButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = ImGuiButtonFlags_None);

	////////////////////////////////////////////////////////////////////////////

	#ifdef USE_GRADIENT
	IMGUI_API void RenderGradFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_start_col, ImU32 fill_end_col, bool border, float rounding);
	IMGUI_API bool GradButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = ImGuiButtonFlags_None);
	#endif

	////////////////////////////////////////////////////////////////////////////
	
	IMGUI_API bool SliderScalarCompact(float width,
		const char* label, ImGuiDataType data_type, 
		void* p_data, const void* p_min, const void* p_max, 
		const char* format = NULL);
	IMGUI_API bool SliderUIntCompact(float width,
		const char* label, uint32_t* v, uint32_t v_min,
		uint32_t v_max, const char* format = "%d");
	IMGUI_API bool SliderIntCompact(float width,
		const char* label, int* v, int v_min,
		int v_max, const char* format = "%d");
	IMGUI_API bool SliderFloatCompact(float width,
		const char* label, float* v, float v_min,
		float v_max, const char* format = "%.3f");

	IMGUI_API bool SliderScalarDefaultCompact(float width,
		const char* label, ImGuiDataType data_type,
		void* p_data, const void* p_min, const void* p_max,
		const void* p_default, const char* format = NULL);
	IMGUI_API bool SliderUIntDefaultCompact(float width,
		const char* label, uint32_t* v, uint32_t v_min,
		uint32_t v_max, uint32_t v_default, const char* format = "%d");
	IMGUI_API bool SliderIntDefaultCompact(float width,
		const char* label, int* v, int v_min, 
		int v_max, int v_default, const char* format = "%d");
	IMGUI_API bool SliderFloatDefaultCompact(float width,
		const char* label, float* v, float v_min, 
		float v_max, float v_default, const char* format = "%.3f");

	////////////////////////////////////////////////////////////////////////////

	IMGUI_API bool TransparentButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = 0);

	////////////////////////////////////////////////////////////////////////////

	IMGUI_API void PlainImageWithBG(ImTextureID user_texture_id, const ImVec2& size, const ImVec4& bg_col, const ImVec4& tint_col);
}


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

#include <cstdint> // types like uint32_t

#define ImWidgets_VERSION "ImWidgets v1.0"

struct ImGuiWindow;
struct GLFWwindow;

namespace ImGui
{
	class CustomStyle
	{
	public:
		static float puContrastRatio;
		static ImU32 puContrastedTextColor;
		static int pushId;
		static int minorNumber;
		static int majorNumber;
		static int buildNumber;
		static ImVec4 GoodColor;
		static ImVec4 BadColor;
		static ImVec4 GlyphButtonColor;
		static ImVec4 GlyphButtonColorActive;
		static void Init();
		static void ResetCustomId();
	};

	IMGUI_API int IncPUSHID();
	IMGUI_API int GetPUSHID();
	IMGUI_API void SetPUSHID(int vID);

	IMGUI_API ImVec4 GetGoodOrBadColorForUse(bool vUsed); // return a "good" color if true or "bad" color if false

	IMGUI_API ImVec2 GetLocalMousePos(GLFWwindow* vWin = nullptr); // return local window mouse pos

	IMGUI_API void SetContrastRatio(float vRatio);
	IMGUI_API void SetContrastedTextColor(ImU32 vColor);
	IMGUI_API void DrawContrastWidgets();

	IMGUI_API float CalcContrastRatio(const ImU32& backGroundColor, const ImU32& foreGroundColor);
	IMGUI_API bool PushStyleColorWithContrast(const ImGuiCol& backGroundColor, const ImGuiCol& foreGroundColor, const ImU32& invertedColor, const float& minContrastRatio);
	IMGUI_API bool PushStyleColorWithContrast(const ImGuiCol& backGroundColor, const ImGuiCol& foreGroundColor, const ImVec4& invertedColor, const float& maxContrastRatio);
	IMGUI_API bool PushStyleColorWithContrast(const ImU32& backGroundColor, const ImGuiCol& foreGroundColor, const ImU32& invertedColor, const float& maxContrastRatio);
	IMGUI_API bool PushStyleColorWithContrast(const ImU32& backGroundColor, const ImGuiCol& foreGroundColor, const ImVec4& invertedColor, const float& maxContrastRatio);

	IMGUI_API void AddInvertedRectFilled(ImDrawList* vDrawList, const ImVec2& p_min, const ImVec2& p_max, ImU32 col, float rounding, ImDrawFlags rounding_corners);
	IMGUI_API void RenderInnerShadowFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_col, ImU32 fill_col_darker, ImU32 bg_Color, bool border, float rounding);
	IMGUI_API void DrawShadowImage(ImTextureID vShadowImage, const ImVec2& vSize, ImU32 col);

	IMGUI_API bool SelectableWithBtn(const char* label, bool selected = false, const char* btnLabel = nullptr, bool* btnClicked = nullptr, bool* btnHovered = nullptr, ImVec4 vBtnColor = ImVec4(0.8f, 0.5f, 0.2f, 1.0f), ImVec4 vBtnHoveredColor = ImVec4(1.0f, 0.8f, 0.2f, 1.0f), ImGuiSelectableFlags flags = 0, const ImVec2& size_args = ImVec2(0.0f, 0.0f));
	IMGUI_API bool ImageCheckButton(ImTextureID user_texture_id, bool* v, const ImVec2& size, const ImVec2& uv0 = ImVec2(0.0f, 0.0f), const ImVec2& uv1 = ImVec2(1.0f, 1.0f), const ImVec2& vHostTextureSize = ImVec2(0.0f, 0.0f), int frame_padding = -1, float vRectThickNess = 0.0f, ImVec4 vRectColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f));

	IMGUI_API bool BeginFramedGroup(const char *vLabel, ImGuiCol vHoveredIdx = ImGuiCol_TableHeaderBg, ImGuiCol NormalIdx = ImGuiCol_TableHeaderBg);
	IMGUI_API void EndFramedGroup();
	IMGUI_API void FramedGroupSeparator();
	IMGUI_API bool FramedGroupButtonText(const char* vFmt, ...);
	IMGUI_API void FramedGroupText(ImVec4* vTextColor, const char* vHelp, const char* vFmt, va_list vArgs);
	IMGUI_API void FramedGroupText(const char* vFmt, ...);
	IMGUI_API void FramedGroupTextHelp(const char* vHelp, const char* vFmt, ...);
	IMGUI_API void FramedGroupText(ImVec4 vTextColor, const char* vFmt, ...);

	IMGUI_API bool CollapsingHeader_SmallHeight(const char* vName, float vHeightRatio, float vWidth, bool vDefaulExpanded, bool* vIsOpen = nullptr);
	IMGUI_API bool CollapsingHeader_CheckBox(const char* vName, float vWidth = -1, bool vDefaulExpanded = false, bool vShowCheckBox = false, bool* vCheckCatched = 0);
	IMGUI_API bool CollapsingHeader_Button(const char* vName, float vWidth = -1, bool vDefaulExpanded = false, const char* vLabelButton = 0, bool vShowButton = false, bool* vButtonPressed = 0, ImFont* vButtonFont = nullptr);

	IMGUI_API bool CheckBoxBoolDefault(const char* vName, bool* vVar, bool vDefault, const char* vHelp = 0, ImFont* vLabelFont = nullptr);

	IMGUI_API bool RadioButtonLabeled(float vWidth, const char* label, bool active, bool disabled);
	IMGUI_API bool RadioButtonLabeled(float vWidth, const char* label, const char* help, bool active, bool disabled = false, ImFont* vLabelFont = nullptr);
	IMGUI_API bool RadioButtonLabeled(float vWidth, const char* label, const char* help, bool* active, bool disabled = false, ImFont* vLabelFont = nullptr);
	template<typename T>
	IMGUI_API bool RadioButtonLabeled_BitWize(
		float vWidth,
		const char* vLabel, const char* vHelp, T* vContainer, T vFlag,
		bool vOneOrZeroAtTime = false, //only one selcted at a time
		bool vAlwaysOne = true, // radio behavior, always one selected
		T vFlagsToTakeIntoAccount = (T)0,
		bool vDisableSelection = false,
		ImFont* vLabelFont = nullptr) // radio witl use only theses flags
	{
		bool selected = *vContainer & vFlag;
		const bool res = RadioButtonLabeled(vWidth, vLabel, vHelp, &selected, vDisableSelection, vLabelFont);
		if (res) {
			if (selected) {
				if (vOneOrZeroAtTime) {
					if (vFlagsToTakeIntoAccount) {
						if (vFlag & vFlagsToTakeIntoAccount) {
							*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
							*vContainer = (T)(*vContainer | vFlag); // add
						}
					}
					else *vContainer = vFlag; // set
				}
				else {
					if (vFlagsToTakeIntoAccount) {
						if (vFlag & vFlagsToTakeIntoAccount) {
							*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
							*vContainer = (T)(*vContainer | vFlag); // add
						}
					}
					else *vContainer = (T)(*vContainer | vFlag); // add
				}
			}
			else {
				if (vOneOrZeroAtTime) {
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
		bool vDisableSelection = false,
		ImFont* vLabelFont = nullptr) // radio witl use only theses flags
	{
		bool selected = *vContainer & vFlag;
		const char* label = (selected ? vLabelOK : vLabelNOK);
		const bool res = RadioButtonLabeled(vWidth, label, vHelp, &selected, vDisableSelection, vLabelFont);
		if (res) {
			if (selected) {
				if (vOneOrZeroAtTime) {
					if (vFlagsToTakeIntoAccount) {
						if (vFlag & vFlagsToTakeIntoAccount) {
							*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
							*vContainer = (T)(*vContainer | vFlag); // add
						}
					}
					else *vContainer = vFlag; // set
				}
				else {
					if (vFlagsToTakeIntoAccount) {
						if (vFlag & vFlagsToTakeIntoAccount) {
							*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
							*vContainer = (T)(*vContainer | vFlag); // add
						}
					}
					else *vContainer = (T)(*vContainer | vFlag); // add
				}
			}
			else {
				if (vOneOrZeroAtTime) {
					if (!vAlwaysOne) *vContainer = (T)(0); // remove all
				}
				else *vContainer = (T)(*vContainer & ~vFlag); // remove one
			}
		}
		return res;
	}
	template<typename T>

	IMGUI_API bool MenuItem(const char* label, const char* shortcut, T* vContainer, T vFlag, bool vOnlyOneSameTime = false)
	{
		bool selected = *vContainer & vFlag;
		const bool res = MenuItem(label, shortcut, &selected, true);
		if (res) {
			if (selected) {
				if (vOnlyOneSameTime) {
					*vContainer = vFlag; // set
				}
				else {
					*vContainer = (T)(*vContainer | vFlag);// add
				}
			}
			else {
				if (!vOnlyOneSameTime) {
					*vContainer = (T)(*vContainer & ~vFlag); // remove
				}
			}
		}
		return res;
	}
	template<typename T>

	IMGUI_API bool Begin(const char* name, T* vContainer, T vFlag, ImGuiWindowFlags flags)
	{
		bool check = *vContainer & vFlag;
		const bool res = Begin(name, &check, flags);
		if (check) *vContainer = (T)(*vContainer | vFlag); // add
		else *vContainer = (T)(*vContainer & ~vFlag); // remove
		return res;
	}

	IMGUI_API bool ClickableTextUrl(const char* label, const char* url, bool vOnlined = true);
	IMGUI_API bool ClickableTextFile(const char* label, const char* file, bool vOnlined = true);

	IMGUI_API void Spacing(float vSpace);
	IMGUI_API ImGuiWindow* GetHoveredWindow();

	IMGUI_API bool BeginMainStatusBar();
	IMGUI_API void EndMainStatusBar();

	IMGUI_API bool BeginLeftToolBar(float vWidth);
	IMGUI_API void EndLeftToolBar();

	IMGUI_API bool ContrastedButton(const char* label, const char* help = nullptr, ImFont* imfont = nullptr, float vWidth = 0.0f);
	IMGUI_API bool ToggleContrastedButton(const char* vLabelTrue, const char* vLabelFalse, bool* vValue, const char* vHelp = nullptr, ImFont* vImfont = nullptr);
	IMGUI_API bool ButtonNoFrame(const char* vName, ImVec2 size = ImVec2(-1, -1), ImVec4 vColor = ImVec4(1, 1, 1, 1), const char* vHelp = 0, ImFont* vLabelFont = nullptr);

	IMGUI_API bool Selectable_FramedText(const char* fmt, ...);

	void PlotFVec4Histo(
		const char* vLabel, ct::fvec4* vDatas, int vDataCount,
		bool* vShowChannel = 0,
		ImVec2 frame_size = ImVec2(0, 0),
		ct::fvec4 scale_min = FLT_MAX,
		ct::fvec4 scale_max = FLT_MAX,
		int* vHoveredIdx = 0);

	void ImageZoomPoint(ImTextureID vUserTextureId, const float vWidth, const ImVec2& vCenter, const ImVec2& vPoint, const ImVec2& vRadiusInPixels);
	//void ImageZoomLine(ImTextureID vUserTextureId, const float vWidth, const ImVec2& vStart, const ImVec2& vEnd);

	IMGUI_API bool InputText_Validation(const char* label, char* buf, size_t buf_size,
		const bool* vValidation = nullptr, const char* vValidationHelp = nullptr,
		ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);

	IMGUI_API void HelpMarker(const char* desc);

#ifdef USE_GRADIENT
	IMGUI_API void RenderGradFrame(ImVec2 p_min, ImVec2 p_max, ImU32 fill_start_col, ImU32 fill_end_col, bool border, float rounding);
	IMGUI_API bool GradButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = ImGuiButtonFlags_None);
#endif

	IMGUI_API bool TransparentButton(const char* label, const ImVec2& size_arg = ImVec2(0, 0), ImGuiButtonFlags flags = 0);

	IMGUI_API void PlainImageWithBG(ImTextureID user_texture_id, const ImVec2& size, const ImVec4& bg_col, const ImVec4& tint_col);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// SLIDERS //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IMGUI_API bool SliderScalarCompact(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const void* p_step = nullptr, const char* format = nullptr);
	IMGUI_API bool SliderIntCompact(float width, const char* label, int* v, int v_min, int v_max, int v_step = 0, const char* format = "%d");
	IMGUI_API bool SliderUIntCompact(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_step = 0U, const char* format = "%d");
	IMGUI_API bool SliderSizeTCompact(float width, const char* label, size_t* v, size_t v_min, size_t v_max, size_t v_step = 0U, const char* format = "%d");
	IMGUI_API bool SliderFloatCompact(float width, const char* label, float* v, float v_min, float v_max, float v_step = 0.0f, const char* format = "%.3f");

	IMGUI_API bool SliderScalarDefaultCompact(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const void* p_default, const void* p_step = nullptr, const char* format = nullptr);
	IMGUI_API bool SliderIntDefaultCompact(float width, const char* label, int* v, int v_min, int v_max, int v_default, int v_step = 0, const char* format = "%d");
	IMGUI_API bool SliderUIntDefaultCompact(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_default, uint32_t v_step = 0U, const char* format = "%d");
	IMGUI_API bool SliderSizeTDefaultCompact(float width, const char* label, size_t* v, size_t v_min, size_t v_max, size_t v_default, size_t v_step = 0U, const char* format = "%d");
	IMGUI_API bool SliderFloatDefaultCompact(float width, const char* label, float* v, float v_min, float v_max, float v_default, float v_step = 0.0f, const char* format = "%.3f");

	IMGUI_API bool SliderScalar(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const void* p_step = nullptr, const char* format = nullptr, ImGuiSliderFlags flags = 0);
	IMGUI_API bool SliderInt(float width, const char* label, int* v, int v_min, int v_max, int v_step = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
	IMGUI_API bool SliderUInt(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_step = 0U, const char* format = "%d", ImGuiSliderFlags flags = 0);
	IMGUI_API bool SliderSizeT(float width, const char* label, size_t* v, size_t v_min, size_t v_max, size_t v_step = 0U, const char* format = "%zu", ImGuiSliderFlags flags = 0);
	IMGUI_API bool SliderFloat(float width, const char* label, float* v, float v_min, float v_max, float v_step = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.

	IMGUI_API bool SliderScalarDefault(float width, const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const void* p_default, const void* p_step, const char* format = nullptr, ImGuiSliderFlags flags = 0);
	IMGUI_API bool SliderIntDefault(float width, const char* label, int* v, int v_min, int v_max, int v_default, int v_step = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
	IMGUI_API bool SliderUIntDefault(float width, const char* label, uint32_t* v, uint32_t v_min, uint32_t v_max, uint32_t v_default, uint32_t v_step = 0U, const char* format = "%d", ImGuiSliderFlags flags = 0);
	IMGUI_API bool SliderSizeTDefault(float width, const char* label, size_t* v, size_t v_min, size_t v_max, size_t v_default, size_t v_step = 0U, const char* format = "%zu", ImGuiSliderFlags flags = 0);
	IMGUI_API bool SliderFloatDefault(float width, const char* label, float* v, float v_min, float v_max, float v_default, float v_step = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);     // adjust format to decorate the value with a prefix or a suffix for in-slider labels or unit display.

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// COMBO ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IMGUI_API bool BeginContrastedCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);
	IMGUI_API bool ContrastedCombo(float vWidth, const char* label, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);
	IMGUI_API bool ContrastedCombo(float vWidth, const char* label, int* current_item, const char* items_separated_by_zeros, int popup_max_height_in_items = -1);
	IMGUI_API bool ContrastedCombo(float vWidth, const char* label, int* current_item, bool(*items_getter)(void* data, int idx, const char** out_text), void* data, int items_count, int popup_max_height_in_items = -1);
	IMGUI_API bool ContrastedComboVectorDefault(float vWidth, const char* label, int* current_item, const std::vector<std::string>& items, int items_count, int vDefault, int height_in_items = -1);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///// INPUT ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	IMGUI_API bool InputFloatDefault(float vWidth, const char* vName, float* vVar, float vDefault, const char* vInputPrec = "%.3f", const char* vPopupPrec = "%.3f", bool vShowResetButton = true, float vStep = 0.0f, float vStepFast = 0.0f);
	IMGUI_API bool InputIntDefault(float vWidth, const char* vName, int* vVar, int step, int step_fast, int vDefault);
}


#pragma once

#include <imgui/imgui.h>
#include <cstdint>

class ProjectFile;
class RibbonBar
{
private:
	ImFont* m_Font = 0;
	ImFontAtlas m_ImFontAtlas;
	ImFontConfig m_FontConfig;
	int m_Oversample = 1;
	int32_t fontPadding = 1;
	float fontMultiply = 1.0f;

public:
	bool Init();
	bool LoadFont(float vFontSize);
	void Draw(ProjectFile* vProjectFile);

private:
	bool RibbonButton(const char* icon, const char* label, float height, ImGuiButtonFlags flags = 0);
	bool RibbonToggleButton(const char* icon, const char* label, float height, bool& selected);

	bool BeginRibbonButtonMenu(const char* icon, const char* label, float height, bool enabled = true); // like ImGui::BeginMenu
	void EndRibbonButtonMenu(); // like ImGui::EndMenu()

	template<typename T>
	bool RibbonToggleButton(const char* icon, const char* label, float height, T* vContainer, T vFlag, bool vOnlyOneSameTime = false)
	{
		bool selected = *vContainer & vFlag;
		bool pressed = RibbonToggleButton(icon, label, height, selected);
		if (pressed)
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
		return pressed;
	}

private:
	void CreateFontTexture();
	void DestroyFontTexture();
};
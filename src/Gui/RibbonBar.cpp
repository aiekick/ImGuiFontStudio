#include "RibbonBar.h"

#include <Res/CustomFont.cpp>
#include <Project/ProjectFile.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui_internal.h>
#include <ImguiImpl/freetype/imgui_freetype.h>

#include <MainFrame.h>
#include <Panes/Manager/LayoutManager.h>
#include <Helper/SettingsDlg.h>
#include <Helper/ImGuiThemeHelper.h>

bool RibbonBar::Init()
{
	return LoadFont(50.0f);
}

bool RibbonBar::LoadFont(float vFontSize)
{
	bool res = false;

	m_ImFontAtlas.Clear();
	
	static const ImWchar ranges[] =
	{
		ICON_MIN_IGFS,
		ICON_MAX_IGFS, // Full Range
		0,
	};
	m_FontConfig.GlyphRanges = &ranges[0];
	m_FontConfig.OversampleH = 1;
	m_FontConfig.OversampleV = 1;
	m_FontConfig.PixelSnapH = true;
	
	m_ImFontAtlas.Flags |=
		ImFontAtlasFlags_NoMouseCursors | // hte mouse cursors
		ImFontAtlasFlags_NoBakedLines; // the big triangle

	m_Font = m_ImFontAtlas.AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFS, vFontSize, &m_FontConfig, ranges);
	if (m_Font)
	{
		m_ImFontAtlas.TexGlyphPadding = fontPadding;

		uint32_t freeTypeFlags = ImGuiFreeType::FreeType_Default;

		for (int n = 0; n < m_ImFontAtlas.ConfigData.Size; n++)
		{
			ImFontConfig* font_config = (ImFontConfig*)&m_ImFontAtlas.ConfigData[n];
			font_config->RasterizerMultiply = fontMultiply;
			font_config->RasterizerFlags = freeTypeFlags;
			font_config->OversampleH = m_Oversample;
			font_config->OversampleV = m_Oversample;
		}

		ImGuiFreeType::FT_Error freetypeError = 0;
		res = ImGuiFreeType::BuildFontAtlas(&m_ImFontAtlas, freeTypeFlags, &freetypeError);
	}

	if (!res)
	{
		m_ImFontAtlas.Clear();
		m_Font = 0;
	}
	else
	{
		DestroyFontTexture();
		CreateFontTexture();
	}

	return res;
}
//////////////////////////////////////////////////////////////////////////////
//// FONT TEXTURE ////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

void RibbonBar::CreateFontTexture()
{
	if (!m_ImFontAtlas.Fonts.empty())
	{
		unsigned char* pixels;
		int width, height;
		m_ImFontAtlas.GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);

		GLuint id = 0;
		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

		// size_t is 4 bytes sized for x32 and 8 bytes sizes for x64.
		// TexID is ImTextureID is a void so same size as size_t
		// id is a uint so 4 bytes on x32 and x64
		// so conversion first on size_t (uint32/64) and after on ImTextureID give no warnings
		m_ImFontAtlas.TexID = (ImTextureID)(size_t)id;

		glBindTexture(GL_TEXTURE_2D, last_texture);
	}
}

void RibbonBar::DestroyFontTexture()
{
	// size_t is 4 bytes sized for x32 and 8 bytes sizes for x64.
	// TexID is ImTextureID is a void so same size as size_t
	// id is a uint so 4 bytes on x32 and x64
	// so conversion first on size_t (uint32/64) and after on GLuint give no warnings
	GLuint id = (GLuint)(size_t)m_ImFontAtlas.TexID;
	if (id)
	{
		glDeleteTextures(1, &id);
		m_ImFontAtlas.TexID = nullptr;
	}
}

void RibbonBar::Draw(ProjectFile *vProjectFile)
{
	const float h = 75.0f;

	if (ImGui::BeginTabBar("Main Tab Bar"))
	{
		if (ImGui::BeginTabItem(ICON_IGFS_PROJECT " Project"))
		{
			if (RibbonButton(ICON_IGFS_FILE, "New", h))
			{
				MainFrame::Instance()->Action_Menu_NewProject();
			}

			ImGui::SameLine();

			if (RibbonButton(ICON_IGFS_FOLDER_OPEN, "Open", h))
			{
				MainFrame::Instance()->Action_Menu_OpenProject();
			}

			if (vProjectFile && vProjectFile->IsLoaded())
			{
				ImGui::SameLine();

				if (RibbonButton(ICON_IGFS_FOLDER_OPEN, "Re Open", h))
				{
					MainFrame::Instance()->Action_Menu_ReOpenProject();
				}

				ImGui::SameLine();

				if (RibbonButton(ICON_IGFS_SAVE, "Save", h))
				{
					MainFrame::Instance()->Action_Menu_SaveProject();
				}

				ImGui::SameLine();

				if (RibbonButton(ICON_IGFS_SAVE, "Save As", h))
				{
					MainFrame::Instance()->Action_Menu_SaveAsProject();
				}

				ImGui::SameLine();

				if (RibbonButton(ICON_IGFS_DESTROY, "Close", h))
				{
					MainFrame::Instance()->Action_Menu_CloseProject();
				}
			}

			ImGui::SameLine();

			if (RibbonButton(ICON_IGFS_ABOUT, "About", h))
			{
				MainFrame::Instance()->OpenAboutDialog();
			}

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(ICON_IGFS_LAYOUT " Layout"))
		{
			if (RibbonButton("", "Default\nLayout", h))
			{
				LayoutManager::Instance()->ApplyInitialDockingLayout();
			}

			ImGui::SameLine();

			RibbonToggleButton<PaneFlags>("", "Params\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_PARAM); ImGui::SameLine();
			RibbonToggleButton<PaneFlags>("", "Generator\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_GENERATOR); ImGui::SameLine();
			RibbonToggleButton<PaneFlags>("", "Source\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_SOURCE); ImGui::SameLine();
			RibbonToggleButton<PaneFlags>("", "Final\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_FINAL); ImGui::SameLine();
			RibbonToggleButton<PaneFlags>("", "Font\nPreview\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_FONT_PREVIEW); ImGui::SameLine();
			RibbonToggleButton<PaneFlags>("", "Selected\nFont\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_SELECTED_FONT); ImGui::SameLine();
			RibbonToggleButton<PaneFlags>("", "Glyph\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_GLYPH); ImGui::SameLine();
			RibbonToggleButton<PaneFlags>("", "Font\nStructure\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_FONT_STRUCTURE);

#ifdef _DEBUG
			ImGui::SameLine();

			RibbonToggleButton<PaneFlags>("", "Debug\nPane", h, &LayoutManager::m_Pane_Shown, PaneFlags::PANE_DEBUG);
#endif

			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem(ICON_IGFS_SETTINGS " Settings"))
		{
			if (RibbonButton("", "Settings", h))
			{
				SettingsDlg::Instance()->OpenDialog();
			}
			ImGui::SameLine();

			if (BeginRibbonButtonMenu("", "Themes", h))
			{
				ImGuiThemeHelper::Instance()->DrawMenu();

				EndRibbonButtonMenu();
			}

			ImGui::SameLine();

			// group 'styles'
			RibbonToggleButton("", "Show\nImGui", h, MainFrame::Instance()->m_ShowImGui); ImGui::SameLine();
			RibbonToggleButton("", "Show\nImGui\nStyle", h, MainFrame::Instance()->m_ShowImGuiStyle); ImGui::SameLine();
			RibbonToggleButton("", "Show\nImGui\nMetric\nDebug", h, MainFrame::Instance()->m_ShowMetric);	
			//////////////////

			ImGui::EndTabItem();
		}

		if (vProjectFile && vProjectFile->IsThereAnyNotSavedChanged())
		{
			if (ImGui::BeginTabItem("Save Needed"))
			{
				if (RibbonButton("", "Save", h))
				{
					MainFrame::Instance()->Action_Menu_SaveProject();
				}

				ImGui::EndTabItem();
			}
		}

		ImGui::EndTabBar();
	}
}

bool RibbonBar::RibbonButton(const char* icon, const char* label, float height, ImGuiButtonFlags flags)
{
	using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	ImVec2 label_size = CalcTextSize(label, NULL, true);
	if (m_Font)
		ImGui::PushFont(m_Font);
	ImVec2 icon_size = CalcTextSize(icon, NULL, true);
	if (m_Font)
		ImGui::PopFont();

	label_size.x = ImMax(label_size.x, icon_size.x);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(ImVec2(0, height), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, flags);

	// Render
	const ImU32 col = GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);
	
	if (m_Font)
	{
		ImGui::PushFont(m_Font);
		window->DrawList->AddText(bb.Min + style.FramePadding, GetColorU32(ImGuiCol_Text), icon);
		ImGui::PopFont();
	}

	RenderTextClipped(
		ImVec2(bb.Min.x + style.FramePadding.x, bb.Max.y - label_size.y - style.FramePadding.y), 
		bb.Max - style.FramePadding, label, NULL, &label_size, ImVec2(0.5f, 0.5f), &bb);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);
	return pressed;
}

bool RibbonBar::RibbonToggleButton(const char* icon, const char* label, float height, bool& selected)
{
	using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);

	ImVec2 label_size = CalcTextSize(label, NULL, true);
	if (m_Font)
		ImGui::PushFont(m_Font);
	ImVec2 icon_size = CalcTextSize(icon, NULL, true);
	if (m_Font)
		ImGui::PopFont();

	label_size.x = ImMax(label_size.x, icon_size.x);

	ImVec2 pos = window->DC.CursorPos;
	//if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
	//	pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;
	ImVec2 size = CalcItemSize(ImVec2(0, height), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	//if (window->DC.ItemFlags & ImGuiItemFlags_ButtonRepeat)
	//	flags |= ImGuiButtonFlags_Repeat;
	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held);// , flags);
	if (pressed)
	{
		selected = !selected;
	}

	// Render
	const ImU32 col = (GetColorU32((held && hovered) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : (selected ? ImGuiCol_Button : ImGuiCol_FrameBg)));
	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, col, true, style.FrameRounding);

	if (m_Font)
	{
		ImGui::PushFont(m_Font);
		window->DrawList->AddText(bb.Min + style.FramePadding, GetColorU32(ImGuiCol_Text), icon);
		ImGui::PopFont();
	}

	RenderTextClipped(
		ImVec2(bb.Min.x + style.FramePadding.x, bb.Max.y - label_size.y - style.FramePadding.y),
		bb.Max - style.FramePadding, label, NULL, &label_size, ImVec2(0.5f, 0.5f), &bb);

	IMGUI_TEST_ENGINE_ITEM_INFO(id, label, window->DC.LastItemStatusFlags);

	return pressed;
}

// like ImGui::BeginMenu
bool RibbonBar::BeginRibbonButtonMenu(const char* icon, const char* label, float height, bool /*enabled*/)
{
	using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImVec2 popup_pos = window->DC.CursorPos;
	bool pressed = RibbonButton(icon, label, height);
	popup_pos.y += GetItemRectSize().y;
	if (pressed && IsMouseReleased(0) && IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByPopup))
	{
		OpenPopupEx(window->GetID(label), 0);
		ImGui::SetNextWindowPos(popup_pos);
	}
	return ImGui::BeginPopup(label);
}

// like ImGui::EndMenu()
void RibbonBar::EndRibbonButtonMenu()
{
	ImGui::EndPopup();
}

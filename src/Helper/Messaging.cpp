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
 
#include "Messaging.h"

#include "Res/CustomFont.h"
#include "Gui/ImGuiWidgets.h"

#include "imgui/imgui.h"

Messaging::Messaging()
{

}

Messaging::~Messaging()
{
	
}

///////////////////////////////////////////////////////////////////////////////////////////
///// PRIVATE /////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static char Messaging_Message_Buffer[2048] = "\0";

void Messaging::AddMessage(MessageTypeEnum vType, bool vSelect, void* vDatas, MessageFunc vFunction, const char* fmt, va_list args)
{
	int size = vsnprintf(Messaging_Message_Buffer, 2047, fmt, args);
	if (size > 0)
		AddMessage(std::string(Messaging_Message_Buffer, size), vType, vSelect, vDatas, vFunction);
}

void Messaging::AddMessage(std::string vMsg, MessageTypeEnum vType, bool vSelect, void* vDatas, MessageFunc vFunction)
{
	if (vSelect)
	{
		currentMsgIdx = m_Messages.size();
	}

	m_Messages.push_back(std::make_tuple(vMsg, vType, vDatas, vFunction));
}

bool Messaging::DrawMessage(const size_t& vMsgIdx)
{
	bool res = false;

	if (/*vMsgIdx >= 0 (always true) && */vMsgIdx < m_Messages.size())
	{
		auto pa = m_Messages[vMsgIdx];
		res |= DrawMessage(pa);
	}

	return res;
}

bool Messaging::DrawMessage(const Messagekey& vMsg)
{
	if (std::get<1>(vMsg) == MessageTypeEnum::MESSAGE_TYPE_INFOS)
	{
		ImGui::Text("%s ", ICON_IGFS_INFOS);
	}
	else if (std::get<1>(vMsg) == MessageTypeEnum::MESSAGE_TYPE_WARNING)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, m_WarningColor);
		ImGui::Text("%s ", ICON_IGFS_WARNING);
		ImGui::PopStyleColor();
	}
	else if (std::get<1>(vMsg) == MessageTypeEnum::MESSAGE_TYPE_ERROR)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, m_ErrorColor);
		ImGui::Text("%s ", ICON_IGFS_ERROR);
		ImGui::PopStyleColor();
	}
	ImGui::SameLine(); // used only for when displayed in list. no effect when diplsayed in status bar
	ImGui::PushID(&vMsg);
	bool check = ImGui::Selectable_FramedText("%s", std::get<0>(vMsg).c_str());
	ImGui::PopID();
	if (check)
	{
		auto datas = std::get<2>(vMsg);
		auto func = std::get<3>(vMsg);
		if (func)
			func(datas);
	}
	return check;
}

///////////////////////////////////////////////////////////////////////////////////////////
///// PUBLIC //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void Messaging::Draw()
{
	ImGui::Text("Messages :");

	if (!m_Messages.empty())
	{
		if (ImGui::BeginMenu(ICON_IGFS_DESTROY "##clear"))
		{
			if (ImGui::MenuItem("All")) Clear();
			ImGui::Separator();
			if (m_HasInfos)
				if (ImGui::MenuItem("Infos")) ClearInfos();
			if (m_HasWarnings)
				if (ImGui::MenuItem("Warnings")) ClearWarnings();
			if (m_HasErrors)
				if (ImGui::MenuItem("Errors")) ClearErrors();

			ImGui::EndMenu();
		}
	}
	if (!m_Messages.empty())
	{
		if (m_Messages.size() > 1)
		{
			if (ImGui::MenuItem(ICON_IGFS_MENU_LEFT "##left"))
			{
				currentMsgIdx = ct::maxi<int>(--currentMsgIdx, 0);
			}
			if (ImGui::MenuItem(ICON_IGFS_MENU_RIGHT "##right"))
			{
				currentMsgIdx = ct::maxi<int>(++currentMsgIdx, m_Messages.size() - 1);
			}
			if (ImGui::BeginMenu(ICON_IGFS_MENU_UP "##up"))
			{
				for (auto & msg : m_Messages)
				{
					if (DrawMessage(msg))
						break;
				}
				ImGui::EndMenu();
			}
		}
		currentMsgIdx = ct::clamp<int>(currentMsgIdx, 0, m_Messages.size() - 1);
		DrawMessage(currentMsgIdx);
	}
}

void Messaging::AddInfos(bool vSelect, void* vDatas, MessageFunc vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_INFOS, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	m_HasInfos = true;
}

void Messaging::AddWarning(bool vSelect, void* vDatas, MessageFunc vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_WARNING, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	m_HasWarnings = true;
}

void Messaging::AddError(bool vSelect, void* vDatas, MessageFunc vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_ERROR, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	m_HasErrors = true;
}

void Messaging::ClearErrors()
{
	std::list<int> msgToErase;
	int idx = 0;
	for (auto & msg : m_Messages)
	{
		if (std::get<1>(msg) == MessageTypeEnum::MESSAGE_TYPE_ERROR)
			msgToErase.push_front(idx);
		idx++;
	}

	for (auto & id : msgToErase)
	{
		m_Messages.erase(m_Messages.begin() + id);
	}
	m_HasErrors = false;
}

void Messaging::ClearWarnings()
{
	std::list<int> msgToErase;
	int idx = 0;
	for (auto & msg : m_Messages)
	{
		if (std::get<1>(msg) == MessageTypeEnum::MESSAGE_TYPE_WARNING)
			msgToErase.push_front(idx);
		idx++;
	}

	for (auto & id : msgToErase)
	{
		m_Messages.erase(m_Messages.begin() + id);
	}
	m_HasWarnings = false;
}

void Messaging::ClearInfos()
{
	std::list<int> msgToErase;
	int idx = 0;
	for (auto & msg : m_Messages)
	{
		if (std::get<1>(msg) == MessageTypeEnum::MESSAGE_TYPE_INFOS)
			msgToErase.push_front(idx);
		idx++;
	}

	for (auto & id : msgToErase)
	{
		m_Messages.erase(m_Messages.begin() + id);
	}
	m_HasInfos = false;
}

void Messaging::Clear()
{
	m_Messages.clear();
	m_HasInfos = false;
	m_HasWarnings = false;
	m_HasErrors = false;
}
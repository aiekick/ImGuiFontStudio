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

#include <Res/CustomFont.h>
#include <Gui/ImWidgets.h>
#include <Helper/SelectionHelper.h>

Messaging::Messaging() = default;
Messaging::~Messaging() = default;

///////////////////////////////////////////////////////////////////////////////////////////
///// PRIVATE /////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

static char Messaging_Message_Buffer[2048] = "\0";

void Messaging::AddMessage(MessageTypeEnum vType, bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, va_list args)
{
	int size = vsnprintf(Messaging_Message_Buffer, 2047, fmt, args);
	if (size > 0)
		AddMessage(std::string(Messaging_Message_Buffer, size), vType, vSelect, vDatas, vFunction);
}

void Messaging::AddMessage(const std::string& vMsg, MessageTypeEnum vType, bool vSelect, MessageData vDatas, const MessageFunc& vFunction)
{
	if (vSelect)
	{
		currentMsgIdx = (int32_t)m_Messages.size();
	}

	m_Messages.emplace_back(vMsg, vType, vDatas, vFunction);
}

bool Messaging::DrawMessage(const size_t& vMsgIdx)
{
	bool res = false;

	if (vMsgIdx < m_Messages.size())
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
		const auto& func = std::get<3>(vMsg);
		if (func)
			func(datas);
	}
	return check;
}

///////////////////////////////////////////////////////////////////////////////////////////
///// PUBLIC //////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void Messaging::Draw(ProjectFile *vProjectFile)
{
	ImGui::Text("Messages :");

	if (ImGui::MenuItem(ICON_IGFS_REFRESH "##Refresh"))
	{
		Messaging::Instance()->Clear();
		SelectionHelper::Instance()->AnalyseSourceSelection(vProjectFile);
	}

	if (!m_Messages.empty())
	{
		// on type of message only
		if (m_MessageExistFlags == MESSAGE_EXIST_INFOS ||
			m_MessageExistFlags == MESSAGE_EXIST_WARNING ||
			m_MessageExistFlags == MESSAGE_EXIST_ERROR)
		{
			if (ImGui::MenuItem(ICON_IGFS_DESTROY "##clear"))
			{
				Clear();
			}
		}
		else
		{
			if (ImGui::BeginMenu(ICON_IGFS_DESTROY "##clear"))
			{
				if (ImGui::MenuItem("All")) Clear();
				ImGui::Separator();
				if (m_MessageExistFlags & MESSAGE_EXIST_INFOS)
					if (ImGui::MenuItem("Infos")) ClearInfos();
				if (m_MessageExistFlags & MESSAGE_EXIST_WARNING)
					if (ImGui::MenuItem("Warnings")) ClearWarnings();
				if (m_MessageExistFlags & MESSAGE_EXIST_ERROR)
					if (ImGui::MenuItem("Errors")) ClearErrors();

				ImGui::EndMenu();
			}
		}
	}
	if (!m_Messages.empty())
	{
		if (m_Messages.size() > 1)
		{
			if (ImGui::MenuItem(ICON_IGFS_MENU_LEFT "##left"))
			{
				currentMsgIdx = ct::maxi<int32_t>(--currentMsgIdx, 0);
			}
			if (ImGui::MenuItem(ICON_IGFS_MENU_RIGHT "##right"))
			{
				currentMsgIdx = ct::maxi<int32_t>(++currentMsgIdx, (int32_t)m_Messages.size() - 1);
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
		currentMsgIdx = ct::clamp<int32_t>(currentMsgIdx, 0, (int32_t)m_Messages.size() - 1);
		DrawMessage(currentMsgIdx);
	}
}

void Messaging::AddInfos(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_INFOS, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	m_MessageExistFlags = (MessageExistFlags)(m_MessageExistFlags | MESSAGE_EXIST_INFOS);
}

void Messaging::AddWarning(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_WARNING, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	m_MessageExistFlags = (MessageExistFlags)(m_MessageExistFlags | MESSAGE_EXIST_WARNING);
}

void Messaging::AddError(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	AddMessage(MessageTypeEnum::MESSAGE_TYPE_ERROR, vSelect, vDatas, vFunction, fmt, args);
	va_end(args);
	m_MessageExistFlags = (MessageExistFlags)(m_MessageExistFlags | MESSAGE_EXIST_ERROR);
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

	m_MessageExistFlags &= ~MESSAGE_EXIST_ERROR;
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
	
	m_MessageExistFlags &= ~MESSAGE_EXIST_WARNING;
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
	
	m_MessageExistFlags &= ~MESSAGE_EXIST_INFOS;
}

void Messaging::Clear()
{
	m_Messages.clear();
	m_MessageExistFlags = MESSAGE_EXIST_NONE;
}
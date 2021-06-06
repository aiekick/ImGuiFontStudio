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

#include <ctools/cTools.h>

#include <functional>
#include <cstdarg>
#include <string>
#include <utility> // std::pair
#include <list>
#include <vector>
#include <memory>

class MessageData
{
private:
	std::shared_ptr<void> m_Datas;

public:
	MessageData() {}
	MessageData(std::nullptr_t) {}
	template<typename T>
	MessageData(const std::shared_ptr<T>& vDatas)
	{
		SetUserDatas(vDatas);
	}
	template<typename T>
	void SetUserDatas(const std::shared_ptr<T>& vDatas)
	{
		m_Datas = vDatas;
	}
	template<typename T>
	std::shared_ptr<T> GetUserDatas()
	{
		return std::static_pointer_cast<T>(m_Datas);
	}
};

class ProjectFile;
class Messaging
{
private:
	const ImVec4 m_ErrorColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	const ImVec4 m_WarningColor = ImVec4(0.8f, 0.8f, 0.0f, 1.0f);

private:
	enum MessageTypeEnum
	{
		MESSAGE_TYPE_INFOS = 0,
		MESSAGE_TYPE_ERROR,
		MESSAGE_TYPE_WARNING
	};

	enum _MessageExistFlags
	{
		MESSAGE_EXIST_NONE = 0,
		MESSAGE_EXIST_INFOS = (1<<0),
		MESSAGE_EXIST_ERROR = (1<<1),
		MESSAGE_EXIST_WARNING = (1<<2)
	};
    typedef int MessageExistFlags;
    MessageExistFlags m_MessageExistFlags = MESSAGE_EXIST_NONE;

	int32_t currentMsgIdx = 0;
	typedef std::function<void(MessageData)> MessageFunc;
	typedef std::tuple<std::string, MessageTypeEnum, MessageData, MessageFunc> Messagekey;
	std::vector<Messagekey> m_Messages;

private:
	void AddMessage(const std::string& vMsg, MessageTypeEnum vType, bool vSelect, MessageData vDatas, const MessageFunc& vFunction);
	void AddMessage(MessageTypeEnum vType, bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, va_list args);
	bool DrawMessage(const size_t& vMsgIdx);
	bool DrawMessage(const Messagekey& vMsg);

public:
	void Draw();
	void AddInfos(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...); // select => set currentMsgIdx to this msg idx
	void AddWarning(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...); // select => set currentMsgIdx to this msg idx
	void AddError(bool vSelect, MessageData vDatas, const MessageFunc& vFunction, const char* fmt, ...); // select => set currentMsgIdx to this msg idx
	void ClearErrors();
	void ClearWarnings();
	void ClearInfos();
	void Clear();

public: // singleton
	static Messaging *Instance()
	{
		static Messaging *_instance = new Messaging();
		return _instance;
	}

protected:
	Messaging(); // Prevent construction
	Messaging(const Messaging&) {}; // Prevent construction by copying
	Messaging& operator =(const Messaging&) { return *this; }; // Prevent assignment
	~Messaging(); // Prevent unwanted destruction
};


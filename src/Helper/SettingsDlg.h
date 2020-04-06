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

#include <vector>
#include <map>
#include <string>

#include <future>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <string>
#include <vector>
#include <list>

#include "tinyxml2/tinyxml2.h"

enum SettingsPaneModeEnum
{
	SETTINGS_PANE_MODE_CONTENT = 0,
	SETTINGS_PANE_MODE_LOAD,
	SETTINGS_PANE_MODE_SAVE,
	SETTINGS_PANE_MODE_Count
};

class SettingsDlg
{
private:
	bool m_ShowDialog = false;
	std::string dlg_key;
	std::map<int, std::map<std::string, std::function<void(SettingsPaneModeEnum)>>> m_Categories;
	std::string m_CurrentCategory;

public:
	static SettingsDlg* Instance()
	{
		static SettingsDlg *_instance = new SettingsDlg();
		return _instance;
	}

protected:
	SettingsDlg(); // Prevent construction
	SettingsDlg(const SettingsDlg&) {}; // Prevent construction by copying
	SettingsDlg& operator =(const SettingsDlg&) { return *this; }; // Prevent assignment
	~SettingsDlg(); // Prevent unwanted destruction

public:
	void Init();

	void OpenDialog();
	void CloseDialog();
	bool DrawDialog();

private:
	void DrawCategoryPanes();
	void DrawContentPane();
	void DrawButtonsPane();

private:
	void Load();
	void Save();

private:
	void DrawPane_General(SettingsPaneModeEnum vMode);
	void DrawPane_Style(SettingsPaneModeEnum vMode);

};

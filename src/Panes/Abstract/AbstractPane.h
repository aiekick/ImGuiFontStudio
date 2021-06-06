/*
MIT License

Copyright (c) 2021 Stephane Cuillerdier (aka Aiekick)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <memory> // smart ptr
#include <string>

typedef int PaneFlags;
enum class PaneDisposal
{
	CENTRAL = 0,
	LEFT,
	RIGHT,
	BOTTOM,
	TOP,
	Count
};

#define PANE_NAME_BUFFER_SIZE 100

class GenericRenderer;
class AbstractPane
{
public:
	char m_PaneName[PANE_NAME_BUFFER_SIZE + 1] = "";
	PaneFlags m_PaneFlag = 0;
	PaneDisposal m_PaneDisposal = PaneDisposal::CENTRAL;
	bool m_OpenedDefault = false;
	bool m_FocusedDefault = false;

public:
	int m_PaneWidgetId = 0;
	int NewWidgetId() { return ++m_PaneWidgetId; }
	
public:
	virtual bool Init() = 0;
	virtual void Unit() = 0;
	virtual int DrawPanes(int vWidgetId, std::string vUserDatas) = 0;
	virtual void DrawDialogsAndPopups(std::string vUserDatas) = 0;
	virtual int DrawWidgets(int vWidgetId, std::string vUserDatas) = 0;
};

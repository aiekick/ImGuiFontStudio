#include "FrameActionSystem.h"
#include <Project/ProjectFile.h>

void FrameActionSystem::Insert(ActionStamp vAction)
{
	if (vAction)
		m_Actions.push_front(vAction);
}

void FrameActionSystem::Add(ActionStamp vAction)
{
	if (vAction)
		m_Actions.push_back(vAction);
}

void FrameActionSystem::Clear()
{
	m_Actions.clear();
}

void FrameActionSystem::Apply()
{
	if (!m_Actions.empty())
	{
		auto action = *m_Actions.begin();
		if (action()) // one action per frame, it true we can continue by deleting the current
		{
			if (!m_Actions.empty()) // because an action can clear actions
			{
				m_Actions.pop_front();
			}
		}
	}
}
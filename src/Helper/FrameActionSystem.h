#pragma once

#include <list>
#include <functional>

/*
 list of sequential actions
 executed time to time only if executed action retrun true
 each succesfull action is erased
 not sucessfull action is applied again each frame
*/

class ProjectFile;
class FrameActionSystem
{
private:
	typedef std::function<bool()> ActionStamp;
	std::list<ActionStamp> m_Actions;

public:
	// insert an action at first, cause :
	// this action will be executed first at the next frame
	void Insert(ActionStamp vAction);
	// add an action at end
	// this action will be executed at least after all others
	void Add(ActionStamp vAction);
	// clear all actions
	void Clear();
	// apply first action each frame until true is returned
	// if return true, erase action
	// let the next frame call the next action
	// il false, action executed until true
	void Apply();
};
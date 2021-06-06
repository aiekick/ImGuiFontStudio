#pragma once

#if VULKAN
	#include <GLFW/glfw3.h>
	#include <vulkan/vulkan.h>
#else
	#include <glad/glad.h>
	#include <GLFW/glfw3.h>
#endif

enum PaneFlags_
{
	PARAM_PANE = (1 << 0),
	SOURCE_PANE = (1 << 1),
	FINAL_PANE = (1 << 2),
	GENERATOR_PANE = (1 << 3),
	STRUCTURE_PANE = (1 << 4),
	GLYPH_PANE = (1 << 5),
	PREVIEW_PANE = (1 << 6),
#ifdef _DEBUG
	DEBUG_PANE = (1 << 7)
#endif
};
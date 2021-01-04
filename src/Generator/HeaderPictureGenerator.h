#pragma once

/*
this class will generate a picture representation of the content of a header file
will contain, icon + header name in 2 column i think
to generate with stb. like imgui use stb for rasterize and write an image
*/
#include <Project/FontInfos.h>

#include <imgui/imgui.h>
#include <ctools/cTools.h>
#include <cstdint>
#include <string>
#include <map>

struct GLFWwindow;
class HeaderPictureGenerator
{
public:
	static bool SaveTextureToPng(GLFWwindow* vWin, const std::string& vFilePathName, GLuint vTextureId, ct::uvec2 vTextureSize, uint32_t vChannelCount);
public:
	HeaderPictureGenerator();
	~HeaderPictureGenerator();

	void Generate(const std::string& vFilePathName, FontInfos* vFontInfos);
	void WriteEachGlyphsToPicture(const std::string& vPath, std::map<uint32_t, std::string> vLabels, 
		FontInfos* vFontInfos, uint32_t vHeight);
	void WriteEachGlyphsLabelToPicture(const std::string& vPath, std::map<uint32_t, std::string> vLabels, 
		FontInfos* vFontInfos, uint32_t vHeight);
	void WriteEachGlyphsLabeledToPicture(const std::string& vPath, std::map<uint32_t, std::string> vLabels, 
		FontInfos* vFontInfos, uint32_t vGlyphHeight, uint32_t vLabelHeight);
	void WriteGlyphCardToPicture(const std::string& vFilePathName, std::map<uint32_t, std::string> vLabels, 
		FontInfos* vFontInfos, const uint32_t& vGlyphHeight, const uint32_t& vMaxRows);
};
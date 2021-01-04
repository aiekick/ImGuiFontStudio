#include "HeaderPictureGenerator.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ctools/Logger.h>
#include <MainFrame.h>
#include <ctools/FileHelper.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <imgui/imstb_truetype.h>

#include <map>

HeaderPictureGenerator::HeaderPictureGenerator()
{

}

HeaderPictureGenerator::~HeaderPictureGenerator()
{

}

static std::vector<uint8_t> GetBytesFromTexture(GLFWwindow* vWin, GLuint vTextureId, ct::uvec2 vTextureSize, uint32_t vChannelCount)
{
	glfwMakeContextCurrent(vWin);

	std::vector<uint8_t> buf;

	buf.resize((size_t)vTextureSize.x * (size_t)vTextureSize.y * (size_t)vChannelCount); // 1 channel only

	GLenum format = GL_RGBA;
	if (vChannelCount == 1) format = GL_RED;
	if (vChannelCount == 2) format = GL_RG;
	if (vChannelCount == 3) format = GL_RGB;
	if (vChannelCount == 4) format = GL_RGBA;

	// Upload texture to graphics system
	GLint last_texture;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
	glBindTexture(GL_TEXTURE_2D, vTextureId);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 0, format, GL_UNSIGNED_BYTE, buf.data());
	glBindTexture(GL_TEXTURE_2D, last_texture);

	return buf;
}

bool HeaderPictureGenerator::SaveTextureToPng(GLFWwindow* vWin, const std::string& vFilePathName, GLuint vTextureId, ct::uvec2 vTextureSize, uint32_t vChannelCount)
{
	bool res = false;

	if (!vFilePathName.empty() && vWin)
	{
		std::vector<uint8_t> bytes = GetBytesFromTexture(vWin, vTextureId, vTextureSize, vChannelCount);

		int resWrite = stbi_write_png(
			vFilePathName.c_str(),
			vTextureSize.x,
			vTextureSize.y,
			vChannelCount,
			bytes.data(),
			vTextureSize.x * vChannelCount);

		res = (resWrite > 0);
	}

	return res;
}

void HeaderPictureGenerator::Generate(const std::string& vFilePathName, FontInfos* vFontInfos)
{
	if (vFontInfos)
	{
		std::string filePathName = vFilePathName;
		auto ps = FileHelper::Instance()->ParsePathFileName(vFilePathName);
		if (ps.isOk)
		{
			std::string name = ps.name;
			ct::replaceString(name, "-", "_");
			filePathName = ps.GetFilePathWithNameExt(name, ".png");

			std::string prefix = "";
			if (!vFontInfos->m_FontPrefix.empty())
				prefix = "ICON_" + vFontInfos->m_FontPrefix;

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// prepare names and codepoint
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			std::map<uint32_t, std::string> glyphCodePoints;

			for (auto it : vFontInfos->m_SelectedGlyphs)
			{
				glyphCodePoints[it.first] = it.second.newHeaderName;
			}

			uint32_t minCodePoint = 65535;
			uint32_t maxCodePoint = 0;

			std::map<uint32_t, std::string> finalGlyphCodePoints;

			for (const auto& it : glyphCodePoints)
			{
				uint32_t codePoint = it.first;

				minCodePoint = ct::mini(minCodePoint, codePoint);
				maxCodePoint = ct::maxi(maxCodePoint, codePoint);

				std::string glyphName = it.second;
				ct::replaceString(glyphName, " ", "_");
				ct::replaceString(glyphName, "-", "_");

				// by ex .notdef will become DOT_notdef
				// because a define with '.' is a problem for the compiler
				ct::replaceString(glyphName, ".", "DOT_");

				for (auto& c : glyphName)
					c = (char)toupper((int)c);

				finalGlyphCodePoints[codePoint] = prefix + "_" + glyphName;
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// write to texture
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			//WriteEachGlyphsToPicture(finalGlyphCodePoints, vFontInfos, 50U);
			WriteEachGlyphsLabelToPicture(finalGlyphCodePoints, vFontInfos, 50U);
		}
	}
}

void HeaderPictureGenerator::WriteEachGlyphsToPicture(std::map<uint32_t, std::string> vLabels, FontInfos* vFontInfos, uint32_t vHeight)
{
	if (vFontInfos)
	{
		if (!vFontInfos->m_ImFontAtlas.ConfigData.empty())
		{
			stbtt_fontinfo fontInfo;
			const int font_offset = stbtt_GetFontOffsetForIndex(
				(unsigned char*)vFontInfos->m_ImFontAtlas.ConfigData[0].FontData,
				vFontInfos->m_ImFontAtlas.ConfigData[0].FontNo);
			if (stbtt_InitFont(&fontInfo, (unsigned char*)vFontInfos->m_ImFontAtlas.ConfigData[0].FontData, font_offset))
			{
				float scale = stbtt_ScaleForPixelHeight(&fontInfo, vHeight);
				for (const auto& it : vLabels)
				{
					int w, h, ox, oy;
					uint8_t* buffer = stbtt_GetCodepointBitmap(&fontInfo, scale, scale, it.first, &w, &h, &ox, &oy);
					if (buffer)
					{
						std::string file = it.second + ".png";

						int res = stbi_write_png(
							file.c_str(),
							w,
							h,
							1,
							buffer,
							w);

						if (res)
						{
							printf("Succes writing of a glyph to %s\n", file.c_str());
						}
						else
						{
							printf("Failed writing of a glyph to %s\n", file.c_str());
						}

						stbtt_FreeBitmap(buffer, fontInfo.userdata);
					}
				}
			}
		}
	}
}

void HeaderPictureGenerator::WriteEachGlyphsLabelToPicture(std::map<uint32_t, std::string> vLabels, FontInfos* vFontInfos, uint32_t vHeight)
{
	if (vFontInfos)
	{
		auto io = &ImGui::GetIO();
		if (!io->Fonts->ConfigData.empty())
		{
			stbtt_fontinfo fontInfo;
			
			const int font_offset = stbtt_GetFontOffsetForIndex(
				(unsigned char*)io->Fonts->ConfigData[0].FontData,
				io->Fonts->ConfigData[0].FontNo);
			if (stbtt_InitFont(&fontInfo, (unsigned char*)io->Fonts->ConfigData[0].FontData, font_offset))
			{
				int ascent, baseline;
				float scale = stbtt_ScaleForPixelHeight(&fontInfo, vHeight);
				stbtt_GetFontVMetrics(&fontInfo, &ascent, 0, 0);
				baseline = (int)(ascent * scale);

				std::vector<uint8_t> arr;
				for (const auto& it : vLabels)
				{
					arr.clear();
					std::string lblToRender = it.second;
					uint32_t finalHeight = vHeight + 5U;
					uint32_t finalWidth = (uint32_t)lblToRender.size() * finalHeight;
					arr.resize((size_t)finalWidth * (size_t)finalHeight);
					auto text = lblToRender.c_str();

					int ch = 0, xpos = 2; // leave a little padding in case the character extends left;
					while (text[ch])
					{
						int advance, lsb, x0, y0, x1, y1;
						float x_shift = xpos - (float)floor(xpos);
						stbtt_GetCodepointHMetrics(&fontInfo, text[ch], &advance, &lsb);
						stbtt_GetCodepointBitmapBoxSubpixel(&fontInfo, text[ch], scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);

						//&screen[baseline + y0][(int) xpos + x0]
						//arr of w * h
						//arr[x][y] == arr[w * y + x]
						uint32_t x = (uint32_t)xpos + x0;
						uint32_t y = baseline + y0;
						if (x >= 0 && y >= 0)
						{
							uint8_t* ptr = arr.data() + finalWidth * y + x;
							stbtt_MakeCodepointBitmapSubpixel(&fontInfo, ptr, x1 - x0, y1 - y0, finalWidth, scale, scale, x_shift, 0, text[ch]);
						}
						// note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
						// because this API is really for baking character bitmaps into textures. if you want to render
						// a sequence of characters, you really need to render each bitmap to a temp buffer, then
						// "alpha blend" that into the working buffer
						xpos += (advance * scale);
						if (text[ch + 1])
							xpos += scale * stbtt_GetCodepointKernAdvance(&fontInfo, text[ch], text[ch + 1]);
						++ch;
					}

					std::string file = lblToRender + "_TEXT.png";

					int res = stbi_write_png(
						file.c_str(),
						finalWidth,
						finalHeight,
						1,
						arr.data(),
						finalWidth);

					if (res)
					{
						printf("Succes writing of a glyph label to %s\n", file.c_str());
					}
					else
					{
						printf("Failed writing of a glyph label to %s\n", file.c_str());
					}
				}
			}
		}
	}
}

void HeaderPictureGenerator::WriteEachGlyphsLabeledToPicture(std::map<uint32_t, std::string> vLabels, FontInfos* vFontInfos, uint32_t vHeight)
{
	if (vFontInfos)
	{
		auto io = &ImGui::GetIO();
		if (!io->Fonts->ConfigData.empty())
		{
			stbtt_fontinfo fontInfo;

			const int font_offset = stbtt_GetFontOffsetForIndex(
				(unsigned char*)io->Fonts->ConfigData[0].FontData,
				io->Fonts->ConfigData[0].FontNo);
			if (stbtt_InitFont(&fontInfo, (unsigned char*)io->Fonts->ConfigData[0].FontData, font_offset))
			{
				int ascent, baseline;
				float scale = stbtt_ScaleForPixelHeight(&fontInfo, vHeight);
				stbtt_GetFontVMetrics(&fontInfo, &ascent, 0, 0);
				baseline = (int)(ascent * scale);

				std::vector<uint8_t> arr;
				for (const auto& it : vLabels)
				{
					arr.clear();
					std::string lblToRender = it.second;
					uint32_t finalHeight = vHeight + 5U;
					uint32_t finalWidth = (uint32_t)lblToRender.size() * finalHeight;
					arr.resize((size_t)finalWidth * (size_t)finalHeight);
					auto text = lblToRender.c_str();

					int ch = 0, xpos = 2; // leave a little padding in case the character extends left;
					while (text[ch])
					{
						int advance, lsb, x0, y0, x1, y1;
						float x_shift = xpos - (float)floor(xpos);
						stbtt_GetCodepointHMetrics(&fontInfo, text[ch], &advance, &lsb);
						stbtt_GetCodepointBitmapBoxSubpixel(&fontInfo, text[ch], scale, scale, x_shift, 0, &x0, &y0, &x1, &y1);

						//&screen[baseline + y0][(int) xpos + x0]
						//arr of w * h
						//arr[x][y] == arr[w * y + x]
						uint32_t x = (uint32_t)xpos + x0;
						uint32_t y = baseline + y0;
						if (x >= 0 && y >= 0)
						{
							uint8_t* ptr = arr.data() + finalWidth * y + x;
							stbtt_MakeCodepointBitmapSubpixel(&fontInfo, ptr, x1 - x0, y1 - y0, finalWidth, scale, scale, x_shift, 0, text[ch]);
						}
						// note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
						// because this API is really for baking character bitmaps into textures. if you want to render
						// a sequence of characters, you really need to render each bitmap to a temp buffer, then
						// "alpha blend" that into the working buffer
						xpos += (advance * scale);
						if (text[ch + 1])
							xpos += scale * stbtt_GetCodepointKernAdvance(&fontInfo, text[ch], text[ch + 1]);
						++ch;
					}

					std::string file = lblToRender + "_TEXT.png";

					int res = stbi_write_png(
						file.c_str(),
						finalWidth,
						finalHeight,
						1,
						arr.data(),
						finalWidth);

					if (res)
					{
						printf("Succes writing of a glyph label to %s\n", file.c_str());
					}
					else
					{
						printf("Failed writing of a glyph label to %s\n", file.c_str());
					}
				}
			}
		}
	}
}

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

			//WriteEachGlyphsToPicture("exports\\", finalGlyphCodePoints, vFontInfos, 150U);
			//WriteEachGlyphsLabelToPicture("exports\\", finalGlyphCodePoints, vFontInfos, 50U);
			//WriteEachGlyphsLabeledToPicture("exports\\", finalGlyphCodePoints, vFontInfos, 150U, 50U);
			WriteGlyphCardToPicture(filePathName, finalGlyphCodePoints, vFontInfos, 150U, 10U);
		}
	}
}

void HeaderPictureGenerator::WriteEachGlyphsToPicture(
	const std::string& vPath, std::map<uint32_t, std::string> vLabels, 
	FontInfos* vFontInfos, uint32_t vHeight)
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
						std::string file = vPath + it.second + ".png";

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

void HeaderPictureGenerator::WriteEachGlyphsLabelToPicture(
	const std::string& vPath, std::map<uint32_t, std::string> vLabels, 
	FontInfos* vFontInfos, uint32_t vHeight)
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
						int32_t x = (uint32_t)xpos + x0;
						int32_t y = baseline + y0;
						if (x >= 0 && y >= 0)
						{
							uint8_t* ptr = arr.data() + (size_t)finalWidth * (size_t)y + (size_t)x;
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

					std::string file = vPath + lblToRender + "_TEXT.png";

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

void HeaderPictureGenerator::WriteEachGlyphsLabeledToPicture(
	const std::string& vPath, std::map<uint32_t, std::string> vLabels, 
	FontInfos* vFontInfos, uint32_t vGlyphHeight, uint32_t vLabelHeight)
{
	if (vFontInfos)
	{
		stbtt_fontinfo glyphFontInfo;
		stbtt_fontinfo labelFontInfo;

		bool glyphFontLoaded = false;
		bool labelFontLoaded = false;

		if (!vFontInfos->m_ImFontAtlas.ConfigData.empty())
		{
			const int font_offset = stbtt_GetFontOffsetForIndex(
				(unsigned char*)vFontInfos->m_ImFontAtlas.ConfigData[0].FontData,
				vFontInfos->m_ImFontAtlas.ConfigData[0].FontNo);
			if (stbtt_InitFont(&glyphFontInfo, (unsigned char*)vFontInfos->m_ImFontAtlas.ConfigData[0].FontData, font_offset))
			{
				glyphFontLoaded = true;
			}
		}

		auto io = &ImGui::GetIO();
		if (!io->Fonts->ConfigData.empty())
		{
			const int font_offset = stbtt_GetFontOffsetForIndex(
				(unsigned char*)io->Fonts->ConfigData[0].FontData,
				io->Fonts->ConfigData[0].FontNo);
			if (stbtt_InitFont(&labelFontInfo, (unsigned char*)io->Fonts->ConfigData[0].FontData, font_offset))
			{
				labelFontLoaded = true;
			}
		}
		
		if (glyphFontLoaded && labelFontLoaded)
		{
			// will write one glyph labeled

			// array of bytes
			std::vector<uint8_t> arr;
			for (const auto& it : vLabels)
			{
				uint32_t codePoint = it.first;
				std::string finalLabelFile = vPath + it.second + ".png";
				std::string lblToRender = " " + it.second;
				uint32_t finalHeight = ct::maxi(vGlyphHeight, vLabelHeight);
				uint32_t finalWidth = vGlyphHeight + (uint32_t)lblToRender.size() * vLabelHeight;
				arr.resize((size_t)finalWidth * (size_t)finalHeight);
				memset(arr.data(), 0, arr.size());
				auto text = lblToRender.c_str();

				int xpos = 2; // leave a little padding in case the character extends left;
				int ypos = 2; // leave a little padding in case the character extends left;

				// one char for the glyph
				float glyphScale = stbtt_ScaleForPixelHeight(&glyphFontInfo, (float)(vGlyphHeight));
				int ascent, baseline;
				stbtt_GetFontVMetrics(&glyphFontInfo, &ascent, 0, 0);
				baseline = (int)(ascent * glyphScale);
				int advance, lsb, x0, y0, x1, y1;
				float x_shift = xpos - (float)floor(xpos);
				float y_shift = ypos - (float)floor(ypos);
				stbtt_GetCodepointHMetrics(&glyphFontInfo, codePoint, &advance, &lsb);
				stbtt_GetCodepointBitmapBoxSubpixel(&glyphFontInfo, codePoint, glyphScale, glyphScale, x_shift, y_shift, &x0, &y0, &x1, &y1);
				int32_t x = (uint32_t)xpos + x0;
				int32_t y = baseline + y0;
				while (x < 0 || y < 0) // we decrease scale until glyph can be added in picture
				{
					glyphScale *= 0.9f;
					stbtt_GetCodepointBitmapBoxSubpixel(&glyphFontInfo, codePoint, glyphScale, glyphScale, x_shift, y_shift, &x0, &y0, &x1, &y1);
					x = (uint32_t)xpos + x0;
					y = baseline + y0;
				}
				//y_shift = finalHeight * 0.5f - (float)(y1 - y0) * 0.5f;
				if (x >= 0 && y >= 0)
				{
					uint8_t* ptr = arr.data() + (size_t)finalWidth * (size_t)y + (size_t)x;
					stbtt_MakeCodepointBitmapSubpixel(&glyphFontInfo, ptr, x1 - x0, y1 - y0, finalWidth, glyphScale, glyphScale, x_shift, y_shift, codePoint);
				}
				// note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
				// because this API is really for baking character bitmaps into textures. if you want to render
				// a sequence of characters, you really need to render each bitmap to a temp buffer, then
				// "alpha blend" that into the working buffer
				xpos += (advance * glyphScale);
				if (text[0])
					xpos += glyphScale * stbtt_GetCodepointKernAdvance(&labelFontInfo, codePoint, text[0]);

				// the rest for the label
				float labelScale = stbtt_ScaleForPixelHeight(&labelFontInfo, (float)(vLabelHeight));
				stbtt_GetFontVMetrics(&labelFontInfo, &ascent, 0, 0);
				baseline = (int)(ascent * labelScale);

				int ch = 0;
				while (text[ch])
				{
					int advance, lsb, x0, y0, x1, y1;
					x_shift = xpos - (float)floor(xpos);
					stbtt_GetCodepointHMetrics(&labelFontInfo, text[ch], &advance, &lsb);
					y_shift = vGlyphHeight * 0.5f - vLabelHeight * 0.5f;
					stbtt_GetCodepointBitmapBoxSubpixel(&labelFontInfo, text[ch], labelScale, labelScale, x_shift, y_shift, &x0, &y0, &x1, &y1);

					//&screen[baseline + y0][(int) xpos + x0]
					//arr of w * h
					//arr[x][y] == arr[w * y + x]
					uint32_t x = (uint32_t)xpos + x0;
					uint32_t y = baseline + y0;
					if (x >= 0 && y >= 0)
					{
						uint8_t* ptr = arr.data() + (size_t)finalWidth * (size_t)y + (size_t)x;
						stbtt_MakeCodepointBitmapSubpixel(&labelFontInfo, ptr, x1 - x0, y1 - y0, finalWidth, labelScale, labelScale, x_shift, y_shift, text[ch]);
					}
					// note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
					// because this API is really for baking character bitmaps into textures. if you want to render
					// a sequence of characters, you really need to render each bitmap to a temp buffer, then
					// "alpha blend" that into the working buffer
					xpos += (advance * labelScale);
					if (text[ch + 1])
						xpos += labelScale * stbtt_GetCodepointKernAdvance(&labelFontInfo, text[ch], text[ch + 1]);
					++ch;
				}

				int res = stbi_write_png(
					finalLabelFile.c_str(),
					finalWidth,
					finalHeight,
					1,
					arr.data(),
					finalWidth);

				if (res)
				{
					printf("Succes writing of a glyph + label to %s\n", finalLabelFile.c_str());
				}
				else
				{
					printf("Failed writing of a glyph + label to %s\n", finalLabelFile.c_str());
				}
			}
		}
	}
}

void HeaderPictureGenerator::WriteGlyphCardToPicture(
	const std::string& vFilePathName, std::map<uint32_t, std::string> vLabels, 
	FontInfos* vFontInfos, const uint32_t& vGlyphHeight, const uint32_t& vMaxRows)
{
	if (vFontInfos && vGlyphHeight && vMaxRows)
	{
		stbtt_fontinfo glyphFontInfo;
		stbtt_fontinfo labelFontInfo;

		bool glyphFontLoaded = false;
		bool labelFontLoaded = false;

		if (!vFontInfos->m_ImFontAtlas.ConfigData.empty())
		{
			const int font_offset = stbtt_GetFontOffsetForIndex(
				(unsigned char*)vFontInfos->m_ImFontAtlas.ConfigData[0].FontData,
				vFontInfos->m_ImFontAtlas.ConfigData[0].FontNo);
			if (stbtt_InitFont(&glyphFontInfo, (unsigned char*)vFontInfos->m_ImFontAtlas.ConfigData[0].FontData, font_offset))
			{
				glyphFontLoaded = true;
			}
		}

		auto io = &ImGui::GetIO();
		if (!io->Fonts->ConfigData.empty())
		{
			const int font_offset = stbtt_GetFontOffsetForIndex(
				(unsigned char*)io->Fonts->ConfigData[0].FontData,
				io->Fonts->ConfigData[0].FontNo);
			if (stbtt_InitFont(&labelFontInfo, (unsigned char*)io->Fonts->ConfigData[0].FontData, font_offset))
			{
				labelFontLoaded = true;
			}
		}

		if (glyphFontLoaded && labelFontLoaded)
		{
			// will write one glyph labeled

			uint32_t labelHeight = (uint32_t)(vGlyphHeight * 0.5f);
			uint32_t glyphHeight = (uint32_t)(vGlyphHeight * 0.8f);
			uint32_t padding_x = (uint32_t)(vGlyphHeight * 0.1f);
			uint32_t padding_y = 2U;
			uint32_t columnCount = (uint32_t)ceil((double)vLabels.size() / (double)vMaxRows);

			// max width of the labels
			uint32_t maxLabelWidth = 0U;
			for (const auto& it : vLabels)
			{
				maxLabelWidth = ct::maxi(maxLabelWidth, (uint32_t)it.second.size());
			}
			maxLabelWidth *= labelHeight; // one mult instead of many in loops for same result

			// extend max width of the buffer for column count
			uint32_t maxWidthOfOneItem = glyphHeight + maxLabelWidth;

			// array of bytes
			std::vector<uint8_t> buffer;
			uint32_t bufferWidth = maxWidthOfOneItem * columnCount;
			uint32_t bufferHeight = vGlyphHeight * (vMaxRows + 2U);
			buffer.resize((size_t)bufferWidth * (size_t)bufferHeight);
			memset(buffer.data(), 0, buffer.size());
			
			// iteration pof labels
			int32_t xpos = padding_x;
			int32_t ypos = padding_y;
			int32_t CurColumnOffset = 0;

			// we will compute final size accoridng to the glyph and labels
			int32_t finalWidth = 0;
			int32_t finalHeight = 0;
			int32_t columnMaxWidth = 0;

			int32_t countRows = 0;
			for (const auto& it : vLabels)
			{
				uint32_t codePoint = it.first;
				std::string lblToRender = " " + it.second;
				auto text = lblToRender.c_str();

				xpos = CurColumnOffset + padding_x;

				// one char for the glyph
				float glyphScale = stbtt_ScaleForPixelHeight(&glyphFontInfo, (float)(glyphHeight));
				int32_t ascent, descent, baseline;
				stbtt_GetFontVMetrics(&glyphFontInfo, &ascent, &descent, 0);
				baseline = (int)(ascent * glyphScale);
				int32_t advance, lsb, x0, y0, x1, y1;
				stbtt_GetCodepointHMetrics(&glyphFontInfo, codePoint, &advance, &lsb);
				float x_shift = vGlyphHeight * 0.5f - (advance * glyphScale) * 0.5f;
				float y_shift = vGlyphHeight * 0.5f - (ascent - descent) * glyphScale * 0.5f;
				stbtt_GetCodepointBitmapBoxSubpixel(&glyphFontInfo, codePoint, glyphScale, glyphScale, x_shift, y_shift, &x0, &y0, &x1, &y1);
				int32_t x = (uint32_t)xpos + x0;
				int32_t y = baseline + y0;
				while (x < 0 || y < 0) // we decrease scale until glyph can be added in picture
				{
					glyphScale *= 0.9f;
					x_shift = vGlyphHeight * 0.5f - (advance * glyphScale) * 0.5f;
					y_shift = vGlyphHeight * 0.5f - (ascent - descent) * glyphScale * 0.5f;
					stbtt_GetCodepointBitmapBoxSubpixel(&glyphFontInfo, codePoint, glyphScale, glyphScale, x_shift, y_shift, &x0, &y0, &x1, &y1);
					x = (uint32_t)xpos + x0;
					y = baseline + y0;
				}
				if (x >= 0 && y >= 0)
				{
					uint8_t* ptr = buffer.data() + bufferWidth * (ypos + y) + x;
					stbtt_MakeCodepointBitmapSubpixel(&glyphFontInfo, ptr, x1 - x0, y1 - y0, bufferWidth, glyphScale, glyphScale, 0, 0, codePoint);
				}
				xpos += vGlyphHeight;

				// the rest for the label
				float labelScale = stbtt_ScaleForPixelHeight(&labelFontInfo, (float)(labelHeight));
				stbtt_GetFontVMetrics(&labelFontInfo, &ascent, &descent, 0);
				baseline = (int32_t)(ascent * labelScale);

				int32_t ch = 0;
				while (text[ch])
				{
					stbtt_GetCodepointHMetrics(&labelFontInfo, text[ch], &advance, &lsb);
					float y_shift = vGlyphHeight * 0.5f - labelHeight * 0.5f;
					stbtt_GetCodepointBitmapBoxSubpixel(&labelFontInfo, text[ch], labelScale, labelScale, 0, y_shift, &x0, &y0, &x1, &y1);
					
					x = (uint32_t)xpos + x0;
					y = baseline + y0;
					float newLabelScale = labelScale;
					while (x < 0 || y < 0) // we decrease scale until glyph can be added in picture
					{
						newLabelScale *= 0.9f;
						stbtt_GetCodepointBitmapBoxSubpixel(&labelFontInfo, codePoint, newLabelScale, newLabelScale, 0, y_shift, &x0, &y0, &x1, &y1);
						x = (uint32_t)xpos + x0;
						y = baseline + y0;
					}
					if (x >= 0 && y >= 0)
					{
						uint8_t* ptr = buffer.data() + bufferWidth * (ypos + y) + x;
						stbtt_MakeCodepointBitmapSubpixel(&labelFontInfo, ptr, x1 - x0, y1 - y0, bufferWidth, newLabelScale, newLabelScale, 0, 0, text[ch]);
					}
					// note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
					// because this API is really for baking character bitmaps into textures. if you want to render
					// a sequence of characters, you really need to render each bitmap to a temp buffer, then
					// "alpha blend" that into the working buffer
					xpos += (advance * newLabelScale);
					if (text[ch + 1])
						xpos += labelScale * stbtt_GetCodepointKernAdvance(&labelFontInfo, text[ch], text[ch + 1]);
					++ch;
				}
				
				// inc of the row count
				countRows++;
				
				// extra space
				xpos += (advance * labelScale);
				ypos += vGlyphHeight;

				// max width of the current column
				columnMaxWidth = ct::maxi(columnMaxWidth, xpos - CurColumnOffset);

				// we compute final size according to the glyph and labels wrotes in buffer
				finalWidth = ct::maxi(finalWidth, xpos);
				finalHeight = ct::maxi(finalHeight, ypos);

				// column change if needed
				if (countRows % vMaxRows == 0) // we need to change the column
				{
					ypos = padding_y;
					CurColumnOffset += columnMaxWidth + padding_x;
					columnMaxWidth = 0;
				}
			}
			
			if (finalWidth && finalHeight)
			{
				int32_t res = stbi_write_png(
					vFilePathName.c_str(),
					finalWidth + padding_x,
					finalHeight,
					1,
					buffer.data(),
					bufferWidth);

				if (res)
				{
					printf("Succes writing of the card to %s\n", vFilePathName.c_str());
				}
				else
				{
					printf("Failed writing of the card to %s\n", vFilePathName.c_str());
				}
			}
			else
			{
				printf("Failed writing of the card to %s, final computed size not ok %i, %i\n", vFilePathName.c_str(), finalWidth, finalHeight);
			}
		}
	}
}
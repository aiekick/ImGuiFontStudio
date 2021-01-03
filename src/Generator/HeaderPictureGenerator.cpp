#include "HeaderPictureGenerator.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <ctools/Logger.h>
#include <MainFrame.h>
#include <ctools/FileHelper.h>

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

	buf.resize(vChannelCount * vTextureSize.x * vTextureSize.y); // 1 channel only

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

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// prepare names and codepoint
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			std::map<uint32_t, std::string> glyphCodePoints;

			std::string prefix = "TEST";

			for (auto& it : vFontInfos->m_SelectedGlyphs)
			{
				glyphCodePoints[it.second.newCodePoint] = it.second.newHeaderName;
			}

			uint32_t minCodePoint = 65535;
			uint32_t maxCodePoint = 0;
			std::string glyphs;

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

				glyphs += "#define ICON" + prefix + "_" + glyphName + " u8\"\\u" + ct::toHexStr(codePoint) + "\"\n";
			}

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// write to texture
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			// Build texture atlas
			/*ImGuiIO& io = ImGui::GetIO();
			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bit (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.
			*/
			// Upload texture to graphics system
			/*GLint last_texture;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			glGenTextures(1, &g_FontTexture);
			glBindTexture(GL_TEXTURE_2D, g_FontTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	#ifdef GL_UNPACK_ROW_LENGTH
			glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
	#endif
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			// Store our identifier
			io.Fonts->TexID = (ImTextureID)(intptr_t)g_FontTexture;

			// Restore state
			glBindTexture(GL_TEXTURE_2D, last_texture);*/

			//////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// 
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////

			GLuint textureToSave = (GLuint)(size_t)vFontInfos->m_ImFontAtlas.TexID;
			if (textureToSave)
			{
				auto win = MainFrame::Instance()->GetGLFWwindow();
				SaveTextureToPng(win, filePathName, textureToSave, ct::uvec2(vFontInfos->m_ImFontAtlas.TexWidth, vFontInfos->m_ImFontAtlas.TexHeight), 4U);
			}
		}
	}
}
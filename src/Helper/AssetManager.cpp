// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#ifdef USE_SHADOW
#include "AssetManager.h"
#include <ctools/Logger.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

AssetManager::AssetManager()
{

}

AssetManager::~AssetManager()
{

}

void AssetManager::Clear()
{
	for (auto tex : m_Textures)
	{
		glDeleteTextures(1, &tex.second.glTex);
	}
	m_Textures.clear();
}

void AssetManager::LoadTexture2D(const std::string& vKey, const std::string vFilePathName)
{
	auto res = CreateFromFile(vFilePathName.c_str());
	if (res.glTex)
	{
		m_Textures[vKey] = res;
	}
}

ct::texture AssetManager::CreateFromFile(const char* vFilePathName, GLenum vTexType, bool vInvertY, bool vGenMipMap, std::string vWrap, std::string vFilter)
{
	ct::texture res;

	res.flipY = vInvertY;
	res.useMipMap = vGenMipMap;
	res.relativPath = vFilePathName;
	res.glTextureType = vTexType;

	int w = 0;
	int h = 0;
	int comp = 0;

	stbi_set_flip_vertically_on_load(vInvertY);

	unsigned char* image = stbi_load(vFilePathName, &w, &h, &comp, STBI_rgb);
	if (image != 0 && (comp == 4 || comp == 2))
	{
		stbi_image_free(image);
		image = stbi_load(vFilePathName, &w, &h, &comp, STBI_rgb_alpha);
	}
	if (image != 0)
	{
		res.w = w;
		res.h = h;

		if (vTexType == GL_TEXTURE_2D)
		{
			glGenTextures(1, &res.glTex);
			LogGlError();

			glBindTexture(vTexType, res.glTex);
			LogGlError();

			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			LogGlError();

			if (vWrap == "repeat")
			{
				res.glWrapS = GL_REPEAT;
				res.glWrapT = GL_REPEAT;

				glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, GL_REPEAT);
				LogGlError();
			}
			else if (vWrap == "mirror")
			{
				res.glWrapS = GL_MIRRORED_REPEAT;
				res.glWrapT = GL_MIRRORED_REPEAT;

				glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
				glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
				LogGlError();
			}
			else if (vWrap == "clamp")
			{
				res.glWrapS = GL_CLAMP_TO_EDGE;
				res.glWrapT = GL_CLAMP_TO_EDGE;

				glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				LogGlError();
			}
			else
			{
				res.glWrapS = GL_CLAMP_TO_EDGE;
				res.glWrapT = GL_CLAMP_TO_EDGE;

				glTexParameteri(vTexType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(vTexType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				LogGlError();
			}

			if (vFilter == "linear")
			{
				if (vGenMipMap)
				{
					res.glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
					res.glMagFilter = GL_LINEAR;

					glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
					glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					LogGlError();
				}
				else
				{
					res.glMinFilter = GL_LINEAR;
					res.glMagFilter = GL_LINEAR;

					glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					LogGlError();
				}
			}
			else if (vFilter == "nearest")
			{
				res.glMinFilter = GL_NEAREST;
				res.glMagFilter = GL_NEAREST;

				glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
				LogGlError();
			}
			else
			{
				res.glMinFilter = GL_LINEAR;
				res.glMagFilter = GL_LINEAR;

				glTexParameteri(vTexType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(vTexType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				LogGlError();
			}
		}

		//     N=#comp     components
		//       1           red
		//       2           red, green
		//       3           red, green, blue
		//       4           red, green, blue, alpha

		res.gldatatype = GL_UNSIGNED_BYTE;

		if (comp == 1)
		{
			res.glformat = GL_RED;
			res.glinternalformat = GL_RED;
		}
		else if (comp == 2)
		{
			res.glformat = GL_RG;
			res.glinternalformat = GL_RG;
		}
		else if (comp == 3)
		{
			res.glformat = GL_RGB;
			res.glinternalformat = GL_RGB;
		}
		else if (comp == 4)
		{
			res.glformat = GL_RGBA;
			res.glinternalformat = GL_RGBA;
		}

		glTexImage2D(vTexType, 0, res.glinternalformat, (GLsizei)res.w, (GLsizei)res.h, 0, res.glformat, GL_UNSIGNED_BYTE, image);
		LogGlError();

		if (vGenMipMap)
		{
			glGenerateMipmap(vTexType);
			LogGlError();
		}

		glFinish();
		LogGlError();

		glBindTexture(GL_TEXTURE_2D, 0);
		LogGlError();

		stbi_image_free(image);
	}
	else
	{
		LogStr("Failed to load texture");
	}

	return res;
}
#endif
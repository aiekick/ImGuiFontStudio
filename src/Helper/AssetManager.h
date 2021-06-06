#pragma once

#ifdef USE_SHADOW
#include <glad/glad.h>
#include <ctools/cTools.h>

#include <string>
#include <iostream>
#include <exception>
#include <unordered_map>

class AssetManager
{
public:
	std::unordered_map<std::string, ct::texture> m_Textures;

public:
	void LoadTexture2D(const std::string& vKey, const std::string vFilePathName);
	void Clear();

private:
	ct::texture CreateFromFile(const char* vFilePathName, GLenum vTexType = GL_TEXTURE_2D, bool vInvertY = false,
		bool vGenMipMap = true, std::string vWrap = "clamp", std::string vFilter = "linear");

public: // singleton
	static AssetManager* Instance()
	{
		static AssetManager _instance;
		return &_instance;
	}

protected:
	AssetManager(); // Prevent construction
	AssetManager(const AssetManager&) {}; // Prevent construction by copying
	AssetManager& operator =(const AssetManager&) { return *this; }; // Prevent assignment
	~AssetManager(); // Prevent unwanted destruction
};
#endif
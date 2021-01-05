#pragma once

#include <string>
#include <cstdint>

class Compress
{
public:
	static std::string GetCompressedBase85BytesArray(
		const std::string& vFilePathName,
		const std::string& vPrefix,
		std::string* vBufferName,
		size_t* vBufferSize = 0);
};
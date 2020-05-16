/*
 * Copyright 2020 Stephane Cuillerdier (aka Aiekick)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <cstdint>
#include <vector>
#include <string>

class MemoryStream
{
public:
	MemoryStream();
	MemoryStream(uint8_t *vDatas, size_t vSize);
	~MemoryStream();

	void WriteByte(uint8_t b);
	void WriteBytes(std::vector<uint8_t> *buffer);
	void WriteShort(int32_t i);
	void WriteUShort(int32_t us);
	void WriteInt(int32_t i);
	void WriteUInt24(int32_t ui);
	void WriteULong(int64_t ul);
	void WriteLong(int64_t l);
	void WriteFixed(int32_t f);
	void WriteDateTime(int64_t date);

	uint8_t* Get();
	void Set(uint8_t *vDatas, size_t vSize);
	
	size_t Size();

	size_t GetPos();
	void SetPos(size_t vPos);
	
	int32_t ReadByte();
	int32_t ReadUShort();
	int32_t ReadShort();
	int32_t ReadUInt24();
	int64_t ReadULong();
	int32_t ReadULongAsInt();
	int32_t ReadLong();
	int32_t ReadFixed();
	int64_t ReadDateTimeAsLong();
	std::string ReadString(size_t vLen);

private:
	std::vector<uint8_t> m_Datas;
	size_t m_ReadPos = 0;
};


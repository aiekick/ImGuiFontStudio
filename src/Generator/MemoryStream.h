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
#include <cmath>

class MemoryStream
{
public:
	struct Fixed // its like a float with int32 : high.low ex : 1 high 0 low => 1.0
	{
		int16_t high = 0;
		int16_t low = 0;
	};
	struct F2DOT14
	{
		int16_t value = 0;
		F2DOT14() { value = 0; }
		F2DOT14(const int16_t& v) { value = v; }
		void operator = (const int16_t& v) { value = v; }
		void SetFloat(const float& vValue) { value = (int16_t)roundf(vValue * 16384.f); }
		float GetFloat() { return (float)(value >> 14) + (float)(value & 0x3FFF) / 16384.0f; }
	};
	typedef uint16_t UFWord;
	typedef int16_t FWord;
	typedef int64_t longDateTime;
	
public:
	MemoryStream();
	MemoryStream(uint8_t *vDatas, size_t vSize);
	~MemoryStream();

	void WriteByte(uint8_t b);
	void WriteBytes(std::vector<uint8_t> *buffer);
	void WriteShort(int32_t i);
	void WriteUShort(int32_t us);
	void WriteFWord(int32_t us);
	void WriteInt(int32_t i);
	void WriteUInt24(int32_t ui);
	void WriteULong(int64_t ul);
	void WriteLong(int64_t l);
	void WriteFixed(Fixed f);
	void WriteF2DOT14(F2DOT14 f);
	void WriteDateTime(longDateTime date);

	uint8_t* Get();
	void Set(uint8_t *vDatas, size_t vSize);
	
	size_t Size();

	size_t GetPos();
	void SetPos(size_t vPos);
	
	uint8_t ReadByte();
	int32_t ReadUShort();
	int32_t ReadShort();
	FWord ReadFWord();
	uint32_t ReadUInt24();
	uint64_t ReadULong();
	uint32_t ReadULongAsInt();
	int32_t ReadLong();
	Fixed ReadFixed();
	F2DOT14 ReadF2DOT14();
	longDateTime ReadDateTime();
	std::string ReadString(size_t vLen);

private:
	std::vector<uint8_t> m_Datas;
	size_t m_ReadPos = 0;
};


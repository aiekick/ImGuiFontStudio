// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "MemoryStream.h"

#include <cstring>

MemoryStream::MemoryStream() = default;

MemoryStream::MemoryStream(uint8_t *vDatas, size_t vSize)
{
	Set(vDatas, vSize);
}

MemoryStream::~MemoryStream() = default;

void MemoryStream::WriteByte(uint8_t b)
{
	m_Datas.push_back(b);
}

void MemoryStream::WriteBytes(std::vector<uint8_t> *buffer)
{
	if (buffer)
	{
		m_Datas.insert(m_Datas.end(), buffer->begin(), buffer->end());
	}
}

void MemoryStream::WriteInt(int32_t i)
{
	WriteByte((uint8_t)((i >> 24) & 0xff));
	WriteByte((uint8_t)((i >> 16) & 0xff));
	WriteByte((uint8_t)((i >> 8) & 0xff));
	WriteByte((uint8_t)(i & 0xff));
}

void MemoryStream::WriteUShort(int32_t us)
{
	WriteByte((uint8_t)((us >> 8) & 0xff));
	WriteByte((uint8_t)(us & 0xff));
}

void MemoryStream::WriteFWord(int32_t us)
{
	WriteUShort(us);
}

void MemoryStream::WriteShort(int32_t s)
{
	WriteUShort(s);
}

void MemoryStream::WriteUInt24(int32_t ui)
{
	WriteByte((uint8_t)(ui >> 16) & 0xff);
	WriteByte((uint8_t)(ui >> 8) & 0xff);
	WriteByte((uint8_t)ui & 0xff);
}

void MemoryStream::WriteULong(int64_t ul)
{
	WriteByte((uint8_t)((ul >> 24) & 0xff));
	WriteByte((uint8_t)((ul >> 16) & 0xff));
	WriteByte((uint8_t)((ul >> 8) & 0xff));
	WriteByte((uint8_t)(ul & 0xff));
}

void MemoryStream::WriteLong(int64_t l)
{
	WriteULong(l);
}

void MemoryStream::WriteFixed(Fixed f)
{
	WriteByte((uint8_t)((f.high >> 24) & 0xff));
	WriteByte((uint8_t)((f.high >> 16) & 0xff));
	WriteByte((uint8_t)((f.low >> 8) & 0xff));
	WriteByte((uint8_t)(f.low & 0xff));
}

void MemoryStream::WriteDateTime(longDateTime date)
{
	WriteULong((date >> 32) & 0xffffffff);
	WriteULong(date & 0xffffffff);
}

uint8_t* MemoryStream::Get()
{
	return m_Datas.data();
}

size_t MemoryStream::Size()
{
	return m_Datas.size();
}

size_t MemoryStream::GetPos()
{
	return m_ReadPos;
}

void MemoryStream::SetPos(size_t vPos)
{
	m_ReadPos = vPos;
}

void MemoryStream::Set(uint8_t *vDatas, size_t vSize)
{
	if (vDatas && vSize)
	{
		m_Datas.clear();
		m_Datas.resize(vSize);

		memcpy(m_Datas.data(), vDatas, vSize);
	}
}

uint8_t MemoryStream::ReadByte()
{
	if (m_ReadPos < m_Datas.size())
		return m_Datas[m_ReadPos++];
	return 0;
}

int32_t MemoryStream::ReadUShort()
{
	return 0xffff & (ReadByte() << 8 | ReadByte());
}

int32_t MemoryStream::ReadShort()
{
	return ((ReadByte() << 8 | ReadByte()) << 16) >> 16;
}

MemoryStream::FWord MemoryStream::ReadFWord()
{
	return ReadShort();
}

uint32_t MemoryStream::ReadUInt24()
{
	return 0xffffff & (ReadByte() << 16 | ReadByte() << 8 | ReadByte());
}

uint64_t MemoryStream::ReadULong()
{
	return 0xffffffffL & ReadLong();
}

uint32_t MemoryStream::ReadULongAsInt()
{
	int64_t ulong = ReadULong();
	return ((int32_t)ulong) & ~0x80000000;
}

int32_t MemoryStream::ReadLong()
{
	return ReadByte() << 24 | ReadByte() << 16 | ReadByte() << 8 | ReadByte();
}

MemoryStream::Fixed MemoryStream::ReadFixed()
{
	Fixed res;
	int32_t f = ReadLong();
	res.high = (int16_t)((f >> 16) & 0xff);
	res.low = (int16_t)(f & 0xff);
	return res;
}

MemoryStream::longDateTime MemoryStream::ReadDateTime()
{
	return (int64_t)ReadULong() << 32 | ReadULong();
}

std::string MemoryStream::ReadString(size_t vLen)
{
	std::string res = std::string((char*)(m_Datas.data() + m_ReadPos), vLen);
	m_ReadPos += vLen;
	return res;
}
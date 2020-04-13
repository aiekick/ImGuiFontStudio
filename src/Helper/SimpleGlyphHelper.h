#pragma once

#include <string>
#include <set>
#include <map>
#include <unordered_map>

#include "sfntly/tag.h"
#include "sfntly/font.h"
#include "sfntly/port/type.h"
#include "sfntly/port/refcount.h"
#include "sfntly/table/core/cmap_table.h"
#include "sfntly/table/core/post_script_table.h"
#include "sfntly/table/truetype/glyph_table.h"
#include "sfntly/table/truetype/loca_table.h"

class SimpleGlyphHelper
{
private:
	int32_t m_InstructionSize = 0;
	int32_t m_InstructionOffset = 0;
	sfntly::ReadableFontData* m_Datas;

private:
	int OFFSET_NumberOfContours = 0;
	int OFFSET_XMin = 2;
	int OFFSET_YMin = 4;
	int OFFSET_XMax = 6;
	int OFFSET_YMax = 8;
	int OFFSET_SimpleEndPtsOfCountours = 10;
	int OFFSET_SimpleInstructionLength = 0;
	int OFFSET_SimpleInstructions = 2;

public:
	SimpleGlyphHelper(sfntly::ReadableFontData *vReadableFontData);
	~SimpleGlyphHelper();

private:
	/*void Initialize()
	{
		if (m_Datas->Length() == 0) {
			return;
		}

		m_InstructionSize = m_Datas->ReadUShort(OFFSET_SimpleEndPtsOfCountours + NumberOfContours() * DataSize::kUSHORT);
		instructions_offset_ = Offset::kSimpleEndPtsOfCountours +
			(NumberOfContours() + 1) * DataSize::kUSHORT;
		flags_offset_ = instructions_offset_ + instruction_size_ * DataSize::kBYTE;
		number_of_points_ = ContourEndPoint(NumberOfContours() - 1) + 1;
		x_coordinates_.resize(number_of_points_);
		y_coordinates_.resize(number_of_points_);
		on_curve_.resize(number_of_points_);
		ParseData(false);
		x_coordinates_offset_ = flags_offset_ + flag_byte_count_ * DataSize::kBYTE;
		y_coordinates_offset_ = x_coordinates_offset_ + x_byte_count_ *
			DataSize::kBYTE;
		contour_index_.resize(NumberOfContours() + 1);
		contour_index_[0] = 0;
		for (uint32_t contour = 0; contour < contour_index_.size() - 1; ++contour) {
			contour_index_[contour + 1] = ContourEndPoint(contour) + 1;
		}
		ParseData(true);
		int32_t non_padded_data_length =
			5 * DataSize::kSHORT +
			(NumberOfContours() * DataSize::kUSHORT) +
			DataSize::kUSHORT +
			(instruction_size_ * DataSize::kBYTE) +
			(flag_byte_count_ * DataSize::kBYTE) +
			(x_byte_count_ * DataSize::kBYTE) +
			(y_byte_count_ * DataSize::kBYTE);
		set_padding(DataLength() - non_padded_data_length);
		initialized_ = true;
	}

	int32_t GetNumPoints(int32_t contour)
	{
		Initialize();
		if (contour >= NumberOfContours())
		{
			return 0;
		}
		return contour_index_[contour + 1] - contour_index_[contour];
	}

	int32_t xCoordinate(int32_t contour, int32_t point)
	{
		Initialize();
		return x_coordinates_[contour_index_[contour] + point];
	}

	int32_t yCoordinate(int32_t contour, int32_t point)
	{
		Initialize();
		return y_coordinates_[contour_index_[contour] + point];
	}

	bool onCurve(int32_t contour, int32_t point)
	{
		Initialize();
		return on_curve_[contour_index_[contour] + point];
	}

	void ParseData(bool fill_arrays) 
	{
		int32_t flag = 0;
		int32_t flag_repeat = 0;
		int32_t flag_index = 0;
		int32_t x_byte_index = 0;
		int32_t y_byte_index = 0;

		for (int32_t point_index = 0; point_index < number_of_points_;
			++point_index) 
		{
			// get the flag for the current point
			if (flag_repeat == 0) {
				flag = FlagAsInt(flag_index++);
				if ((flag & kFLAG_REPEAT) == kFLAG_REPEAT) {
					flag_repeat = FlagAsInt(flag_index++);
				}
			}
			else {
				flag_repeat--;
			}

			// on the curve?
			if (fill_arrays) {
				on_curve_[point_index] = ((flag & kFLAG_ONCURVE) == kFLAG_ONCURVE);
			}
			// get the x coordinate
			if ((flag & kFLAG_XSHORT) == kFLAG_XSHORT) {
				// single byte x coord value
				if (fill_arrays) {
					x_coordinates_[point_index] =
						data_->ReadUByte(x_coordinates_offset_ + x_byte_index);
					x_coordinates_[point_index] *=
						((flag & kFLAG_XREPEATSIGN) == kFLAG_XREPEATSIGN) ? 1 : -1;
				}
				x_byte_index++;
			}
			else {
				// double byte coord value
				if (!((flag & kFLAG_XREPEATSIGN) == kFLAG_XREPEATSIGN)) {
					if (fill_arrays) {
						x_coordinates_[point_index] =
							data_->ReadShort(x_coordinates_offset_ + x_byte_index);
					}
					x_byte_index += 2;
				}
			}
			if (fill_arrays && point_index > 0) {
				x_coordinates_[point_index] += x_coordinates_[point_index - 1];
			}

			// get the y coordinate
			if ((flag & kFLAG_YSHORT) == kFLAG_YSHORT) {
				if (fill_arrays) {
					y_coordinates_[point_index] =
						data_->ReadUByte(y_coordinates_offset_ + y_byte_index);
					y_coordinates_[point_index] *=
						((flag & kFLAG_YREPEATSIGN) == kFLAG_YREPEATSIGN) ? 1 : -1;
				}
				y_byte_index++;
			}
			else {
				if (!((flag & kFLAG_YREPEATSIGN) == kFLAG_YREPEATSIGN)) {
					if (fill_arrays) {
						y_coordinates_[point_index] =
							data_->ReadShort(y_coordinates_offset_ + y_byte_index);
					}
					y_byte_index += 2;
				}
			}
			if (fill_arrays && point_index > 0) {
				y_coordinates_[point_index] += y_coordinates_[point_index - 1];
			}
		}
		flag_byte_count_ = flag_index;
		x_byte_count_ = x_byte_index;
		y_byte_count_ = y_byte_index;
	}

	int32_t FlagAsInt(int32_t index) 
	{
		return data_->ReadUByte(flags_offset_ + index * DataSize::kBYTE);
	}

	int32_t ContourEndPoint(int32_t contour) 
	{
		return data_->ReadUShort(contour * DataSize::kUSHORT +
			Offset::kSimpleEndPtsOfCountours);
	}*/
};

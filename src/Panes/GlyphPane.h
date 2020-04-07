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

#include "Helper/FontHelper.h"

#include <imgui/imgui.h>

#include <stdint.h>
#include <string>
#include <map>

#include "sfntly/table/truetype/glyph_table.h"

// Imported from https://github.com/rillig/sfntly/blob/master/java/src/com/google/typography/font/tools/fontviewer/GlyphNode.java
// Translates coordinates from the glyph coordinate for a specific contour system to the screen.
class ScreenCoordinateMapper
{
private:
	sfntly::GlyphTable::SimpleGlyph *glyph;
	int contour;
	int points;
	int margin;
	double scale;
	double minX;
	double maxY;

public:
	ScreenCoordinateMapper(sfntly::GlyphTable::SimpleGlyph *vGlyph, int vContour, int vMargin, double vScale, double vMinX, double vMaxY)
	{
		glyph = vGlyph;
		contour = vContour;
		points = vGlyph->numberOfPoints(contour);
		margin = vMargin;
		scale = vScale;
		minX = vMinX;
		maxY = vMaxY;
	}

	/** The x coordinate on the screen for the given x coordinate in the glyph coordinate system. */
	int x(double x)
	{
		return margin + (int)round(scale * (x - minX));
	}

	/** The y coordinate on the screen for the given y coordinate in the glyph coordinate system. */
	int y(int y)
	{
		return margin + (int)round(scale * (maxY - y));
	}

	/** For a simple glyph, the x screen coordinate for the given point on the contour. */
	int cx(int point)
	{
		int _x = glyph->xCoordinate(contour, index(point));
		return x((double)_x);
	}

	/** For a simple glyph, the y screen coordinate for the given point on the contour. */
	int cy(int point)
	{
		int _y = glyph->yCoordinate(contour, index(point));
		return y((double)_y);
	}

	int index(int point)
	{
		return (point + points) % points;
	}

	/** For a simple glyph, whether the point is on the curve, or off the curve. */
	bool onCurve(int point)
	{
		return glyph->onCurve(contour, index(point));
	}
};

class ProjectFile;
class FontInfos;
class GlyphPane
{
private:
	sfntly::Ptr<sfntly::GlyphTable::SimpleGlyph> m_SimpleGlyph;
	FontInstance m_fontInstance;

public:
	int DrawGlyphPane(ProjectFile *vProjectFile, int vWidgetId);
	bool LoadGlyph(ProjectFile *vProjectFile, FontInfos* vFontInfos, ImWchar vCodepoint);

private:
	bool DrawSimpleGlyph(sfntly::Ptr<sfntly::GlyphTable::SimpleGlyph> vSimpleGlyph, float vScale);

public: // singleton
	static GlyphPane *Instance()
	{
		static GlyphPane *_instance = new GlyphPane();
		return _instance;
	}

protected:
	GlyphPane(); // Prevent construction
	GlyphPane(const GlyphPane&) {}; // Prevent construction by copying
	GlyphPane& operator =(const GlyphPane&) { return *this; }; // Prevent assignment
	~GlyphPane(); // Prevent unwanted destruction};
};


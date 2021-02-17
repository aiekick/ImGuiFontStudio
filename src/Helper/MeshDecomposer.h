#pragma once

#include <vector>
#include <set>
#include <ctools/cTools.h>
#include <Project/BaseGlyph.h>

typedef int PartitionAlgoFlags;
enum PartitionAlgoFlags_
{
	PARTITION_ALGO_NONE = 0,
	PARTITION_ALGO_EAR_CLIPPING = (1 << 0),
	PARTITION_ALGO_MONO = (1 << 1),
	PARTITION_ALGO_CONVEX_HM = (1 << 2),
	PARTITION_ALGO_RADIO = PARTITION_ALGO_EAR_CLIPPING | PARTITION_ALGO_MONO | PARTITION_ALGO_CONVEX_HM
};

class MeshDecomposer
{
public:
	static GlyphMesh Decompose(const GlyphIndex& vGlyphIndex, const GlyphMesh& vGlyphMesh, PartitionAlgoFlags vPartitionAlgo);
};
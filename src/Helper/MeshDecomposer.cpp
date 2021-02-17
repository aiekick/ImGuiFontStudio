#include "MeshDecomposer.h"
#include <cmath>
#include <Helper/Profiler.h>

#include <polypartition/src/polypartition.h>

GlyphMesh MeshDecomposer::Decompose(const GlyphIndex& vGlyphIndex, const GlyphMesh& vGlyphMesh, PartitionAlgoFlags vPartitionAlgo)
{
    ZoneScoped;

    TPPLPartition pp;

    TPPLPolyList polys_with_holes;
    for (auto c : vGlyphMesh)
    {
        TPPLPoly poly;
        poly.Init((long)c.size());
        for (size_t i = 0; i < c.size(); i++)
        {
            poly[i].x = c[i].x;
            poly[i].y = c[i].y;
        }
        
        poly.Invert();

        const auto ori = poly.GetOrientation();
        if (ori == TPPLOrientation::TPPL_ORIENTATION_CW)
            poly.SetHole(true);

        polys_with_holes.push_back(poly);
    }

    GlyphMesh res = vGlyphMesh;

    if ((vPartitionAlgo & PARTITION_ALGO_EAR_CLIPPING) == PARTITION_ALGO_EAR_CLIPPING)
    {
        ZoneScopedN("RemoveHoles");

        TPPLPolyList polys;
        if (pp.RemoveHoles(&polys_with_holes, &polys))
        {
            ZoneScopedN("Triangulate_EC");

            TPPLPolyList result;
            if (pp.Triangulate_EC(&polys, &result))
            {
                res.clear();
                for (const auto poly : result)
                {
                    GlyphContour c;
                    for (long i = 0; i < poly.GetNumPoints(); i++)
                    {
                        c.push_back(ct::fvec2(poly[i].x, poly[i].y));
                    }
                    res.push_back(c);
                }
            }
            else
            {
                printf("p ---------\n"); 
                printf("Fail to mesh glyph %u\n", vGlyphIndex);
                for (auto po : polys)
                {
                    auto pts = po.GetPoints();
                    size_t pmax = (size_t)po.GetNumPoints();
                    for (size_t i = 0; i < po.GetNumPoints(); i++)
                    {
                        printf("p %.2f %.2f\n", pts[i].x, pts[i].y);
                    }
                    printf("p ---------\n");
                }
            }
        }
        else
            printf("Fail to mesh glyph %u\n", vGlyphIndex);
    }
    else if ((vPartitionAlgo & PARTITION_ALGO_MONO) == PARTITION_ALGO_MONO)
    {
        ZoneScopedN("Triangulate_MONO");

        TPPLPolyList result;
        if (pp.Triangulate_MONO(&polys_with_holes, &result))
        {
            res.clear();
            for (const auto poly : result)
            {
                GlyphContour c;
                for (long i = 0; i < poly.GetNumPoints(); i++)
                {
                    c.push_back(ct::fvec2(poly[i].x, poly[i].y));
                }
                res.push_back(c);
            }
        }
        else
            printf("Fail to mesh glyph %u\n", vGlyphIndex);
    }
    else if ((vPartitionAlgo & PARTITION_ALGO_CONVEX_HM) == PARTITION_ALGO_CONVEX_HM)
    {
        ZoneScopedN("RemoveHoles");

        TPPLPolyList polys;
        if (pp.RemoveHoles(&polys_with_holes, &polys))
        {
            ZoneScopedN("ConvexPartition_HM");

            TPPLPolyList result;
            if (pp.ConvexPartition_HM(&polys, &result))
            {
                res.clear();
                for (const auto poly : result)
                {
                    GlyphContour c;
                    for (long i = 0; i < poly.GetNumPoints(); i++)
                    {
                        c.push_back(ct::fvec2(poly[i].x, poly[i].y));
                    }
                    res.push_back(c);
                }
            }
            else
                printf("Fail to mesh glyph %u\n", vGlyphIndex);
        }
        else
            printf("Fail to mesh glyph %u\n", vGlyphIndex);
    }

    return res;
}
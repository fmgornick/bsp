#ifndef DCEL_H_
#define DCEL_H_

#include "bsp.h"
#include "f64_segment.h"
#include "f64_vector.h"

typedef struct VertexEntry VertexEntry;
typedef struct FaceEntry FaceEntry;
typedef struct HalfEdgeEntry HalfEdgeEntry;

typedef struct VertexEntry {
    DVector2 coordinates;
    HalfEdgeEntry *incidentEdge;
} VertexEntry;

typedef struct FaceEntry {
    HalfEdgeEntry *outerComponent;
    HalfEdgeEntry **innerComponents;
    usize numInnerComponents;
} FaceEntry;

typedef struct HalfEdgeEntry {
    VertexEntry *origin;
    HalfEdgeEntry *twin;
    FaceEntry *incidentFace;
    HalfEdgeEntry *next;
    HalfEdgeEntry *prev;
} HalfEdgeEntry;

/*
 * this datastructure does not cover all cases needed to be a proper DCEL...
 *
 * 1. assumes no holes, only the unbounded face has inner components
 * 2. inner component size assumed to be 1 (still malloc it tho *shrug*)
 * 3. assumes at least 3 edges (6 half-edges)
 * 4. assumes graph stored follows euler's formula
 * 5. assumes when adding a diagonal, it's an actual diagonal and the two
 *    vertices provided share an incident face
 *    i.  it will loop infinitely if diagonals aren't proper
 *        (diagonals only added in triangulation alg tho so no worries)
 *
 * i know this isn't all that ideal, but currently this is only getting used
 * for triangluating BSP tree regions which are all convex with around 4-6
 * edges so i didn't really care to make it full-proof
 */
typedef struct DCEL {
    VertexEntry **vertices;
    FaceEntry **faces;
    HalfEdgeEntry **halfEdges;
    usize numVertices;
    usize numFaces;
    usize numHalfEdges;
    usize openVertex;
    usize openFace;
    usize openHalfEdge;
} DCEL;

DCEL *NewDCEL(usize numVertices, usize numFaces, usize numEdges);
DCEL *BuildSimpleDCEL(DSegment *segments, usize numSegments);
void FreeDCEL(DCEL *dcel);

void ResizeDCEL(DCEL *dcel, usize numVertices, usize numFaces, usize numHalfEdges);
void AddDiagonalDCEL(DCEL *dcel, VertexEntry *v1, VertexEntry *v2);
void PrintDCEL(DCEL *dcel);

#endif // DCEL_H_

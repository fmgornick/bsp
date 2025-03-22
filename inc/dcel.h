#ifndef DCEL_H_
#define DCEL_H_

#include "bsp_segment.h"
#include "bsp_utils.h"
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
DCEL *BuildSimpleDCEL(Segment *edges, usize numEdges);
void FreeDCEL(DCEL *dcel);

void ResizeDCEL(DCEL *dcel, usize numVertices, usize numFaces, usize numHalfEdges);
void AddDiagonalDCEL(DCEL *dcel, VertexEntry *v1, VertexEntry *v2);
void PrintDCEL(DCEL *dcel);

#endif // DCEL_H_

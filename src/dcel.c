#include "dcel.h"
#include "bsp.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

DCEL *
NewDCEL(usize numVertices, usize numFaces, usize numHalfEdges)
{
    assert(numVertices >= 3);
    assert(numFaces >= 2);
    assert(numHalfEdges >= 6);
    assert(numFaces - (numHalfEdges / 2) + numVertices == 2);
    DCEL *dcel = (DCEL *)malloc(sizeof(DCEL));
    dcel->vertices = (VertexEntry **)malloc(numVertices * sizeof(VertexEntry *));
    for (usize i = 0; i < numVertices; i++)
        dcel->vertices[i] = (VertexEntry *)malloc(sizeof(VertexEntry));
    dcel->faces = (FaceEntry **)malloc(numFaces * sizeof(FaceEntry *));
    for (usize i = 0; i < numFaces; i++)
        dcel->faces[i] = (FaceEntry *)malloc(sizeof(FaceEntry));
    dcel->halfEdges = (HalfEdgeEntry **)malloc(numHalfEdges * 2 * sizeof(HalfEdgeEntry *));
    for (usize i = 0; i < numHalfEdges; i++)
        dcel->halfEdges[i] = (HalfEdgeEntry *)malloc(sizeof(HalfEdgeEntry));
    dcel->numVertices = numVertices;
    dcel->numFaces = numFaces;
    dcel->numHalfEdges = numHalfEdges;
    dcel->openVertex = 0;
    dcel->openFace = 0;
    dcel->openHalfEdge = 0;
    return dcel;
}

void
ResizeDCEL(DCEL *dcel, usize numVertices, usize numFaces, usize numHalfEdges)
{
    assert(numVertices >= dcel->numVertices);
    assert(numFaces >= dcel->numFaces);
    assert(numHalfEdges >= dcel->numHalfEdges);
    assert(numVertices >= 3);
    assert(numFaces >= 2);
    assert(numHalfEdges >= 6);
    assert(numFaces - (numHalfEdges / 2) + numVertices == 2);
    VertexEntry **vertices = (VertexEntry **)malloc(numVertices * sizeof(VertexEntry *));
    FaceEntry **faces = (FaceEntry **)malloc(numFaces * sizeof(FaceEntry *));
    HalfEdgeEntry **halfEdges = (HalfEdgeEntry **)malloc(numHalfEdges * sizeof(HalfEdgeEntry *));
    for (usize i = 0; i < numVertices; i++)
    {
        if (i < dcel->numVertices) vertices[i] = dcel->vertices[i];
        else vertices[i] = (VertexEntry *)malloc(sizeof(VertexEntry));
    }
    for (usize i = 0; i < numFaces; i++)
    {
        if (i < dcel->numFaces) faces[i] = dcel->faces[i];
        else faces[i] = (FaceEntry *)malloc(sizeof(FaceEntry));
    }
    for (usize i = 0; i < numHalfEdges; i++)
    {
        if (i < dcel->numHalfEdges) halfEdges[i] = dcel->halfEdges[i];
        else halfEdges[i] = (HalfEdgeEntry *)malloc(sizeof(HalfEdgeEntry));
    }
    free(dcel->vertices);
    free(dcel->faces);
    free(dcel->halfEdges);
    dcel->vertices = vertices;
    dcel->faces = faces;
    dcel->halfEdges = halfEdges;
    dcel->openVertex = dcel->numVertices;
    dcel->openFace = dcel->numFaces;
    dcel->openHalfEdge = dcel->numHalfEdges;
    dcel->numVertices = numVertices;
    dcel->numFaces = numFaces;
    dcel->numHalfEdges = numHalfEdges;
}

DCEL *
BuildSimpleDCEL(Segment *segments, usize numSegments)
{
    assert(numSegments >= 1);
    DCEL *dcel = NewDCEL(numSegments, 2, numSegments * 2);
    {
        for (usize i = 0; i < numSegments; i++)
        {
            dcel->vertices[i]->coordinates = segments[i].left;
            dcel->vertices[i]->incidentEdge = dcel->halfEdges[2 * i];
        }
        {
            /* outer face (even edges move counter-clockwise) */
            dcel->faces[0]->innerComponents = (HalfEdgeEntry **)malloc(1 * sizeof(HalfEdgeEntry *));
            dcel->faces[0]->numInnerComponents = 1;
            dcel->faces[0]->innerComponents[0] = dcel->halfEdges[1];
            dcel->faces[0]->outerComponent = NULL;
            /* inner face (odd edges move clockwise) */
            dcel->faces[1]->numInnerComponents = 0;
            dcel->faces[1]->innerComponents = NULL;
            dcel->faces[1]->outerComponent = dcel->halfEdges[0];
        }
        for (usize j = 0; j < numSegments; j++)
        {
            usize i = mod(j - 1, numSegments);
            usize k = mod(j + 1, numSegments);
            /* inside half-edge (counter-clockwise) */
            dcel->halfEdges[2 * j]->origin = dcel->vertices[j];
            dcel->halfEdges[2 * j]->twin = dcel->halfEdges[2 * j + 1];
            dcel->halfEdges[2 * j]->incidentFace = dcel->faces[1];
            dcel->halfEdges[2 * j]->next = dcel->halfEdges[2 * k];
            dcel->halfEdges[2 * j]->prev = dcel->halfEdges[2 * i];
            /* outside half-edge (clockwise) */
            dcel->halfEdges[2 * j + 1]->origin = dcel->vertices[k];
            dcel->halfEdges[2 * j + 1]->twin = dcel->halfEdges[2 * j];
            dcel->halfEdges[2 * j + 1]->incidentFace = dcel->faces[0];
            dcel->halfEdges[2 * j + 1]->next = dcel->halfEdges[2 * i + 1];
            dcel->halfEdges[2 * j + 1]->prev = dcel->halfEdges[2 * k + 1];
        }
    }
    dcel->openVertex = dcel->numVertices;
    dcel->openFace = dcel->numFaces;
    dcel->openHalfEdge = dcel->numHalfEdges;
    return dcel;
}

void
FreeDCEL(DCEL *dcel)
{
    for (usize i = 0; i < dcel->numVertices; i++)
        free(dcel->vertices[i]);
    for (usize i = 0; i < dcel->numFaces; i++)
    {
        if (dcel->faces[i]->numInnerComponents > 0) free(dcel->faces[i]->innerComponents);
        free(dcel->faces[i]);
    }
    for (usize i = 0; i < dcel->numHalfEdges; i++)
        free(dcel->halfEdges[i]);
    free(dcel->vertices);
    free(dcel->faces);
    free(dcel->halfEdges);
    free(dcel);
}

void
AddDiagonalDCEL(DCEL *dcel, VertexEntry *v1, VertexEntry *v2)
{
    assert(dcel->openHalfEdge + 2 <= dcel->numHalfEdges);
    assert(dcel->openFace + 1 <= dcel->numFaces);

    FaceEntry *f1 = v1->incidentEdge->incidentFace;              /* old face getting split by diagonal */
    FaceEntry *f2 = dcel->faces[dcel->openFace];                 /* new face introducted by split */
    HalfEdgeEntry *e1 = dcel->halfEdges[dcel->openHalfEdge];     /* v1 -> v2 half-edge (incident to old face) */
    HalfEdgeEntry *e2 = dcel->halfEdges[dcel->openHalfEdge + 1]; /* v2 -> v1 half-edge (incident to new face) */

    { /*
       * before adding diagonal between v1 and v2, they must first agree on the
       * incident face (common face must exist, otherwise not true diagonal)
       */
        FaceEntry *f2 = v2->incidentEdge->incidentFace;
        while (f1 != f2)
        {
            v2->incidentEdge = v2->incidentEdge->prev->twin;
            FaceEntry *tmp = v2->incidentEdge->incidentFace;
            while (tmp != f1 && tmp != f2)
            {
                v2->incidentEdge = v2->incidentEdge->prev->twin;
                tmp = v2->incidentEdge->incidentFace;
            }
            if (tmp == f1) break;
            else
            {
                v1->incidentEdge = v1->incidentEdge->prev->twin;
                f1 = v1->incidentEdge->incidentFace;
            }
        }
        assert(v1->incidentEdge->incidentFace == v2->incidentEdge->incidentFace);
    }

    f1->outerComponent = e1;
    f2->numInnerComponents = 0;
    f2->innerComponents = NULL;
    f2->outerComponent = e2;

    e1->origin = v1;
    e2->origin = v2;
    e1->twin = e2;
    e2->twin = e1;

    e1->next = v2->incidentEdge;
    e1->prev = v1->incidentEdge->prev;
    e2->next = v1->incidentEdge;
    e2->prev = v2->incidentEdge->prev;

    v1->incidentEdge->prev->next = e1;
    v2->incidentEdge->prev->next = e2;
    v1->incidentEdge->prev = e2;
    v2->incidentEdge->prev = e1;

    e1->incidentFace = f1;
    e2->incidentFace = f2;

    for (HalfEdgeEntry *e = e2->next; e != e2; e = e->next)
        e->incidentFace = f2;

    dcel->openHalfEdge += 2;
    dcel->openFace += 1;
}

void
PrintDCEL(DCEL *dcel)
{
    HalfEdgeEntry *e = dcel->faces[0]->innerComponents[0];
    printf("0: (%lf, %lf)", e->origin->coordinates.x, e->origin->coordinates.y);
    HalfEdgeEntry *tmp = e->next;
    while (tmp != e)
    {
        printf(" (%lf, %lf)", tmp->origin->coordinates.x, tmp->origin->coordinates.y);
        tmp = tmp->next;
    }
    printf("\n");
    for (usize i = 1; i < dcel->openFace; i++)
    {
        HalfEdgeEntry *e = dcel->faces[i]->outerComponent;
        printf("%d: (%lf, %lf)", i, e->origin->coordinates.x, e->origin->coordinates.y);
        HalfEdgeEntry *tmp = e->next;
        while (tmp != e)
        {
            printf(" (%lf, %lf)", tmp->origin->coordinates.x, tmp->origin->coordinates.y);
            tmp = tmp->next;
        }
        printf("\n");
    }
}

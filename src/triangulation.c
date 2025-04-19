#include "triangulation.h"
#include "bsp.h"
#include "dcel.h"
#include "f64_vector.h"
#include "raylib.h"
#include <assert.h>
#include <stdlib.h>

/* ******************* helpers ******************* */
i32 vtxcmp(const VertexEntry u, const VertexEntry v);
/* *********************************************** */

Triangle *
MonotoneTriangulation(DSegment *segments, usize numSegments)
{
    DCEL *dcel = BuildSimpleDCEL(segments, numSegments);
    MonotoneTriangulateDCEL(dcel);
    Triangle *triangles = (Triangle *)malloc((dcel->numFaces - 1) * sizeof(Triangle));
    for (usize i = 1; i < dcel->numFaces; i++)
    {
        DVector2 v1 = dcel->faces[i]->outerComponent->prev->origin->coordinates;
        DVector2 v2 = dcel->faces[i]->outerComponent->origin->coordinates;
        DVector2 v3 = dcel->faces[i]->outerComponent->next->origin->coordinates;
        triangles[i - 1] = (Triangle){
            .v1 = (Vector2){ v1.x, v1.y },
            .v2 = (Vector2){ v2.x, v2.y },
            .v3 = (Vector2){ v3.x, v3.y },
        };
    }
    FreeDCEL(dcel);
    return triangles;
}

void
MonotoneTriangulateDCEL(DCEL *dcel)
{
    { /*
       * euler's formula          : f - e + n == 2
       * trianglulation diagonals : d == n-3
       * trianglulation triangles : t == n-2
       * =>
       * new face count == t+1 == n-1
       * new edge count == d + e == (n-3) + (n) == 2n-3
       * =>
       * new half-edge count == 4n-6
       */
        usize n = dcel->numVertices;
        ResizeDCEL(dcel, n, n - 1, 4 * n - 6);
    }

    const bool RIGHT = true;
    const bool LEFT = false;
    bool *chainSide = (bool *)malloc((dcel->numVertices) * sizeof(bool));
    { /*
       * 1. find vertex v with biggest y-coordinate and add to array
       * 2. traverse down left/right chains from v
       * 3. add higher vertex to array between chains and increment higher side
       *    i.  track which node is on which chain when adding vertex
       *    ii. repeat until both chains reach bottom node u
       * 4. add bottom vertex u to array
       *
       * *NOTE* this only works if polygon is y-monotone!!
       */
        usize top = 0;
        for (usize i = 1; i < dcel->numVertices; i++)
            if (vtxcmp(*dcel->vertices[i], *dcel->vertices[top]) > 0) top = i;
        dcel->vertices[0] = dcel->vertices[top];
        HalfEdgeEntry *el = dcel->vertices[top]->incidentEdge->next;
        HalfEdgeEntry *er = dcel->vertices[top]->incidentEdge->twin->next->next;
        for (usize i = 1; i < dcel->numVertices - 1; i++)
        {
            i32 cmp = vtxcmp(*el->origin, *er->origin);
            if (cmp > 0)
            {
                dcel->vertices[i] = el->origin;
                chainSide[i] = LEFT;
                el = el->next;
            }
            else
            {
                dcel->vertices[i] = er->origin;
                chainSide[i] = RIGHT;
                er = er->next;
            }
        }
        assert(vtxcmp(*el->origin, *er->origin) == 0);
        dcel->vertices[dcel->numVertices - 1] = el->origin;
    }

    { /*
       * 1. initialize stack S, push u1 and u2 onto it
       * 2. for j <- 3 to n-1...
       *    a. if uj and vertex at the top of S are on different chains...
       *       i.   pop all vertices from S
       *       ii.  add diagonal from uj to each popped vertex (except the last one)
       *       iii. push u(j-1) and uj onto S
       *    b. else
       *       i.   pop one vertex from S
       *       ii.  pop other vertices from S as long as diagonals from uj to
       *            them are inside P, add these diagonals
       *       iii. push the last vertex popped back onto S
       *       iv.  push uj onto S
       * 3. add diagonals from un to all stack vertices except first and last
       */
        VertexEntry **stack = (VertexEntry **)malloc(dcel->numVertices * sizeof(VertexEntry *));
        usize stackSize = 0;
        stack[stackSize++] = dcel->vertices[0];
        stack[stackSize++] = dcel->vertices[1];
        bool stackSide = chainSide[1];
        for (usize i = 2; i < dcel->numVertices - 1; i++)
        {
            if (chainSide[i] != stackSide)
            {
                for (; stackSize > 1; stackSize -= 1)
                    AddDiagonalDCEL(dcel, dcel->vertices[i], stack[stackSize - 1]);
                stackSize -= 1;
                stack[stackSize++] = dcel->vertices[i - 1];
                stack[stackSize++] = dcel->vertices[i];
            }
            else
            {
                VertexEntry *u = dcel->vertices[i];
                VertexEntry *v = stack[--stackSize];
                while (stackSize >= 2)
                {
                    VertexEntry *w = stack[stackSize - 1];
                    DVector2 uv = DVector2Subtract(v->coordinates, u->coordinates);
                    DVector2 uw = DVector2Subtract(w->coordinates, u->coordinates);
                    f64 det = DVector2Determinant(uv, uw);
                    if (det < 0.0f && chainSide[i] == RIGHT) break;
                    else if (det > 0.0f && chainSide[i] == LEFT) break;
                    else
                    {
                        AddDiagonalDCEL(dcel, u, w);
                        v = stack[--stackSize];
                    }
                }
                stack[stackSize++] = v;
                stack[stackSize++] = u;
            }
            stackSide = chainSide[i];
        }
        for (usize i = 1; i < stackSize - 1; i++)
            AddDiagonalDCEL(dcel, dcel->vertices[dcel->numVertices - 1], stack[i]);
        free(stack);
        free(chainSide);
    }
}

Vector2
TriangleCenter(Triangle t)
{
    return (Vector2){
        .x = (t.v1.x + t.v2.x + t.v3.x) / 3.0f,
        .y = (t.v1.y + t.v2.y + t.v3.y) / 3.0f,
    };
}

i32
vtxcmp(const VertexEntry u, const VertexEntry v)
{ /*
   * y-coordinates different => larger y-coordinate bigger
   * y-coordinates the same => smaller x-coordinate bigger
   */
    f64 dy = u.coordinates.y - v.coordinates.y;
    if (babs(dy) < BSP_EPSILON)
    {
        f64 dx = u.coordinates.x - v.coordinates.x;
        if (babs(dx) < BSP_EPSILON) return 0;
        else return (dx < 0) ? 1 : -1;
    }
    else return (dy > 0) ? 1 : -1;
}

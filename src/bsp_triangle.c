#include "bsp_triangle.h"
#include "bsp_utils.h"
#include "dcel.h"
#include "f64_vector.h"
#include "raymath.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* helpers */
i32 vtxcmp(const VertexEntry u, const VertexEntry v);
/* helpers */

Triangle *
MonotoneTriangulation(Segment *edges, usize numEdges)
{
    /* vertex array stores index of associated edge (corresponds to left endpoint) */
    assert(numEdges >= 3);
    usize numVertices = numEdges;
    usize *vertices = (usize *)malloc(numVertices * sizeof(usize));

    usize topIdx = 0;
    usize bottomIdx = 0;

    { /*
       * construct vertex array sorted by y-coordinate
       *  - we can take advantage of the fact that the polygon is y-monotone to sort the edges in
       *    linear time
       *  - simply find the vertex with the largest y-coordinate and, cascade down the left/right
       *    chains, adding the vertex with the higher vertex at each step until we reach the bottom
       *    vertex
       */
        usize vertexIdx = 0;
        for (usize i = 0; i < numEdges; i++)
        {
            if (edges[i].left.y > edges[topIdx].left.y) topIdx = i;
            else if (edges[i].left.y == edges[topIdx].left.y && edges[i].left.x < edges[topIdx].left.x) topIdx = i;
            if (edges[i].left.y < edges[bottomIdx].left.y) bottomIdx = i;
            else if (edges[i].left.y == edges[topIdx].left.y && edges[i].left.x > edges[bottomIdx].left.x) topIdx = i;
        }
        vertices[vertexIdx++] = topIdx;
        usize leftIdx = mod(topIdx + 1, numVertices);
        usize rightIdx = mod(topIdx - 1, numVertices);
        while (leftIdx != rightIdx)
        {
            if (edges[leftIdx].left.y > edges[rightIdx].left.y
                || (edges[leftIdx].left.y == edges[rightIdx].left.y && edges[leftIdx].left.x < edges[rightIdx].left.x))
            {
                vertices[vertexIdx++] = leftIdx;
                leftIdx = mod(leftIdx + 1, numVertices);
            }
            else
            {
                vertices[vertexIdx++] = rightIdx;
                rightIdx = mod(rightIdx - 1, numVertices);
            }
        }
        vertices[vertexIdx++] = bottomIdx;
        for (usize i = 0; i < numVertices; i++)
        {
            DVector2 v = edges[vertices[i]].left;
            printf("x: %.2lf\ty: %.2lf\n", v.x, v.y);
        }
    }

    {
        usize *stack = (usize *)malloc(numVertices * sizeof(usize));
        usize stackIdx = 0;
        stack[stackIdx++] = vertices[0];
        stack[stackIdx++] = vertices[1];
        for (usize i = 2; i < numVertices - 1; i++)
        {
        }
    }

    return NULL;
}

void
MonotoneTriangulate(DCEL *dcel)
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

i32
vtxcmp(const VertexEntry u, const VertexEntry v)
{
    f64 dy = u.coordinates.y - v.coordinates.y;
    if (babs(dy) < EPSILON)
    {
        f64 dx = u.coordinates.x - v.coordinates.x;
        if (babs(dx) < EPSILON) return 0;
        else return (dx < 0) ? 1 : -1;
    }
    else return (dy > 0) ? 1 : -1;
}

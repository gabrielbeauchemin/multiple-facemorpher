#include "Subdiv2DIndex.h"
#include <set>


Subdiv2DIndex::Subdiv2DIndex(Rect rectangle) : Subdiv2D{rectangle}
{
}

void Subdiv2DIndex::getTrianglesIndices(std::vector<int> &triangleList) const
{
  triangleList.clear();
  int i, total = (int)(qedges.size() * 4);
  std::vector<bool> edgemask(total, false);

  for (i = 4; i < total; i += 2)
  {
    if (edgemask[i])
      continue;

    Point2f a, b, c;
    int edge_a = i;
    int indexA = edgeOrg(edge_a, &a) - 4;
    if (indexA < 0) continue;

    int edge_b = getEdge(edge_a, NEXT_AROUND_LEFT);
    int indexB = edgeOrg(edge_b, &b) - 4;
    if (indexB < 0) continue;

    int edge_c = getEdge(edge_b, NEXT_AROUND_LEFT);
    int indexC = edgeOrg(edge_c, &c) - 4;
    if (indexC < 0) continue;

    edgemask[edge_a] = true;
    edgemask[edge_b] = true;
    edgemask[edge_c] = true;

    triangleList.push_back(indexA);
    triangleList.push_back(indexB);
    triangleList.push_back(indexC);
  }
}

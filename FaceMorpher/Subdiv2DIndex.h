#ifndef __Subdiv2DIndex__
#define __Subdiv2DIndex__

#include <vector>
#include <opencv2\opencv.hpp>
using namespace cv;

class Subdiv2DIndex : public Subdiv2D
{
public :
  Subdiv2DIndex(Rect rectangle);

  //Source code of Subdiv2D: https://github.com/opencv/opencv/blob/master/modules/imgproc/src/subdivision2d.cpp#L762
  //The implementation tweaks getTrianglesList() so that only the indice of the triangle inside the image are returned
  void getTrianglesIndices(std::vector<int> &ind) const;
};

#endif


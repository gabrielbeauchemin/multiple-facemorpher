#ifndef __FaceMorpher__
#define __FaceMorpher__

#include <string>
//#include <opencv2/core.hpp>
//#include <opencv2/imgcodecs.hpp>
//#include <opencv2/highgui.hpp>
//#include <opencv2/imgproc.hpp>
//#include <opencv2\objdetect.hpp>
#include <opencv2\opencv.hpp>
#include <opencv2\face.hpp>
#include <iostream>

using namespace cv;
using namespace cv::face;

class FaceMorpher
{
public:
  //This constructor can be used if later the function generateImages is sent to sent the path to the two images
  //This way, it is possible to morph many images with the same object FaceMorpher 
  //its what is done by class FaceMorphers which used this class
  FaceMorpher(std::string outputFolderPath, int nbrMorphedImages, int heightImage, int widthImage);

  FaceMorpher(std::string firstImagePath, std::string secondImagePath, std::string outputFolderPath, int nbrMorphedImages, int heightImage, int widthImage);
  std::vector<std::string> generateImages(bool trimFace = false);
  std::vector<std::string> generateImages(std::string firstImagePath, std::string secondImagePath, bool trimFace = false);
  ~FaceMorpher();

private:
  std::string _firstImagePath;
  std::string _secondImagePath;
  std::string _outputFolderPath;
  int _nbrMorphedImages;
  int _heightmage;
  int _widthImage;
  int indexImgOutput = 0; //used to have unique output filename


  CascadeClassifier _faceDetector;
  Ptr<Facemark> _facemark;

  //Credit: https://www.learnopencv.com/facemark-facial-landmark-detection-using-opencv/#cpp-code
  //Get points of the eyes, eyes brows, nose and mouth
  //The number of points found doesnt change from face to face
  std::vector < std::vector<Point2f>> getFaceLandmark(Mat& image);

  
  //The image of the face is getting deformed during the morphing so that the transition is smoother
  //this function gets the morphed points at a specific interpolation
  std::vector<Point2f> getMorphedPoints(std::vector<Point2f>& pointsImage1,
                                        std::vector<Point2f>& pointsImage2,
                                        double interpolation);

  //To keep the background of the faces, add the 4 corners points of the image
  void addCornersPoints(std::vector<Point2f>& points);

  //Credit: https://www.learnopencv.com/delaunay-triangulation-and-voronoi-diagram-using-opencv-c-python/
  //From points, find the triangles that pass through all of them
  std::vector<int> pointsToTrianglesIndices(std::vector<Point2f> points);

  //Credit https://www.learnopencv.com/face-morph-using-opencv-cpp-python/ 
  // From two triangles (srcTriangle and dstTriangle) in an image (imgSrc),
  // Warp srcTriangle so its pixels now fit inside dstTriangle
  // And store this in wrapedImg
  void warpTriangle(Mat & wrapedImg, Mat & imgSrc, Vec6f& srcTriangle, Vec6f& dstTriangle);

  // Find the affine transformation from srcTriangle to dstTri
  // And Apply the transformation to each pixel of the triangle 
  // Which result of the warped triangle stored in parameter warpImage
  void getWarpTriangle(Mat & warpedImg, Mat & imgSrc, std::vector<Point2f>& srcTriangle, std::vector<Point2f>& dstTriangle);

  //AlphaBlend two images, with an interpolation indice for the blend
  //The result is store in parameter alphaBlendedImage
  void alphaBlend(Mat img1, Mat img2, Mat & alphaBlendedImg, double interpolation);

  //Utility function to split a triangle stored in a vector into three points
  std::vector<Point2f> splitVec6fInPoints2f(Vec6f vec);
};

#endif


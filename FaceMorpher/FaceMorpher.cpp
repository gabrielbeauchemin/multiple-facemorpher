#include "FaceMorpher.h"
#include <vector>
#include "Subdiv2DIndex.h"
#include <filesystem>
#include <iostream>

FaceMorpher::FaceMorpher(std::string outputFolderPath, int nbrMorphedImages, int heightImage, int widthImage)
  : _nbrMorphedImages{ nbrMorphedImages },
  _heightmage{ heightImage },
  _outputFolderPath{ outputFolderPath },
  _widthImage{ widthImage }
{
  //init objects to find face landmark
  std::string workingDirectory = std::experimental::filesystem::current_path().string();
  _faceDetector = CascadeClassifier(workingDirectory + "\\haarcascade_frontalface_alt2.xml"); // Load Face Detector
  _facemark = FacemarkLBF::create();
  _facemark->loadModel(workingDirectory + "\\lbfmodel.yaml"); // Load landmark detector (algorithm already trained with thousand of face images)
}

FaceMorpher::FaceMorpher(std::string firstImagePath, std::string secondImagePath, std::string outputFolderPath, int nbrMorphedImages, int heightImage, int widthImage)
  : _firstImagePath{ firstImagePath },
  _secondImagePath{ secondImagePath },
  _outputFolderPath{outputFolderPath},
  _nbrMorphedImages{nbrMorphedImages},
  _heightmage{heightImage},
  _widthImage{widthImage}
{
  //init objects to find face landmark
  std::string workingDirectory = std::experimental::filesystem::current_path().string();
  _faceDetector = CascadeClassifier(workingDirectory + "\\haarcascade_frontalface_alt2.xml"); // Load Face Detector
  _facemark = FacemarkLBF::create();
  _facemark->loadModel(workingDirectory + "\\lbfmodel.yaml"); // Load landmark detector (algorithm already trained with thousand of face images)
}

FaceMorpher::~FaceMorpher()
{
}

std::vector<std::string> FaceMorpher::generateImages(std::string firstImagePath, std::string secondImagePath, bool trimFace)
{
  _firstImagePath = firstImagePath;
  _secondImagePath = secondImagePath;
  return generateImages(trimFace);
}

std::vector<std::string> FaceMorpher::generateImages(bool trimFace)
{
  if (_firstImagePath.empty() || _secondImagePath.empty())
  {
    throw std::exception("No path to the images to morph was given.");
  }

  std::vector<std::string> ouputedImgsPath;

   Mat image1 = imread(_firstImagePath, IMREAD_COLOR);
   Mat image2 = imread(_secondImagePath, IMREAD_COLOR);

   Mat image1Resized, image2Resized;
   Size sizeImageGenerated(_widthImage, _heightmage);
   resize(image1, image1Resized, sizeImageGenerated);
   resize(image2, image2Resized, sizeImageGenerated);

   imwrite( _outputFolderPath + "Morphing_0.jpg", image1Resized);
   ouputedImgsPath.push_back(_outputFolderPath + "Morphing_0.jpg");
   imwrite(_outputFolderPath + "Morphing_1.jpg", image2Resized);
   ouputedImgsPath.push_back(_outputFolderPath + "Morphing_1.jpg");

   //if there is many faces in the picture, take only the first face
   std::vector<Point2f> pointsImage1 = getFaceLandmark(image1Resized)[0];
   std::vector<Point2f> pointsImage2 = getFaceLandmark(image2Resized)[0];

   if (!trimFace)
   {
     addCornersPoints(pointsImage1); //So that the background of the face image is included in the output
     addCornersPoints(pointsImage2);
   }
   
   //The indexes for the triangles of the points of image1, image2 and the morphed images are going to be the same
   //So only the points of image1 are going to be triangulate properly accoring the Delayney
   //This wont be the case from the points of image2 and the morphed Images,
   //But the topology will be mainted in all images, so the warping will work
   std::vector<int> trianglesIndexes = pointsToTrianglesIndices(pointsImage1);

   double stepInterpolation = 1.0 / (_nbrMorphedImages + 1);
   std::vector<double> interpolationIndexes;
   for (int i = 1; i <= _nbrMorphedImages; ++i)
     interpolationIndexes.push_back(i*stepInterpolation);

   for (double interpolation : interpolationIndexes)
   {
     std::vector<Point2f> morphedPoints = getMorphedPoints(pointsImage1, pointsImage2, interpolation);

     Mat imgWraped1 = Mat::zeros(image1Resized.size(), CV_32FC3);
     Mat imgWraped2 = Mat::zeros(image1Resized.size(), CV_32FC3);
     Mat morphedImg = Mat::zeros(image1Resized.size(), CV_32FC3);

     for (int indexTriangle = 0; indexTriangle < trianglesIndexes.size(); indexTriangle += 3)
     {
       int indexP1 = trianglesIndexes[indexTriangle];
       int indexP2 = trianglesIndexes[indexTriangle+1];
       int indexP3 = trianglesIndexes[indexTriangle+2];
       Vec6f triangleImage1(pointsImage1[indexP1].x, pointsImage1[indexP1].y, pointsImage1[indexP2].x, pointsImage1[indexP2].y, pointsImage1[indexP3].x, pointsImage1[indexP3].y)
            ,triangleImage2(pointsImage2[indexP1].x, pointsImage2[indexP1].y, pointsImage2[indexP2].x, pointsImage2[indexP2].y, pointsImage2[indexP3].x, pointsImage2[indexP3].y),
            morphedTriangle(morphedPoints[indexP1].x, morphedPoints[indexP1].y, morphedPoints[indexP2].x, morphedPoints[indexP2].y, morphedPoints[indexP3].x, morphedPoints[indexP3].y);

       warpTriangle(imgWraped1, image1Resized, triangleImage1, morphedTriangle);
       warpTriangle(imgWraped2, image2Resized, triangleImage2, morphedTriangle);
     }

     alphaBlend(imgWraped1, imgWraped2, morphedImg, interpolation);

     
     std::string imgOutputPath = _outputFolderPath + "Morphing" + std::to_string(++indexImgOutput) + "_" + std::to_string(interpolation) + ".jpg";
     ouputedImgsPath.push_back(imgOutputPath);
     imwrite(imgOutputPath, morphedImg);
   }

   return ouputedImgsPath;
}

std::vector <std::vector<Point2f>> FaceMorpher::getFaceLandmark(Mat& img)
{
  // Find faces
  std::vector<Rect> faces;
  Mat gray; //Convert image to grey because its needed for faceDetector
  cvtColor(img, gray, COLOR_BGR2GRAY);
  _faceDetector.detectMultiScale(gray, faces);
  if (faces.empty())
  {
    throw new std::exception("The image doesn't contain a face.");
  }

  // Find landmarks on the faces
  std::vector< std::vector<Point2f> > landmarks; //Each face is represented by a vector of points
  if (!_facemark->fit(img, faces, landmarks))
  {
    throw new std::exception("The face landmark could not be found.");
  }

  img.convertTo(img, CV_32FC3); //The past format of img was necessary to get the face landmark, now put it back to format easier for mat multiplications for later
  return landmarks;
}

std::vector<Point2f> FaceMorpher::getMorphedPoints(std::vector<Point2f>& pointsImage1, std::vector<Point2f>& pointsImage2, double interpolationIndice)
{
  std::vector<Point2f> morphedPoints;
  for (int indexPoint = 0; indexPoint < pointsImage1.size(); ++indexPoint)
  {
    Point2f currentPoint;
    currentPoint.x = (1 - interpolationIndice) * pointsImage1[indexPoint].x +
                     interpolationIndice * pointsImage2[indexPoint].x;
    currentPoint.y = (1 - interpolationIndice) * pointsImage1[indexPoint].y +
                     interpolationIndice * pointsImage2[indexPoint].y;
    morphedPoints.push_back(currentPoint);
  }
  return morphedPoints;
}

void FaceMorpher::addCornersPoints(std::vector<Point2f>& points)
{
  //center points
  points.push_back(Point2f(0, 0));
  points.push_back(Point2f(0, _heightmage - 1));
  points.push_back(Point2f(_widthImage - 1, _heightmage - 1));
  points.push_back(Point2f(_widthImage - 1, 0));
  //center of boundaries 
  points.push_back(Point2f( (_widthImage-1)/2, 0));
  points.push_back(Point2f((_widthImage - 1) / 2, _heightmage - 1));
  points.push_back(Point2f(0,  (_heightmage-1)/2 ));
  points.push_back(Point2f(_widthImage - 1, (_heightmage - 1) / 2));
}

std::vector<int> FaceMorpher::pointsToTrianglesIndices(std::vector<Point2f> points)
{
  Rect rect(0, 0, _widthImage, _heightmage); //choose the space we want to work in, in this case all the image
  Subdiv2DIndex subdiv(rect); //object representing the points is a Voronoi diagram so it can easily triangulise the points
  for (auto& point : points)
    subdiv.insert(point); //subdiv updates the Voronoi diagram each time a point is inserted

  std::vector<int> triangles;
  subdiv.getTrianglesIndices(triangles); //generate the triangles by splitting polygons formed by Voronoi diagram into indices of triangles

  return triangles; 
}

void FaceMorpher::warpTriangle(Mat &wrapedImg, Mat &imgSrc, Vec6f &srcTriangle, Vec6f &dstTriangle)
{
  Rect rectangleDstTriangle = boundingRect(splitVec6fInPoints2f(dstTriangle));
  Rect rectangleSrcTriangle = boundingRect(splitVec6fInPoints2f(srcTriangle));

  // Get offset points from left top corner of the rectangles
  std::vector<Point2f> offsetRectangleSrc, offsetRectangleDst;
  std::vector<Point> offsetRectangleDst_p;
  for (int i = 0; i < 6; i+=2)
  {
    offsetRectangleDst.push_back(Point2f(dstTriangle[i] - rectangleDstTriangle.x, dstTriangle[i+1] - rectangleDstTriangle.y));
    // for fillConvexPoly (to get mask)
    offsetRectangleDst_p.push_back(Point(dstTriangle[i] - rectangleDstTriangle.x, dstTriangle[i+1] - rectangleDstTriangle.y)); 

    offsetRectangleSrc.push_back(Point2f(srcTriangle[i] - rectangleSrcTriangle.x, srcTriangle[i+1] - rectangleSrcTriangle.y));
  }

  // Warp the triangle
  Mat imgSrcTrimmed;
  imgSrc(rectangleSrcTriangle).copyTo(imgSrcTrimmed); //trim to keep on the smallest rectangle containing the triangle to warp
  Mat warpedTriangleImg = Mat::zeros(rectangleDstTriangle.height, rectangleDstTriangle.width, imgSrcTrimmed.type());
  getWarpTriangle(warpedTriangleImg, imgSrcTrimmed, offsetRectangleSrc, offsetRectangleDst);

  //Copy triangular region of the rectangular image (warpedTriangleImg) to the output image (wrapedImg) 
  Mat mask = Mat::zeros(rectangleDstTriangle.height, rectangleDstTriangle.width, CV_32FC3); //todo check if type is ok it was before: CV_32FC3
  fillConvexPoly(mask, offsetRectangleDst_p, Scalar(1.0, 1.0, 1.0), 16, 0); // Get mask by filling triangle

  multiply(warpedTriangleImg, mask, warpedTriangleImg);
  multiply(wrapedImg(rectangleDstTriangle), Scalar(1.0, 1.0, 1.0) - mask, wrapedImg(rectangleDstTriangle));
  wrapedImg(rectangleDstTriangle) = wrapedImg(rectangleDstTriangle) + warpedTriangleImg;
}

void FaceMorpher::getWarpTriangle(Mat &warpedImg, Mat &imgSrc, std::vector<Point2f> &srcTriangle, std::vector<Point2f> &dstTriangle)
{
  Mat warpTransformation = getAffineTransform(srcTriangle, dstTriangle);
  warpAffine(imgSrc, warpedImg, warpTransformation, warpedImg.size(), INTER_LINEAR, BORDER_REFLECT_101);
}

void FaceMorpher::alphaBlend(Mat img1, Mat img2, Mat& alphaBlendedImg, double interpolation)
{
  alphaBlendedImg = (1.0 - interpolation) * img1 + interpolation * img2;
}

std::vector<Point2f> FaceMorpher::splitVec6fInPoints2f(Vec6f vec)
{
  std::vector<Point2f> points;
  points.push_back(Point2f(vec[0], vec[1]));
  points.push_back(Point2f(vec[2], vec[3]));
  points.push_back(Point2f(vec[4], vec[5]));
  return points;
}



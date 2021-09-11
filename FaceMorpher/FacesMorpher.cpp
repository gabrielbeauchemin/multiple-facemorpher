#include "FacesMorpher.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include  <random>
#include  <iterator>
#include <filesystem>
#include <stdio.h>

namespace fs = std::experimental::filesystem;

FacesMorpher::FacesMorpher(std::string pathImagesFolder, std::string pathOutputFolder, int nbrMorphedImages, int heightImage, int widthImage)
  : _pathImagesFolder{pathImagesFolder},
  _pathOutputFolder{pathOutputFolder},
  _nbrMorphedImages{nbrMorphedImages},
  _faceMorpher(pathOutputFolder, 1, heightImage, widthImage),
  _widthImage{widthImage},
  _heightmage{heightImage}
{
}

FacesMorpher::~FacesMorpher()
{
}

void FacesMorpher::generateImages()
{
  std::vector<std::string> imagesPath = getImagesPath();
  std::map<int, std::vector<int>> imagesMorphed; //indexImage -> list images morphed with    
  std::ofstream parentTxtFile;
  parentTxtFile.open(_pathOutputFolder + "morphingParents.txt", std::ofstream::out | std::ios_base::app);
  parentTxtFile << "ImageName Parent1 Parent2 \r\n" << "------------------------------------------------------ \r\n" << std::flush;

  for (int nbrImgGenerated = 0; nbrImgGenerated < _nbrMorphedImages; ++nbrImgGenerated)
  {
    //get two different Images that were never morphed before
    int imgPathIndex1, imgPathIndex2;
    do
    {
      imgPathIndex1 = select_randomly(imagesPath.begin(), imagesPath.end()) - imagesPath.begin();
      imgPathIndex2 = select_randomly(imagesPath.begin(), imagesPath.end()) - imagesPath.begin();
    }
    while (imgPathIndex1 == imgPathIndex2 ||
      areImagesMorphed(imagesMorphed, imgPathIndex1, imgPathIndex2));
    
    //morphed the two images and assure they wont be morphed again in the futur
    std::string img1Path = imagesPath[imgPathIndex1];
    std::string img2Path = imagesPath[imgPathIndex2];
    auto pathImgsGenerated = _faceMorpher.generateImages(img1Path, img2Path, true);
    std::string pathImgGenerated = pathImgsGenerated[pathImgsGenerated.size() - 1];
    imagesPath.push_back(pathImgGenerated);
    addMorphedImages(imagesMorphed, imgPathIndex1, imgPathIndex2);
    addMorphedImages(imagesMorphed, imgPathIndex2, imgPathIndex1);

    //first and last images generated are not useful, delete them
    remove(pathImgsGenerated[0].c_str());
    remove(pathImgsGenerated[1].c_str());

    //append to parents file the parents of the generated image
    parentTxtFile << pathToFilename(pathImgGenerated) + " " + pathToFilename(img1Path) + " " + pathToFilename(img2Path) + "\r\n" << std::flush;
  }

}

std::string FacesMorpher::pathToFilename(std::string path)
{
  std::vector<std::string> result;

  std::replace(path.begin(), path.end(),'\\', '/');

  while(true)
  {
    int indexSeparator = path.find("/");
    if (indexSeparator != std::string::npos)
    {
      result.push_back(path.substr(0, indexSeparator));
      path = path.substr(indexSeparator+1);
    }
    else
    {
      result.push_back(path);
      break;
    }
  }

  return result[result.size() -1];
}

std::vector<std::string> FacesMorpher::getImagesPath()
{
  std::vector<std::string> imagesPath;

  static std::vector<std::string> supportedFormats
  {
    ".bmp",".cur",".gif",".hdf4",".ico",".jpg",".jp2",".pmb",".pcx",".pgm",".png",".ppm",".ras",".tiff",".xwd"
  };

  for (const auto & p : fs::directory_iterator(_pathImagesFolder))
  {
    std::string filePath = p.path().string();
    std::string fileExtension = p.path().extension().string();

    if (std::find(supportedFormats.begin(), supportedFormats.end(), fileExtension) == supportedFormats.end()) {
      continue; //skip file that is not an image
    }

    imagesPath.push_back(filePath);
  }

  return imagesPath;
}

template<typename Iter, typename RandomGenerator>
Iter FacesMorpher::select_randomly(Iter start, Iter end, RandomGenerator& g) {
  std::uniform_int_distribution<> dis(0, std::distance(start, end) - 1);
  std::advance(start, dis(g));
  return start;
}

template<typename Iter>
Iter FacesMorpher::select_randomly(Iter start, Iter end) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  return select_randomly(start, end, gen);
}

void FacesMorpher::addMorphedImages(std::map<int, std::vector<int>>& imagesMorphed, int indexImg1, int indexImg2)
{
  auto it = imagesMorphed.find(indexImg1);
  if (it != imagesMorphed.end())
  {
    //add other morphing to img1
    auto imagesMorphedWith = it->second;
    imagesMorphedWith.push_back(indexImg2);
  }
  else //first morphing for img1
  {
    imagesMorphed[indexImg1] = std::vector<int>{ indexImg2 };
  }
}

bool FacesMorpher::areImagesMorphed(std::map<int, std::vector<int>>& imagesMorphed, int indexImg1, int indexImg2)
{
  auto it = imagesMorphed.find(indexImg1);
  if (it != imagesMorphed.end())
  {
    auto imagesMorphedWith = it->second;
    return std::find(imagesMorphedWith.begin(), imagesMorphedWith.end(), indexImg2) 
            != imagesMorphedWith.end();
  }
  return false;
}





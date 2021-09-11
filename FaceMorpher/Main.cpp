#include <iostream>
#include <string>
#include "FaceMorpher.h"
#include "FacesMorpher.h"

inline bool fileExists(const std::string& name) {
  struct stat buffer;
  return (stat(name.c_str(), &buffer) == 0);
}

void checkParameters(std::string firstImagePath, std::string secondImagePath, std::string pathOutputImgs, int nbrMorphedImages, int heightImage, int widthImage)
{
  constexpr int MAX_HEIGHT_IMAGE = 1000;
  constexpr int MAX_WIDTH_IMAGE = 1000;

  if (!fileExists(firstImagePath) || !fileExists(secondImagePath) || !fileExists(pathOutputImgs))
  {
    throw new std::exception("The path to the images is not valid");
  }

  if (heightImage > MAX_HEIGHT_IMAGE || widthImage > MAX_WIDTH_IMAGE)
  {
    std::string exceptionMessage = "The height of the generated images must be smaller than " + std::to_string(MAX_HEIGHT_IMAGE) + "x" + std::to_string(MAX_WIDTH_IMAGE);
    throw new std::exception(exceptionMessage.c_str());
  }
}

void checkParameters(std::string pathFolderImages, std::string pathOutputImgs, int nbrMorphedImages, int heightImage, int widthImage)
{
  constexpr int MAX_HEIGHT_IMAGE = 1000;
  constexpr int MAX_WIDTH_IMAGE = 1000;

  if (!fileExists(pathFolderImages) || !fileExists(pathOutputImgs))
  {
    throw new std::exception("The path to the folder containing the images is not valid");
  }

  if (heightImage > MAX_HEIGHT_IMAGE || widthImage > MAX_WIDTH_IMAGE)
  {
    std::string exceptionMessage = "The height of the generated images must be smaller than " + std::to_string(MAX_HEIGHT_IMAGE) + "x" + std::to_string(MAX_WIDTH_IMAGE);
    throw new std::exception(exceptionMessage.c_str());
  }
}

int main(int argc, char** argv)
{
  if (argc < 7 || argc > 8)
    std::cout << "The arguments needed to use the Face Morpher are"
    "In case of normal face morphing between two faces: "
    "twofaces <Path to first image> <Path to second image> <Path to the output folder of the generated images> <Number of morphed images to generate> <Height of images to generate in pixels> <Width of images to generate in pixels>"
    "In case of a face morphing of multiple faces:"
    "multiplefaces <Path to the folder containing the images> <Path to the output folder of the generated images> <Number of morphed images to generate> <Height of images to generate in pixels> <Width of images to generate in pixels>";

  std::string faceMorphingMode = argv[1];
  if (faceMorphingMode == "twofaces")
  {
    std::string pathFirstImage = argv[2];
    std::string pathSecondImage = argv[3];
    std::string pathFolderOutput = argv[4];
    int nbrImages = std::stoi(argv[5]);
    int heightImage = std::stoi(argv[6]);
    int widthImage = std::stoi(argv[7]);
    checkParameters(pathFirstImage, pathSecondImage, pathFolderOutput, nbrImages, heightImage, widthImage);

    FaceMorpher faceMorpher(pathFirstImage, pathSecondImage, pathFolderOutput, nbrImages, heightImage, widthImage);
    faceMorpher.generateImages();
  }
  else if (faceMorphingMode == "multiplefaces")
  {
    std::string pathFolderImages = argv[2];
    std::string pathFolderOutput = argv[3];
    int nbrImages = std::stoi(argv[4]);
    int heightImage = std::stoi(argv[5]);
    int widthImage = std::stoi(argv[6]);
    checkParameters(pathFolderImages, pathFolderOutput, nbrImages, heightImage, widthImage);

    FacesMorpher facesMorpher(pathFolderImages, pathFolderOutput, nbrImages, heightImage, widthImage);
    facesMorpher.generateImages();
  }
  else
  {
    throw new std::exception("This mode of face morphing doesn't exist");
  }
  
    
}

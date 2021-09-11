#ifndef __FacesMorpher__ 
#define __FacesMorpher__

#include <string>
#include "FaceMorpher.h"
#include <map>


class FacesMorpher
{
public:
  FacesMorpher() = delete;
  FacesMorpher(std::string pathImagesFolder, std::string pathOutputFolder, int nbrMorphedImages, int heightImage, int widthImage);
  ~FacesMorpher();
  void generateImages();

private:
  FaceMorpher _faceMorpher;
  int _nbrMorphedImages;
  std::string _pathImagesFolder;
  std::string _pathOutputFolder;
  int _heightmage;
  int _widthImage;

  //get random element from list. Credit: https://stackoverflow.com/questions/6942273/how-to-get-a-random-element-from-a-c-container
  template<typename Iter, typename RandomGenerator> Iter select_randomly(Iter start, Iter end, RandomGenerator & g);
  template<typename Iter>Iter select_randomly(Iter start, Iter end);

  std::vector<std::string> getImagesPath();
  void addMorphedImages(std::map<int, std::vector<int>>& imagesMorphed, int indexImg1, int indexImg2);
  bool areImagesMorphed(std::map<int, std::vector<int>>& imagesMorphed, int indexImg1, int indexImg2);
  std::string pathToFilename(std::string path);
};

 
  

#endif 



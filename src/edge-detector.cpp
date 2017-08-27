#include <iostream>
#include <vector>
#include <thread>
#include <stdio.h>
#include <fstream>
#include <streambuf>
#include <ostream>
#include <time.h>
#include "../include/json.hpp"

#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;
using json = nlohmann::json;

Mat image;
Mat originalImage;
Mat edgeImage;
Mat resultingImage;

vector<Rect> subRegions;
vector<Mat> subImages;

string imageFilename;
string outputFile;
string fileCoord;

// Canny parameters
int kernelSize      = 3;
int edgeThreshold   = 1;
int lowThreshold;
int maxThreshold    = 100;

// Edge params
int width;
int height;
int radiusPadding;

// Dimensionality of main image
int dWidth;
int dHeight;

int scaleTargetWidth;

vector<vector<Point> > contours;
vector<Vec4i> hierarchy;

Scalar circleLineColor = Scalar(0, 255, 255);
Scalar squareLineColor  = Scalar(0, 0, 255);

void help() {
  cout << "edge-detector [config.json] [imageFile]" << endl;
}

void saveCoordinates(string filename) {
  json j = {};

  vector<json> coordinates;
  for(int i = 0; i < subRegions.size(); i++) {
    json subR = {};
    subR["tl_x"]  = subRegions.at(i).x;
    subR["tl_y"]  = subRegions.at(i).y;
    subR["br_x"]  = subRegions.at(i).x + subRegions.at(i).width;
    subR["br_y"]  = subRegions.at(i).y + subRegions.at(i).height;

    coordinates.push_back(subR);
  }

  j["filename"]     = imageFilename;
  j["coordinates"]  = coordinates;

  std::ofstream o(filename);
  o << std::setw(4) << j << endl;
}

// Source: https://stackoverflow.com/questions/28562401/resize-an-image-to-a-square-but-keep-aspect-ratio-c-opencv
cv::Mat getSquareImage( const cv::Mat& img, int target_width = 500 )
{
  int width   = img.cols,
  height  = img.rows;

  cv::Mat square = cv::Mat::zeros( target_width, target_width, img.type() );

  int max_dim = ( width >= height ) ? width : height;
  float scale = ( ( float ) target_width ) / max_dim;

  cv::Rect roi;

  if(width >= height) {
    roi.width = target_width;
    roi.x = 0;
    roi.height = height * scale;
    roi.y = ( target_width - roi.height ) / 2;
  } else {
    roi.y = 0;
    roi.height = target_width;
    roi.width = width * scale;
    roi.x = ( target_width - roi.width ) / 2;
  }

  cv::resize( img, square( roi ), roi.size() );

  return square;
}


void cannyThreshold(int, void*) {
  cout << "Initializing cannyThreshold with min: " << lowThreshold << " and max: " << lowThreshold * 3 << endl;
  blur(image, edgeImage, Size(3, 3));
  Canny(edgeImage, edgeImage, lowThreshold, lowThreshold * 3, kernelSize);

  findContours(edgeImage, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0) );
  vector<vector<Point> > contours_poly( contours.size() );
  vector<Point2f>center( contours.size() );
  vector<float>radius( contours.size() );

  for(int i = 0; i < contours.size(); i++) {
    approxPolyDP( Mat(contours[i]), contours_poly[i], 3, true );
    minEnclosingCircle( contours_poly[i], center[i], radius[i] );
  }

  Mat drawing = Mat::zeros( edgeImage.size(), CV_8UC3 );
  originalImage.copyTo(drawing);

  // Draw
  for(int i = 0; i < contours.size(); i++) {
    // Approximate center c and radius r as integers with radiusPadding
    Point c((int)center[i].x, (int)center[i].y);
    int r = (int)radius[i] + radiusPadding;

    /**
     *
     * Determine top left (tl) and bottom right (br) coordinates for square
     *
     **/
    Point tl(c.x - r, c.y - r);
    Point br(c.x + r, c.y + r);

    /**
     * Only include rectangles whose bounds are withing the image
     */
    subRegions.empty();
    subImages.empty();
    if(tl.x > 0 && tl.y > 0 && br.x <= (dWidth - 1) && br.y <= (dHeight - 1)) {
      // Push a square subregion
      Rect tempR(tl.x, tl.y, r, r);
      subRegions.push_back(tempR);

      Mat m;
      originalImage(tempR).copyTo(m);
      subImages.push_back(m);

      // Draw the circle
      circle(drawing, c, r, circleLineColor, 1, 8, 0);

      // Draw the square
      rectangle( drawing, tl, br, squareLineColor, 1, 8, 0 );
    }
  }

  imwrite(outputFile, drawing);
  imshow("Display", getSquareImage(drawing, scaleTargetWidth));
}

int main(int argc, char **argv) {

  ifstream configFile(argv[1]);
  string str((std::istreambuf_iterator<char>(configFile)),
              std::istreambuf_iterator<char>());

  auto configJson = json::parse(str);

  width             = configJson["width"];
  height            = configJson["height"];
  radiusPadding     = configJson["radiusPadding"];
  scaleTargetWidth  = configJson["scaleTargetWidth"];
  outputFile        = configJson["outputFile"];
  fileCoord         = configJson["fileCoord"];

  imageFilename = argv[2];
  image = imread(argv[2]);
  image.copyTo(originalImage);
  image.copyTo(edgeImage);
  cvtColor(image, image, COLOR_BGR2GRAY);

  dWidth  = image.cols;
  dHeight = image.rows;
  cout << "Width: " << dWidth << " Height: " << dHeight << endl;

  namedWindow("Display");
  imshow("Display", getSquareImage(originalImage, scaleTargetWidth));

  createTrackbar("Min threshold", "Display", &lowThreshold, maxThreshold, cannyThreshold);

  waitKey(0);

  saveCoordinates(fileCoord);

  return 0;
}

# object-detector

A tool for extracting generic objects from a scene using various computer vision techniques.

This is still under heavy development...

## Early development targets (2017)
  - Edge based generic objects
  - Salient object regions

## Usage (Edge Detector)
```sh
$ ./edge-detector [config-file] [image-file]
```

### Sample Config file 
```
{
  "width": 50,
  "height": 50,
  "radiusPadding": 5,
  "cannyMinThreshold": 25,
  "scaleTargetWidth": 800,
  "fileCoord": "coordinates.json",
  "outputFile": "output.jpg"
}
```

## Build Requirements
  - OpenCV 2.4.x or higher
  - CMake
  - g++ 6.3.x or higher

## Installation
```sh
$ git clone https://github.com/ralampay/object-detector.git
$ cd object-detector
$ cmake .
$ make
```

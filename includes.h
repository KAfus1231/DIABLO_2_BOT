#pragma once
#include <Windows.h>
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/core/cuda.hpp>
#include <opencv2/cudaimgproc.hpp>
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudafilters.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/ximgproc.hpp>

#include <vector>
#include <iostream>
#include <chrono>
#include <string.h>
#include <fstream>
#include <sstream>
#include <cmath>

#define PATH_TO_BASEZONE_MODEL "assets/best2.onnx"

#define IMG_WIDTH 1930
#define IMG_HEIGHT 1020
// 1930 1020

#define GREEN "\033[32m"
#define RED "\033[31m"
#define STANDART "\033[0m"
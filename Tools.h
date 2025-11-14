#pragma once
#include "includes.h"

class Tools
{
public: 
	Tools(HWND hwnd);
	~Tools();

	cv::Mat hwnd2mat(HWND hwnd);
	void saveImgToFile(cv::Mat img, std::string count);
	void mouseClickWithShift(HWND hwnd, int holdMs = 50);
	void setCursor(int x, int y);
	cv::Point2d computePhaseShift(const cv::Mat& prevGray, const cv::Mat& curGray,
		double& response, double reziseScale = 0.5);
private:
	HDC hdcMem;
	HBITMAP hBmp;
	void* pBits;
};
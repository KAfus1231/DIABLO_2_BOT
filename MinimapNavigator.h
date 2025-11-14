#pragma once
#include "includes.h"
class MinimapNavigator
{
public:
	MinimapNavigator();
	~MinimapNavigator() = default;

	void computeSkeleton(cv::Mat& img, int lowH, int lowS, int lowV, int highH, int highS, int highV,
		double minAreaTreshold, double maxAreaTreshold);

	std::vector<cv::Point> findEndPoints();

	void minimapMovement();

private:
	cv::Rect roiRect; // Œ¡À¿—“‹ ƒÀﬂ œŒ»— ¿ Ã»Õ» ¿–“€
	cv::Mat sceleton; // — ≈À≈“ Ã»Õ» ¿–“€

	std::vector<cv::Point> sceletonPoints; // “Œ◊ » Õ¿ — ≈À≈“≈
};
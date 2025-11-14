#pragma once
#include "includes.h"
#include "Tools.h"

struct Detection
{
	int classId;
	float confidence;
	cv::Rect box;
};

class BaseZone
{
public:
	BaseZone(HWND hwnd);
	~BaseZone();

	void update(Tools& tools, cv::Mat& img, HWND hwnd);

	void createBoxes(cv::Mat& img);
	bool enemyIsAlive(cv::Mat& img);
	void deleteIgnoredBoxes(Tools& tools, cv::Mat& img);
	cv::Mat detectBoxes(Tools& tools, cv::Mat& img, HWND hwnd);
private:
	cv::Rect roiRect; // ÎÁËÀÑÒÜ ÄËß ÏÎÈÑÊÀ ÈÊÎÍÊÈ ÂĞÀÃÀ

	struct BoxesItem
	{
		cv::Rect box;
		int distanceToBox;
	};
	std::vector<BoxesItem> boxesPositions;

	struct IgnoredItem 
	{
		cv::Rect box;
		std::chrono::steady_clock::time_point lastIgnoredTime;
	};
	std::vector<IgnoredItem> ignoredBoxes;

	cv::dnn::Net net;

	cv::Rect closestBox; // ÁËÈÆÀÉØÈÉ ÁÎÊÑ
	int minDetectDistance = INT_MAX; // ÌÈÍÈÌÀËÜÍÎÅ ĞÀÑÑÒÎßÍÈÅ ÄÎ ÁÎÊÑÀ
	bool foundEnemy = false; // ÔËÀÃ, ×ÒÎ ÂĞÀÃ ÍÀÉÄÅÍ

	double accSubpixX = 0.0;
	double accSubpixY = 0.0;

	cv::Mat prevGray; // ÏĞÅÄÛÄÓÙÈÉ ÊÀÄĞ ÄËß ÑÄÂÈÃÀ
};

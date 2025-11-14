#include "MinimapNavigator.h"

// пюглеп лхмхйюпрш 640 мю 360

MinimapNavigator::MinimapNavigator()
{
	roiRect = cv::Rect(1290, 70, 640, 330);
}

void MinimapNavigator::computeSkeleton(cv::Mat& img, int lowH, int lowS, int lowV, int highH, int highS, int highV,
	double minAreaTreshold, double maxAreaTreshold)
{
	cv::Mat ROI = img(roiRect);

	cv::Mat hsv, mask;
	cv::cvtColor(ROI, hsv, cv::COLOR_BGR2HSV);

	//мХФМЪЪ ЦПЮМХЖЮ: (10, 0, 115)
	//бЕПУМЪЪ ЦПЮМХЖЮ : (22, 100, 255)

	cv::Scalar lowerBound(lowH, lowS, lowV);
	cv::Scalar upperBound(highH, highS, highV);

	cv::inRange(hsv, lowerBound, upperBound, mask);

	cv::dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 4);
	cv::erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 2);
	cv::medianBlur(mask, mask, 5);

	std::vector<std::vector<cv::Point>> contours; // йнмрспш дкъ лхмхйюпрш
	cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
	cv::ximgproc::thinning(mask, sceleton, cv::ximgproc::THINNING_ZHANGSUEN); // рнмйхи йнмрсп дкъ яйекерю

	cv::Mat nz; cv::findNonZero(sceleton, nz); // ме мскебше рнвйх яйекерю --бюфмн-- nz асдер йюй cv::Point
	sceletonPoints.clear();
	sceletonPoints.reserve(nz.total()); // пегепб оюлърх онд рнвйх яйекерю

	const cv::Point* pptr = nz.ptr<cv::Point>(); // сйюгюрекэ мю рнвйх
	for (size_t i = 0; i < nz.total(); i++) sceletonPoints.push_back(pptr[i]);	// гюонкмемхе бейрнпю рнвей яйекерю

	contours.erase( // сдюкемхе лекйху йнмрспнб
		std::remove_if(
			contours.begin(), contours.end(),
			[&](const std::vector<cv::Point>& c)
			{
				double area = cv::contourArea(c);
				return area < minAreaTreshold || area > maxAreaTreshold;
			}
		), contours.end()
	);

	for (auto& point : sceletonPoints)
	{
		point.x += roiRect.x;
		point.y += roiRect.y;
	}

	cv::imshow("Mask", mask);
}

std::vector<cv::Point> MinimapNavigator::findEndPoints()
{
	cv::Mat bin01; // ахмюпмне хгнапюфемхе 0-1
	if (sceleton.channels() == 1)
		cv::threshold(sceleton, bin01, 0, 1, cv::THRESH_BINARY);
	else
	{
		MessageBox(NULL, L"мЕБЕПМЮЪ ЛЮРПХЖЮ sceleton", L"нЬХАЙЮ", MB_OK | MB_ICONERROR);
		return {};
	}

	cv::Mat k = cv::Mat::ones(3, 3, CV_8U); // ъдпн дкъ ябепрйх
	
	cv::Mat neighSum; // ясллю яняедеи
	cv::filter2D(bin01, neighSum, CV_8U, k, cv::Point(-1, -1), 0, cv::BORDER_CONSTANT);


	std::vector<cv::Point> endPoints; // рнвйх йнмжю кхмхи
	for (int y = 0; y < neighSum.rows; ++y)
	{
		const uchar* pSum = neighSum.ptr<uchar>(y);
		const uchar* pBin = bin01.ptr<uchar>(y);

		for (int x = 0; x < neighSum.cols; ++x)
		{
			if (pBin[x])
				if (pSum[x] == 2) endPoints.emplace_back(x, y);
		}
	}
	if (!endPoints.empty())
	{
		for (auto& point : endPoints)
		{
			point.x += roiRect.x;
			point.y += roiRect.y;
		}
	}

	return endPoints;
}

void MinimapNavigator::minimapMovement()
{
	if (!findEndPoints().empty())
	{
		std::vector<cv::Point> points = findEndPoints();
		/*for (const auto& point : points)
			std::cout << GREEN << "END POINT: " << point.x << " " << point.y << STANDART << std::endl;*/

		/*std::cout << RED << points.size() << STANDART << std::endl;*/
	}

	cv::imshow("Sceleton", sceleton);
}

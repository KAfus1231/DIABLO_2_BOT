#include "BaseZone.h"

BaseZone::BaseZone(HWND hwnd)
{
	net = cv::dnn::readNet(PATH_TO_BASEZONE_MODEL);

	net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
	net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);

	roiRect = cv::Rect(830, 60, 280, 50);
}

bool BaseZone::enemyIsAlive(cv::Mat& img)
{
	cv::Mat ROI = img(roiRect);

	cv::Mat hsv, mask;
	cv::cvtColor(ROI, hsv, cv::COLOR_BGR2HSV);

	cv::inRange(hsv, cv::Scalar(0, 150, 31), cv::Scalar(40, 220, 80), mask);

	cv::dilate(mask, mask, cv::Mat(), cv::Point(-1, -1), 4);
	cv::erode(mask, mask, cv::Mat(), cv::Point(-1, -1), 1);

	std::vector<std::vector<cv::Point>> contours;
	cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

	if (contours.empty()) return false;

	const double areaThreshold = 3000.0;
	bool anyContourFound = false;
	for (const auto& contour : contours)
	{
		double area = cv::contourArea(contour);
		if (area > areaThreshold)
		{
			cv::Rect rect = cv::boundingRect(contour);
			cv::Rect drawRect(rect.x + roiRect.x, rect.y + roiRect.y, rect.width, rect.height);
			cv::rectangle(img, drawRect, cv::Scalar(0, 255, 255), 2);
			anyContourFound = true;
			break;
		}
	}
	return anyContourFound;
}

void BaseZone::deleteIgnoredBoxes(Tools& tools, cv::Mat& img)
{
	//----------------------------------------------IGNORED-BOXES---------------------------------------------
	auto now = std::chrono::steady_clock::now();
	// сдюкъел анйяш вепег бпелъ
	ignoredBoxes.erase(
		std::remove_if(
			ignoredBoxes.begin(), ignoredBoxes.end(),
			[&](const auto& item) {
				return now > item.lastIgnoredTime;
			}
		), ignoredBoxes.end()
	);

	cv::Mat gray;
	cv::cvtColor(img, gray, cv::COLOR_BGRA2GRAY);

	// бшвхякъел ядбхц лефдс PREVGRAY х GRAY
	double response = 0.0;
	double downscale = 0.5;
	// ядбхцх он X х Y
	cv::Point2d shift = tools.computePhaseShift(prevGray, gray, response, downscale);

	// опнбепъел онйюгюрекэ сбепеммнярх
	const double minResponse = 0.01;
	if (prevGray.empty() || response < minResponse)
		prevGray = gray.clone();
	else
	{
		// мюйнокемхе дпнамни вюярх
		accSubpixX += shift.x;
		accSubpixY += shift.y;
		int dx = int(std::round(accSubpixX));
		int dy = int(std::round(accSubpixY));
		// нярюбкъел дпнамсч вюярэ
		accSubpixX -= dx;
		accSubpixY -= dy;

		// опхлемъел DX х DY йн бяел IGNOREDBOXES
		for (auto& it : ignoredBoxes)
		{
			it.box.x += dx;
			it.box.y += dy;
			// нцпюмхвемхе пюлйюлх хгнапюфемхъ
			it.box.x = std::max(0, std::min(it.box.x, IMG_WIDTH - it.box.width));
			it.box.y = std::max(0, std::min(it.box.y, IMG_HEIGHT - it.box.height));
		}

		// намнбкъел PREVGRAY
		prevGray = gray.clone();
	}

	//----------------------------------------------DETECT----------------------------------------------------

	if (!boxesPositions.empty() && !ignoredBoxes.empty())
	{
		const double overlapThreshold = 0.5;

		boxesPositions.erase(
			std::remove_if(
				boxesPositions.begin(), boxesPositions.end(),
				[&](const BoxesItem& pos) -> bool
				{
					int posArea = pos.box.area();
					if (posArea <= 0) return true; // сдюкъел анйя

					for (const auto& ib : ignoredBoxes)
					{
						cv::Rect intersection = pos.box & ib.box; // оепеяевемхе я хцмнп анйянл
						if (intersection.area() <= 0) continue; // мер оепеяевемхъ

						double rel = static_cast<double>(intersection.area() / static_cast<double>(posArea));
						if (rel > overlapThreshold)
							return true; // сдюкъел анйя, еякх оепеяевемхе я хцмнп анйянл анкэье опедекю
					}
					return false;
				}),
			boxesPositions.end()
		);
	}

	if (boxesPositions.empty())
		foundEnemy = false;
	else
	{
		std::sort(boxesPositions.begin(), boxesPositions.end(),
			[](const BoxesItem& a, const BoxesItem& b) {
				return a.distanceToBox < b.distanceToBox;
			});
		foundEnemy = true;
		closestBox = boxesPositions.front().box; // акхфюиьхи анйя
	}
}

void BaseZone::createBoxes(cv::Mat& img)
{
	//----------------------------------------------DNN-------------------------------------------------------
	cv::Mat imgBGR, imgResized;

	cv::cvtColor(img, imgBGR, cv::COLOR_BGRA2BGR); // опенапюгсел б BGR
	cv::resize(imgBGR, imgResized, cv::Size(640, 640), 0, 0, cv::INTER_LINEAR); // пюглеп BGR йюдпю

	cv::Mat blob = cv::dnn::blobFromImage(
		imgResized, 1.0 / 255.0, cv::Size(640, 640), cv::Scalar(0, 0, 0), true, false); // опенапюгсел б BLOBS)
	net.setInput(blob);

	std::vector<cv::Mat> outputs;
	net.forward(outputs, net.getUnconnectedOutLayersNames()); // оепедюел OUTPUTS б лндекэ

	const float confThreshold = 0.5f;
	const float nmsThreshold = 0.5f;

	cv::Mat detMat = outputs[0];
	detMat = detMat.reshape(1, detMat.size[1]);

	std::vector<Detection> detections;
	for (int i = 0; i < detMat.rows; ++i)
	{
		float* data = (float*)detMat.ptr(i); // люяяхб хг 9 назейрнб ярпнйх
		float objectness = data[4];			 // опнбепйю еярэ кх рюл бннаые врн-рн
		if (objectness < confThreshold) continue;

		// онхяй ксвьецн йкюяяю
		cv::Mat scores(1, detMat.cols - 5, CV_32FC1, data + 5); // бейрнп йкюяянб, мювхмюъ я DATA[5]
		cv::Point classIdPoint;
		double maxClassScore;
		cv::minMaxLoc(scores, nullptr, &maxClassScore, nullptr, &classIdPoint); // мюханкэьюъ бепнърмнярэ йкюяяю
		float conf = objectness * (float)maxClassScore;
		if (conf < confThreshold) continue;

		// хгбкейюел BBOX
		float cx = data[0] / 640, cy = data[1] / 640,
			w = data[2] / 640, h = data[3] / 640;

		int left = int((cx - 0.5f * w) * IMG_WIDTH);
		int top = int((cy - 0.5f * h) * IMG_HEIGHT);
		int width = int(w * IMG_WIDTH);
		int height = int(h * IMG_HEIGHT);

		detections.push_back({ classIdPoint.x, conf, cv::Rect(left, top, width, height) });
	}

	//----------------------------------------------NMS------------------------------------------------------
	std::vector<cv::Rect> boxes;	// анйяш
	std::vector<float> confidences; // сбепеммнярэ б мху
	for (auto& d : detections)
	{
		if (d.box.width <= 0 || d.box.height <= 0)
		{
			std::cerr << "опносяйюел ноюямше анйяш: " << d.box << std::endl;
			continue;
		}

		if (!std::isfinite(d.confidence))
		{
			std::cerr << "опносяйюел ноюямше сбепеммнярх" << std::endl;
			continue;
		}

		boxes.push_back(d.box);
		confidences.push_back(d.confidence);
	}

	std::vector<int> indices;
	if (boxes.size() == confidences.size() && !boxes.empty())
		cv::dnn::NMSBoxes(boxes, confidences, confThreshold, nmsThreshold, indices);

	//----------------------------------------------DRAW------------------------------------------------------
	cv::Point center(IMG_WIDTH / 2, IMG_HEIGHT / 2);
	boxesPositions.clear(); // нвхыюел онгхжхх оепед мнбни дерейжхеи

	// нрпхянбйю
	for (int idx : indices)
	{
		auto& d = detections[idx]; // рнкэйн нярюбьхияъ оняке NMS BBOX
		cv::Point boxCenter(d.box.x + d.box.width / 2, d.box.y + d.box.height / 2);
		int distanceToEnemy = std::abs(boxCenter.x - center.x) + std::abs(boxCenter.y - center.y);
		cv::rectangle(img, d.box, cv::Scalar(0, 0, 255), 2);

		boxesPositions.push_back({ d.box, distanceToEnemy });
	}
}

cv::Mat BaseZone::detectBoxes(Tools& tools, cv::Mat& img, HWND hwnd)
{
	if (foundEnemy)
	{
		POINT imgPt = {
			closestBox.x + closestBox.width / 2,
			closestBox.y + closestBox.height / 2
		};	

		// янгдюел мнплюкхгнбюммше йннпдхмюрш дкъ лшьйх
		RECT cr; GetClientRect(hwnd, &cr);
		POINT clientWH = { cr.right - cr.left, cr.bottom - cr.top };
		POINT scalePt = { imgPt.x * clientWH.x / IMG_WIDTH, imgPt.y * clientWH.y / IMG_HEIGHT };

		// сряюмюбкхбюел йспянп б мнплюкхгнбюммше йннпдхмюрш
		tools.setCursor(scalePt.x, scalePt.y);
		bool alive = enemyIsAlive(img);

		static auto lastClickTime = std::chrono::steady_clock::now();
		auto currentTime = std::chrono::steady_clock::now();
		auto elepsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastClickTime).count();

		if (elepsedTime >= 500 && alive) // йкхй рнкэйн пюг б онкяейсмдш
		{
			lastClickTime = currentTime; // намнбкъел бпелъ онякедмецн йкхйю

			tools.mouseClickWithShift(hwnd);
		}
		else if (!alive)
		{
			// янгдюел накюярэ б йнрнпни лнфер нйюгюрэяъ анйя дкъ хцмнпю
			int padX = std::max(4, closestBox.width / 8);
			int padY = std::max(4, closestBox.height / 8);
			cv::Rect padded(closestBox.x - padX, closestBox.y - padY,
				closestBox.width + 2 * padX, closestBox.height + 2 * padY
			);

			/*padded.x = std::max(0, std::min(padded.x, IMG_WIDTH - padded.width));
			padded.y = std::max(0, std::min(padded.y, IMG_HEIGHT - padded.height));*/

			ignoredBoxes.push_back({padded, std::chrono::steady_clock::now() + std::chrono::seconds(6)});
		}
	}

	return img;
}

void BaseZone::update(Tools& tools, cv::Mat& img, HWND hwnd)
{
	createBoxes(img);
	deleteIgnoredBoxes(tools, img);

	img = detectBoxes(tools, img, hwnd);
}

BaseZone::~BaseZone(){}
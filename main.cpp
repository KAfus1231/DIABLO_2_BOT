#include "includes.h"
#include "BaseZone.h"
#include "MinimapNavigator.h"

void on_trackbar(int, void*) {}

std::atomic<int> runTime{ 0 };
void runTimer()
{
	std::thread([]
		{
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(1));
				runTime++;
			}

		}).detach();
}

VOID showBalloon(LPCWSTR title, LPCWSTR msg)
{
	NOTIFYICONDATA nid = {};
	nid.cbSize = sizeof(nid);
	nid.hWnd = GetConsoleWindow();
	nid.uID = 1;
	nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = WM_USER + 1;
	nid.hIcon = LoadIcon(NULL, IDI_INFORMATION);
	lstrcpy(nid.szTip, L"Diablo App");
	lstrcpy(nid.szInfo, msg);
	lstrcpy(nid.szInfoTitle, title);
	nid.dwInfoFlags = NIIF_INFO;

	Shell_NotifyIcon(NIM_ADD, &nid);
	Sleep(3000);
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

int main()
{
	setlocale(LC_ALL, "ru");
	runTimer();
	int lastTimeUpdate = 0;
	int frame = 0;

	cv::namedWindow("Diablo", cv::WINDOW_NORMAL);
	cv::resizeWindow("Diablo", 700, 300);
	HWND hwndDesktop = FindWindow(NULL, L"Diablo II: Resurrected");

	Tools tools(hwndDesktop);
	BaseZone baseZone(hwndDesktop);
	MinimapNavigator minimapNavigator;

	cv::Mat img = cv::imread("assets/test2.jpg");

	int lowH = 0, highH = 20;
	int lowS = 50, highS = 130;
	int lowV = 110, highV = 240;

	int minAreaTreshold = 100;
	int maxAreaTreshold = 100;

	cv::createTrackbar("Min Area", "Diablo", &minAreaTreshold, 10000, on_trackbar);
	cv::createTrackbar("Max Area", "Diablo", &maxAreaTreshold, 10000, on_trackbar);

	/*cv::createTrackbar("Low H", "Diablo", &lowH, 180, on_trackbar);
	cv::createTrackbar("High H", "Diablo", &highH, 180, on_trackbar);

	cv::createTrackbar("Low S", "Diablo", &lowS, 255, on_trackbar);
	cv::createTrackbar("High S", "Diablo", &highS, 255, on_trackbar);

	cv::createTrackbar("Low V", "Diablo", &lowV, 255, on_trackbar);
	cv::createTrackbar("High V", "Diablo", &highV, 255, on_trackbar);*/

	cv::Rect roiRect(1290, 70, 640, 330);
	while (true)
	{
		cv::Mat img = tools.hwnd2mat(hwndDesktop);

		/*if (runTime.load() > 2 && runTime.load() % 3 == 0 && runTime.load() != lastTimeUpdate)
		{
			lastTimeUpdate = runTime.load();
			int count = 380 + (runTime.load() / 3);
			std::string strCount = std::to_string(count);

			if (strCount == "500") showBalloon(L"Успех!", L"500 снимков есть!");

			baseZone.saveImgToFile(img, strCount);
		}*/

		/*baseZone.update(tools, img, hwndDesktop);*/

		minimapNavigator.computeSkeleton(img, lowH, lowS, lowV, highH, highS, highV, minAreaTreshold, maxAreaTreshold);
		minimapNavigator.minimapMovement();

		img = img(roiRect);

		cv::imshow("Diablo", img);

		if (cv::waitKey(1) == 27) break;
	}

	return 0;
}
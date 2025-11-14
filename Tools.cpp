#include "Tools.h"

Tools::Tools(HWND hwnd)
{
	HDC hdcWindow = GetDC(hwnd);
	hdcMem = CreateCompatibleDC(hdcWindow);
	BITMAPINFO bi = {};
	bi.bmiHeader = { sizeof(bi.bmiHeader), IMG_WIDTH, -IMG_HEIGHT, 1, 32, BI_RGB };
	hBmp = CreateDIBSection(hdcWindow, &bi, DIB_RGB_COLORS, &pBits, NULL, 0);
	SelectObject(hdcMem, hBmp);
	ReleaseDC(hwnd, hdcWindow);
}
Tools::~Tools()
{
	if (hdcMem) DeleteDC(hdcMem);
	if (hBmp) DeleteObject(hBmp);
	if (pBits) pBits = nullptr;
}

cv::Mat Tools::hwnd2mat(HWND hwnd)
{
	PrintWindow(hwnd, hdcMem, PW_RENDERFULLCONTENT);

	cv::Mat img(IMG_HEIGHT, IMG_WIDTH, CV_8UC4, pBits);
	return img;
}

void Tools::saveImgToFile(cv::Mat img, std::string count)
{
	if (!cv::imwrite("assets/baseZone/" + count + ".png", img))
		MessageBox(NULL, L"Œ¯Ë·Í‡ ÒÓı‡ÌÂÌËˇ Ù‡ÈÎ‡", L"Œ¯Ë·Í‡", MB_OK | MB_ICONERROR);
}

void Tools::mouseClickWithShift(HWND hwnd, int holdMs)
{
	if (GetAsyncKeyState(VK_F12) & 0x8000) return;

	if (hwnd)
	{
		SetForegroundWindow(hwnd); // ”—“¿Õ¿¬À»¬¿≈Ã Œ ÕŒ Õ¿ œ≈–≈ƒÕ»… œÀ¿Õ
		std::this_thread::sleep_for(std::chrono::milliseconds(15));
	}

	INPUT input = {};
	input.type = INPUT_KEYBOARD;
	input.ki.wVk = VK_LSHIFT;
	input.ki.dwFlags = 0;
	SendInput(1, &input, sizeof(INPUT));

	std::this_thread::sleep_for(std::chrono::milliseconds(8));

	INPUT mouse = {};
	mouse.type = INPUT_MOUSE;
	mouse.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
	SendInput(1, &mouse, sizeof(INPUT));

	std::this_thread::sleep_for(std::chrono::milliseconds(holdMs));

	mouse.mi.dwFlags = MOUSEEVENTF_LEFTUP;
	SendInput(1, &mouse, sizeof(INPUT));

	std::this_thread::sleep_for(std::chrono::milliseconds(8));

	input.ki.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, &input, sizeof(INPUT));
}

void Tools::setCursor(int x, int y)
{
	if (GetAsyncKeyState(VK_F12) & 0x8000) return;

	SetCursorPos(x, y);
}

cv::Point2d Tools::computePhaseShift(const cv::Mat& prevGray, const cv::Mat& curGray, double& response, double resizeScale)
{
	if (prevGray.empty()) return cv::Point(0, 0);

	// Ã≈Õﬂ≈Ã –¿«Ã≈–€  ¿ƒ–Œ¬, ≈—À» Õ≈Œ¡’Œƒ»ÃŒ
	cv::Mat pSmall, cSmall;
	if (resizeScale != 1.0)
	{
		cv::resize(prevGray, pSmall, cv::Size(), resizeScale, resizeScale, cv::INTER_LINEAR);
		cv::resize(curGray, cSmall, cv::Size(), resizeScale, resizeScale, cv::INTER_LINEAR);
	}
	else
	{
		pSmall = prevGray;
		cSmall = curGray;
	}

	// œ–≈Œ¡–¿«”≈Ã ¬ FLOAT » œ–»Ã≈Õﬂ≈Ã Œ ÕŒ ’¿ÕÕ¿
	cv::Mat f1, f2;
	pSmall.convertTo(f1, CV_32F);
	cSmall.convertTo(f2, CV_32F);

	cv::Mat hann;
	cv::createHanningWindow(hann, f1.size(), CV_32F);
	f1 = f1.mul(hann);
	f2 = f2.mul(hann);

	// ¬€◊»—Àﬂ≈Ã —ƒ¬»√
	cv::Point2d shift = cv::phaseCorrelate(f1, f2, hann, &response);

	if (resizeScale != 1.0)
	{
		shift.x /= resizeScale;
		shift.y /= resizeScale;
	}

	return shift;
}


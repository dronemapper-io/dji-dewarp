/**
 * Authors: Jon-Pierre Stoermer
 * Institutions: DroneMapper.com
 * Date: 05/01/2019
 * Email: jp@dronemapper.com
 */
#include <iostream>
#include <sstream>
#include <string>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

/*
drnmppr-dewarp: Quick tool to apply EXIF dewarp data to images from DJI Drones. Uses exiftool to preserve EXIF metadata.

Example:
c:\my-images> MKDIR dewarped
c:\my-images> drnmppr-dewarp.exe "*.JPG" dewarped\

[!] Tested w/ Windows 10 (VS 2017), You must build OpenCV 3.4.5 and have release dlls available. Building and modification for ubuntu / linux should be trivial.
[!] At this time you will need to add your dewarp EXIF to the code below and set the correct image width & height.
*/

// replace a pattern in a string
void replace(std::string & data, std::string toSearch, std::string replaceStr)
{
	size_t pos = data.find(toSearch);
	while (pos != std::string::npos)
	{
		data.replace(pos, toSearch.size(), replaceStr);
		pos = data.find(toSearch, pos + replaceStr.size());
	}
}

// get application directory
inline std::string getApplicationDir() {
	char path[MAX_PATH] = "";
	GetModuleFileNameA(NULL, path, MAX_PATH);
	PathRemoveFileSpecA(path);
	PathAddBackslashA(path);
	return path;
}

// meat and cheese
int main(int argc, char ** argv)
{
	std::vector<cv::String> fn;
	glob(argv[1], fn, false);
	size_t count = fn.size();

	std::cout << "[*] Image Pattern: " << argv[1] << "\n";
	std::cout << "[*] Output Folder: " << argv[2] << "\n";

	std::string appDir = getApplicationDir();

	/*
	Example from Phantom 4 RTK

	Dewarp Data                     : 2018-09-04;3678.870000000000,3671.840000000000,10.100000000000,27.290000000000,-0.268652000000,0.114663000000,0.000015268800,-0.000046070700,-0.035026100000
	Dewarp Flag                     : 0

	Date and Time, fx, fy, cx, cy, k1, k2, p1, p2, k3
	*/

	for (size_t i = 0; i < count; i++) {
		std::string img = fn[i];
		replace(img, ".\\", "");
		std::cout << "[*] Open: " << img << "\n";

		cv::Mat src = cv::imread(img);
		cv::Mat distCoeff;
		distCoeff = cv::Mat::zeros(5, 1, CV_64FC1);

		distCoeff.at<double>(0, 0) = -0.268652000000;		// k1
		distCoeff.at<double>(1, 0) = 0.114663000000;		// k2
		distCoeff.at<double>(2, 0) = 0.000015268800;		// p1
		distCoeff.at<double>(3, 0) = -0.000046070700;		// p2
		distCoeff.at<double>(4, 0) = -0.035026100000;		// k3

		cv::Mat cam1;
		cam1 = cv::Mat::zeros(3, 3, CV_32FC1);
		cam1.at<float>(0, 2) = 2736 - 10.100000000000;		// cX (5472x3648 Width / 2)
		cam1.at<float>(1, 2) = 1824 + 27.290000000000;		// cY (5472x3648 Height / 2)
		cam1.at<float>(0, 0) = 3678.870000000000;			// fx
		cam1.at<float>(1, 1) = 3671.840000000000;			// fy
		cam1.at<float>(2, 2) = 1;

		cv::Mat dst, map1, map2;
		cv::initUndistortRectifyMap(cam1, distCoeff, cv::Mat(), cam1, src.size(), CV_32FC1, map1, map2);
		cv::remap(src, dst, map1, map2, cv::INTER_LINEAR);

		std::string dst_img = argv[2] + img;
		std::cout << "\t[*] Dewarp Write: " << dst_img << "\n";
		cv::imwrite(dst_img, dst);

		std::string exiftool = appDir + "exiftool.exe -overwrite_original_in_place -TagsFromFile ";
		exiftool += img + " ";
		exiftool += dst_img;
		std::cout << "\t[*] Transfer EXIF\n";

		system(exiftool.c_str());
	}
}
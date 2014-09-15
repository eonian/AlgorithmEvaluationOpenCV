#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>

using namespace cv;
using namespace std;

Mat imageOriginal, imageGray, imageCanny, imageOutput, integralSum;

#pragma region part1

#pragma endregion

#pragma region part2

#pragma endregion

/*
	Functions:
	ShowImage()
	LoadImage()
	Preprocess()
	DetectContours()
*/
#pragma region ZheyunYao

void ShowImage(Mat image, string windowName){
	namedWindow(windowName, CV_WINDOW_AUTOSIZE);
	imshow(windowName, image);
	waitKey();
}

void LoadImage(string path = "C:\\HuaWeiImage\\华为拍照_校正\\华为拍照_校正\\正常光照\\u_60.jpg"){
	imageOriginal = imread(path);
	ShowImage(imageOriginal, "Original");

}

void Preprocess(){
	cvtColor(imageOriginal, imageGray, CV_BGR2GRAY);
	ShowImage(imageGray, "Gray");

}
void DetectContours(double thresh = 100){
	Canny(imageOriginal, imageCanny, thresh, thresh * 2);
	ShowImage(imageCanny, "Canny");

}

void test1(){
	int h = imageCanny.rows;
	int w = imageCanny.cols;
	integral(imageCanny, integralSum);
	
}
#pragma endregion

int main(){
	LoadImage();
	Preprocess();
	DetectContours();
	test1();


	waitKey();
	destroyAllWindows();
	return 0;
}
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>

using namespace cv;
using namespace std;

#define MAXWIDTH  640
#define MAXHEIGHT 480

struct GraphPixel{
	int r, g, b;
	int edge;
};

GraphPixel g[MAXWIDTH][MAXHEIGHT];

int w, h;
Mat imageOriginal, imageGray, imageCanny, imageOutput, imageLabel;
Mat mask;
vector<Vec2f> lines;
vector<vector<Point>> contours;


#pragma region Qian Zifei

#pragma endregion

#pragma region Qu Yingze

#pragma endregion

/*
	Functions:
	ShowImage()				显示图像
	LoadImage()				载入图像
	Preprocess()			预处理
	DetectContours()		边缘检测
	OptimizationCanny()		Canny优化（腐蚀膨胀）
	DetectLines()			直线检测
	CheckContour()			边缘验证（面积、形状）
	ImageOutput()			图像输出，用于MFR Graph Cut
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
	w = imageOriginal.cols;
	h = imageOriginal.rows;
	printf("%dx%d\n", w, h);

}

void Preprocess(){
	cvtColor(imageOriginal, imageGray, CV_BGR2GRAY);
	ShowImage(imageGray, "Gray");
	//亮度调整~

}

bool checkContour(vector<Point> contour){
	double area = contourArea(contour);
	if (area < (w * h / 100)) return false;
	RotatedRect rect = minAreaRect(contour);
	if (rect.size.area() > area * 1.5) return false;
	return true;
}

void OptimizeCanny(){
	Mat imageTemp1, imageTemp2, imageTemp3, imageTemp4;
	dilate(imageCanny, imageTemp1, Mat(3, 3, CV_8U), Point(-1, -1), 2);
	erode(imageTemp1, imageTemp2, Mat(3, 3, CV_8U), Point(-1, -1), 2);
	
	ShowImage(imageTemp1, "Dilate");
	ShowImage(imageTemp2, "Erode");
	//Canny(imageTemp2, imageCanny, 100, 200);
	imageCanny = imageTemp2.clone();
	ShowImage(imageCanny, "Optimized Canny");
}

void DetectContours(double thresh = 100){
	Canny(imageOriginal, imageCanny, thresh, thresh * 2);
	line(imageCanny, Point(0, 0), Point(0, h), Scalar(255, 255, 255), 1);
	ShowImage(imageCanny, "Canny");
	//OptimizeCanny();
	findContours(imageCanny, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
	imageOutput = imageOriginal.clone();
	for (int i = 0; i < contours.size(); i++){
		if (checkContour(contours[i])){
			drawContours(imageOutput, contours, i, Scalar(0, 255, 0), 2);
		}
	}
	ShowImage(imageOutput, "Contours");
}

void DetectLines(){
	HoughLines(imageCanny, lines, 1, CV_PI / 720, 150, 0, 0);
	cout << lines.size() << endl;
	imageOutput = imageOriginal.clone();
	for (int i = 0; i < lines.size(); i++){
		float rho = lines[i][0];
		float theta = lines[i][1];
		float y = rho / sin(theta);
		float x = rho / cos(theta);
		line(imageOutput, Point(0, (int)y), Point((int)x, 0), Scalar(0, 0, 255), 2);
	}
	ShowImage(imageOutput, "Lines");
}

void ImageOutput(){
	FILE *fp;
	fopen_s(&fp, "ImageOutput.txt", "w");
	fprintf(fp, "%d %d\n", imageOriginal.rows, imageOriginal.cols);
	for (MatIterator_<Vec3b> it = imageOriginal.begin<Vec3b>(); it != imageOriginal.end<Vec3b>(); it++){
		fprintf(fp, "%d %d %d\n", (*it)[0], (*it)[1], (*it)[2]);
	}
	for (MatIterator_<uchar> it = imageCanny.begin<uchar>(); it != imageCanny.end<uchar>(); it++){
		fprintf(fp, "%d\n", (*it)? 1 : 0);
	}
	fclose(fp);
}

void ImageLabelInput(){
	FILE *fp;
	fopen_s(&fp, "ImageLabel.txt", "r");
	imageLabel = imageOriginal.clone();
	for (MatIterator_<Vec3b> it = imageLabel.begin<Vec3b>(); it != imageLabel.end<Vec3b>(); it++){
		int label;
		fscanf_s(fp, "%d", &label);
		if (label == 0){
			(*it)[0] = 0;
			(*it)[1] = 0;
			(*it)[2] = 0;
		}
		else if (label == 1)
		{
			(*it)[0] = 255;
			(*it)[1] = 255;
			(*it)[2] = 255;
		}
		else if (label == 2)
		{
			(*it)[0] = 0;
			(*it)[1] = 255;
			(*it)[2] = 0;

		}
		else if (label == 3)
		{
			(*it)[0] = 0;
			(*it)[1] = 255;
			(*it)[2] = 0;
		}
	}
	fclose(fp);
	ShowImage(imageLabel, "Label");
}

int Rgb2Gray(GraphPixel p){
	return (int)(p.r * 0.299 + p.g * 0.587 + p.b * 0.114);
}

#pragma endregion

int main(){
	string user = "yzy";

	if (user == "yzy")
	{
		LoadImage();
		Preprocess();
		DetectContours();
		DetectLines();

		ImageOutput();

		ImageLabelInput();

		waitKey();
		destroyAllWindows();

	} else
	if (user == "qzf"){
		//put your test code here :>
	} else
	if (user == "qyz"){
		//put your test code here :>
	}
	return 0;
}

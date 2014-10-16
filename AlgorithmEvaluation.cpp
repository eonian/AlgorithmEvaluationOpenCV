#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <opencv2/opencv_modules.hpp>
#include "GCoptimization.h"

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


#pragma region part1

#pragma endregion

#pragma region part2

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
	GraphCut()				图像分割
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

void GraphCut(){
	Rect rect = Rect(0, 0, w, h);
	Mat bgdModel, fgdModel;
	grabCut(imageOriginal, mask, rect, bgdModel, fgdModel, 1);
	ShowImage(mask, "Grab Cut");
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
	fopen_s(&fp, "C:\\HuaWeiImage\\gco-v3\\GCO\\ImageLabel.txt", "r");
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

bool ReadImage(){
	FILE *fp;
	if ((fopen_s(&fp, "C:\\HuaWeiImage\\AlgorithmEvaluationOpenCV\\AlgorithmEvaluationOpenCV\\ImageOutput.txt", "r")) == NULL){
		printf("can't open the file.\n");
		return false;
	}

	fscanf_s(fp, "%d %d", &h, &w);
	for (int i = 0; i < h; i++)
	for (int j = 0; j < w; j++){
		fscanf_s(fp, "%d %d %d", &g[i][j].r, &g[i][j].g, &g[i][j].b);
	}
	for (int i = 0; i < h; i++)
	for (int j = 0; j < w; j++){
		fscanf_s(fp, "%d", &g[i][j].edge);
	}

	fclose(fp);
	return true;
}

int ComputeCost(int i, int j, int l){
	int cost = 0;
	GraphPixel p = g[i][j];
	int gray = Rgb2Gray(p);
	if (l == 0){
		cost = max(gray - 10, 0) + 80;
	}
	else
	if (l == 1){
		cost = max(210 - gray, 0) + 10;
	}
	else
	if (l == 2){
		cost = max(max(p.r, p.g, p.b), max(255 - p.r, 255 - p.g, 255 - p.b)) * 2 / 3;
	}
	else
	if (l == 3){
		cost = p.edge ? 0 : 300;
	}
	else
	{
		cost = 0;
	}
	return cost;
}

void Gco(){
	int width = w;
	int height = h;
	int num_pixels = width*height;
	int num_labels = 4; // 0-空闲区域 1-假面板 2-杂物 3-设备 4-机柜 5-外界
	int *result = new int[num_pixels];

	GCoptimizationGridGraph *gc = new GCoptimizationGridGraph(width, height, num_labels);
	for (int l = 0; l < num_labels; l++)
	for (int i = 0; i < height; i++)
	for (int j = 0; j < width; j++){
		gc->setDataCost(i * width + j, l, ComputeCost(i, j, l));
	}
	int varity = 6;
	gc->setSmoothCost(0, 1, 100 / varity);
	gc->setSmoothCost(1, 0, 100 / varity);
	gc->setSmoothCost(0, 2, 100 / varity);
	gc->setSmoothCost(2, 0, 100 / varity);
	gc->setSmoothCost(1, 2, 10 / varity);
	gc->setSmoothCost(2, 1, 10 / varity);

	gc->setSmoothCost(0, 3, 100 / varity);
	gc->setSmoothCost(3, 0, 100 / varity);
	gc->setSmoothCost(1, 3, 100 / varity);
	gc->setSmoothCost(3, 1, 100 / varity);
	gc->setSmoothCost(2, 3, 6 / varity);
	gc->setSmoothCost(3, 2, 6 / varity);


	//gc->setLabelCost(0);
	printf("before: %d\n", gc->compute_energy());
	gc->swap();
	printf("after: %d\n", gc->compute_energy());
	FILE *fout;
	fopen_s(&fout, "ImageLabel.txt", "w");
	for (int i = 0; i < num_pixels; i++){
		result[i] = gc->whatLabel(i);
		fprintf(fout, "%d\n", result[i]);
	}
	fclose(fout);
	delete[] result;
}
#pragma endregion

int main(){
	LoadImage();
	Preprocess();
	DetectContours();
	//DetectLines();

	ImageOutput();
	
	Gco();
	ImageLabelInput();

	waitKey();
	destroyAllWindows();
	return 0;
}


//
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//
//#include <iostream>
//
//using namespace std;
//using namespace cv;
//
//static void help()
//{
//	cout << "\nThis program demonstrates GrabCut segmentation -- select an object in a region\n"
//		"and then grabcut will attempt to segment it out.\n"
//		"Call:\n"
//		"./grabcut <image_name>\n"
//		"\nSelect a rectangular area around the object you want to segment\n" <<
//		"\nHot keys: \n"
//		"\tESC - quit the program\n"
//		"\tr - restore the original image\n"
//		"\tn - next iteration\n"
//		"\n"
//		"\tleft mouse button - set rectangle\n"
//		"\n"
//		"\tCTRL+left mouse button - set GC_BGD pixels\n"
//		"\tSHIFT+left mouse button - set CG_FGD pixels\n"
//		"\n"
//		"\tCTRL+right mouse button - set GC_PR_BGD pixels\n"
//		"\tSHIFT+right mouse button - set CG_PR_FGD pixels\n" << endl;
//}
//
//const Scalar RED = Scalar(0, 0, 255);
//const Scalar PINK = Scalar(230, 130, 255);
//const Scalar BLUE = Scalar(255, 0, 0);
//const Scalar LIGHTBLUE = Scalar(255, 255, 160);
//const Scalar GREEN = Scalar(0, 255, 0);
//
//const int BGD_KEY = CV_EVENT_FLAG_CTRLKEY;
//const int FGD_KEY = CV_EVENT_FLAG_SHIFTKEY;
//
//static void getBinMask(const Mat& comMask, Mat& binMask)
//{
//	if (comMask.empty() || comMask.type() != CV_8UC1)
//		CV_Error(CV_StsBadArg, "comMask is empty or has incorrect type (not CV_8UC1)");
//	if (binMask.empty() || binMask.rows != comMask.rows || binMask.cols != comMask.cols)
//		binMask.create(comMask.size(), CV_8UC1);
//	binMask = comMask & 1;
//}
//
//class GCApplication
//{
//public:
//	enum{ NOT_SET = 0, IN_PROCESS = 1, SET = 2 };
//	static const int radius = 2;
//	static const int thickness = -1;
//
//	void reset();
//	void setImageAndWinName(const Mat& _image, const string& _winName);
//	void showImage() const;
//	void mouseClick(int event, int x, int y, int flags, void* param);
//	int nextIter();
//	int getIterCount() const { return iterCount; }
//private:
//	void setRectInMask();
//	void setLblsInMask(int flags, Point p, bool isPr);
//
//	const string* winName;
//	const Mat* image;
//	Mat mask;
//	Mat bgdModel, fgdModel;
//
//	uchar rectState, lblsState, prLblsState;
//	bool isInitialized;
//
//	Rect rect;
//	vector<Point> fgdPxls, bgdPxls, prFgdPxls, prBgdPxls;
//	int iterCount;
//};
//
//void GCApplication::reset()
//{
//	if (!mask.empty())
//		mask.setTo(Scalar::all(GC_BGD));
//	bgdPxls.clear(); fgdPxls.clear();
//	prBgdPxls.clear();  prFgdPxls.clear();
//
//	isInitialized = false;
//	rectState = NOT_SET;
//	lblsState = NOT_SET;
//	prLblsState = NOT_SET;
//	iterCount = 0;
//}
//
//void GCApplication::setImageAndWinName(const Mat& _image, const string& _winName)
//{
//	if (_image.empty() || _winName.empty())
//		return;
//	image = &_image;
//	winName = &_winName;
//	mask.create(image->size(), CV_8UC1);
//	reset();
//}
//
//void GCApplication::showImage() const
//{
//	if (image->empty() || winName->empty())
//		return;
//
//	Mat res;
//	Mat binMask;
//	if (!isInitialized)
//		image->copyTo(res);
//	else
//	{
//		getBinMask(mask, binMask);
//		image->copyTo(res, binMask);
//	}
//
//	vector<Point>::const_iterator it;
//	for (it = bgdPxls.begin(); it != bgdPxls.end(); ++it)
//		circle(res, *it, radius, BLUE, thickness);
//	for (it = fgdPxls.begin(); it != fgdPxls.end(); ++it)
//		circle(res, *it, radius, RED, thickness);
//	for (it = prBgdPxls.begin(); it != prBgdPxls.end(); ++it)
//		circle(res, *it, radius, LIGHTBLUE, thickness);
//	for (it = prFgdPxls.begin(); it != prFgdPxls.end(); ++it)
//		circle(res, *it, radius, PINK, thickness);
//
//	if (rectState == IN_PROCESS || rectState == SET)
//		rectangle(res, Point(rect.x, rect.y), Point(rect.x + rect.width, rect.y + rect.height), GREEN, 2);
//
//	imshow(*winName, res);
//}
//
//void GCApplication::setRectInMask()
//{
//	assert(!mask.empty());
//	mask.setTo(GC_BGD);
//	rect.x = max(0, rect.x);
//	rect.y = max(0, rect.y);
//	rect.width = min(rect.width, image->cols - rect.x);
//	rect.height = min(rect.height, image->rows - rect.y);
//	(mask(rect)).setTo(Scalar(GC_PR_FGD));
//}
//
//void GCApplication::setLblsInMask(int flags, Point p, bool isPr)
//{
//	vector<Point> *bpxls, *fpxls;
//	uchar bvalue, fvalue;
//	if (!isPr)
//	{
//		bpxls = &bgdPxls;
//		fpxls = &fgdPxls;
//		bvalue = GC_BGD;
//		fvalue = GC_FGD;
//	}
//	else
//	{
//		bpxls = &prBgdPxls;
//		fpxls = &prFgdPxls;
//		bvalue = GC_PR_BGD;
//		fvalue = GC_PR_FGD;
//	}
//	if (flags & BGD_KEY)
//	{
//		bpxls->push_back(p);
//		circle(mask, p, radius, bvalue, thickness);
//	}
//	if (flags & FGD_KEY)
//	{
//		fpxls->push_back(p);
//		circle(mask, p, radius, fvalue, thickness);
//	}
//}
//
//void GCApplication::mouseClick(int event, int x, int y, int flags, void*)
//{
//	// TODO add bad args check
//	switch (event)
//	{
//	case CV_EVENT_LBUTTONDOWN: // set rect or GC_BGD(GC_FGD) labels
//	{
//								   bool isb = (flags & BGD_KEY) != 0,
//									   isf = (flags & FGD_KEY) != 0;
//								   if (rectState == NOT_SET && !isb && !isf)
//								   {
//									   rectState = IN_PROCESS;
//									   rect = Rect(x, y, 1, 1);
//								   }
//								   if ((isb || isf) && rectState == SET)
//									   lblsState = IN_PROCESS;
//	}
//		break;
//	case CV_EVENT_RBUTTONDOWN: // set GC_PR_BGD(GC_PR_FGD) labels
//	{
//								   bool isb = (flags & BGD_KEY) != 0,
//									   isf = (flags & FGD_KEY) != 0;
//								   if ((isb || isf) && rectState == SET)
//									   prLblsState = IN_PROCESS;
//	}
//		break;
//	case CV_EVENT_LBUTTONUP:
//		if (rectState == IN_PROCESS)
//		{
//			rect = Rect(Point(rect.x, rect.y), Point(x, y));
//			rectState = SET;
//			setRectInMask();
//			assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
//			showImage();
//		}
//		if (lblsState == IN_PROCESS)
//		{
//			setLblsInMask(flags, Point(x, y), false);
//			lblsState = SET;
//			showImage();
//		}
//		break;
//	case CV_EVENT_RBUTTONUP:
//		if (prLblsState == IN_PROCESS)
//		{
//			setLblsInMask(flags, Point(x, y), true);
//			prLblsState = SET;
//			showImage();
//		}
//		break;
//	case CV_EVENT_MOUSEMOVE:
//		if (rectState == IN_PROCESS)
//		{
//			rect = Rect(Point(rect.x, rect.y), Point(x, y));
//			assert(bgdPxls.empty() && fgdPxls.empty() && prBgdPxls.empty() && prFgdPxls.empty());
//			showImage();
//		}
//		else if (lblsState == IN_PROCESS)
//		{
//			setLblsInMask(flags, Point(x, y), false);
//			showImage();
//		}
//		else if (prLblsState == IN_PROCESS)
//		{
//			setLblsInMask(flags, Point(x, y), true);
//			showImage();
//		}
//		break;
//	}
//}
//
//int GCApplication::nextIter()
//{
//	if (isInitialized)
//		grabCut(*image, mask, rect, bgdModel, fgdModel, 1);
//	else
//	{
//		if (rectState != SET)
//			return iterCount;
//
//		if (lblsState == SET || prLblsState == SET)
//			grabCut(*image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_MASK);
//		else
//			grabCut(*image, mask, rect, bgdModel, fgdModel, 1, GC_INIT_WITH_RECT);
//
//		isInitialized = true;
//	}
//	iterCount++;
//
//	bgdPxls.clear(); fgdPxls.clear();
//	prBgdPxls.clear(); prFgdPxls.clear();
//
//	return iterCount;
//}
//
//GCApplication gcapp;
//
//static void on_mouse(int event, int x, int y, int flags, void* param)
//{
//	gcapp.mouseClick(event, x, y, flags, param);
//}
//
//int main()
//{
//	string filename = "C:\\HuaWeiImage\\华为拍照_校正\\华为拍照_校正\\正常光照带干扰电线\\u_60.jpg";
//	if (filename.empty())
//	{
//		cout << "\nDurn, couldn't read in " << filename << endl;
//		return 1;
//	}
//	Mat image = imread(filename, 1);
//	if (image.empty())
//	{
//		cout << "\n Durn, couldn't read image filename " << filename << endl;
//		return 1;
//	}
//
//	help();
//
//	const string winName = "image";
//	namedWindow(winName, WINDOW_AUTOSIZE);
//	setMouseCallback(winName, on_mouse, 0);
//
//	gcapp.setImageAndWinName(image, winName);
//	gcapp.showImage();
//
//	for (;;)
//	{
//		int c = waitKey(0);
//		switch ((char)c)
//		{
//		case '\x1b':
//			cout << "Exiting ..." << endl;
//			goto exit_main;
//		case 'r':
//			cout << endl;
//			gcapp.reset();
//			gcapp.showImage();
//			break;
//		case 'n':
//			int iterCount = gcapp.getIterCount();
//			cout << "<" << iterCount << "... ";
//			int newIterCount = gcapp.nextIter();
//			if (newIterCount > iterCount)
//			{
//				gcapp.showImage();
//				cout << iterCount << ">" << endl;
//			}
//			else
//				cout << "rect must be determined>" << endl;
//			break;
//		}
//	}
//
//exit_main:
//	destroyWindow(winName);
//	return 0;
//}

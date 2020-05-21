#include "pch.h"
#include "opencv2/opencv.hpp"
#include "opencv2/face.hpp"
//#include "dlib-master/dlib/image_processing/frontal_face_detector.h"
//#include "dlib-master/dlib/opencv/cv_image.h"
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include "Object_Selection.h"
#define _CRTDBG_MAP_ALLOC
#include <cstdlib>
#include <crtdbg.h>

using namespace std;
using namespace cv;

Object_Selection::Object_Selection(Algorithms Select_alg, Algorithms Crop_alg)
{
	switch (Select_alg) {
	case ViolaJones:
		Alg = new Viola_Jones();
		break;
	case DNN:
		Alg = new MyDNN();
		break;
	case Hog:
		Alg = new HoG();
		break;
	case None:
		Alg = new EmptySelect;
		break;
	default:
		Alg = new Viola_Jones();
		break;
	}
	switch (Crop_alg) {
	case Marks:
		Crop = new Marks_Cropping();
		break;
	case Grab:
		Crop = new GrabCrop();
		break;
	case ColorWithMarks:
		Crop = new MarksColor_Cropping();
		break;
	case None:
		Crop = new EmptyCrop;
		break;
	default:
		Crop = new MarksColor_Cropping();
		break;
	}
	this->Crop_alg = Crop_alg;
	this->Select_alg = Select_alg;
}

MyDNN::MyDNN(double min_confidence, bool needs_resizing)
{
	this->min_confidence = min_confidence;
	caffeConfigFile = "./deploy.prototxt";
	caffeWeightFile = "./res10_300x300_ssd_iter_140000_fp16.caffemodel";
	size = Size(300, 300);
	scalar = Scalar(104, 117, 123);
	NeedsResize = needs_resizing;
	net = dnn::readNetFromCaffe(caffeConfigFile, caffeWeightFile);
	/*const std::string tensorflowConfigFile = "./opencv_face_detector.pbtxt";
	const std::string tensorflowWeightFile = "./opencv_face_detector_uint8.pb";
	dnn::Net net = dnn::readNetFromTensorflow(tensorflowWeightFile, tensorflowConfigFile);*/
}

vector<Rect2d> MyDNN::Get_Frames(const Mat& image)
{
	Mat img, blob;
	vector<Rect2d> frames;
	if (NeedsResize)
	{
		resize(image, img, size);
		blob = dnn::blobFromImage(img, 1.0, size, scalar, true, false);
	}
	else
		blob = dnn::blobFromImage(image, 1.0, size, scalar, true, false);
	net.setInput(blob, "");
	Mat detection = net.forward("detection_out");
	Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
	for (int i = 0; i < detectionMat.rows; i++)
	{
		float confidence = detectionMat.at<float>(i, 2);
		if (confidence > min_confidence)
		{
			cout << confidence << endl;
			int x1 = static_cast<int>(detectionMat.at<float>(i, 3) * image.cols);
			int y1 = static_cast<int>(detectionMat.at<float>(i, 4) * image.rows);
			int x2 = static_cast<int>(detectionMat.at<float>(i, 5) * image.cols);
			int y2 = static_cast<int>(detectionMat.at<float>(i, 6) * image.rows);
			if ((x1 <= image.cols) && (x2 <= image.cols) && (y1 <= image.rows) && (y2 <= image.rows))
				frames.push_back(Rect2d(Point(x1, y1), Point(x2, y2)));
		}
	}
	return frames;
}

vector<Rect2d> Viola_Jones::Get_Frames(const Mat& image)
{
	vector<Rect> faces;
	face_cascade.detectMultiScale(image, faces, 1.1, 3, 0, min_face_size);
	vector<Rect2d> new_faces(faces.size());
	for (int i = 0; i < faces.size(); i++)
		new_faces[i] = faces[i];
	return new_faces;
}

vector<Rect2d> HoG::Get_Frames(const Mat& image)
{
	vector<Rect2d> faces;
	/*Mat image1;
	resize(image, image1, Size(500, 350));
	dlib::cv_image<dlib::bgr_pixel> img(image1);
	imwrite("E://2//000.jpg", image);
	vector<dlib::rectangle> facesDlib = hogFaceDetector(img);
	for (int i = 0; i < facesDlib.size(); i++)
		faces.push_back(Rect2d(facesDlib[i].left() * image.cols / 500, facesDlib[i].top() * image.rows / 350, facesDlib[i].width() * image.cols / 500, facesDlib[i].height() * image.rows / 350));
	*/return faces;
}


vector<Rect2d> Selection_Algorithm::Get_Enlarged_Frames(const Mat& image, float ratio_x, float ratio_y)
{
	vector<Rect2d> faces = Get_Frames(image);
	if (ratio_x == 1 && ratio_y == 1)
		return faces;
	int new_y, new_x, new_width, new_height;
	for (int i = 0; i < faces.size(); i++)
	{
		new_y = (faces[i].y - faces[i].height * (ratio_y / 2 - 0.5)) < 0 ? 0 : faces[i].y - faces[i].height * (ratio_y / 2 - 0.5);
		new_x = (faces[i].x - faces[i].width * (ratio_x / 2 - 0.5)) < 0 ? 0 : faces[i].x - faces[i].width * (ratio_x / 2 - 0.5);
		new_height = (new_y + ratio_y * faces[i].height) > image.rows - 1 ? (image.rows - 1 - new_y) : ratio_y * faces[i].height;
		new_width = (new_x + ratio_x * faces[i].width) > image.cols - 1 ? (image.cols - 1 - new_x) : ratio_x * faces[i].width;
		faces[i] = Rect2d(new_x, new_y, new_width, new_height);
	}
	return faces;
}


Mat GrabCrop::Get_Mask(const Mat& image, Rect2d frame)
{
	Mat res, temp = Mat::ones(image.rows, image.cols, CV_8UC1), mask = Mat::zeros(image.rows, image.cols, CV_8U), bgd_model = Mat::zeros(1, 65, CV_64FC1), fgd_model = Mat::zeros(1, 65, CV_64FC1);
	grabCut(image, mask, frame, bgd_model, fgd_model, 5, GC_INIT_WITH_RECT);
	temp.copyTo(res, (mask == 1) | (mask == 3));
	return res;
}

Mat GrabCrop::Get_Mask(const Mat& image, vector<Rect2d> frames)
{
	if (frames.size() == 0)
		return Mat(0, 0, CV_8UC1);
	Mat res(image.rows, image.cols, CV_8UC1);
	Mat temp = Mat::ones(image.rows, image.cols, CV_8UC1), mask = Mat::zeros(image.rows, image.cols, CV_8U), bgd_model = Mat::zeros(1, 65, CV_64FC1), fgd_model = Mat::zeros(1, 65, CV_64FC1);
	vector<Mat> temp_results(frames.size());
	for (int i = 0; i < frames.size(); i++)
		grabCut(image, temp_results[i], frames[i], bgd_model, fgd_model, 5, GC_INIT_WITH_RECT);
	for (int k = 0; k < frames.size(); k++)
		for (int i = 0; i < res.rows; i++)
			for (int j = 0; j < res.cols; j++)
				if (temp_results[k].at<uchar>(i, j) == 1 || temp_results[k].at<uchar>(i, j) == 3)
					res.at<uchar>(i, j) = 1;
	return res;
}

vector<Point> Marks_Cropping::Get_Object_Contour(const Mat& image, Rect2d frame)
{
	vector<vector<Point2f>> marks;
	facemark->fit(image, vector<Rect2d> {frame}, marks);
	if (marks[0].size() < 16)
		return vector<Point>(0);
	vector<Point> contour(marks[0].begin(), marks[0].begin() + 16);
	double x, y, Ox = ((double)contour[0].x + contour[15].x) / 2, Oy = ((double)contour[0].y + contour[15].y) / 2;
	for (int i = 1; i < 15; i++)
	{
		x = 2 * Ox - contour[i].x;
		x = (x > 0 ? x : 0) < image.cols - 1 ? x : image.cols - 1;
		y = 2 * Oy - contour[i].y;
		y = (y > 0 ? y : 0) < image.rows - 1 ? y : image.rows - 1;
		contour.push_back(Point(x, y));
	}
	return contour;
}

vector<vector<Point>> Marks_Cropping::Get_All_Contours(const Mat& image, vector<Rect2d> frames)
{
	vector<vector<Point2f>> marks;
	vector<vector<Point>> res;
	double x, y, Ox, Oy;
	facemark->fit(image, frames, marks);
	for (int k = 0; k < frames.size(); k++)
	{
		if (marks[k].size() > 15)
		{
			vector<Point> contour(marks[k].begin(), marks[k].begin() + 16);
			Ox = ((double)contour[0].x + contour[15].x) / 2;
			Oy = ((double)contour[0].y + contour[15].y) / 2;
			for (int i = 0; i < 15; i++)
			{
				x = 2 * Ox - contour[i].x;
				x = (x > 0 ? x : 0) < image.cols - 1 ? x : image.cols - 1;
				y = 2 * Oy - contour[i].y;
				y = (y > 0 ? y : 0) < image.rows - 1 ? y : image.rows - 1;
				contour.push_back(Point(x, y));
			}
			res.push_back(contour);
		}
	}
	return res;
}


vector<Point> Line(Point a1, Point a2)
{
	if (a1.y == a2.y)
		return vector<Point> {a1};
	vector<Point> res{ a1 };
	double a = ((double)a1.x - a2.x) / (a1.y - a2.y), b = ((double)a2.y * a1.x - a1.y * a2.x) / (a2.y - a1.y);
	int x, y = a1.y, iter = a2.y > a1.y ? 1 : -1;
	for (int i = 0; i < abs(a2.y - a1.y) - 1; i++)
	{
		y += iter;
		x = (int)(y * a + b);
		res.push_back(Point(x, y));
	}
	return res;
}

vector<Point> MarksColor_Cropping::Color_cropping(float delta, const Mat& image, vector<Point> contour)
{
	Point p;
	vector<Point> new_contour(contour), min_contour(contour), line;
	double x, y, Ox = ((double)contour[0].x + contour[15].x) / 2, Oy = ((double)contour[0].y + contour[15].y) / 2;
	for (int i = 1; i < 15; i++)
	{
		x = 2 * Ox - new_contour[i].x;
		x = (x > 0 ? x : 0) < image.cols - 1 ? x : image.cols - 1;
		y = 2 * Oy - new_contour[i].y;
		y = (y > 0 ? y : 0) < image.rows - 1 ? y : image.rows - 1;
		new_contour.push_back(Point(x, y));
		x = 1.5 * Ox - min_contour[i].x / 2;
		x = (x > 0 ? x : 0) < image.cols - 1 ? x : image.cols - 1;
		y = 1.5 * Oy - min_contour[i].y / 2;
		y = (y > 0 ? y : 0) < image.rows - 1 ? y : image.rows - 1;
		min_contour.push_back(Point(x, y));
	}
	double k1, k2, k3;
	Mat mask = Object_Selection::Get_Obj_Mask(image.rows, image.cols, new_contour);
	//считаем средний цвет лица
	double r = 0, g = 0, b = 0, all = 0;
	for (int i = 0; i < image.rows; i++)
		for (int j = 0; j < image.cols; j++)
			if (mask.at<char>(i, j) == 1)
			{
				b += image.at<Vec3b>(i, j)[0];
				g += image.at<Vec3b>(i, j)[1];
				r += image.at<Vec3b>(i, j)[2];
				all++;
			}
	Scalar FaceColor = { b / all, g / all, r / all };
	//расставляем щупы, сдвигая их к центру, пока не дойдут до цвета кожи		
	for (int l = 0; l < contour.size() / 2; l++)
	{
		p = new_contour[contour.size() + l];
		line = Line(p, Point(Ox, Oy));
		int n = 1;
		Scalar Color;
		while (p.x > min_contour[contour.size() + l].x && p.y < min_contour[contour.size() + l].y)
		{
			if (p.y < 2 || p.y > image.rows - 3 || p.x < 2 || p.x > image.cols - 3)
			{
				b = image.at<Vec3b>(p.y, p.x)[0];
				g = image.at<Vec3b>(p.y, p.x)[1];
				r = image.at<Vec3b>(p.y, p.x)[2];
				Color = { b , g , r };
			}
			else
			{
				b = 0; g = 0; r = 0;
				for (int a1 = p.y - 2; a1 <= p.y + 2; a1++)
					for (int a2 = p.x - 2; a2 <= p.x + 2; a2++)
					{
						b += image.at<Vec3b>(a1, a2)[0];
						g += image.at<Vec3b>(a1, a2)[1];
						r += image.at<Vec3b>(a1, a2)[2];
					}
				Color = { b / 25, g / 25, r / 25 };
			}
			k1 = Color.val[0] / FaceColor.val[0];
			k2 = Color.val[1] / FaceColor.val[1];
			k3 = Color.val[2] / FaceColor.val[2];
			if (abs(1 - k1) < delta && abs(1 - k2) < delta && abs(1 - k3) < delta)
				break;
			p = line[n * line.size() / 50];
			n++;
		}
		new_contour[contour.size() + l] = p;
	}
	for (int l = 0; l < contour.size() / 2 - 1; l++)
	{
		p = new_contour[contour.size() * 3 / 2 - 1 + l];
		line = Line(p, Point(Ox, Oy));
		int n = 1;
		Scalar Color;
		while (p.x < min_contour[contour.size() * 3 / 2 - 1 + l].x && p.y < min_contour[contour.size() * 3 / 2 - 1 + l].y)
		{
			if (p.y < 2 || p.y > image.rows - 3 || p.x < 2 || p.x > image.cols - 3)
			{
				b = image.at<Vec3b>(p.y, p.x)[0];
				g = image.at<Vec3b>(p.y, p.x)[1];
				r = image.at<Vec3b>(p.y, p.x)[2];
				Color = { b , g , r };
			}
			else
			{
				b = 0; g = 0; r = 0;
				for (int a1 = p.y - 2; a1 <= p.y + 2; a1++)
					for (int a2 = p.x - 2; a2 <= p.x + 2; a2++)
					{
						b += image.at<Vec3b>(a1, a2)[0];
						g += image.at<Vec3b>(a1, a2)[1];
						r += image.at<Vec3b>(a1, a2)[2];
					}
				Color = { b / 25, g / 25, r / 25 };
			}
			k1 = Color.val[0] / FaceColor.val[0];
			k2 = Color.val[1] / FaceColor.val[1];
			k3 = Color.val[2] / FaceColor.val[2];
			if (abs(1 - k1) < delta && abs(1 - k2) < delta && abs(1 - k3) < delta)
				break;
			p = line[n * line.size() / 50];
			n++;
		}
		new_contour[contour.size() * 3 / 2 - 1 + l] = p;
	}
	return new_contour;
}

vector<Point> MarksColor_Cropping::Get_Object_Contour(const Mat& image, Rect2d frame)
{
	vector<vector<Point2f>> marks;
	facemark->fit(image, vector<Rect2d> {frame}, marks);
	if (marks[0].size() < 16)
		return vector<Point>(0);
	vector<Point> contour(marks[0].begin(), marks[0].begin() + 16);
	return MarksColor_Cropping::Color_cropping(1.5f, image, contour);
}

vector<vector<Point>> MarksColor_Cropping::Get_All_Contours(const Mat& image, vector<Rect2d> frames)
{
	vector<vector<Point2f>> marks;
	vector<vector<Point>> res;
	double x, y, Ox, Oy;
	facemark->fit(image, frames, marks);
	for (int k = 0; k < frames.size(); k++)
	{
		if (marks[k].size() > 15)
		{
			vector<Point> contour(marks[k].begin(), marks[k].begin() + 16);
			res.push_back(MarksColor_Cropping::Color_cropping(0.58f, image, contour));
		}
	}
	return res;
}


Mat Object_Selection::Get_Obj_Mask(int rows, int cols, vector<Point> contour)
{
	int frame_height = rows, frame_width = cols;
	if (contour.size() == 0)
		return Mat(0, 0, CV_8UC1);
	int x1, x2, min_y = frame_height, max_y = 0;
	Mat res(rows, cols, CV_8UC1);
	vector<Point> new_contour, line;
	if (contour.size() > 0)
		contour.push_back(contour[0]);
	for (int i = 0; i < contour.size() - 1; i++)
	{
		if (contour[i].y < min_y)
			min_y = contour[i].y;
		if (contour[i].y > max_y)
			max_y = contour[i].y;
		line = Line(contour[i], contour[i + 1]);
		new_contour.reserve(line.size() + new_contour.size());
		new_contour.insert(new_contour.end(), line.begin(), line.end());
	}
	for (int y = max(0, min_y); y <= min(max_y, frame_height - 1); y++)
	{
		x1 = frame_width; x2 = 0;
		for (int i = 0; i < new_contour.size(); i++)
			if (new_contour[i].y == y)
			{
				if (new_contour[i].x < x1)
					x1 = new_contour[i].x;
				if (new_contour[i].x > x2)
					x2 = new_contour[i].x;
			}
		for (int i = max(0, x1); i <= min(x2, frame_width - 1); i++)
			res.at<uchar>(y, i) = 1;
	}
	return res;
}

Mat Object_Selection::Get_Img_Mask(int rows, int cols, vector<vector<Point>> contours)
{
	int frame_height = rows, frame_width = cols;
	if (contours.size() == 0)
		return Mat(0, 0, CV_8UC1);
	if (contours.size() == 1)
		return Get_Obj_Mask(rows, cols, contours[0]);
	Mat cur_mask, res(rows, cols, CV_8UC1);
	vector<Mat> masks;
	for (int i = 0; i < contours.size(); i++)
	{
		cur_mask = Get_Obj_Mask(rows, cols, contours[i]);
		if (cur_mask.cols > 0)
			masks.push_back(cur_mask);
	}
	for (int i = 0; i < rows; i++)
		for (int j = 0; j < cols; j++)
			for (int k = 0; k < masks.size(); k++)
				if (masks[k].at<uchar>(i, j) == 1)
					res.at<uchar>(i, j) = 1;
	return res;
}

vector<Point> Object_Selection::Get_Contour_from_Mask(Mat mask, Rect2d frame)
{
	vector<Point> res_left, res_right, res;
	int x, y, j1, j2;
	for (int i = max(0, (int)frame.y); i < min((int)(frame.y + frame.height), mask.rows); i++)
	{
		j1 = min(max(0, (int)frame.x), mask.cols);
		while (j1 < mask.cols && mask.at<uchar>(i, j1) != 1 && j1 < frame.x + frame.width)
			j1++;
		if (j1 != frame.x + frame.width)
			res_left.push_back(Point(j1, i));
		j2 = max(0, min((int)(frame.x + frame.width), mask.cols));
		while (j2 >= 0 && mask.at<uchar>(i, j2) != 1 && j2 > frame.x)
			j2--;
		if (j2 != frame.x)
			res_right.push_back(Point(j2, i));
	}
	for (int i = 0; i < res_left.size(); i += 10)
		res.push_back(res_left[i]);
	for (int i = res_right.size() - 1; i >= 0; i -= 10)
		res.push_back(res_right[i]);
	return res;
}

vector<vector<Point>> Object_Selection::Get_Contour_from_Mask(Mat mask, vector<Rect2d> frames)
{
	vector<vector<Point>> res;
	for (int i = 0; i < frames.size(); i++)
		res.push_back(Get_Contour_from_Mask(mask, frames[i]));
	return res;
}

#include "opencv2/opencv.hpp"
#include "opencv2/face.hpp"
//#include "dlib-master/dlib/image_processing/frontal_face_detector.h"
#include <stdio.h>


using namespace std;
using namespace cv;


enum Algorithms { None = 0, ViolaJones = 1, DNN = 2, Hog = 3, ColorWithMarks = 1, Marks = 2, Grab = 3, FaceTrack = 1, ObjectTrack = 2, WithoutTracking = 3 };

struct Selection_Algorithm
{
	virtual vector<Rect2d> Get_Frames(const Mat& image) = 0;
	vector<Rect2d> Get_Enlarged_Frames(const Mat& image, float ratio_x, float ratio_y);
	virtual ~Selection_Algorithm() {}
};

struct Cropping_Algorithm
{
	virtual vector<vector<Point>> Get_All_Contours(const Mat& image, vector<Rect2d> frames) = 0;
	virtual vector<Point> Get_Object_Contour(const Mat& image, Rect2d frame) = 0;
	virtual Mat Get_Mask(const Mat& image, vector<Rect2d> frames) = 0;
	virtual Mat Get_Mask(const Mat& image, Rect2d frame) = 0;
	virtual ~Cropping_Algorithm() {}
};

class Object_Selection {
public:
	Object_Selection(Algorithms Select_alg, Algorithms Crop_alg);
	Algorithms Get_Select_Alg() { return Select_alg; }
	Algorithms Get_Crop_Alg() { return Crop_alg; }
	vector<Rect2d> Get_Object_Frames(const Mat& image) { return Alg->Get_Frames(image); };
	vector<Rect2d> Get_Enlarged_Frames(const Mat& image, float ratio_x = 1.4, float ratio_y = 1.3) { return Alg->Get_Enlarged_Frames(image, ratio_x, ratio_y); };
	vector<Point> Get_Object_Contour(const Mat& image, Rect2d frame) { return Crop->Get_Object_Contour(image, frame); };
	vector<vector<Point>> Get_All_Contours(const Mat& image) { return Crop->Get_All_Contours(image, Get_Object_Frames(image)); };
	vector<vector<Point>> Get_All_Contours(const Mat& image, vector<Rect2d> frames) { return Crop->Get_All_Contours(image, frames); };
	Mat Get_Obj_Mask(const Mat& image, Rect2d frame) { return Crop->Get_Mask(image, frame); };
	Mat Get_Img_Mask(const Mat& image, vector<Rect2d> frames) { return Crop->Get_Mask(image, frames); };
	static Mat Get_Obj_Mask(int rows, int cols, vector<Point> contour);
	static Mat Get_Img_Mask(int rows, int cols, vector<vector<Point>> contours);
	static vector<Point> Get_Contour_from_Mask(Mat mask, Rect2d frame);
	static vector<vector<Point>> Get_Contour_from_Mask(Mat mask, vector<Rect2d> frames);
	~Object_Selection() { delete Alg; delete Crop; }
private:
	Algorithms Select_alg, Crop_alg;
	Selection_Algorithm* Alg;
	Cropping_Algorithm* Crop;
};

struct EmptySelect : Selection_Algorithm
{
public:
	vector<Rect2d> Get_Frames(const Mat& image) { return vector<Rect2d>(0); }
	vector<vector<Rect>> Get_Frames(vector<Mat> images) { return vector<vector<Rect>>(0); }
};

struct Viola_Jones : Selection_Algorithm
{
	CascadeClassifier face_cascade;
	Size min_face_size;
	Viola_Jones(int min_face_width = 70, int min_face_height = 70) : min_face_size(min_face_width, min_face_height) { face_cascade.load("haarcascade_frontalface_alt2.xml"); };
	vector<Rect2d> Get_Frames(const Mat& image);
};

struct MyDNN : Selection_Algorithm
{
public:
	MyDNN(double min_confidence = 0.4, bool needs_resizing = true);
	vector<Rect2d> Get_Frames(const Mat& image);
private:
	string caffeConfigFile;
	string caffeWeightFile;
	double min_confidence;
	dnn::Net net;
	bool NeedsResize;
	Size size;
	Scalar scalar;
};

struct HoG : Selection_Algorithm
{
public:
	HoG() {/* hogFaceDetector = dlib::get_frontal_face_detector(); */ }
	vector<Rect2d> Get_Frames(const Mat& image);
private:
	//dlib::frontal_face_detector hogFaceDetector;
};


struct EmptyCrop : Cropping_Algorithm
{
	vector<vector<Point>> Get_All_Contours(const Mat& image, vector<Rect2d> frames) { vector<vector<Point>> res; for (int i = 0; i < frames.size(); i++) res.push_back(Get_Object_Contour(image, frames[i])); return res; }
	vector<Point> Get_Object_Contour(const Mat& image, Rect2d frame) { return vector<Point>{Point(frame.x, frame.y), Point(frame.x, frame.y + frame.height), Point(frame.x + frame.width, frame.y + frame.height), Point(frame.x + frame.width, frame.y)}; }
	Mat Get_Mask(const Mat& image, vector<Rect2d> frames) { return Mat(); }
	Mat Get_Mask(const Mat& image, Rect2d frame) { return Mat(); }
};

struct GrabCrop : Cropping_Algorithm
{
	vector<Point> Get_Object_Contour(const Mat& image, Rect2d frame) { return Object_Selection::Get_Contour_from_Mask(this->Get_Mask(image, frame), frame); }
	vector<vector<Point>> Get_All_Contours(const Mat& image, vector<Rect2d> frames) { return Object_Selection::Get_Contour_from_Mask(this->Get_Mask(image, frames), frames); }
	Mat Get_Mask(const Mat& image, vector<Rect2d> frames);
	Mat Get_Mask(const Mat& image, Rect2d frame);
};

struct Marks_Cropping : Cropping_Algorithm
{
	Marks_Cropping() { facemark = face::FacemarkLBF::create(); facemark->loadModel("lbfmodel.yaml"); }
	vector<Point> Get_Object_Contour(const Mat& image, Rect2d frame);
	vector<vector<Point>> Get_All_Contours(const Mat& image, vector<Rect2d> frames);
	Mat Get_Mask(const Mat& image, vector<Rect2d> frames) { return Object_Selection::Get_Img_Mask(image.cols, image.rows, Get_All_Contours(image, frames)); }
	Mat Get_Mask(const Mat& image, Rect2d frame) { return Object_Selection::Get_Obj_Mask(image.cols, image.rows, Get_Object_Contour(image, frame)); }
	Ptr<face::Facemark> facemark;
};

struct MarksColor_Cropping : Cropping_Algorithm
{
	MarksColor_Cropping() { facemark = face::FacemarkLBF::create(); facemark->loadModel("lbfmodel.yaml"); }
	vector<Point> Color_cropping(float delta, const Mat& image, vector<Point> contour);
	vector<Point> Get_Object_Contour(const Mat& image, Rect2d frame);
	vector<vector<Point>> Get_All_Contours(const Mat& image, vector<Rect2d> frames);
	Mat Get_Mask(const Mat& image, vector<Rect2d> frames) { return Object_Selection::Get_Img_Mask(image.cols, image.rows, Get_All_Contours(image, frames)); }
	Mat Get_Mask(const Mat& image, Rect2d frame) { return Object_Selection::Get_Obj_Mask(image.cols, image.rows, Get_Object_Contour(image, frame)); }
	Ptr<face::Facemark> facemark;
};
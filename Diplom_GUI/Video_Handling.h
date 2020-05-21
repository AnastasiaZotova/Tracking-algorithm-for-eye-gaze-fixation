#include <stdio.h>
#include <opencv2/opencv.hpp>
#include "Track.h"
#include "Data.h"

using namespace std;
using namespace cv;

struct ClassifiedFixation
{
	int fix_class;
	data_for_use fix;
	ClassifiedFixation(int cl, data_for_use fixations) : fix_class(cl), fix(fixations) {};
};

class VideoHandling
{
public:
	VideoHandling(string VideoFileName, alg gaze_fixation_algorithm, MyTracker* tracker, Object_Selection* obj_select, double IVT_velocity = 90, int IDT_time_lenght = 100, double IDT_dispersion = 0.8);
	VideoHandling(string FileName, alg gaze_fixation_algorithm, double IVT_velocity = 110, int IDT_time_lenght = 100, double IDT_dispersion = 0.8);
	void SaveContoursInFile(string OutFileName = "");
	int DrawFixations(string DataFilename, string OutVideoFile = "res.avi");
	void NumOfGoodFixations(string DataFileName, int& goodfixations, int& allfixations);
private:
	vector<ClassifiedFixation> ÑlassifyFixation(string DataFilename);
	string VideoFileName;
	alg fixation_algorithm;
	double ivt_velocity, idt_time_lenght, idt_dispersion;
	int frame_width, frame_height, fps;
	void MasksInit();
	vector<Mat> Masks;
	vector<vector<vector<Point>>> Contours;
};

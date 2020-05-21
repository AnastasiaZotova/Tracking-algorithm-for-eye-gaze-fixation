#include <stdio.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

enum alg { IVT_alg = 1, IDT_alg = 2 };

struct raw_data
{
	double Timestamp, EyePosX, EyePosY, EyePosZ, GazePointXmm, GazePointYmm, GazePointXpx, GazePointYpx;
};

struct data_for_use
{
	int first_frame, last_frame, x, y;
};
struct fix_data
{
	int first_point, last_point, x, y, start_time, end_time;
};

class Data
{
public:
	Data(string Filename, alg Algorithm, double IVT_velocity, int IDT_time_lenght, double IDT_dispersion);
	int Show(VideoCapture video);
	vector<raw_data> Get_Data() { return GazeData; };
	vector<data_for_use> Get_fixations(double Fps);

private:
	string Filename;
	int ColRecordingTimestamp, ColEyePosLeftX, ColEyePosLeftY, ColEyePosLeftZ, ColEyePosRightX, ColEyePosRightY, ColEyePosRightZ, ColGazePointXmm, ColGazePointYmm;
	int ColGazePointXpx, ColGazePointYpx, ColumnMaxNum;
	double ivt_velocity, idt_dispersion;
	int idt_timelenght;
	alg Algorithm;
	vector<raw_data> GazeData;
	void ColumnsInit(const string& s);
	void Read(const string& s);
	double Velocity(raw_data p1, raw_data p2);
	double Get_Angle(raw_data p1, raw_data p2);
	fix_data Get_Fixation_IVT(int i);
	fix_data Get_Fixation_IDT(int i);

};
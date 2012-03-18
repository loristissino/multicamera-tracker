// MultiCameraTrackerDlg.h : header file
//

#pragma once

#include "cv.h"
#include "cvaux.h"
#include "cxcore.h"
#include "highgui.h"



#define MCT_VERSION "1.30alfa"

#define DEFAULT_MINIMAL_HEIGHT 1.50
#define BLOB_MINIMAL_HEIGHT 20

#define VIDEOWIDTH  360
#define VIDEOHEIGHT 288

#define BORDER 16
#define MAXCAMERAS 2

#define BLOB_LOCATION_RADIUS 7

#define PERSON_CIRCLE_RADIUS 12
#define PERSON_CIRCLE_DIRECTION_OFFSET 4
#define PERSON_CIRCLE_DIRECTION_RADIUS 5

#define M_INCH 0.0254

#define CALIB_CROSSLINE 10
#define CALIB_MAXPOINTS 10


using namespace std;

// CMultiCameraTrackerDlg dialog
class CMultiCameraTrackerDlg : public CDialog
{
// Construction
public:
	CMultiCameraTrackerDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MULTICAMERATRACKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	HWND vc_handle, map_handle;



public:
	CString info;
	CString PauseCaption;
	CString CalibCaption;

	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedPlay();
	afx_msg void OnBnClickedDebug();
	afx_msg void OnBnClickedPause();
	afx_msg void ButtonsUpdate();


public:
	afx_msg void OnBnClickedCalibrate();
public:
};


class CCamera
{
public:
	char id[255];
	char fileSaveName[255];
	char source[255];
	int priority;
	float hm[3][3];
	int frameWidth;
	int frameHeight;
	int frameCount; 
	int fps;
	float pos_h;
	float pos_x;
	float pos_y;
	float focal_length;
	int pathcolor_r;
	int pathcolor_g;
	int pathcolor_b;
	int threshold;
	int fromBottom;
	int erode;
	int dilate;
	float maxBlobHeight;
	int maxBlobHeightPx;
	char pictureBox[255];
	int personCountDown;

	CvCapture *videoStream;
	

	IplImage *frameImage;
	IplImage *backgroundImage;
	IplImage *differenceImage;
	IplImage *grayImage;

	IplImage *associatedMapImage;

	IplImage * mapMask;

//	int getMaxPersonCountDown();

};

class CAreaMap
{
public:
	char fileSaveName[255];
	float real_size_y; 
	float real_size_x;
	int pathcolor_r;
	int pathcolor_g;
	int pathcolor_b;
	IplImage *mapImage;
	IplImage *mapImage2Show;

};

int LoadFirstFrame(CCamera *cam);
void mapCoordinates(int frameX, int frameY, CCamera *cam, float &out_x, float &out_y);

static CCamera *cam[MAXCAMERAS];

static CAreaMap *map;

static FILE* fp_bvh_output = NULL;
static FILE* fp_bvh_header = NULL;
static char bvh_format[10000];

static char bvh_output_filename[255]="";
static char bvh_header_filename[255]="";

static int ma_window;  // moving average window (in frames)
static char ma_method;  // moving average method ('s' or 'w')

static float minimal_height;
static bool ma_follow_to_the_end=true;

static int info_x = 400;
static int info_y = 200;

static bool running=false;
static unsigned int status=0;

#define MCT_STATUS_JUSTSTARTED 1
#define MCT_STATUS_CONFREAD 2
#define MCT_STATUS_DEBUGGED 3
#define MCT_STATUS_PLAYING 4
#define MCT_STATUS_PAUSED 5

static int calibration_step = 0;
static int calibration_clicks = 0;
static FILE *fp_calib = NULL;

static CvPoint2D32f calibration_cvsrc[CALIB_MAXPOINTS];
static CvPoint2D32f calibration_cvdst[CALIB_MAXPOINTS];



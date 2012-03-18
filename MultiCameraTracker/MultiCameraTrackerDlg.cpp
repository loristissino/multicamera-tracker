// MultiCameraTrackerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MultiCameraTracker.h"
#include "MultiCameraTrackerDlg.h"
#include "xml.h"
#include "MO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CMultiCameraTrackerDlg dialog

float m2inches(float m)
{
return m/M_INCH;
}

bool validXY(int x, int y, IplImage *img)
{
if ( (x < 0) || (y < 0) )
	return false;
if ( (x >= img->width) || (y >= img->height) )
	return false;
return true;
}

int maxCountDownValue(CCamera **cam, int size)
{
int i, m=-1;
for (i=0; i<size; i++)
if (cam[i]->personCountDown > m)
		m=cam[i]->personCountDown;
return m;
}

void Matrix_Mult(float a1[3][3], float a2[3], float r[3])
// multiplies a1 (mXn matrix) by a2 (nXp matrix), giving r (mXp matrix)
{
int i, j;	//loop control
for(i = 0; i < 3; i++){
	r[i]=0;
	for(j = 0; j < 3; j++){
       r[i]+=a2[j]*a1[i][j];
		}
	}
};

void mapCoordinates(int frameX, int frameY, CCamera *cam, float &out_x, float &out_y)
{
	float inm[3];
	float outm[3];
	inm[0]=(float)frameX/cam->frameImage->width;
	inm[1]=(float)frameY/cam->frameImage->height;
	inm[2]=1;

	Matrix_Mult(cam->hm, inm, outm);

	out_x=outm[0]/outm[2];
	out_y=outm[1]/outm[2];
}

void mouseHandlerMap(int event, int x, int y, int flags, void *param)
  {
    if (calibration_step!=1)
		return;

	IplImage *mapImage=(IplImage *) param;

	CvFont font;
	double hScale=0.5;
	double vScale=0.5;
	int    lineWidth=2;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

	char number[2];


    switch(event){

      case CV_EVENT_LBUTTONUP:
		  if (calibration_clicks%2 == 0)
				break;
		  if (calibration_clicks /2 >= CALIB_MAXPOINTS)
				break;
		  TRACE("MAP: Left button up: %d, %d\n", x, y);
		  cvLine(mapImage, cvPoint(x-CALIB_CROSSLINE, y), cvPoint(x+CALIB_CROSSLINE, y), CV_RGB(255,0,0));
		  cvLine(mapImage, cvPoint(x, y-CALIB_CROSSLINE), cvPoint(x, y+CALIB_CROSSLINE), CV_RGB(255,0,0));


		  int posx= x+CALIB_CROSSLINE;
		  if (posx +20 > mapImage->width)
			  posx = x - CALIB_CROSSLINE -10;
		  int posy= y+CALIB_CROSSLINE;
		  if (posy +20 > mapImage->height)
			  posy = y - CALIB_CROSSLINE -5;

		  sprintf(number, "%d", calibration_clicks/2);
		  cvPutText (mapImage,number,cvPoint(posx,posy), &font, CV_RGB(255,0,0));


		  cvShowImage("map", mapImage);
		  calibration_cvdst[calibration_clicks/2].x=x/(float)mapImage->width;
		  calibration_cvdst[calibration_clicks/2].y=y/(float)mapImage->height;
		  TRACE("Inserito dst in pos %d\n", calibration_clicks/2);
		  
		  calibration_clicks++;
        break;
    }
  }

void mouseHandlerFrame(int event, int x, int y, int flags, void *param)
	{

	float out_x, out_y;
	int posx, posy;
	CCamera *cam = (CCamera *) param;

	CvFont font;
	double hScale=0.5;
	double vScale=0.5;
	int    lineWidth=2;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);

	char number[2];

	switch(calibration_step)
		{
			case(1):
				switch(event)
					{
						case CV_EVENT_LBUTTONUP:
							if (calibration_clicks % 2 == 1)
								return;
							if (calibration_clicks /2 >= CALIB_MAXPOINTS)
								return;
							TRACE("FRAME: Left button up: %d, %d\n", x, y);
							cvLine(cam->frameImage, cvPoint(x-CALIB_CROSSLINE, y), cvPoint(x+CALIB_CROSSLINE, y), CV_RGB(0, 255,0));
							cvLine(cam->frameImage, cvPoint(x, y-CALIB_CROSSLINE), cvPoint(x, y+CALIB_CROSSLINE), CV_RGB(0,255,0));
							posx= x+CALIB_CROSSLINE;
							if (posx +20 > cam->frameImage->width)
							  posx = x - CALIB_CROSSLINE -10;
							posy= y+CALIB_CROSSLINE;
							if (posy +20 > cam->frameImage->height)
							  posy = y - CALIB_CROSSLINE -5;

							sprintf(number, "%d", calibration_clicks/2);
							cvPutText (cam->frameImage,number,cvPoint(posx,posy), &font, CV_RGB(0,255,0));

							cvShowImage("frame", cam->frameImage);
							calibration_cvsrc[calibration_clicks/2].x=x/(float)cam->frameImage->width;
							calibration_cvsrc[calibration_clicks/2].y=y/(float)cam->frameImage->height;
							TRACE("inserito src in pos %d\n", calibration_clicks/2);
							calibration_clicks++;
							return;
					}
				break;
			case(2):
				// il test
				switch(event)
					{
						case CV_EVENT_LBUTTONUP:

							mapCoordinates(x, y, cam, out_x, out_y);

							cvCircle(cam->frameImage, cvPoint(x, y), 2, CV_RGB(255,255,0), -1);
							cvCircle(cam->associatedMapImage, cvPoint(cvRound(out_x*cam->associatedMapImage->width), cvRound(out_y*cam->associatedMapImage->height)), 2, CV_RGB(0,0,255), -1);
							cvShowImage("map", cam->associatedMapImage);
							cvShowImage("frame", cam->frameImage);
							return;
					}
				break;
				
		
			default:
				TRACE("DEFAULT: step: %d\n", calibration_step);
				return;
		}

}


void mouseHandlerVideoFrame(int event, int x, int y, int flags, void *param)
  {
	CCamera *cam =(CCamera *) param;
	  switch(event){
      case CV_EVENT_LBUTTONUP:
				if (status!=MCT_STATUS_PAUSED)
					return;
				if (cam){
						string szFilters= "PNG image (*.png)|*.png|JPEG image (*.jpg)|*.jpg|";
						string filename;
	
						CFileDialog dlgS(FALSE, "png", cam->fileSaveName, OFN_CREATEPROMPT | OFN_HIDEREADONLY | OFN_ENABLESIZING, szFilters.c_str(), NULL);

						if (dlgS.DoModal()==IDOK)
							{
							filename = (dlgS.GetFileName()).GetBuffer();
							cvSaveImage(filename.c_str(), cam->frameImage);
							}

				}
			break;
    }
	return;
  }

void mouseHandlerVideoMap(int event, int x, int y, int flags, void *param)
  {
	CAreaMap *map =(CAreaMap *) param;
	  switch(event){
      case CV_EVENT_LBUTTONUP:
				if ((status == MCT_STATUS_PLAYING) || (status == MCT_STATUS_JUSTSTARTED))
					{
					TRACE("cliccato sulla mappa... status=%d\n", status);
					return;
					}
				if (map && ((status==MCT_STATUS_PAUSED) || (status == MCT_STATUS_DEBUGGED))){
					TRACE("Mouse event sulla mappa, %s\n", map->fileSaveName);
					TRACE("Lo status è %d\n", status);
					string szFilters= "PNG image (*.png)|*.png|JPEG image (*.jpg)|*.jpg|";
					string filename;

					CFileDialog dlgS(FALSE, "png", map->fileSaveName, OFN_CREATEPROMPT | OFN_HIDEREADONLY | OFN_ENABLESIZING, szFilters.c_str(), NULL);

					if (dlgS.DoModal()==IDOK)
						{
						filename = (dlgS.GetFileName()).GetBuffer();
						if (status==MCT_STATUS_PAUSED)
							cvSaveImage(filename.c_str(), map->mapImage2Show);
						if (status==MCT_STATUS_DEBUGGED)
							cvSaveImage(filename.c_str(), map->mapImage);
						}
				}
			break;
    }
	return;
  }


CMultiCameraTrackerDlg::CMultiCameraTrackerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CMultiCameraTrackerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMultiCameraTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INFO, info);
	DDX_Text(pDX, IDC_PAUSE, PauseCaption);
	DDX_Text(pDX, IDC_CALIBRATE, CalibCaption);
}

BEGIN_MESSAGE_MAP(CMultiCameraTrackerDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDCANCEL, &CMultiCameraTrackerDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_OPEN, &CMultiCameraTrackerDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_PLAY, &CMultiCameraTrackerDlg::OnBnClickedPlay)
	ON_BN_CLICKED(IDC_DEBUG, &CMultiCameraTrackerDlg::OnBnClickedDebug)
	ON_BN_CLICKED(IDC_PAUSE, &CMultiCameraTrackerDlg::OnBnClickedPause)
	ON_BN_CLICKED(IDC_CALIBRATE, &CMultiCameraTrackerDlg::OnBnClickedCalibrate)
END_MESSAGE_MAP()


// CMultiCameraTrackerDlg message handlers

BOOL CMultiCameraTrackerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	int cameraNo;
	char wn[100];
	for (cameraNo=0; cameraNo<MAXCAMERAS; cameraNo++)
		{
			sprintf(wn, "videocamera%d", cameraNo);
			cvNamedWindow(wn, 0); //Create window
			cvResizeWindow(wn, VIDEOWIDTH, VIDEOHEIGHT );

			// Set image resolution and position
			vc_handle = ::GetParent((HWND)cvGetWindowHandle(wn));
			long styles = ::GetWindowLong(vc_handle, GWL_STYLE);
			styles |= WS_CHILD;
//			styles &= ~ (WS_BORDER|WS_CAPTION|WS_POPUP|WS_DLGFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU|WS_THICKFRAME);
			styles &= ~ (WS_BORDER|WS_CAPTION|WS_POPUP|WS_DLGFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU|WS_THICKFRAME);
			::SetWindowLong(vc_handle, GWL_STYLE, styles);
			::SetParent(vc_handle, GetSafeHwnd());
			::SetWindowPos(vc_handle, HWND_TOP, 10+((VIDEOWIDTH+BORDER)*cameraNo), 10, VIDEOWIDTH+BORDER, VIDEOHEIGHT+BORDER, 0);
			//

		}


	cvNamedWindow("mapwindow", 0); //Create window

	// Set image resolution and position
	// map window
	map_handle = ::GetParent((HWND)cvGetWindowHandle("mapwindow"));
	long styles = ::GetWindowLong(map_handle, GWL_STYLE);
	styles |= WS_CHILD;
	styles &= ~ (WS_BORDER|WS_CAPTION|WS_POPUP|WS_DLGFRAME|WS_MINIMIZEBOX|WS_MAXIMIZEBOX|WS_SYSMENU|WS_THICKFRAME);
	::SetWindowLong(map_handle, GWL_STYLE, styles);
	::SetParent(map_handle, GetSafeHwnd());
	::SetWindowPos(map_handle, HWND_TOP, 10, VIDEOHEIGHT+BORDER+20, 584+BORDER, 360+BORDER, 0);




	


	for (cameraNo=0; cameraNo<MAXCAMERAS; cameraNo++){
		cam[cameraNo] = new CCamera;  
		sprintf(cam[cameraNo]->pictureBox, "videocamera%d", cameraNo);
		cam[cameraNo]->threshold=35;
		cam[cameraNo]->mapMask = NULL;
		cam[cameraNo]->frameImage = NULL;
		cam[cameraNo]->backgroundImage = NULL;
		cam[cameraNo]->differenceImage = NULL;
		cam[cameraNo]->grayImage = NULL;
		cam[cameraNo]->videoStream = NULL;
		cam[cameraNo]->personCountDown = 0;
		cvSetMouseCallback(cam[cameraNo]->pictureBox,mouseHandlerVideoFrame,cam[cameraNo]);
	}

	map = new CAreaMap;

	map->mapImage = NULL;
	map->mapImage2Show = NULL;

	running=false;
	char info_text[1000];
	sprintf(info_text,"Multicamera Tracker\nVersion %s\n\nUsing OpenCV %s",
		MCT_VERSION,
		CV_VERSION);
	UpdateData(true);
	info=(CString)info_text;

	PauseCaption=running?"|| (playing)":"|| (paused)";
	CalibCaption="Calibrate...";
	UpdateData(false);

	status=MCT_STATUS_JUSTSTARTED;
	ButtonsUpdate();


	cvSetMouseCallback("mapwindow",mouseHandlerVideoMap,map);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMultiCameraTrackerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMultiCameraTrackerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMultiCameraTrackerDlg::OnBnClickedCancel()
{
	// TODO: aggiungere qui il codice per la gestione della notifica del controllo.
	TRACE("Cliccato Cancel\n");

		TRACE("Inizio la deallocazione...\n");
	int cameraNo;
	for (cameraNo=0; cameraNo<MAXCAMERAS; cameraNo++)
	{
		if (cam[cameraNo]->mapMask)
			cvReleaseImage(&(cam[cameraNo]->mapMask));
		delete cam[cameraNo];
	}
	delete map;
	TRACE("Deallocazione completata...\n");

	OnCancel();
}

void CMultiCameraTrackerDlg::OnBnClickedOpen()
{

	char message[1000];


	string filename = "";
	XML *x = 0;

	int cameraNo;

	char szFilters[]= "XML configuration file (*.xml)|*.xml|All Files (*.*)|*.*|";
	CFileDialog dlg(TRUE, "XML", "*.xml", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_ENABLESIZING, szFilters, this);
	if (dlg.DoModal()==IDOK) {
		filename = (dlg.GetFileName()).GetBuffer();
		FILE* fp = fopen(filename.c_str(), "r");
		if (fp)
			{
			// Load from file
			fclose(fp);
			x = new XML(filename.c_str());
			}
		else
			{
			MessageBox("Sorry, I could not read the file", "MCT", MB_OK | MB_ICONSTOP);
			return;
			}

		int iPS = x->ParseStatus(); // 0 OK , 1 Header warning (not fatal) , 2 Error in parse (fatal)
		bool iTT = x->IntegrityTest(); // TRUE OK
		if (iPS != 0 || iTT == false)
			{
//			TRACE("Error: XML file %s is corrupt (or not a XML file).\r\n\r\n",filename);
			sprintf(message, "Error: XML file %s is corrupt (or not a XML file).\n\n",filename);
			
//			UpdateData(true);
//			info=(CString)message;;
//			UpdateData(false);
			MessageBox(message, "MCT", MB_OK | MB_ICONSTOP);
			fclose(fp);
			delete x;
			return;
			}

		TRACE("XML header: %s\r\n",x->GetHeader()->operator char *());
		XMLElement* r = x->GetRootElement();
		int nC = r->GetChildrenNum();
		TRACE("Root element has %u children.\r\n",nC);
		
		cameraNo=0;
		for(int i = 0 ; i < nC ; i++)
		{
			XMLElement* ch = r->GetChildren()[i];
			int nV = ch->GetVariableNum();
			int nMaxElName = ch->GetElementName(0);
			char* n = new char[nMaxElName + 1];
			ch->GetElementName(n);

			TRACE("\t Child %u: Variables: %u, Name: %s\r\n",i,nV,n);

			if (!strcmp("camera",n)){
				if (cameraNo<MAXCAMERAS)
				{

					char value[255];

					ch->FindVariableZ("id", true)->GetValue(value);

					strcpy((cam[cameraNo]->id),value);
					TRACE("  -> id=%s\n", cam[cameraNo]->id);
					ch->FindVariableZ("source", true)->GetValue(value);
					TRACE("source %s\r\n", value);

					strcpy((cam[cameraNo]->source),value);
					TRACE("  -> source=%s\n", cam[cameraNo]->source);

					ch->FindVariableZ("priority", true)->GetValue(value);
					sscanf_s(value, "%d", &(cam[cameraNo]->priority));

					ch->FindVariableZ("hm", true)->GetValue(value);
					TRACE("hm %s\r\n", value);
					sscanf_s(value, "%f%f%f%f%f%f%f%f%f", 
					
						&(cam[cameraNo]->hm[0][0]), 
						&(cam[cameraNo]->hm[0][1]), 
						&(cam[cameraNo]->hm[0][2]), 
						&(cam[cameraNo]->hm[1][0]), 
						&(cam[cameraNo]->hm[1][1]), 
						&(cam[cameraNo]->hm[1][2]), 
						&(cam[cameraNo]->hm[2][0]), 
						&(cam[cameraNo]->hm[2][1]), 
						&(cam[cameraNo]->hm[2][2]));

					ch->FindVariableZ("pathcolor", true)->GetValue(value);
					TRACE("pathcolor %s\r\n", value);
					sscanf_s(value, "%d%d%d", 
						&cam[cameraNo]->pathcolor_r, 
						&cam[cameraNo]->pathcolor_g, 
						&cam[cameraNo]->pathcolor_b);

					ch->FindVariableZ("threshold", true)->GetValue(value);
					TRACE("threshold %s\r\n", value);
					sscanf_s(value, "%d", &(cam[cameraNo]->threshold));
					TRACE("   ->threshold %d\r\n", cam[cameraNo]->threshold);

					ch->FindVariableZ("frombottom", true)->GetValue(value);
					TRACE("frombottom %s\r\n", value);
					sscanf_s(value, "%d", &(cam[cameraNo]->fromBottom));

					ch->FindVariableZ("dilate", true)->GetValue(value);
					sscanf_s(value, "%d%", &(cam[cameraNo]->dilate));

					ch->FindVariableZ("erode", true)->GetValue(value);
					sscanf_s(value, "%d", &(cam[cameraNo]->erode));

		//			ch->FindVariableZ("maxblobheight", true)->GetValue(value);
			//		sscanf_s(value, "%f", &(cam[cameraNo]->maxBlobHeight));

					ch->FindVariableZ("pos_h", true)->GetValue(value);
					sscanf_s(value, "%f", &(cam[cameraNo]->pos_h));

					ch->FindVariableZ("pos_x", true)->GetValue(value);
					sscanf_s(value, "%f", &(cam[cameraNo]->pos_x));

					ch->FindVariableZ("pos_y", true)->GetValue(value);
					sscanf_s(value, "%f", &(cam[cameraNo]->pos_y));

					ch->FindVariableZ("focal_length", true)->GetValue(value);
					sscanf_s(value, "%f", &(cam[cameraNo]->focal_length));

					ch->FindVariableZ("mask", true)->GetValue(value);

					cam[cameraNo]->mapMask = cvLoadImage(value,CV_LOAD_IMAGE_GRAYSCALE);
					TRACE("Aperta mappa maschera %s\n", value);


					if (LoadFirstFrame(cam[cameraNo])!=0){

						sprintf(message, "Error: video file %s is corrupt.\n",cam[cameraNo]->source);
			
//						UpdateData(true);
//						info=(CString)message;;
//						UpdateData(false);

						MessageBox(message, "MCT", MB_OK | MB_ICONSTOP);
						return;
					}


					cameraNo++;

				}
		}

	 	 if (!strcmp("map",n)){
			char value[255];
			ch->FindVariableZ("source", true)->GetValue(value);
			TRACE("source %s\r\n", value);

			map->mapImage = cvLoadImage(value,CV_LOAD_IMAGE_COLOR);
			cvShowImage("mapwindow", map->mapImage);

			ch->FindVariableZ("pathcolor", true)->GetValue(value);
			TRACE("pathcolor %s\r\n", value);
			sscanf_s(value, "%d%d%d", 
				&map->pathcolor_r, 
				&map->pathcolor_g, 
				&map->pathcolor_b);



			ch->FindVariableZ("real_size_x", true)->GetValue(value);
			sscanf_s(value, "%f%", &(map->real_size_x));
			ch->FindVariableZ("real_size_y", true)->GetValue(value);
			sscanf_s(value, "%f%", &(map->real_size_y));
//			TRACE("Mappa, dimensioni reali: %f. %f\n", map->real_size_x, map->real_size_y);

			ch->FindVariableZ("info_x", true)->GetValue(value);
			sscanf(value, "%d", &info_x);

			ch->FindVariableZ("info_y", true)->GetValue(value);
			sscanf(value, "%d", &info_y);

		}	

 	 	 if (!strcmp("bvh",n)){
			char value[255];
			ch->FindVariableZ("outputfile", true)->GetValue(value);
			strncpy(bvh_output_filename, value, 255);

			FILE* fpbvh = fopen(bvh_output_filename, "w");
			if (fpbvh)
				{
					fclose(fpbvh);
				}
			else
				strncpy(bvh_output_filename, "", 255);

			ch->FindVariableZ("headerfile", true)->GetValue(value);
			strncpy(bvh_header_filename, value, 255);
			ch->FindVariableZ("columns", true)->GetValue(value);

			int i, columns;
			sscanf(value, "%d", &columns);

			sprintf(bvh_format, " %4.2f\t %4.2f\t %4.2f\t 0.00\t 0.00\t %4.2f");
			for (i=6; i< columns; i++)
				sprintf(bvh_format, "%s%s", bvh_format, "\t 0.00");
			sprintf(bvh_format, "%s%s", bvh_format, "\n");

		}

 	 	 if (!strcmp("config",n)){
			char value[255];
			ch->FindVariableZ("ma_window", true)->GetValue(value);
			TRACE("ma_window %d\r\n", value);
			sscanf(value, "%d", &ma_window);
			if (ma_window<2)
				ma_window=2;

			ch->FindVariableZ("ma_method", true)->GetValue(value);
			sscanf(value, "%c", &ma_method);

			ch->FindVariableZ("ma_follow_to_the_end", true)->GetValue(value);
			
			char c;	sscanf(value, "%c", &c);
			ma_follow_to_the_end=(c!='n')&&(c!='N');

			ch->FindVariableZ("minimal_height", true)->GetValue(value);
			sscanf(value, "%f", &minimal_height);
			if (minimal_height<0.10)
				minimal_height=DEFAULT_MINIMAL_HEIGHT;


		 
		 }	


	 delete[] n;
	 }

delete x;

char message[1000];

sprintf(message, "Configuration file read\n\nMoving average\n  -window: %d\n  -method: %s\n\nMinimal height: %3.2fm", ma_window, ma_method=='w'?"weighted":"simple", minimal_height);
UpdateData(true);
info=(CString)message;;
UpdateData(false);

	}
	
	else
		
	{
		
		TRACE("Cancel pressed\n");
		return;
		}
	
TRACE("Valori dell'array Camera\n");
	for (cameraNo=0; cameraNo<MAXCAMERAS; cameraNo++)
	{
		TRACE("Cam n. %d\n", cameraNo);
		TRACE("  threshold=%d\n", cam[cameraNo]->threshold);
		TRACE("  rtm_0_0=%.4f\n", cam[cameraNo]->hm[2][2]);
	}
	TRACE("Max height: %f\n", minimal_height);
	TRACE("Moving average window: %d\n", ma_window);

	status=MCT_STATUS_CONFREAD;
	ButtonsUpdate();
/*	GetDlgItem(IDC_PLAY)->EnableWindow(TRUE);
	GetDlgItem(IDC_DEBUG)->EnableWindow(TRUE);
	GetDlgItem(IDC_CALIBRATE)->EnableWindow(FALSE);
*/
}

int LoadFirstFrame(CCamera *cam)
{


	TRACE("I should open file %s\n", cam->source);
	//video stream
	//CvCapture * videoStream = NULL;

	//images
	//IplImage * frameImage = NULL;
		
	if (!(cam->videoStream = cvCaptureFromAVI(cam->source))){
		TRACE("Error while attempting to open the file");
		fprintf(stderr, "Error while attempting to open the file %s\r\n", cam->source);
		return 1;
	}

	//video properties
	cam->frameWidth = (int)cvGetCaptureProperty(cam->videoStream, CV_CAP_PROP_FRAME_WIDTH);
	cam->frameHeight = (int)cvGetCaptureProperty(cam->videoStream, CV_CAP_PROP_FRAME_HEIGHT);
	cam->frameCount = (int)cvGetCaptureProperty(cam->videoStream, CV_CAP_PROP_FRAME_COUNT);
	cam->fps = (int)cvGetCaptureProperty(cam->videoStream, CV_CAP_PROP_FPS);
//	cam->maxBlobHeightPx=(int)(cam->maxBlobHeight*cam->frameHeight+.5);
//	TRACE("max alt blob %d\n", cam->maxBlobHeightPx);
	cam->frameImage = cvCreateImage(cvSize(cam->frameWidth, cam->frameHeight), IPL_DEPTH_8U, 3);
	cam->frameImage->origin = 1;

	//get the first frame
	cvGrabFrame(cam->videoStream);
	cam->frameImage = cvQueryFrame(cam->videoStream);
		
	if( !cam->frameImage ){
		fprintf(stderr, "Error while decoding frame\r\n");
		return 2;
		}

	cvShowImage(cam->pictureBox, cam->frameImage); 


	return 0;

}

void CMultiCameraTrackerDlg::ButtonsUpdate()
{
	TRACE("Gestione pulsanti, status vale %d...\n", status);
	UpdateData(true);

	switch (status)
	{
		case(MCT_STATUS_PLAYING):
			TRACE("**playing\n");
			GetDlgItem(IDC_OPEN)->EnableWindow(FALSE);
			GetDlgItem(IDC_CALIBRATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_DEBUG)->EnableWindow(FALSE);
			GetDlgItem(IDC_PAUSE)->EnableWindow(TRUE);
			GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
			PauseCaption="|| (playing)";
			break;
		case(MCT_STATUS_PAUSED):
			TRACE("**paused\n");
			GetDlgItem(IDC_OPEN)->EnableWindow(FALSE);
			GetDlgItem(IDC_CALIBRATE)->EnableWindow(FALSE);
			GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_DEBUG)->EnableWindow(FALSE);
			GetDlgItem(IDC_PAUSE)->EnableWindow(TRUE);
			GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
			PauseCaption="|| (paused)";
			break;
		case(MCT_STATUS_JUSTSTARTED):
			TRACE("**just started\n");
			GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
			GetDlgItem(IDC_CALIBRATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
			GetDlgItem(IDC_DEBUG)->EnableWindow(FALSE);
			GetDlgItem(IDC_PAUSE)->EnableWindow(FALSE);
			GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
			PauseCaption="|| (playing)";
			break;
		case(MCT_STATUS_CONFREAD):
			TRACE("**just confread\n");
			GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
			GetDlgItem(IDC_CALIBRATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_PLAY)->EnableWindow(TRUE);
			GetDlgItem(IDC_DEBUG)->EnableWindow(TRUE);
			GetDlgItem(IDC_PAUSE)->EnableWindow(FALSE);
			GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
			PauseCaption="|| (playing)";
			break;
		case(MCT_STATUS_DEBUGGED):
			TRACE("**just debugged\n");
			GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
			GetDlgItem(IDC_CALIBRATE)->EnableWindow(TRUE);
			GetDlgItem(IDC_PLAY)->EnableWindow(TRUE);
			GetDlgItem(IDC_DEBUG)->EnableWindow(TRUE);
			GetDlgItem(IDC_PAUSE)->EnableWindow(FALSE);
			GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
			PauseCaption="|| (playing)";
			break;

	}

	UpdateData(false);

}

void CMultiCameraTrackerDlg::OnBnClickedPlay()
{
	char key=' ';

	//running=true;
	status = MCT_STATUS_PLAYING;
	ButtonsUpdate();
/*	GetDlgItem(IDC_OPEN)->EnableWindow(FALSE);
	GetDlgItem(IDC_PLAY)->EnableWindow(FALSE);
	GetDlgItem(IDC_DEBUG)->EnableWindow(FALSE);
	GetDlgItem(IDC_PAUSE)->EnableWindow(TRUE);
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
*/

	MovingObject *person = new MovingObject(ma_window);
	person->setMethod(ma_method);

	CvFont font;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX_SMALL, 0.8, 1.0, 0, 2);

	unsigned char grayLevel;

	char info_text[10000];

	char map_text[1000];


	TRACE("Prima della copia dell'intestazione...\n");
	if (strcmp("",bvh_output_filename) && strcmp("",bvh_header_filename))
		{
			fp_bvh_output = fopen(bvh_output_filename, "w");
			if (fp_bvh_output)
				{
					fp_bvh_header = fopen(bvh_header_filename, "r");
					char c;
					while (fread(&c, sizeof(c), 1, fp_bvh_header))
						{
						fwrite(&c, sizeof(c), 1, fp_bvh_output);
						}
					fprintf(fp_bvh_output, "Frames:    %d\n", cam[0]->frameCount);
					fprintf(fp_bvh_output, "Frame Time: %f\n", (1/(float)cam[0]->fps));
				}
		}
	TRACE("Dopo la copia dell'intestazione...\n");

	
	int  y;
	int cameraNo;
	int cameraHl=0;
	int videoLength=cam[0]->frameCount;
	int frameNo=0;

	int mapX, mapY;

	double personCurrentRealX, personCurrentRealY;
	int personCurrentMapX, personCurrentMapY;
	int maxPersonCountDown;

	double threeX, threeY, threeZ, threeR; //used for 3DStudioMax output

	int priority;

	bool visible;
	CvScalar s;

	float out_x, out_y;
	char nf[20];


	//video streams



	//Average positions
	//int avgX, avgY;
	//Bounding box
	CvRect boundingRect;
	CvPoint pt1, pt2;

	int blobNo;
	int avgX;


	for (cameraNo=0; cameraNo<MAXCAMERAS; cameraNo++)  // initialization
		{
		cam[cameraNo]->videoStream = cvCaptureFromAVI(cam[cameraNo]->source);
		cam[cameraNo]->frameImage = cvCreateImage(cvSize(cam[cameraNo]->frameWidth, cam[cameraNo]->frameHeight), IPL_DEPTH_8U, 3);
		cam[cameraNo]->frameImage->origin = 1;
		//get the first frame
		cvGrabFrame(cam[cameraNo]->videoStream);
		cam[cameraNo]->frameImage = cvQueryFrame(cam[cameraNo]->videoStream);
		cvShowImage(cam[cameraNo]->pictureBox, cam[cameraNo]->frameImage); 
		if( !(cam[cameraNo]->frameImage) )
				exit(-1);

		//first frame as the background
		cam[cameraNo]->backgroundImage = cvCloneImage(cam[cameraNo]->frameImage);

		//and initialize other structures
		cam[cameraNo]->differenceImage = cvCloneImage(cam[cameraNo]->frameImage);

		cam[cameraNo]->grayImage = cvCreateImage( cvGetSize(cam[cameraNo]->frameImage), IPL_DEPTH_8U, 1);
		cam[cameraNo]->grayImage->origin = 1;
		cam[cameraNo]->personCountDown = 0;

		// we check whether one videoStream is shorter than the other(s)
		if (cam[cameraNo]->frameCount<videoLength)
			videoLength=cam[cameraNo]->frameCount;
		
		}	// for (initialization)	

	person->setFrameRate(cam[0]->fps);
	TRACE("Prima del ciclone...\n");

	while( frameNo < videoLength )  // sequence
		{
		TRACE("Frame %d\n", frameNo);
		
		if (map->mapImage2Show)
			cvReleaseImage(&map->mapImage2Show);
			// we kept the image in order to be able to save it interactively...

		map->mapImage2Show = cvCloneImage(map->mapImage);
		sprintf(info_text, "Frame #%d\nTime: %4.2f\n\n", frameNo, (float)frameNo/cam[0]->fps);
		for(cameraNo=0,priority=0;cameraNo<MAXCAMERAS;cameraNo++)  // multicamera
			{
			TRACE("  Camera %d\n", cameraNo);

			sprintf(cam[cameraNo]->fileSaveName, "frame%d_cam%d.png", frameNo, cameraNo);
			sprintf(map->fileSaveName, "frame%d_map.png", frameNo);
			cam[cameraNo]->frameImage = cvQueryFrame(cam[cameraNo]->videoStream);
			if( !cam[cameraNo]->frameImage)
				  break;
			//Background subtraction
			cvAbsDiff(cam[cameraNo]->frameImage,cam[cameraNo]->backgroundImage,cam[cameraNo]->differenceImage);
			//Convert the image to grayscale
			cvCvtColor(cam[cameraNo]->differenceImage,cam[cameraNo]->grayImage,CV_RGB2GRAY);

			//Thresold the image
			cvThreshold(cam[cameraNo]->grayImage, cam[cameraNo]->grayImage, cam[cameraNo]->threshold, 255, CV_THRESH_BINARY);
			//Dilate and erode
			cvDilate(cam[cameraNo]->grayImage, cam[cameraNo]->grayImage, 0, cam[cameraNo]->dilate);
			cvErode(cam[cameraNo]->grayImage, cam[cameraNo]->grayImage, 0, cam[cameraNo]->erode);

			CvMemStorage* storage = cvCreateMemStorage(0);
			CvSeq* contour = 0;
			cvFindContours( cam[cameraNo]->grayImage, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
 
			//Process each moving blob
			for(blobNo=0 ; contour != 0; contour = contour->h_next, blobNo++)  // blob
				{
				TRACE("     blobNo: %d\n", blobNo);

				//Get the bounding box
				boundingRect = cvBoundingRect(contour, 0);
				if (boundingRect.height<BLOB_MINIMAL_HEIGHT)
					continue;


				//Get the coordinates
				pt1.x = boundingRect.x;
				pt1.y = boundingRect.y;
				pt2.x = boundingRect.x + boundingRect.width;
				pt2.y = boundingRect.y + boundingRect.height;
			   
				//Center of the blob -> useful when drawing a circle in the middle of the blob
				avgX = (pt1.x + pt2.x) / 2;
				//bottomY = pt1.y;

				//Draw the bounding rectangle around the moving object.
				/*
				cvRectangle(frameImage[cameraNo], pt1, pt2, CV_RGB(
					cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b), 1);
				*/

				y=cam[cameraNo]->frameHeight-pt1.y-boundingRect.height*cam[cameraNo]->fromBottom/100;

				mapCoordinates(avgX,y,cam[cameraNo], out_x, out_y);

				mapX=cvRound(out_x*map->mapImage->width);
				mapY=cvRound(out_y*map->mapImage->height);
				TRACE("           mappato: %d,%d\n", mapX, mapY);

				visible=false;

				if (validXY(mapX, mapY, cam[cameraNo]->mapMask)){
				  s=cvGet2D(cam[cameraNo]->mapMask, mapY, mapX);
				  if (s.val[0]!=0)
						visible=true;
				  cam[cameraNo]->priority=(int)s.val[0];
				}
				else
				  cam[cameraNo]->priority=0;
				
				TRACE("                Visible: %s\n", visible?"true":"false");


				// let's check if we need to track it 
				if (cam[cameraNo]->priority)  // visible blob?
					{

					float distance;
					float estimated_height;

					distance=sqrt(pow(out_x*map->real_size_x - cam[cameraNo]->pos_x, 2) + pow(out_y*map->real_size_y-cam[cameraNo]->pos_y,2) + pow(cam[cameraNo]->pos_h, 2));
					estimated_height = boundingRect.height*distance/cam[cameraNo]->focal_length/cam[cameraNo]->frameHeight;


					if (estimated_height>minimal_height)

						{							
							cam[cameraNo]->personCountDown=ma_window+1;

							cvRectangle(cam[cameraNo]->frameImage, pt1, pt2, CV_RGB(
											  cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b), 2);

							cvCircle(cam[cameraNo]->frameImage, cvPoint(avgX, cam[cameraNo]->frameHeight-y), 3, CV_RGB(
											  cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b), 1);


							// disegno sulla mappa
							cvCircle(map->mapImage, cvPoint(mapX,mapY), 1, 
									 CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b), 2);
							cvCircle(map->mapImage2Show, cvPoint(mapX,mapY), BLOB_LOCATION_RADIUS, 
									 CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b), -1);
							//cvLine(map->mapImage2Show, cvPoint(mapX,mapY), cvPoint(cvRound(cam[cameraNo]->pos_x/map->real_size_x*map->mapImage->width), cvRound(cam[cameraNo]->pos_y/map->real_size_y*map->mapImage->height)), CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b), 2);

							sprintf(info_text, "%sCamera %d, blob %d:\n  distance %3.2fm\n  size %3.2fm\n  priority %d\n----\n", info_text, cameraNo, blobNo, distance, estimated_height, cam[cameraNo]->priority);

/*							if (fp){
								fprintf(fp, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f,%f\n", frameNo, cameraNo, blobNo, boundingRect.x, boundingRect.y, boundingRect.width, boundingRect.height, avgX, y, out_x*map->real_size_x, out_y*map->real_size_y, distance, estimated_height);
								}
*/
							if (cam[cameraNo]->priority>priority)  // priority
								{
									priority=cam[cameraNo]->priority;
									personCurrentRealX=out_x*map->real_size_x;
									personCurrentRealY=out_y*map->real_size_y;
									personCurrentMapX=cvRound(out_x*map->mapImage->width);
									personCurrentMapY=cvRound(out_y*map->mapImage->height);
									cameraHl=cameraNo;

								} // end if (priority)

						} // end if (estimated_height)
					else {
						cvRectangle(cam[cameraNo]->frameImage, pt1, pt2, CV_RGB(
							cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b), 1);
					}


					}  // fine if (visible)

				}  //end for (blob)
			
			cam[cameraNo]->personCountDown--; 

			// release memory storage
			cvReleaseMemStorage(&storage);
		   
			cvShowImage(cam[cameraNo]->pictureBox, cam[cameraNo]->frameImage);  



			}  // end for multicamera


		maxPersonCountDown = maxCountDownValue(cam, MAXCAMERAS);
			
		if (maxPersonCountDown>0)
			{
			grayLevel=cvRound((ma_window - maxPersonCountDown)*255/ma_window);
					
			TRACE("personCountDown: %d\n", maxPersonCountDown);

			if (maxPersonCountDown==ma_window)
				person->pushCoordinates(personCurrentRealX, personCurrentRealY);
			else
				if (ma_follow_to_the_end) person->removeCoordinates();
				

			//person->status();

			personCurrentRealX=person->getCurrentMA_X();
			personCurrentRealY=person->getCurrentMA_Y();

			cvCircle(map->mapImage2Show,cvPoint(cvRound(personCurrentRealX/map->real_size_x*map->mapImage->width), cvRound(personCurrentRealY/map->real_size_y*map->mapImage->height)), 10, 
				CV_RGB(grayLevel, grayLevel, grayLevel), 2);
			cvCircle(map->mapImage2Show, cvPoint(personCurrentMapX,personCurrentMapY), 2, CV_RGB(0,0,0), -1);


			//cvCircle(map->mapImage2Show,cvPoint(COMPASS_CENTER_X, COMPASS_CENTER_Y), COMPASS_RADIUS, CV_RGB(0, 0, 0), 2);

			sprintf(map_text, "(%s)", cam[cameraHl]->id);
			cvPutText(map->mapImage2Show, map_text, cvPoint(info_x,info_y), &font, CV_RGB(0, 0, 0));

			sprintf(map_text, "%.2f m/s", person->speed());
			cvPutText(map->mapImage2Show,"speed:", cvPoint(info_x,info_y+30), &font, CV_RGB(0, 0, 0));
			cvPutText(map->mapImage2Show, map_text, cvPoint(info_x,info_y+50), &font, CV_RGB(0, 0, 0));
					

			cvCircle(map->mapImage, cvPoint(cvRound(personCurrentRealX/map->real_size_x*map->mapImage->width), cvRound(personCurrentRealY/map->real_size_y*map->mapImage->height)), 1, CV_RGB(0,0,0), 1.5);
					
			if (person->getCount()>1)  // we have a line to track...
				cvLine(map->mapImage,
					cvPoint(cvRound(person->getCurrentMA_X()/map->real_size_x*map->mapImage->width), cvRound(person->getCurrentMA_Y()/map->real_size_y*map->mapImage->height)),
					cvPoint(cvRound(person->getPreviousMA_X()/map->real_size_x*map->mapImage->width), cvRound(person->getPreviousMA_Y()/map->real_size_y*map->mapImage->height)),
						CV_RGB(map->pathcolor_r, map->pathcolor_g, map->pathcolor_b), 2);
				
			if (person->speed()>0)
				{
				cvCircle(map->mapImage2Show, 
				cvPoint(
					cvRound(person->getCurrentMA_X()/map->real_size_x*map->mapImage->width+sin(person->direction())*PERSON_CIRCLE_DIRECTION_OFFSET),
					cvRound(person->getCurrentMA_Y()/map->real_size_y*map->mapImage->height+cos(person->direction())*PERSON_CIRCLE_DIRECTION_OFFSET)),
				PERSON_CIRCLE_DIRECTION_RADIUS,
				CV_RGB(0, 0, 0), -1);



				sprintf(map_text, "%3.2f deg", person->direction()*180/PI);
					cvPutText(map->mapImage2Show,"direction:", cvPoint(info_x,info_y+80), &font, CV_RGB(0, 0, 0));
					cvPutText(map->mapImage2Show, map_text, cvPoint(info_x,info_y+100), &font, CV_RGB(0, 0, 0));
				}  // test on speed
			}
			
			else // maxPersonCountDown < 0
					
			{
						person->resetCount();
			}

			if (fp_bvh_output){
				if (maxPersonCountDown<=0){
					threeX=0;
					threeY=0;
					threeZ=5.0;
					threeR=0;
				}
				else{
					threeX=personCurrentRealX;
					threeY=personCurrentRealY;
					threeR=person->speed()>0?(person->direction())*180/PI:0;
					threeZ=0.001;
				}

					TRACE("Scrivo sul file fp_bvh_output...\n");
				fprintf(fp_bvh_output, " %4.2f\t %4.2f\t %4.2f\t 0.00\t 0.00\t %4.2f\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\t 0.00\n", m2inches(threeX), m2inches(threeZ), m2inches(threeY), threeR); 
				}


			cvShowImage("mapwindow", map->mapImage2Show);

			sprintf(nf, "map%.4d.png", frameNo);
			//TRACE("Saving map ... %s\n", nf); cvSaveImage(nf, map->mapImage2Show); 


//			cvReleaseImage(&map->mapImage2Show);
			/* moved up to keep it for saving... */

			frameNo++;

			   
			UpdateData(true);
			info=(CString)info_text;
			UpdateData(false);

			key=cvWaitKey(1);

			while(status != MCT_STATUS_PLAYING)
				cvWaitKey(1);


			} //end while (sequence)

		   
		   for (cameraNo=0; cameraNo<MAXCAMERAS; cameraNo++){
			   cvReleaseCapture(&(cam[cameraNo]->videoStream));
			   cvReleaseImage(&(cam[cameraNo]->differenceImage));
			   cvReleaseImage(&(cam[cameraNo]->grayImage));
			   cvReleaseImage(&(cam[cameraNo]->backgroundImage));
		   }
		   cvReleaseImage(&map->mapImage);

		   TRACE("Chiudo i file\n");
	if (fp_bvh_output)
		fclose(fp_bvh_output);
	if (fp_bvh_header)
		fclose(fp_bvh_header);

	status=MCT_STATUS_JUSTSTARTED;
	ButtonsUpdate();

	delete person;


}

void CMultiCameraTrackerDlg::OnBnClickedDebug()
{

	int thickness=1;


	int cameraNo;
    int x, y, mapX, mapY;
	float m, q;
	float out_x, out_y;


	for (cameraNo=0; cameraNo<MAXCAMERAS; cameraNo++){
		for (x=0; x<cam[cameraNo]->frameWidth;x++){
			y=0;
			mapCoordinates(x,y,cam[cameraNo],out_x, out_y);
			mapX=cvRound(out_x*map->mapImage->width);
			mapY=cvRound(out_y*map->mapImage->height);
			TRACE("cam: %d; x=%d, y=%d -> mapX=%d; mapY=%d\n", cameraNo, x, y, mapX, mapY);
			cvCircle(cam[cameraNo]->frameImage, cvPoint(x, y), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));
			cvCircle(map->mapImage, cvPoint(mapX, mapY), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));

			y=cam[cameraNo]->frameHeight-1;
			mapCoordinates(x,y,cam[cameraNo],out_x, out_y);
			mapX=cvRound(out_x*map->mapImage->width);
			mapY=cvRound(out_y*map->mapImage->height);
			TRACE("cam: %d; x=%d, y=%d -> mapX=%d; mapY=%d\n", cameraNo, x, y, mapX, mapY);
			cvCircle(cam[cameraNo]->frameImage, cvPoint(x, y), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));
			cvCircle(map->mapImage, cvPoint(mapX, mapY), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));

			y=x*(float)cam[cameraNo]->frameHeight/(float)cam[cameraNo]->frameWidth;
			mapCoordinates(x,y,cam[cameraNo],out_x, out_y);
			mapX=cvRound(out_x*map->mapImage->width);
			mapY=cvRound(out_y*map->mapImage->height);
			TRACE("cam: %d; x=%d, y=%d -> mapX=%d; mapY=%d\n", cameraNo, x, y, mapX, mapY);
			cvCircle(cam[cameraNo]->frameImage, cvPoint(x, y), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));
			cvCircle(map->mapImage, cvPoint(mapX, mapY), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));
			
			y=cam[cameraNo]->frameHeight-y;
			mapCoordinates(x,y,cam[cameraNo],out_x, out_y);
			mapX=cvRound(out_x*map->mapImage->width);
			mapY=cvRound(out_y*map->mapImage->height);
			TRACE("cam: %d; x=%d, y=%d -> mapX=%d; mapY=%d\n", cameraNo, x, y, mapX, mapY);
			cvCircle(cam[cameraNo]->frameImage, cvPoint(x, y), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));
			cvCircle(map->mapImage, cvPoint(mapX, mapY), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));
			}
		for (y=0; y<cam[cameraNo]->frameHeight;y++){
			x=0;
			mapCoordinates(x,y,cam[cameraNo],out_x, out_y);
			mapX=cvRound(out_x*map->mapImage->width);
			mapY=cvRound(out_y*map->mapImage->height);
			cvCircle(cam[cameraNo]->frameImage, cvPoint(x, y), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));
			cvCircle(map->mapImage, cvPoint(mapX, mapY), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));

			x=cam[cameraNo]->frameWidth-1;
			mapCoordinates(x,y,cam[cameraNo],out_x, out_y);
			mapX=cvRound(out_x*map->mapImage->width);
			mapY=cvRound(out_y*map->mapImage->height);
			cvCircle(cam[cameraNo]->frameImage, cvPoint(x, y), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));
			cvCircle(map->mapImage, cvPoint(mapX, mapY), 1, CV_RGB(cam[cameraNo]->pathcolor_r,cam[cameraNo]->pathcolor_g,cam[cameraNo]->pathcolor_b));

			}
		cvShowImage(cam[cameraNo]->pictureBox, cam[cameraNo]->frameImage);
	}


	cvShowImage("mapwindow", map->mapImage);
	strcpy(map->fileSaveName, "debug_map.png");


	status=MCT_STATUS_DEBUGGED;
	ButtonsUpdate();


}





void CMultiCameraTrackerDlg::OnBnClickedPause()
{
	TRACE("Cliccato su PAUSE\n");

	if (status==MCT_STATUS_PLAYING)
		status=MCT_STATUS_PAUSED;
	else
		status=MCT_STATUS_PLAYING;

//	running=!running;
	
	ButtonsUpdate();
	
/*	UpdateData(true);
	PauseCaption=running?"|| (playing)":"|| (paused)";
	UpdateData(false);
*/
}


void CMultiCameraTrackerDlg::OnBnClickedCalibrate()
{
	string filename = "";
	string szFiltersConfig= "Text file (*.txt)|*.txt|All files (*.*)|*.*|";
	string szFiltersFrame= "Frame image (*.png)|*.png|Frame image (*.jpg)|*.jpg|All Files (*.*)|*.*|";
	string szFiltersMap= "Map image (*.png)|*.png|Frame image (*.jpg)|*.jpg|All Files (*.*)|*.*|";

	CFileDialog dlgS(FALSE, "txt", "calib.txt", OFN_CREATEPROMPT | OFN_HIDEREADONLY | OFN_ENABLESIZING, szFiltersConfig.c_str(), this);
	CFileDialog dlgF(TRUE, "Frame", "*.png", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_ENABLESIZING, szFiltersFrame.c_str(), this);
	CFileDialog dlgM(TRUE, "Map", "*.png", OFN_FILEMUSTEXIST| OFN_HIDEREADONLY | OFN_ENABLESIZING, szFiltersMap.c_str(), this);

	static CCamera cam;
	
	
	static IplImage* imgMap=NULL; 

   static CvMat * translate = cvCreateMat(3,3,CV_32FC1);



	switch (calibration_step) {
		case(0):
			TRACE("Calibration started...\n");
			GetDlgItem(IDC_OPEN)->EnableWindow(FALSE);
			GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
			cam.frameImage=NULL;

			if (dlgF.DoModal()==IDOK) {
				filename = (dlgF.GetFileName()).GetBuffer();
				TRACE ("Tento di aprire immagine frame %s...\n", filename.c_str());	
					//char *fileNameFrame=filename.c_str();
					cvNamedWindow("frame", CV_WINDOW_AUTOSIZE); 
					cvMoveWindow("frame",  700, 10); 
					cam.frameImage=cvLoadImage(filename.c_str());
					if(!cam.frameImage) {
						MessageBox("Sorry, I could not load the image.\nPlease ensure it is a PNG or JPG file.", "MCT", MB_OK|MB_ICONSTOP);
						cvDestroyWindow("frame");
						GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
						GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
						return;
					}
			}
			else
				{
					GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
					GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
					return;
				}

			if (dlgM.DoModal()==IDOK) {
				filename = (dlgM.GetFileName()).GetBuffer();
				TRACE ("Tento di aprire immagine mappa %s...\n", filename.c_str());	
					//char *fileNameMap=filename.c_str();
					cvNamedWindow("map", CV_WINDOW_AUTOSIZE); 
					cvMoveWindow("map",  10, 10); 
					imgMap=cvLoadImage(filename.c_str());
					if(!imgMap) {
						MessageBox("Sorry, I could not load the image.\nPlease ensure it is a PNG or JPG file.", "MCT", MB_OK|MB_ICONSTOP);
						cvDestroyWindow("map");
						GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
						GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
						if (cam.frameImage){
							cvReleaseImage(&(cam.frameImage));
							cvDestroyWindow("frame");
							}
						return;
					}
			}
			else
				{
					GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
					GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
					if (cam.frameImage){
						cvReleaseImage(&(cam.frameImage));
							cvDestroyWindow("frame");
							}

					return;
				}

			cvSetMouseCallback("map",mouseHandlerMap,imgMap);

			cvShowImage("map",imgMap);
			cam.associatedMapImage=imgMap;
			cam.frameWidth=cam.frameImage->width;
			cam.frameHeight=cam.frameImage->height;


			cvSetMouseCallback("frame",mouseHandlerFrame,&cam);

			cvShowImage("frame",cam.frameImage);
			calibration_step = 1;

			UpdateData(true);
			CalibCaption="Test matrix";
			UpdateData(false);

			break;
		case(1):
			if (calibration_clicks/2 >=4)
				{

				   CvMat* src_mat = cvCreateMat( calibration_clicks/2, 2, CV_32FC1 );
				   CvMat* dst_mat = cvCreateMat( calibration_clicks/2, 2, CV_32FC1 );

				   //copy our points into the matrixes
				   cvSetData( src_mat, calibration_cvsrc, sizeof(CvPoint2D32f));
				   cvSetData( dst_mat, calibration_cvdst, sizeof(CvPoint2D32f));

			       cvFindHomography(src_mat, dst_mat, translate);

			//	   float *matrix = translate->data.fl;

					TRACE("HM: %f %f %f %f %f %f %f %f %f\n",
						cvmGet(translate,0,0),
						cvmGet(translate,0,1),
						cvmGet(translate,0,2),
						cvmGet(translate,1,0),
						cvmGet(translate,1,1),
						cvmGet(translate,1,2),
						cvmGet(translate,2,0),
						cvmGet(translate,2,1),
						cvmGet(translate,2,2)
						);
					cvReleaseMat(&src_mat);
					cvReleaseMat(&dst_mat);

					calibration_step=2;
					calibration_clicks=0;
				
					cam.hm[0][0]=cvmGet(translate,0,0);
					cam.hm[0][1]=cvmGet(translate,0,1);
					cam.hm[0][2]=cvmGet(translate,0,2);
					cam.hm[1][0]=cvmGet(translate,1,0);
					cam.hm[1][1]=cvmGet(translate,1,1);
					cam.hm[1][2]=cvmGet(translate,1,2);
					cam.hm[2][0]=cvmGet(translate,2,0);
					cam.hm[2][1]=cvmGet(translate,2,1);
					cam.hm[2][2]=cvmGet(translate,2,2);

					//cam.frameWidth=cam.frameImage->width;
					//cam.frameHeight=cam.frameImage->height;
					
					HWND fh = (HWND)cvGetWindowHandle("frame");

					MessageBox("You can now test the Homographic matrix. Click on the frame image and check what you get on the map image. When you are done, click on the Save button.", "MCT", MB_OK | MB_ICONINFORMATION);
					calibration_step=2;
					UpdateData(true);
					CalibCaption="Save...";
					UpdateData(false);
		//			cvSetMouseCallback("frame",mouseHandlerFrame,&cam);
			}
			else
			{
				MessageBox("Sorry, I need at least four pairs of points to find the Homographic matrix.\nPlease add the missing ones.", "MCT", MB_OK|MB_ICONEXCLAMATION);
				cvShowImage("cam", cam.frameImage);
				cvShowImage("map", imgMap);
			}
			break;
		case(2):
			TRACE("Salvo, dealloco e chiudo...\n");
				if (dlgS.DoModal()==IDOK)
					{
					filename = (dlgS.GetFileName()).GetBuffer();
					TRACE ("Apro in scrittura %s...\n", filename.c_str());
					fp_calib = fopen(filename.c_str(), "w");
					if (!fp_calib)
						{
							GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
							GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
							UpdateData(true);
							CalibCaption="Calibrate...";
							UpdateData(false);
					
							calibration_step=0;

							return;
						}
					}
				else
					{
						GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
						GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
						UpdateData(true);
						CalibCaption="Calibrate...";
						UpdateData(false);
						if (translate)
							cvReleaseMat(&translate);
		
						if (imgMap)
							cvReleaseImage(&imgMap);
						if (cam.frameImage)
							cvReleaseImage(&(cam.frameImage));
						cvDestroyWindow("map");
						cvDestroyWindow("frame");
						calibration_step=0;
						return;
					}
					fprintf(fp_calib, "<camera ");
					fprintf(fp_calib, "id=\"camera X\" ");
					fprintf(fp_calib, "mask=\"maps\maskcamN.png\" ");
					fprintf(fp_calib, "source=\"videosources/sourceN.avi\" ");
					fprintf(fp_calib, "pos_h=\"0.00\" ");
					fprintf(fp_calib, "pos_x=\"0.00\" ");
					fprintf(fp_calib, "pos_y=\"0.00\" ");
					fprintf(fp_calib, "focal_length=\"1.0\" ");
					fprintf(fp_calib, "pathcolor=\"255 127 0\" ");
					fprintf(fp_calib, "threshold=\"127\" ");
					fprintf(fp_calib, "frombottom=\"15\" ");
					fprintf(fp_calib, "dilate=\"10\" ");
					fprintf(fp_calib, "erode=\"10\" ");
					fprintf(fp_calib, "hm=\"%f %f %f %f %f %f %f %f %f\"", 
						cvmGet(translate,0,0),
						cvmGet(translate,0,1),
						cvmGet(translate,0,2),
						cvmGet(translate,1,0),
						cvmGet(translate,1,1),
						cvmGet(translate,1,2),
						cvmGet(translate,2,0),
						cvmGet(translate,2,1),
						cvmGet(translate,2,2)
						);
					fprintf(fp_calib, "/>\n");
					fprintf(fp_calib, "<!--\nYou'll need to adjust many of these parameters,\n");
					fprintf(fp_calib, "expecially focal length, pos_h, pos_x, pos_y,\n");
					fprintf(fp_calib, "source, mask, and threshold. Good luck!\n-->\n");
					
					//if (fp_calib)
						fclose(fp_calib);

			if (translate)
				cvReleaseMat(&translate);

			if (imgMap)
				cvReleaseImage(&imgMap);
			if (cam.frameImage)
				cvReleaseImage(&(cam.frameImage));
			cvDestroyWindow("map");
			cvDestroyWindow("frame");

			UpdateData(true);
			CalibCaption="Calibrate...";
			UpdateData(false);

			GetDlgItem(IDC_OPEN)->EnableWindow(TRUE);
			GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
					
			calibration_step=0;
			break;


	}
}

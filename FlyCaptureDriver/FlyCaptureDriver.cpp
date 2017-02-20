#include "FlyCaptureDriver.h"
#include "FlyCaptureUtils.h"
#include <iomanip>//���ڸ�ʽ��stringstream

using namespace FlyCapture2;
using namespace std;

//�������������Ϣ
DriverConfig dconfig;
//�����GUID�б�
vector<PGRGuid*> guid;
//ָ�����߳����еı�־λ 
bool b_isRunning;
//ָ�Ӳɼ��ı�־λ
bool b_isCapturing;
//����
std::list<FrameToSave*> FrameBuff;//��jpg�Ļ���
//ͼ�񱣴�����
std::vector<int> compression_params;

int FlyCaptureDriverInit(int argc, char** argv)
{
	//��ӡSDK�뱾�����Ϣ
	PrintBuildInfo();

	//��������
	if (argc > 1)
	{
		dconfig.s_dst_path = string(argv[1]);
	}
	else
	{
		cout << "Usage:  FlyCaptureDriver.exe [Path to save pics] [Interval(ms) to save pics] [flip?0/1] [Exposure(ms)]" << endl;
		return 1;
	}
	if (argc > 2)
	{
		dconfig.i_interval = atoi(argv[2]);
	}
	if (argc > 3)
	{
		dconfig.b_isFlipped = atoi(argv[3]) > 0;
	}
	if (argc > 4)
	{
		dconfig.d_exposure = atof(argv[4]);
	}

	//��ӡ������Ϣ
	dconfig.print();

	//����ͼ�񱣴�����
	compression_params.push_back(CV_IMWRITE_JPEG_QUALITY);
	compression_params.push_back(100);//��������� Ĭ��ֵ��95

	//��ȡ����
	///��ʼ��
	//�����쳣����
	Error error;

	BusManager busMgr;
	unsigned int u_numCameras;
	error = busMgr.GetNumOfCameras(&u_numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return 2;
	}

	cout << "Number of cameras detected: " << u_numCameras << endl;

	if (u_numCameras < 0){ return 3; }

	guid.reserve(u_numCameras);
	for (unsigned int i = 0; i < u_numCameras; i++)
	{
		PGRGuid* guid_tmp = new PGRGuid();
		error = busMgr.GetCameraFromIndex(i, guid_tmp);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
		guid.push_back(guid_tmp);
	}

	cout << "FlyCaptureDriverInit Done." << endl;
	return 0;
}

/**************************************************************************/
//����¼��ṹ��
struct MouseArgs{
	cv::Mat* mp_frame;
	cv::Rect r_box;
	bool b_isDrawing;

	// init
	MouseArgs() :b_isDrawing(false), mp_frame(0){
		r_box = cv::Rect(0, 0, -1, -1);
	}
};
//���ͼ��ĳһ���ߵĴ�С������Ļ��С���ص��������
void MouseDraw(int event, int x, int y, int flags, void*param)
{
	MouseArgs* m_arg = (MouseArgs*)param;
	if (!m_arg->mp_frame)
		return;

	switch (event)
	{
	case CV_EVENT_LBUTTONDOWN: // �������
	{
		m_arg->r_box = cv::Rect(x, y, 0, 0);
		m_arg->b_isDrawing = true;
	}
	break;
	case CV_EVENT_MOUSEMOVE: // ����ƶ�ʱ
	{
		if (m_arg->b_isDrawing)
		{
			m_arg->r_box.width = x - m_arg->r_box.x;
			m_arg->r_box.height = y - m_arg->r_box.y;
		}
	}
	break;
	case CV_EVENT_LBUTTONUP: // �������
	{
		m_arg->b_isDrawing = false;
		if (m_arg->r_box.width < 0)
		{
			m_arg->r_box.x += m_arg->r_box.width;
			m_arg->r_box.width *= -1;
		}
		if (m_arg->r_box.height < 0)
		{
			m_arg->r_box.y += m_arg->r_box.height;
			m_arg->r_box.height *= -1;
		}
	}
	break;
	}
}
/**************************************************************************/
void CameraConfigInit(Camera* cam)
{
	Error error;
	//��������
	Property prop;
	//����
	prop.type = BRIGHTNESS;
	//Ensure the property is set up to use absolute value control.
	prop.absControl = true;
	//Set the absolute value of brightness to 0.5%.
	prop.absValue = 1.0;
	//Set the property.
	CameraSetProerty(cam, &prop);

	//����
	prop.type = GAIN;
	//Ensure auto-adjust mode is off.
	prop.autoManualMode = false;
	//Ensure the property is set up to use absolute value control.
	prop.absControl = true;
	//Set the absolute value of gain to 0.0 dB.
	prop.absValue = 0.0;
	CameraSetProerty(cam, &prop);

	//EXP
	//Declare a Property struct.
	//Define the property to adjust.
	prop.type = AUTO_EXPOSURE;
	//Ensure the property is on.
	prop.onOff = true;
	//Ensure auto-adjust mode is off.
	prop.autoManualMode = false;
	//Ensure the property is set up to use absolute value control.
	prop.absControl = true;
	//Set the absolute value of auto exposure to -3.5 EV.
	prop.absValue = 0.0;
	//Set the property.
	CameraSetProerty(cam, &prop);
#ifdef FLY_CAPTURE_CONFIG_TRIG_ON
	//���崥��ģʽ
	// Get current trigger settings
	TriggerMode triggerMode;
	error = cam->GetTriggerMode(&triggerMode);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}

	// Set camera to trigger mode 0
	triggerMode.onOff = true;
	triggerMode.mode = 0;
	triggerMode.parameter = 0;

	error = cam->SetTriggerMode(&triggerMode);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}

	// Poll to ensure camera is ready
	bool retVal = PollForTriggerReady(cam);
	if (!retVal)
	{
		cout << endl;
		cout << "Error polling for trigger ready!" << endl;
	}

	// Get the camera configuration
	FC2Config config;
	error = cam->GetConfiguration(&config);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}

	// Set the grab timeout to 5 seconds
	config.grabTimeout = 3000;

	// Set the camera configuration
	error = cam->SetConfiguration(&config);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}
#else
	//Framerate
	//Declare a Property struct.
	//Define the property to adjust.
	prop.type = FRAME_RATE;
	//Ensure the property is on.
	prop.onOff = true;
	//Ensure auto-adjust mode is off.
	prop.autoManualMode = false;
	//Ensure the property is set up to use absolute value control.
	prop.absControl = true;
	//Set the absolute value of auto exposure to -3.5 EV.
	prop.absValue = 1000 / capRest;
	//Set the property.
	CameraSetProerty(cam, &prop);

#endif
}
//����ͼ����
int FlyCaptureDriverRun(std::vector<FlyCapture2::PGRGuid*>& guid)
{
	//������Ŀ
	Error error;
	unsigned int u_numCameras = guid.size();

	if (u_numCameras < 1){ return -1; }
	Camera* c_cam = new Camera[u_numCameras];

	// Connect to a camera
	for (unsigned int i = 0; i < u_numCameras; i++)
	{
		error = c_cam[i].Connect(guid[i]);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
	}

#ifdef FLY_CAPTURE_CONFIG_POWER_ON
	//Ϊ����ϵ�
	for (unsigned int i = 0; i < u_numCameras; i++)
	{
		// Power on the camera
		const unsigned int k_cameraPower = 0x610;
		const unsigned int k_powerVal = 0x80000000;
		error = c_cam[i].WriteRegister(k_cameraPower, k_powerVal);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		const unsigned int millisecondsToSleep = 1000;
		unsigned int regVal = 0;
		unsigned int retries = 10;

		// Wait for camera to complete power-up
		do
		{
			Sleep(millisecondsToSleep);

			error = c_cam[i].ReadRegister(k_cameraPower, &regVal);
			if (error == PGRERROR_TIMEOUT)
			{
				// ignore timeout errors, camera may not be responding to
				// register reads during power-up
			}
			else if (error != PGRERROR_OK)
			{
				PrintError(error);
				return -1;
			}

			retries--;
		} while ((regVal & k_powerVal) == 0 && retries > 0);

		// Check for timeout errors after retrying
		if (error == PGRERROR_TIMEOUT)
		{
			PrintError(error);
			return -1;
		}
	}
#endif
	// Get the camera information
	CameraInfo camInfo;
	for (unsigned int i = 0; i < u_numCameras; i++)
	{
		error = c_cam[i].GetCameraInfo(&camInfo);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}

		PrintCameraInfo(&camInfo);
	}

	Image* rawImage = new Image[u_numCameras];//ԭʼͼ�������
	// Create a converted image
	Image* convertedImage = new Image[u_numCameras];//ת��ͼ������
	cv::Mat* m_rawFrame = new cv::Mat[u_numCameras];//ת��ΪOpenCV��ͼ��
	cv::Mat* m_dispFrame = new cv::Mat[u_numCameras];//��С���ͼ��

	SYSTEMTIME* sys_captureTime = new SYSTEMTIME[u_numCameras];//��������ʱ�������
	Property* p_cameraProp = new Property[u_numCameras];//ÿ����������Խṹ��
	vector<string> vs_winName;//���д��ڵ������б�
	stringstream ss;//�ַ������
	ss << setiosflags(ios::fixed);//�̶��ĸ�����ʾ
	for (unsigned int q = 0; q < u_numCameras; q++)
	{
		ss << "Camera " << setfill('0') << setprecision(3) << q << endl;
		vs_winName.push_back(ss.str());
		cv::namedWindow(vs_winName[q]);
		ss.str("");//��������
	}
#ifdef FLY_CAPTURE_CONFIG_FOCUS_ASSIST
	cv::Mat* m_cannyMat = new cv::Mat[u_numCameras];
#endif
	MouseArgs* ma_mouseArgs = new MouseArgs[u_numCameras];
	//ע�����ص�
	for (unsigned int q = 0; q < u_numCameras; q++)
	{
		ma_mouseArgs[q].mp_frame = &(m_dispFrame[q]);
		cv::setMouseCallback(vs_winName[q], MouseDraw, (void*)&(ma_mouseArgs[q]));
	}

	CvFont font;//��������
	double hscale = 1.0; double vscale = 1.0; int linewidth = 1;
	cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC, hscale, vscale, 0, linewidth);

	cv::Scalar textColor = cvScalar(20, 255, 20);	//�����ı���ɫ
	cv::Scalar recColor = cvScalar(0, 10, 255);		//��ʾ���ڶ�ʱ����
	cv::Scalar boxColor = cvScalar(0, 135, 255);	//������ɫ
	cv::Point textPos = cvPoint(10, 25);			//�ı���ʼλ��

	SYSTEMTIME sys_capTarget, sys_capNow;
	unsigned int i_capCount = 0;//�Զ���¼֡����ʱ;


	//�����ع����Ա���
	Property p_expProp;
	//��ʼ������
	p_expProp.type = AUTO_EXPOSURE;
	//�����ƽ�����Ա���
	Property p_whiteProp;
	//��ȡ��һ������İ�ƽ��
	p_whiteProp.type = WHITE_BALANCE;
	c_cam[0].GetProperty(&p_whiteProp);
	//ȡ���Զ��仯��ƽ��
	p_whiteProp.autoManualMode = false;

	//ͳһ����
	for (unsigned int i = 0; i < u_numCameras; i++)
	{
		//���ư�ƽ������
		CameraSetProerty(&c_cam[i], &p_whiteProp);
		p_cameraProp[i].type = SHUTTER;
		//�ع�Ļ��Զ�
		c_cam[i].GetProperty(&p_cameraProp[i]);
		p_cameraProp[i].autoManualMode = true;
		c_cam[i].SetProperty(&p_cameraProp[i]);
		//��ʼ����������
		CameraConfigInit(&c_cam[i]);
		//�����videomode ��������
		//cam[i].SetVideoModeAndFrameRate(VIDEOMODE_1600x1200Y8,FRAMERATE_3_75);
	}

	// Start capturing images
	for (unsigned int i = 0; i < u_numCameras; i++)
	{
		error = c_cam[i].StartCapture();
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
	}

	while (1)
	{
		if (b_isCapturing)
		{
			//��ȡ��ʼʱ��
			GetLocalTime(&sys_capNow);
			if (CompareSystemTime(sys_capTarget, sys_capNow) > 0)
			{
				//ʱ��δ���������ȴ�
				continue;
			}
			else
			{
				//����ʱ��
				sys_capTarget = SystemtimeForwardMS(sys_capTarget, (int)dconfig.i_interval);
			}
		}
#ifdef FLY_CAPTURE_CONFIG_TRIG_ON
		// Retrieve an image
		for (unsigned int i = 0; i < u_numCameras; i++)
		{
			//����ģ��
			/*
			// Check that the trigger is ready
			bool retVal = PollForTriggerReady(&cam[i]);
			if (!retVal)
			{
			cout << endl;
			cout << "Error polling for trigger ready!" << endl;
			}
			*/
			GetLocalTime(&(sys_captureTime[i]));//������Ϊ����ļ�ʱ��ʼ
			//cout << "Press the Enter key to initiate a software trigger" << endl;
			//cin.ignore();

			// Fire software trigger
			bool bRet = FireSoftwareTrigger(&c_cam[i]);
			if (!bRet)
			{
				cout << "\nError firing software trigger @Camera " << i << endl;
			}
		}
#endif
		for (unsigned int i = 0; i < u_numCameras; i++)
		{
#ifndef FLY_CAPTURE_CONFIG_TRIG_ON
			GetLocalTime(&(sys_captureTime[i]));//������Ϊ����ļ�ʱ��ʼ
#endif
			error = c_cam[i].RetrieveBuffer(&rawImage[i]);
			if (error != PGRERROR_OK)
			{
				PrintError(error);
				break;
				break;
			}
			//TimeStamp mtp = rawImage[i].GetTimeStamp();
			//cout << mtp.microSeconds << endl;;
		}

		for (unsigned int i = 0; i < u_numCameras; i++)
		{
			// Convert the raw image
			error = rawImage[i].Convert(FlyCapture2::PIXEL_FORMAT_BGR, &convertedImage[i]);
			if (error != PGRERROR_OK)
			{
				PrintError(error);
				return -1;
			}
		}

		// convert to OpenCV Mat
		for (unsigned int i = 0; i < u_numCameras; i++)
		{
			float lamda = 0.5;
			unsigned int rowBytes = (double)convertedImage[i].GetReceivedDataSize() / (double)convertedImage[i].GetRows();
			m_rawFrame[i] = cv::Mat(convertedImage[i].GetRows(), convertedImage[i].GetCols(), CV_8UC3, convertedImage[i].GetData(), rowBytes);
			cv::Size size = cv::Size(cvRound(lamda*convertedImage[i].GetCols()), cvRound(lamda*convertedImage[i].GetRows()));//����������Ҫת��
			m_dispFrame[i] = cv::Mat(size, CV_8UC3);

			resize(m_rawFrame[i], m_dispFrame[i], size);
			if (dconfig.b_isFlipped)
			{
				flip(m_dispFrame[i], m_dispFrame[i], -1);
			}

			//			Canny(m_dispFrame[i],m_cannyMat[i],80,250);
			cv::Mat m_dispFrameROI;// = m_dispFrame[i];
			if (CheckROIBox(ma_mouseArgs[i].r_box, *(ma_mouseArgs[i].mp_frame)))
			{
				//ȷ��ROI����
				//m_dispFrameROI=m_dispFrame[i](ma_mouseArgs[i].box);
				//��Ϊ����Ҫ��ת�ɺ�ɫ�İɣ�
				cv::cvtColor(m_dispFrame[i](ma_mouseArgs[i].r_box), m_dispFrameROI, CV_RGB2GRAY);
			}
			else
			{
				cv::cvtColor(m_dispFrame[i], m_dispFrameROI, CV_RGB2GRAY);
			}

			if (ma_mouseArgs[i].b_isDrawing)
			{
				//���ڻ��Ƶ�ROI����
				rectangle(m_dispFrame[i], ma_mouseArgs[i].r_box, boxColor, 2);
			}
			else
			{
				//imshow("p", m_dispFrameROI);
				//��ROI���д���
				rectangle(m_dispFrame[i], ma_mouseArgs[i].r_box, textColor, 2);//����

				//����ֱ��ͼ
				/// Establish the number of bins
				int histSize = 64;

				/// Set the ranges 
				float range[] = { 0, 256 };
				const float* histRange = { range };
				bool uniform = true; bool accumulate = false;
				cv::Mat grayHist;
				//����ֱ��ͼ
				calcHist(&m_dispFrameROI, 1, 0, cv::Mat(), grayHist, 1, &histSize, &histRange, uniform, accumulate);
				/**/
				/*�����ع�*/
				float accBright = 0.0f;
				float curRate = 0.0f;
				for (int b = 0; b < histSize; b++)
				{
					accBright += grayHist.at<float>(b, 0);
					if (accBright > 0.8*m_dispFrameROI.cols*m_dispFrameROI.rows)
					{
						//��õ�ǰ80%���ص��ع�ˮƽ
						curRate = (float)b / histSize;
						break;
					}
				}

				//��һ��ֱ��ͼ
				cv::normalize(grayHist, grayHist, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
				//����ֱ��ͼ
				DrawHist(m_dispFrame[i], grayHist);

				//�������ع�Ρ�
				const float refRate = 0.55f;
				float errExp = refRate - curRate;//���
#ifdef FLY_CAPTURE_CONFIG_TRIG_ON
				float pRatio = 0.7;
#else
				float pRatio = 0.5;
#endif
				float uOut = p_cameraProp[i].absValue * (pRatio *(exp(errExp) - 1.0) + 1.0);//������������
				c_cam[i].GetProperty(&p_cameraProp[i]);
				//cout << curRate <<" "<< p_cameraProp.absValue << " ";
				ss << "Shutter: " << setw(4) << p_cameraProp[i].absValue << "ms. Buffer: " << setw(4) << FrameBuff.size();//ͬʱ������������

				p_cameraProp[i].autoManualMode = false;
				p_cameraProp[i].absControl = true;
				p_cameraProp[i].absValue = (dconfig.d_exposure < 0) ? uOut : dconfig.d_exposure;

				//���ſ���
				//p_cameraProp[i].absValue = uOut;
				//cout << p_cameraProp.absValue << endl;
				c_cam[i].SetProperty(&p_cameraProp[i]);

#ifdef FLY_CAPTURE_CONFIG_FOCUS_ASSIST
				if (!b_isCapturing)
				{
					//������ڶ�ʱ¼����ô�ͼ����Ե
					Laplacian(m_dispFrameROI, m_cannyMat[i], m_dispFrameROI.depth());
					cv::Scalar c_val = sum(abs(m_cannyMat[i]));
					ss << " Canny: " << setw(4) << (c_val.val[0] / 1e5);
				}
#endif 

			}

			if (b_isCapturing)
			{
				ss << " " << dconfig.i_interval << "ms. #" << i_capCount;
			}

			cv::putText(m_dispFrame[i], ss.str(), textPos, 0, 0.6, (b_isCapturing) ? recColor : textColor, 2);
			ss.str("");//��������
			imshow(vs_winName[i], m_dispFrame[i]);

		}

		char k = cvWaitKey(20);

		if (k == 's' || k == 13 || b_isCapturing)
		{
			if (b_isCapturing)
			{
				i_capCount++;
			}
			//�ϴ�ͼ��
			for (unsigned int i = 0; i < u_numCameras; i++)
			{
				FrameToSave* tmp = new FrameToSave(m_rawFrame[i], sys_captureTime[i], i);
				FrameBuff.push_back(tmp);
			}
		}

		if (k == 32)
		{
			b_isCapturing = !b_isCapturing;
			cout << "Capture every " << dconfig.i_interval << "ms : " << ((b_isCapturing) ? "ON" : "OFF") << endl;
			if (b_isCapturing)
			{
				i_capCount = 0;
				GetLocalTime(&sys_capTarget);//���±���ֵ
			}
		}
		if (k == 27 || k == 'q'){ cout << "Grab finished" << endl; break; }

	}
	/*
	for ( int imageCnt=0; imageCnt < k_numImages; imageCnt++ )
	{
	// Retrieve an image
	error = cam.RetrieveBuffer( &rawImage );
	if (error != PGRERROR_OK)
	{
	PrintError( error );
	continue;
	}

	cout << "Grabbed image " << imageCnt << endl;

	// Create a converted image
	Image convertedImage;

	// Convert the raw image
	error = rawImage.Convert( PIXEL_FORMAT_MONO8, &convertedImage );
	if (error != PGRERROR_OK)
	{
	PrintError( error );
	return -1;
	}

	// Create a unique filename

	ostringstream filename;
	filename << "FlyCapture2Test-" << camInfo.serialNumber << "-" << imageCnt << ".pgm";

	// Save the image. If a file format is not passed in, then the file
	// extension is parsed to attempt to determine the file format.
	error = convertedImage.Save( filename.str().c_str() );
	if (error != PGRERROR_OK)
	{
	PrintError( error );
	return -1;
	}
	}
	*/

	// Stop capturing images
	for (unsigned int i = 0; i < u_numCameras; i++)
	{
		//�ع�Ļ��Զ�
		c_cam[i].GetProperty(&p_cameraProp[i]);
		p_cameraProp[i].autoManualMode = true;
		c_cam[i].SetProperty(&p_cameraProp[i]);
#ifdef FLY_CAPTURE_CONFIG_TRIG_ON
		// Turn trigger mode off.
		TriggerMode triggerMode;
		error = c_cam->GetTriggerMode(&triggerMode);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
		}
		triggerMode.onOff = false;
		error = c_cam[i].SetTriggerMode(&triggerMode);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
		}
#endif
		error = c_cam[i].StopCapture();
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
	}

	// Disconnect the camera
	for (unsigned int i = 0; i < u_numCameras; i++)
	{
#ifdef FLY_CAPTURE_CONFIG_TRIG_ON
		// Turn trigger mode off.
		TriggerMode triggerMode;
		error = c_cam->GetTriggerMode(&triggerMode);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
		}
		triggerMode.onOff = false;
		error = c_cam[i].SetTriggerMode(&triggerMode);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
		}
#endif
#ifdef FLY_CAPTURE_CONFIG_POWER_ON
		// Power off the camera
		const unsigned int k_cameraPower = 0x610;
		const unsigned int k_powerVal = 0x00000000;
		error = c_cam[i].WriteRegister(k_cameraPower, k_powerVal);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
#endif
		error = c_cam[i].Disconnect();
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return -1;
		}
	}
	delete[] c_cam;
	delete[] rawImage;
	delete[] convertedImage;
	delete[] m_rawFrame;
	delete[] m_dispFrame;
	delete[] sys_captureTime;
	delete[] p_cameraProp;
#ifdef	FLY_CAPTURE_CONFIG_FOCUS_ASSIST
	delete[] m_cannyMat;
	delete[] ma_mouseArgs;
#endif
	vs_winName.clear();
	return 0;
}

//�������
int FlyCaptureDriverFin()
{
	for (unsigned int i = 0; i < guid.size(); i++)
	{
		if (guid[i] != NULL)
		{
			delete guid[i];
		}
	}

	cout << "FlyCaptureDriverFin()." << endl;
	return 0;
}

//���浥��ͼƬ
void SaveSingleFrame(FrameToSave& pts)
{

	string s_framePath = dconfig.s_dst_path + '\\' + pts.GetFileName(dconfig.b_isFlipped) + ".jpg";
	cv::Mat tmp = pts.m_frame.clone();
	if (dconfig.b_isFlipped)
	{
		flip(tmp, tmp, -1);
	}
	if (cv::imwrite(s_framePath, tmp, compression_params))
	{
		cout << "Saved as " << s_framePath << endl;
	}
	else
	{
		cout << "[ERROR] Save " << s_framePath << " failed." << endl;
	}
}

//����ͼƬ�߳�
int SaveImgThread()
{
	bool b_hasFrames = false;
	while (b_isRunning)
	{
		try
		{
			if (b_hasFrames)
			{
				FrameToSave* tmp = *FrameBuff.begin();
				if (tmp != NULL)
				{
					SaveSingleFrame(*tmp);
					delete tmp;
				}
				FrameBuff.erase(FrameBuff.begin());
			}
			else
			{
				Sleep(100);
			}
			b_hasFrames = FrameBuff.size() > 0;
		}
		catch (exception e)
		{
			cout << e.what();
			return -1;
		}
	}
	return 0;
}


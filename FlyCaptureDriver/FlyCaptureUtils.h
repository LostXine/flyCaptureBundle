#ifndef FLY_CAPTURE_UTILS
#define FLY_CAPTURE_UTILS

#include "FlyCaptureCommon.h"
#include <opencv.hpp>
#include <Windows.h>
//<time.h>

//���غ�
//дXML��Ϣ
#define FLY_CAPTURE_CONFIG_XML
//�Խ�����
#define FLY_CAPTURE_CONFIG_FOCUS_ASSIST
//ʹ�ô�����/֡�ʿ���Ĭ�Ͽ���)
#define FLY_CAPTURE_CONFIG_TRIG_ON
//�򿪵�Դ
#define FLY_CAPTURE_CONFIG_POWER_ON

//ʱ��ת��
std::string Systemtime2String(SYSTEMTIME& sys);

//�Ƚ�����ʱ��� ���غ���
int CompareSystemTime(SYSTEMTIME& after, SYSTEMTIME& previous);
//�����ƽ�һ����������δ�ذ�ȫ��
SYSTEMTIME SystemtimeForwardMS(SYSTEMTIME& st, int ms);
//����ֱ��ͼ
void DrawHist(cv::Mat& img, cv::Mat hist);
//���ROI
bool CheckROIBox(cv::Rect& box, cv::Mat& img);
//���뻺��


//���е�ȫ��������Ϣ
struct DriverConfig
{
	//����ͼƬλ��
	std::string s_dst_path;
	//������
	double i_interval;
	//�̶��ع�ֵ
	double d_exposure;
	//�Ƿ�תͼ��
	bool b_isFlipped;
	//Ĭ�ϳ�ʼ������
	DriverConfig()
	{
		s_dst_path = "";
		i_interval = 50;
		d_exposure = -1;
		b_isFlipped = false;
	};
	//��ӡ����
	void print()
	{
		std::cout << "Local Config:\n"
			<< " --dst_path : " << s_dst_path << "\n"
			<< " --interval(ms) : " << i_interval << "\n"
			<< " --flipped : " << (b_isFlipped ? "yes" : "no") << "\n"
			<< " --exptosure(ms) : ";
		if (d_exposure < 0)
		{
			std::cout << "auto";
		}
		else
		{
			std::cout << d_exposure;
		}

		std::cout << "\n" << std::endl;
	}
};

//����ͼƬ�Ľṹ��
//�洢�ṹ
struct FrameToSave
{
	cv::Mat m_frame;
	SYSTEMTIME sys_timeStamp;
	unsigned int i_camSeq;
	float f_shutter;

	//init
	FrameToSave(cv::Mat _img, SYSTEMTIME _timeStamp, int _seq = 0, float _shutter = -1.0f)
	{
		m_frame = _img.clone();
		sys_timeStamp = _timeStamp;
		i_camSeq = _seq;
		f_shutter = _shutter;
	}

	//����ļ���
	std::string GetFileName(bool isFliped = false)
	{
		std::string time = Systemtime2String(sys_timeStamp);
		char camera[16];
		sprintf_s(camera, "%02d", i_camSeq);
		return time + (isFliped ? 'F' : 'C') + std::string(camera);
	}
};
#endif
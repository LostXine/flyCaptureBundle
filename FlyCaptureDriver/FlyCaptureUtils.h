#ifndef FLY_CAPTURE_UTILS
#define FLY_CAPTURE_UTILS

#include "FlyCaptureCommon.h"
#include <opencv.hpp>
#include <Windows.h>
//<time.h>

//���غ�
//���Ű�װ����ͷʱʹ��
//#define FLY_CAPTURE_CONFIG_FLIP_PIC
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
SYSTEMTIME Longtime2Systemtime(long long int& time);

//�Ƚ�����ʱ��� ���غ���
int CompareSystemTime(SYSTEMTIME& after, SYSTEMTIME& previous);
//�����ƽ�һ����������δ�ذ�ȫ��
SYSTEMTIME SystemtimeForwardMS(SYSTEMTIME& st,int ms);
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
	//Ĭ�ϳ�ʼ������
	DriverConfig()
	{
		s_dst_path = "";
		i_interval = 50;
		d_exposure = -1;
	};
	//��ӡ����
	void print()
	{
		std::cout << "Local Config:\n"
			<< " --dst_path : " << s_dst_path << "\n"
			<< " --interval(ms) : " << i_interval << "\n"
			<< " --exptosure(ms) : " << d_exposure << "\n"
			<< std::endl;
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
	std::string GetFileName()
	{		
		std::string time = Systemtime2String(sys_timeStamp);
		char camera[16];
		sprintf_s(camera, "C%02d", i_camSeq);
		return time + std::string(camera);
	}
};
#endif
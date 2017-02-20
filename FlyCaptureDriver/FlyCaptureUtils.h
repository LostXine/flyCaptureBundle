#ifndef FLY_CAPTURE_UTILS
#define FLY_CAPTURE_UTILS

#include "FlyCaptureCommon.h"
#include <opencv.hpp>
#include <Windows.h>
//<time.h>

//开关宏
//倒着安装摄像头时使用
//#define FLY_CAPTURE_CONFIG_FLIP_PIC
//写XML信息
#define FLY_CAPTURE_CONFIG_XML

//对焦辅助
#define FLY_CAPTURE_CONFIG_FOCUS_ASSIST
//使用触发器/帧率控制默认开启)
#define FLY_CAPTURE_CONFIG_TRIG_ON
//打开电源
#define FLY_CAPTURE_CONFIG_POWER_ON

//时间转换
std::string Systemtime2String(SYSTEMTIME& sys);
SYSTEMTIME Longtime2Systemtime(long long int& time);

//比较两个时间差 返回毫秒
int CompareSystemTime(SYSTEMTIME& after, SYSTEMTIME& previous);
//快速推进一定毫秒数（未必安全）
SYSTEMTIME SystemtimeForwardMS(SYSTEMTIME& st,int ms);
//绘制直方图
void DrawHist(cv::Mat& img, cv::Mat hist);
//检查ROI
bool CheckROIBox(cv::Rect& box, cv::Mat& img);
//推入缓存


//所有的全局配置信息
struct DriverConfig
{
	//保存图片位置
	std::string s_dst_path;
	//保存间隔
	double i_interval;
	//固定曝光值
	double d_exposure;
	//默认初始化参数
	DriverConfig()
	{
		s_dst_path = "";
		i_interval = 50;
		d_exposure = -1;
	};
	//打印配置
	void print()
	{
		std::cout << "Local Config:\n"
			<< " --dst_path : " << s_dst_path << "\n"
			<< " --interval(ms) : " << i_interval << "\n"
			<< " --exptosure(ms) : " << d_exposure << "\n"
			<< std::endl;
	}
};

//保存图片的结构体
//存储结构
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

	//获得文件名
	std::string GetFileName()
	{		
		std::string time = Systemtime2String(sys_timeStamp);
		char camera[16];
		sprintf_s(camera, "C%02d", i_camSeq);
		return time + std::string(camera);
	}
};
#endif
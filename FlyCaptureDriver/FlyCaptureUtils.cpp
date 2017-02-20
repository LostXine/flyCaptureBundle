#include "FlyCaptureUtils.h"
#include <time.h>


//生成文件路径（到时间为止）
std::string Systemtime2String(SYSTEMTIME& sys)
{
	char dateBuf[32];
	sprintf_s(dateBuf, "%4d%02d%02d-%02d%02d%02d-%03d",sys.wYear,sys.wMonth,sys.wDay,sys.wHour,sys.wMinute,sys.wSecond,sys.wMilliseconds);
	return std::string(dateBuf);
}

//以毫秒为单位返回这个月过了多少毫秒
long int SystemtimeInOneDay(SYSTEMTIME time)
{
	long int p = 0;
	p += time.wDay;
	p *= 24;
	p += time.wHour;
	p *= 60;
	p += time.wMinute;
	p *= 60;
	p += time.wSecond;
	p *= 1000;
	p += time.wMilliseconds;
	return p;
}

//比较同一个月内两个时间差 返回毫秒
int CompareSystemTime(SYSTEMTIME& after, SYSTEMTIME& previous)
{
	long int p1 = SystemtimeInOneDay(previous);
	long int p2 = SystemtimeInOneDay(after);
	return p2 - p1;
}

//systemtime进位
inline int SystemtimeBoostBit(WORD& timebit, int mod, int add)
{
	int sum = (int)timebit + add;
	timebit = (WORD)(sum % mod);
	return sum / mod;
}

//让时间前进(非常有限)
SYSTEMTIME SystemtimeForwardMS(SYSTEMTIME& st, int ms)
{
	int boost;
	SYSTEMTIME ntime = st;
	boost = SystemtimeBoostBit((ntime.wMilliseconds), 1000, ms);
	if (boost)
	{
		boost = SystemtimeBoostBit(ntime.wSecond, 60, boost);
	}
	else{ goto  END; }

	if (boost)
	{
		boost = SystemtimeBoostBit(ntime.wMinute, 60, boost);
	}
	else{goto  END;}

	if (boost)
	{
		boost = SystemtimeBoostBit(ntime.wHour, 24, boost);
	}
	else{goto  END;}

END:
	return ntime;
}


//绘制直方图函数
void DrawHist(cv::Mat& img, cv::Mat hist)
{
	//int i_w = img.cols;
	//int i_h = img.rows;

	cv::Rect rec(cv::Point(10, 40), cv::Point(325 , 120));

	//cv::rectangle(img, rec, cv::Scalar(255, 255, 255), 1);

	int bin = hist.rows;

	for (int i = 0; i < bin; i++)
	{
		//cout<<hist;
		int bHeight = cvRound(hist.at<float>(i, 0)*rec.height);
		int bBright = cvRound(((float)i + 0.5)*255.0 / (float)bin);
		int bBroad = (bBright + 128) % 256;
		cv::Scalar bScalar(bBright, bBright, bBright);
		cv::Scalar bBd(bBroad, bBroad, bBroad);
		int bLeft = cvRound(i*(float)rec.width / (float)bin) + rec.x;
		int bRight = bLeft + cvRound((float)rec.width / (float)bin);
		cv::rectangle(img, cv::Point(bLeft, rec.y + rec.height - bHeight), cv::Point(bRight, rec.y + rec.height), bScalar, -1);
		cv::rectangle(img, cv::Point(bLeft, rec.y + rec.height - bHeight), cv::Point(bRight, rec.y + rec.height), bBd, 1);
	}

}

//检查ROI
bool CheckROIBox(cv::Rect& box, cv::Mat& img)
{
	if (box.height <= 0 || box.width <= 0){ return false; }
	int xl = box.x;
	int yu = box.y;
	int xr = box.x + box.width;
	int yd = box.y + box.height;
	int xlim = img.cols;
	int ylim = img.rows;
	if (xl<0 || yu<0 || xr<0 || yd<0){ return false; }
	if (xl>xlim || yd>ylim || xr>xlim || yu>ylim){ return false; }
	return true;
}



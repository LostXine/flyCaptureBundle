#include "FlyCaptureDriver.h"
#include "FlyCaptureUtils.h"
#include <thread>

extern bool b_isRunning;
extern std::vector<FlyCapture2::PGRGuid*> guid;

//主函数
int main(int argc, char** argv)
{
	int iRet = 0;
	//程序初始化
	iRet = FlyCaptureDriverInit(argc, argv);
	if (iRet)
	{
		//异常退出流程
		system("pause");
		return iRet;
	}

	b_isRunning = true;
	std::thread SaveThread(SaveImgThread);
	iRet = FlyCaptureDriverRun(guid);
	b_isRunning = false;
	SaveThread.join();

	if (iRet)
	{
		//异常退出流程
		system("pause");
	}

	//程序结束
	iRet = FlyCaptureDriverFin();
	if (iRet)
	{
		//异常退出流程
		system("pause");
		return iRet;
	}

	return 0;
}
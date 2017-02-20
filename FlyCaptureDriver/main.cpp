#include "FlyCaptureDriver.h"
#include "FlyCaptureUtils.h"
#include <thread>

extern bool b_isRunning;
extern std::vector<FlyCapture2::PGRGuid*> guid;

//������
int main(int argc, char** argv)
{
	int iRet = 0;
	//�����ʼ��
	iRet = FlyCaptureDriverInit(argc, argv);
	if (iRet)
	{
		//�쳣�˳�����
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
		//�쳣�˳�����
		system("pause");
	}

	//�������
	iRet = FlyCaptureDriverFin();
	if (iRet)
	{
		//�쳣�˳�����
		system("pause");
		return iRet;
	}

	return 0;
}
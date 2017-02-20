#include "FlyCaptureCommon.h"
#include <iostream>
#include <sstream>

using namespace FlyCapture2;
using namespace std;

//打印SDK信息的函数
void PrintBuildInfo()
{
	FC2Version fc2Version;
	Utilities::GetLibraryVersion(&fc2Version);

	stringstream version;
	version << "FlyCapture2 library version: " << fc2Version.major << "." << fc2Version.minor << "." << fc2Version.type << "." << fc2Version.build;
	cout << version.str() << endl;

	stringstream timeStamp;
	timeStamp << "Application build date: " << __DATE__ << " " << __TIME__;
	cout << timeStamp.str() << endl << endl;
}

//打印相机信息的函数
void PrintCameraInfo(FlyCapture2::CameraInfo* info)
{
	cout << endl;
	cout << "*** CAMERA INFORMATION ***" << endl;
	cout << "Serial number -" << info->serialNumber << endl;
	cout << "Camera model - " << info->modelName << endl;
	cout << "Camera vendor - " << info->vendorName << endl;
	cout << "Sensor - " << info->sensorInfo << endl;
	cout << "Resolution - " << info->sensorResolution << endl;
	cout << "Firmware version - " << info->firmwareVersion << endl;
	cout << "Firmware build time - " << info->firmwareBuildTime << endl << endl;
}

//打印错误信息
void PrintError(FlyCapture2::Error& error)
{
	error.PrintErrorTrace();
}

//写入相机配置
int CameraSetProerty(Camera* cam, Property * prop)
{
	Error error;
	error = cam->SetProperty(prop);
	//if error->return
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}
	else
	{
		return 0;
	}
}

//检查相机快门状态
bool PollForTriggerReady(FlyCapture2::Camera* cam)
{
	const unsigned int k_softwareTrigger = 0x62C;
	Error error;
	unsigned int regVal = 0;

	do
	{
		error = cam->ReadRegister(k_softwareTrigger, &regVal);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			return false;
		}

	} while ((regVal >> 31) != 0);

	return true;
}

//触发相机拍照
bool FireSoftwareTrigger(FlyCapture2::Camera* cam)
{
	const unsigned int k_softwareTrigger = 0x62C;
	const unsigned int k_fireVal = 0x80000000;
	Error error;

	error = cam->WriteRegister(k_softwareTrigger, k_fireVal);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return false;
	}

	return true;
}
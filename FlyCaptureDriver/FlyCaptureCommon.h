//FlyCapture公共函数

#ifndef FLY_CAPTURE_COMMON
#define FLY_CAPTURE_COMMON

#include <FlyCapture2.h>

//打印SDK信息的函数
void PrintBuildInfo();
//打印相机信息的函数
void PrintCameraInfo(FlyCapture2::CameraInfo* pCamInfo);
//打印错误信息
void PrintError(FlyCapture2::Error& error);
//写入相机配置
int CameraSetProerty(FlyCapture2::Camera* cam, FlyCapture2::Property* prop);
//检查相机快门状态
bool PollForTriggerReady(FlyCapture2::Camera* cam);
//触发相机拍照
bool FireSoftwareTrigger(FlyCapture2::Camera* cam);


#endif
#ifndef FLY_CAPTURE_DRIVER
#define FLY_CAPTURE_DRIVER

#include "FlyCaptureCommon.h"
#include <vector>
#include <iostream>



//程序初始化
int FlyCaptureDriverInit(int argc, char** argv);
//初始化相机配置
void CameraConfigInit(FlyCapture2::Camera* cam);
//主截图程序
int FlyCaptureDriverRun(std::vector<FlyCapture2::PGRGuid*>& guid);
//程序结束
int FlyCaptureDriverFin();
//保存图像线程
int SaveImgThread();

#endif
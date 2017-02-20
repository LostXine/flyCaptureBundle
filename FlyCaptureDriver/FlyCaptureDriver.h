#ifndef FLY_CAPTURE_DRIVER
#define FLY_CAPTURE_DRIVER

#include "FlyCaptureCommon.h"
#include <vector>
#include <iostream>



//�����ʼ��
int FlyCaptureDriverInit(int argc, char** argv);
//��ʼ���������
void CameraConfigInit(FlyCapture2::Camera* cam);
//����ͼ����
int FlyCaptureDriverRun(std::vector<FlyCapture2::PGRGuid*>& guid);
//�������
int FlyCaptureDriverFin();
//����ͼ���߳�
int SaveImgThread();

#endif
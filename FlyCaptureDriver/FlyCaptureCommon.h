//FlyCapture��������

#ifndef FLY_CAPTURE_COMMON
#define FLY_CAPTURE_COMMON

#include <FlyCapture2.h>

//��ӡSDK��Ϣ�ĺ���
void PrintBuildInfo();
//��ӡ�����Ϣ�ĺ���
void PrintCameraInfo(FlyCapture2::CameraInfo* pCamInfo);
//��ӡ������Ϣ
void PrintError(FlyCapture2::Error& error);
//д���������
int CameraSetProerty(FlyCapture2::Camera* cam, FlyCapture2::Property* prop);
//����������״̬
bool PollForTriggerReady(FlyCapture2::Camera* cam);
//�����������
bool FireSoftwareTrigger(FlyCapture2::Camera* cam);


#endif
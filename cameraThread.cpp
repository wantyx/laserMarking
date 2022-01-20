#include "cameraThread.h"
#include <windows.h>
#include"CameraApi.h"
#include <QMessageBox>
#include <qdebug.h>
#include "MvCameraControl.h"
#include "laserMarking.h"
#include "qmessagebox.h"
#include "globalsetting.h"

#define or ||
//SDK使用
extern int                  g_hCamera;          //设备句柄
extern unsigned char        * g_pRawBuffer;     //raw数据
extern unsigned char        * g_pRgbBuffer;     //处理后数据缓存区
extern tSdkFrameHead        g_tFrameHead;       //图像帧头信息
extern tSdkCameraCapbility  g_tCapability;      //设备描述信息
extern BYTE                 *g_readBuf;         //显示数据buffer
extern int                  g_read_fps;         //统计帧率
extern int                  g_SaveImage_type;   //保存图像格式
extern bool                 pause_status;
extern int                  graphicView_width;
extern int					graphicView_height;


void                 * handle;
int                   nRet;
unsigned int          g_nPayloadSize = 0;
bool                  statusFlag = false;

CaptureThread::CaptureThread(QObject *parent):
	QThread(parent)
{
	pause_status = true;
	quit = false;
	
}

bool isColor(int enType) {
	return (enType == PixelType_Gvsp_BGR8_Packed or enType == PixelType_Gvsp_YUV422_Packed
		or enType == PixelType_Gvsp_YUV422_YUYV_Packed or enType == PixelType_Gvsp_BayerGR8
		or enType == PixelType_Gvsp_BayerRG8 or enType == PixelType_Gvsp_BayerGB8
		or enType == PixelType_Gvsp_BayerBG8 or enType == PixelType_Gvsp_BayerGB10
		or enType == PixelType_Gvsp_BayerGB10_Packed or enType == PixelType_Gvsp_BayerBG10
		or enType == PixelType_Gvsp_BayerBG10_Packed or enType == PixelType_Gvsp_BayerRG10
		or enType == PixelType_Gvsp_BayerRG10_Packed or enType == PixelType_Gvsp_BayerGR10
		or enType == PixelType_Gvsp_BayerGR10_Packed or enType == PixelType_Gvsp_BayerGB12
		or enType == PixelType_Gvsp_BayerGB12_Packed or enType == PixelType_Gvsp_BayerBG12
		or enType == PixelType_Gvsp_BayerBG12_Packed or enType == PixelType_Gvsp_BayerRG12
		or enType == PixelType_Gvsp_BayerRG12_Packed or enType == PixelType_Gvsp_BayerGR12
		or enType == PixelType_Gvsp_BayerGR12_Packed or enType == PixelType_Gvsp_RGB8_Packed);
}

bool isMono(int enType) {
	return(enType == PixelType_Gvsp_Mono10 or enType == PixelType_Gvsp_Mono10_Packed
		or enType == PixelType_Gvsp_Mono12 or enType == PixelType_Gvsp_Mono12_Packed
		or enType == PixelType_Gvsp_Mono8);
}

int RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
	if (NULL == pRgbData)
	{
		return MV_E_PARAMETER;
	}

	for (unsigned int j = 0; j < nHeight; j++)
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}

	return MV_OK;
}

bool Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char * pData, cv::Mat &resMat)
{
	cv::Mat srcImage;
	if (isMono(pstImageInfo->enPixelType))
	{
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC1, pData);
	}
	else if (isColor(pstImageInfo->enPixelType))
	{
		RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
	}
	/*MV_CC_PIXEL_CONVERT_PARAM stConvertParam = MV_CC_PIXEL_CONVERT_PARAM();
	memset(&(stConvertParam), 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
	if (isMono(pstImageInfo->enPixelType))
	{
		
	}
	else if (isColor(pstImageInfo->enPixelType))
	{
		int enDstPixelType = PixelType_Gvsp_RGB8_Packed;
		int nConvertDataSize = pstImageInfo->nWidth * pstImageInfo->nHeight * 3;
		stConvertParam.nWidth = pstImageInfo->nWidth;
		stConvertParam.nHeight = pstImageInfo->nHeight;

	}*/
	else
	{
		printf("unsupported pixel format\n");
		return false;
	}

	if (NULL == srcImage.data)
	{
		return false;
	}

	//save converted image in a local file


	resMat = srcImage.clone();
	srcImage.release();

	return true;
}

QImage matToQImage(const cv::Mat& mat)
{
	if (mat.channels() == 1) { // if grayscale image
		return QImage((uchar*)mat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_Grayscale8).copy();
	}
	if (mat.channels() == 3) { // if 3 channel color image
		cv::Mat rgbMat;
		cv::cvtColor(mat, rgbMat, cv::COLOR_BGR2RGB); // invert BGR to RGB
		return QImage((uchar*)rgbMat.data, mat.cols, mat.rows, static_cast<int>(mat.step), QImage::Format_RGB888).copy();
	}
	return QImage();
}

// mat转qimage. 有问题的方法，已弃用
QImage  Mat2QImage(cv::Mat cvImg)//Mat转Qimage
{
	QImage qImg;
	if (cvImg.channels() == 3)                             //三通道彩色图像
	{
		qImg = QImage((const unsigned char*)(cvImg.data), cvImg.cols, cvImg.rows, cvImg.cols*cvImg.channels(), QImage::Format_RGB888);
		cv::cvtColor(cvImg, cvImg, CV_BGR2RGB);
	}
	else if (cvImg.channels() == 1)                    //单通道（灰度图）
	{
		qImg = QImage((const unsigned char*)(cvImg.data), cvImg.cols, cvImg.rows, cvImg.cols*cvImg.channels(), QImage::Format_Indexed8);

		QVector<QRgb> colorTable;
		for (int k = 0; k < 256; ++k)
		{
			colorTable.push_back(qRgb(k, k, k));
		}
		qImg.setColorTable(colorTable);//把qImg的颜色按像素点的颜色给设置
	}
	else
	{
		qImg = QImage((const unsigned char*)(cvImg.data), cvImg.cols, cvImg.rows, cvImg.cols*cvImg.channels(), QImage::Format_RGB888);
	}
	return qImg;

}

// mat转qimage
QImage cvMat2QImage(const cv::Mat& mat)
{
	// 8-bits unsigned, NO. OF CHANNELS = 1
	if (mat.type() == CV_8UC1)
	{
		QImage image(mat.cols, mat.rows, QImage::Format_Indexed8);
		// Set the color table (used to translate colour indexes to qRgb values)
		image.setColorCount(256);
		for (int i = 0; i < 256; i++)
		{
			image.setColor(i, qRgb(i, i, i));
		}
		// Copy input Mat
		uchar* pSrc = mat.data;
		for (int row = 0; row < mat.rows; row++)
		{
			uchar* pDest = image.scanLine(row);
			memcpy(pDest, pSrc, mat.cols);
			pSrc += mat.step;
		}
		return image;
	}
	// 8-bits unsigned, NO. OF CHANNELS = 3
	else if (mat.type() == CV_8UC3)
	{
		// Copy input Mat
		const uchar* pSrc = (const uchar*)mat.data;
		// Create QImage with same dimensions as input Mat
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
		return image.rgbSwapped();
	}
	else if (mat.type() == CV_8UC4)
	{
		// Copy input Mat
		const uchar* pSrc = (const uchar*)mat.data;
		// Create QImage with same dimensions as input Mat
		QImage image(pSrc, mat.cols, mat.rows, mat.step, QImage::Format_ARGB32);
		return image.copy();
	}
	else
	{
		return QImage();
	}
}

//bool CaptureThread::enum_devices() {
//
//}

void CaptureThread::run()
{
	qDebug() << "in CaptureThread::run()" << endl;
	statusFlag = true;

	// Enum device
	MV_CC_DEVICE_INFO_LIST stDeviceList;
	memset(&stDeviceList, 0, sizeof(MV_CC_DEVICE_INFO_LIST));
	nRet = MV_CC_EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &stDeviceList);
	if (MV_OK != nRet)
	{
		printf("Enum Devices fail! nRet [0x%x]\n", nRet);
		return;
	}

	if (stDeviceList.nDeviceNum > 0)
	{
		for (unsigned int i = 0; i < stDeviceList.nDeviceNum; i++)
		{
			printf("[device %d]:\n", i);
			MV_CC_DEVICE_INFO* pDeviceInfo = stDeviceList.pDeviceInfo[i];
			if (NULL == pDeviceInfo)
			{
				break;
			}
			//PrintDeviceInfo(pDeviceInfo);
		}
	}
	else
	{
		printf("Find No Devices!\n");
		//弹出窗口提示信息
		emit(showWrongMessageInCameraThread(1));
		return;
		
	}

	// input the format to convert
	printf("[0] OpenCV_Mat\n");
	printf("[1] OpenCV_IplImage\n");
	printf("Please Input Format to convert:");
	unsigned int nFormat = 0;
	//scanf("%d", &nFormat);
	if (nFormat >= 2)
	{
		printf("Input error!\n");
		return;
	}

	// select device to connect
	printf("Please Input camera index:");
	unsigned int nIndex = 0;
	//scanf("%d", &nIndex);
	if (nIndex >= stDeviceList.nDeviceNum)
	{
		printf("Input error!\n");
		return;
	}
	// Select device and create handle
	nRet = MV_CC_CreateHandle(&handle, stDeviceList.pDeviceInfo[nIndex]);
	if (MV_OK != nRet)
	{
		printf("Create Handle fail! nRet [0x%x]\n", nRet);
		return;
	}

	// open device
	nRet = MV_CC_OpenDevice(handle);
	if (MV_OK != nRet)
	{
		
		printf("Open Device fail! nRet [0x%x]\n", nRet);
		return;
	}

	// Detection network optimal package size(It only works for the GigE camera)
	if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_GIGE_DEVICE)
	{
		int nPacketSize = MV_CC_GetOptimalPacketSize(handle);
		if (nPacketSize > 0)
		{
			nRet = MV_CC_SetIntValue(handle, "GevSCPSPacketSize", nPacketSize);
			if (nRet != MV_OK)
			{
				printf("Warning: Set Packet Size fail nRet [0x%x]!", nRet);
			}
		}
		else
		{
			printf("Warning: Get Packet Size fail nRet [0x%x]!", nPacketSize);
		}
	}
	/*else if (stDeviceList.pDeviceInfo[nIndex]->nTLayerType == MV_USB_DEVICE) 
	{

	}*/

	// Set trigger mode as off
	//nRet = MV_CC_SetEnumValue(handle, "TriggerMode", 0);
	
	if (GlobalSetting::instance()->trigger) {
		setTrigger(1);
	}
	else {
		setTrigger(0);
	}

	if (MV_OK != nRet)
	{
		printf("Set Trigger Mode fail! nRet [0x%x]\n", nRet);
		return;
	}
	nRet = MV_CC_SetFloatValue(handle, "TriggerDelay", time_later*1000000);
	if (MV_OK != nRet)
	{
		printf("Set Trigger Delay fail! nRet [0x%x]\n", nRet);
		return;
	}
	// Get payload size
	MVCC_INTVALUE stParam;
	memset(&stParam, 0, sizeof(MVCC_INTVALUE));
	nRet = MV_CC_GetIntValue(handle, "PayloadSize", &stParam);
	if (MV_OK != nRet)
	{
		printf("Get PayloadSize fail! nRet [0x%x]\n", nRet);
		return;
	}
	g_nPayloadSize = stParam.nCurValue;


	
	// Start grab image
	nRet = MV_CC_StartGrabbing(handle);
	if (MV_OK != nRet)
	{
		printf("Start Grabbing fail! nRet [0x%x]\n", nRet);
		return;
	}

	MV_FRAME_OUT_INFO_EX stImageInfo = { 0 };
	memset(&stImageInfo, 0, sizeof(MV_FRAME_OUT_INFO_EX));
	unsigned char * pData = (unsigned char *)malloc(sizeof(unsigned char) * (g_nPayloadSize));
	if (pData == NULL)
	{
		printf("Allocate memory failed.\n");
		return;
	}
	//MV_CC_ClearImageBuffer(handle);

	while (statusFlag)
	{
		GlobalSetting::instance()->state = true;
		//qDebug() << "in while(statusFlag)" << endl;

		// get one frame from camera with timeout=1000ms
		nRet = MV_CC_GetOneFrameTimeout(handle, pData, g_nPayloadSize, &stImageInfo, 300);
		if (nRet == MV_OK)
		{
			printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
				stImageInfo.nWidth, stImageInfo.nHeight, stImageInfo.nFrameNum);
			
		}
		else
		{
			printf("No data[0x%x]\n", nRet);
			//free(pData);
			//pData = NULL;
			//break;
			Sleep(5);
			continue;
		}
		

		// 数据去转换
		bool bConvertRet = false;
		
		
		cv::Mat resMat;
		/*bConvertRet = Convert2Mat(&stImageInfo, pData, resMat);*/

		/*if (isColor(stImageInfo.enPixelType)) {
			RGB2BGR(pData, stImageInfo.nWidth, stImageInfo.nHeight);
			resMat = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC3, pData);
		}
		else if (isMono(stImageInfo.enPixelType)) {
			resMat = cv::Mat(stImageInfo.nHeight, stImageInfo.nWidth, CV_8UC1, pData);
		}*/
		
		MV_CC_PIXEL_CONVERT_PARAM stConvertParam = MV_CC_PIXEL_CONVERT_PARAM();
		memset(&(stConvertParam), 0, sizeof(MV_CC_PIXEL_CONVERT_PARAM));
		if (isMono(stImageInfo.enPixelType))
		{
			unsigned char* pDataForGRAY = (unsigned char*)malloc(stImageInfo.nWidth * stImageInfo.nHeight * 1);

			if (NULL == pDataForGRAY)
			{
				printf("malloc pDataForGRAY fail !\n");
				break;
			}

			auto nConvertDataSize = stImageInfo.nWidth * stImageInfo.nHeight * 1;
			stConvertParam.nWidth = stImageInfo.nWidth;
			stConvertParam.nHeight = stImageInfo.nHeight;
			stConvertParam.pSrcData = pData;
			stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;
			stConvertParam.enSrcPixelType = stImageInfo.enPixelType;
			stConvertParam.enDstPixelType = PixelType_Gvsp_Mono8;
			stConvertParam.pDstBuffer = pDataForGRAY;
			stConvertParam.nDstBufferSize = nConvertDataSize;
			nRet = MV_CC_ConvertPixelType(handle, &stConvertParam);
			if (nRet != 0) {
				continue;
			}
			bConvertRet = Convert2Mat(&stImageInfo, stConvertParam.pDstBuffer, resMat);
			free(pDataForGRAY);
		}
		else if (isColor(stImageInfo.enPixelType))
		{
			
			//stImageInfo.enPixelType = PixelType_Gvsp_RGB8_Packed;
			unsigned char* pDataForRGB = (unsigned char*)malloc(stImageInfo.nWidth * stImageInfo.nHeight * 3);
			if (NULL == pDataForRGB)
			{
				printf("malloc pDataForRGB fail !\n");
				break;
			}

			auto nConvertDataSize = stImageInfo.nWidth * stImageInfo.nHeight * 3;
			stConvertParam.nWidth = stImageInfo.nWidth;
			stConvertParam.nHeight = stImageInfo.nHeight;
			stConvertParam.pSrcData = pData;
			stConvertParam.nSrcDataLen = stImageInfo.nFrameLen;
			stConvertParam.enSrcPixelType = stImageInfo.enPixelType;
			stConvertParam.enDstPixelType = PixelType_Gvsp_RGB8_Packed;
			stConvertParam.pDstBuffer = pDataForRGB;
			stConvertParam.nDstBufferSize = nConvertDataSize;
			nRet = MV_CC_ConvertPixelType(handle,&stConvertParam);
			if (nRet != 0) {
				continue;
			}
			bConvertRet = Convert2Mat(&stImageInfo, stConvertParam.pDstBuffer, resMat);
			free(pDataForRGB);
		}
		
		// print result
		if (bConvertRet)
		{
			printf("OpenCV format convert finished.\n");
		}
		else
		{
			printf("OpenCV format convert failed.\n");
			//break;
			continue;
		}
		/*cv::imwrite("2.png", resMat);*/
		QImage img = cvMat2QImage(resMat);
		/*img.save("3.png");*/
		//qDebug() << "in camera Thread=>ming:" << GlobalSetting::instance()->ming<<";it seems ok!";
		if (GlobalSetting::instance()->ming) {
			emit captured(img);
		}
		
	}
}

void CaptureThread::stop()
{
	pause_status = true;
	quit = true;
	

	if (statusFlag == true){
		statusFlag = false;
		// Stop grab image
		nRet = MV_CC_StopGrabbing(handle);
		if (MV_OK != nRet)
		{
			printf("Stop Grabbing fail! nRet [0x%x]\n", nRet);

		}

		// Close device
		nRet = MV_CC_CloseDevice(handle);
		if (MV_OK != nRet)
		{
			printf("ClosDevice fail! nRet [0x%x]\n", nRet);

		}

		// Destroy handle
		nRet = MV_CC_DestroyHandle(handle);
		if (MV_OK != nRet)
		{
			printf("Destroy Handle fail! nRet [0x%x]\n", nRet);

		}
	}
}

bool CaptureThread::setExposureTime(float time)
{
	int result = MV_CC_SetFloatValue(handle, "ExposureTime", time);
	return (result == MV_OK);
}

//0是连续
//1是触发
bool CaptureThread::setTrigger(int mode)
{
	return MV_CC_SetEnumValue(handle, "TriggerMode", mode);
}


CaptureThread::~CaptureThread()
{
	stop();
}
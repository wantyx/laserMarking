#ifndef DATA_H
#define DATA_H

/********************************************************************************* 
  *FileName:  data.h
  *Author:  lzb
  *Version:  1.0
  *Date:  
  *Description:  ��Ŀ���õ��Ľṹ��
**********************************************************************************/ 

#include <vector>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
//using namespace cv;

extern double my_scaleValue;//���ű���ȫ�ֱ���
//double my_scaleValue = 1.0;

#pragma pack(push,1)
struct MosaicInfo
{
	int mode;
	int rows;
	int cols;
	int edge;
	double percentage;

	void setMosaicInfo(int m_mode, int m_rows, int m_cols, int m_edge, double m_percentage){
		mode = m_mode;
		rows = m_rows;
		cols = m_cols;
		edge = m_edge;
		percentage = m_percentage;
	}
};
#pragma pack(pop)

#pragma pack(push,1)
struct DisImage
{
	int rows = 0;
	int cols = 0;
	cv::Mat img;
};
#pragma pack(pop)

/*
@param x x����
@param y y����
@param xCut ͼƬ�󲿲ü��˶�����
@param yCut ͼƬ�ϲ��ü��˶�����
*/
struct ImageInfo
{
	int x=0;
	int y=0;
	int xCut=0;
	int yCut=0;
	string name="";
	bool good=true;

	void ClearData(){
		x = 0;
		y = 0;
		xCut = 0;
		yCut = 0;
		name = "";
	}
};

/*
* ͼ������
@param x x����
@param y y����
@param name ͼƬ��
@param good �Ƿ�������
*/
struct ImageCoord
{
	int x = 0;
	int y = 0;
	string name = "";
	bool good = true;

	void ClearData(){
		x = 0;
		y = 0;
		name = "";
	}
};


struct InitValue
{
	int rows;
	int cols;
	int oEdge;
	int oPercentage;
	int hEdge;
	int hPercentage;
	string path;
};

struct DlgSetValue{
	int height = 0;// ��ͼ��
	int width = 0; // ��ͼ��
	int cols = 0; // ƴ��������
	int rows = 0; // ƴ��������
	int oEdge = 0; // ����Ч�ߴ�
	double oPercentage = 0; // �����ص���
	int hEdge = 0; // ����Ч�ߴ�
	double hPercentage = 0; //�����ص���
	int rate = 0; // �ֱ���ϵ��
	string filePath = "";

	int h_xoffset = 0,h_yoffset = 0; // ����ƴ��ʱ��x��y����ƫ����
	int v_xoffset = 0, v_yoffset = 0; // ����ƴ��ʱ��x��y����ƫ����

	double cutwidth_hor = 0;
	double cutwidth_ver = 0;

	double minVal = 0.;//���ù���ʱ�������С����

	void setValue(int m_oEdge, double m_oPercentage, int m_hEdge, double m_hPercentage, int m_rate, string m_filePath){
		oEdge = m_oEdge;
		oPercentage = m_oPercentage;
		hEdge = m_hEdge;
		hPercentage = m_hPercentage;
		rate = m_rate;
		filePath = m_filePath;
	}

	void setValue(int m_oEdge, double m_oPercentage, int m_hEdge, double m_hPercentage, int m_rate){
		oEdge = m_oEdge;
		oPercentage = m_oPercentage;
		hEdge = m_hEdge;
		hPercentage = m_hPercentage;
		rate = m_rate;
	}

	void clear(){
		oEdge = 0;
		oPercentage = 0;
		hEdge = 0;
		hPercentage = 0;
		rate = 0;
		filePath = "";
		h_xoffset = 0;
		h_yoffset = 0;
		v_xoffset = 0;
		v_yoffset = 0;

		cutwidth_hor = 0;
		cutwidth_ver = 0;

		minVal = 0.;
	}

	DlgSetValue operator=(const DlgSetValue& obj){
		this->height = obj.height;
		this->width = obj.width;
		this->cols = obj.cols;
		this->rows = obj.rows;
		this->oEdge = obj.oEdge;
		this->oPercentage = obj.oPercentage;
		this->hEdge = obj.hEdge;
		this->hPercentage = obj.hPercentage;
		this->rate = obj.rate;
		this->filePath = obj.filePath;
		this->h_xoffset = obj.h_xoffset;
		this->h_yoffset = obj.h_yoffset;
		this->v_xoffset = obj.v_xoffset;
		this->v_yoffset = obj.v_yoffset;

		this->cutwidth_hor = obj.cutwidth_hor;
		this->cutwidth_ver = obj.cutwidth_ver;

		this->minVal = obj.minVal;

		return *this;
	}
};

struct mPoint{
	int x;
	int y;
	/*mPoint(int m_x, int m_y){
		x = m_x;
		y = m_y;
	}*/
};


struct Zoom
{
	float rate;
	float originScale;
	vector<float> size;

};

struct ErrorImage
{
	int col;
	int row;
	bool error=false;
	string name;
	double ratio;
};

//ͼƬ�ƶ���¼
struct MoveRecord{
	int row;
	int col;
	int mode;//1 ��ͼ��  2 ��ͼ1��  3 ��ͼ2 4 �ƶ����
	int distanceX;//x����
	int directionX;//0δ�ƶ� 1�ƶ�
	int distanceY;//y����
	int directionY;//0δ�ƶ� 1�ƶ�
};
#endif // DATA_H

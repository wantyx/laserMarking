#ifndef DATA_H
#define DATA_H

/********************************************************************************* 
  *FileName:  data.h
  *Author:  lzb
  *Version:  1.0
  *Date:  
  *Description:  项目中用到的结构体
**********************************************************************************/ 

#include <vector>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;
//using namespace cv;

extern double my_scaleValue;//缩放比例全局变量
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
@param x x坐标
@param y y坐标
@param xCut 图片左部裁剪了多少列
@param yCut 图片上部裁剪了多少行
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
* 图像坐标
@param x x坐标
@param y y坐标
@param name 图片名
@param good 是否可能误差
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
	int height = 0;// 子图高
	int width = 0; // 子图宽
	int cols = 0; // 拼接总列数
	int rows = 0; // 拼接总行数
	int oEdge = 0; // 上无效尺寸
	double oPercentage = 0; // 左右重叠比
	int hEdge = 0; // 左无效尺寸
	double hPercentage = 0; //上下重叠比
	int rate = 0; // 分辨率系数
	string filePath = "";

	int h_xoffset = 0,h_yoffset = 0; // 左右拼接时，x和y方向偏移量
	int v_xoffset = 0, v_yoffset = 0; // 上下拼接时，x和y方向偏移量

	double cutwidth_hor = 0;
	double cutwidth_ver = 0;

	double minVal = 0.;//设置功能时保存的最小分数

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

//图片移动记录
struct MoveRecord{
	int row;
	int col;
	int mode;//1 单图，  2 多图1，  3 多图2 4 移动左侧
	int distanceX;//x方向
	int directionX;//0未移动 1移动
	int distanceY;//y方向
	int directionY;//0未移动 1移动
};
#endif // DATA_H

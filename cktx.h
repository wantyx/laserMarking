// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef CKTX_H
#define CKTX_H

#ifdef  CHUANKOU
#define CHUANKOU extern "C" _declspec(dllimport)
#else
#define CHUANKOU extern "C" _declspec(dllexport)
#endif


// 添加要在此处预编译的标头
//#include "framework.h"
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件

#undef UNICODE
#include <process.h>    
#include "TChar.h"
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <Windows.h>
#include<time.h>
#include<fstream>
using namespace std;
/** 串口通信类
*
*  本类实现了对串口的基本操作
*  例如监听发到指定串口的数据、发送指定数据到串口
*/

typedef enum AxisType
{
	AXIS_XX = 2,
	AXIS_YY = 3,
	AXIS_ZZ = 1,
	AXIS_OO = 4,
}AXIS_TYPE;


class CSerialPort
{
public:
	CSerialPort(void);
	~CSerialPort(void);

public:
	bool readFlag = false;
	UINT BytesInQue_buffer;
	bool receive(int* temp, int start, int time);
	bool send(unsigned char* pData);
	/** 初始化串口函数
	*
	*  @param:  UINT portNo 串口编号,默认值为1,即COM1,注意,尽量不要大于9
	*  @param:  UINT baud   波特率,默认为9600
	*  @param:  char parity 是否进行奇偶校验,'Y'表示需要奇偶校验,'N'表示不需要奇偶校验
	*  @param:  UINT databits 数据位的个数,默认值为8个数据位
	*  @param:  UINT stopsbits 停止位使用格式,默认值为1
	*  @param:  DWORD dwCommEvents 默认为EV_RXCHAR,即只要收发任意一个字符,则产生一个事件
	*  @return: bool  初始化是否成功
	*  @note:   在使用其他本类提供的函数前,请先调用本函数进行串口的初始化
	*　　　　　   /n本函数提供了一些常用的串口参数设置,若需要自行设置详细的DCB参数,可使用重载函数
	*           /n本串口类析构时会自动关闭串口,无需额外执行关闭串口
	*  @see:
	*/
	//bool InitPort(UINT  portNo = 3, UINT  baud = CBR_19200, char  parity = 'N', UINT  databits = 8, UINT  stopsbits = 1, DWORD dwCommEvents = EV_RXCHAR);
	bool InitPort(UINT  portNo, UINT  baud, char  parity, UINT  databits, UINT  stopsbits, DWORD dwCommEvents);


	/** 串口初始化函数
	*
	*  本函数提供直接根据DCB参数设置串口参数
	*  @param:  UINT portNo
	*  @param:  const LPDCB & plDCB
	*  @return: bool  初始化是否成功
	*  @note:   本函数提供用户自定义地串口初始化参数
	*  @see:
	*/
	bool InitPort(UINT  portNo, const LPDCB& plDCB);

	/** 开启监听线程
	*
	*  本监听线程完成对串口数据的监听,并将接收到的数据打印到屏幕输出
	*  @return: bool  操作是否成功
	*  @note:   当线程已经处于开启状态时,返回flase
	*  @see:
	*/
	bool OpenListenThread();

	/** 关闭监听线程
	*
	*
	*  @return: bool  操作是否成功
	*  @note:   调用本函数后,监听串口的线程将会被关闭
	*  @see:
	*/
	bool CloseListenTread();

	/** 向串口写数据
	*
	*  将缓冲区中的数据写入到串口
	*  @param:  unsigned char * pData 指向需要写入串口的数据缓冲区
	*  @param:  unsigned int length 需要写入的数据长度
	*  @return: bool  操作是否成功
	*  @note:   length不要大于pData所指向缓冲区的大小
	*  @see:
	*/
	bool WriteData(unsigned char* pData, int length);

	/** 获取串口缓冲区中的字节数
	*
	*
	*  @return: UINT  操作是否成功
	*  @note:   当串口缓冲区中无数据时,返回0
	*  @see:
	*/
	UINT GetBytesInCOM();
	/*UINT WriteData1(unsigned long long int *pData1, unsigned int length);*/

	/** 读取串口接收缓冲区中一个字节的数据
	*
	*
	*  @param:  char & cRecved 存放读取数据的字符变量
	*  @return: bool  读取是否成功
	*  @note:
	*  @see:
	*/
	//bool ReadChar(unsigned char &cRecved,int byteNumber);
	bool ReadChar(unsigned char* cRecved, int byteNumber);

	unsigned char* MotorMoveXY(unsigned char x, unsigned char y);//xy相对移动
	unsigned char* StopMotor(unsigned char sm1);
	unsigned char* SetSpeed(AXIS_TYPE enAxisType, int speed);
	unsigned char* SetRunSpeed(int TY, int speed);
private:

	/** 打开串口
	*
	*
	*  @param:  UINT portNo 串口设备号
	*  @return: bool  打开是否成功
	*  @note:
	*  @see:
	*/
	bool openPort(UINT  portNo);

	/** 关闭串口
	*
	*
	*  @return: void  操作是否成功
	*  @note:
	*  @see:
	*/
	void ClosePort();

	/** 串口监听线程
	*
	*  监听来自串口的数据和信息
	*  @param:  void * pParam 线程参数
	*  @return: UINT WINAPI 线程返回值
	*  @note:
	*  @see:
	*/
	static UINT WINAPI ListenThread(void* pParam);

private:

	/** 串口句柄 */
	HANDLE  m_hComm;

	/** 线程退出标志变量 */
	static bool s_bExit;

	/** 线程句柄 */
	volatile HANDLE    m_hListenThread;

	/** 同步互斥,临界区保护 */
	CRITICAL_SECTION   m_csCommunicationSync;       //!< 互斥操作串口  


};

extern "C" _declspec(dllexport) int sendOpenSignal(int port);
extern "C" _declspec(dllexport) int sendCloseSignal(int port);
extern "C" _declspec(dllexport) int sendSignalToPLC(int port);

//int sendAndAccept(CSerialPort mySerialPort, unsigned char* pData, int length, int time);


#endif //PCH_H

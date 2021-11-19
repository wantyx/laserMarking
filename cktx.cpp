
//#include "stdafx.h"

//  
/// COPYRIGHT NOTICE  
/// Copyright (c) 2009, 华中科技大学tickTick Group  （版权声明）  
/// All rights reserved.  
///   
/// @file    SerialPort.cpp    
/// @brief   串口通信类的实现文件  
///  
/// 本文件为串口通信类的实现代码  
///  
/// @version 1.0     
/// @author  卢俊    
/// @E-mail：lujun.hust@gmail.com  
/// @date    2010/03/19  
///   
///  
///  修订说明：  
//  

//#include "StdAfx.h"  
#include "cktx.h"  
using namespace std;
/** 线程退出标志 */
bool CSerialPort::s_bExit = false;
/** 当串口无数据时,sleep至下次查询间隔的时间,单位:秒 */
const UINT SLEEP_TIME_INTERVAL = 5;

CSerialPort::CSerialPort(void)
	: m_hListenThread(INVALID_HANDLE_VALUE)
{
	m_hComm = INVALID_HANDLE_VALUE;
	m_hListenThread = INVALID_HANDLE_VALUE;
	InitializeCriticalSection(&m_csCommunicationSync);
}

CSerialPort::~CSerialPort(void)
{
	CloseListenTread();
	ClosePort();
	DeleteCriticalSection(&m_csCommunicationSync);
}

//初始化串口函数
bool CSerialPort::InitPort(UINT portNo /*= 1*/, UINT baud /*= CBR_9600*/, char parity /*= 'N'*/,
	UINT databits /*= 8*/, UINT stopsbits /*= 1*/, DWORD dwCommEvents /*= EV_RXCHAR*/)
{

	/** 临时变量,将制定参数转化为字符串形式,以构造DCB结构 */
	char szDCBparam[50];
	sprintf_s(szDCBparam, "baud=%d parity=%c data=%d stop=%d", baud, parity, databits, stopsbits);

	/** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
	if (!openPort(portNo))
	{
		return false;
	}

	/** 进入临界段 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 是否有错误发生 */
	BOOL bIsSuccess = TRUE;

	/** 在此可以设置输入输出的缓冲区大小,如果不设置,则系统会设置默认值.
	*  自己设置缓冲区大小时,要注意设置稍大一些,避免缓冲区溢出
	*/
	/*if (bIsSuccess )
	{
	bIsSuccess = SetupComm(m_hComm,10,10);
	}*/

	/** 设置串口的超时时间,均设为0,表示不使用超时限制 */
	COMMTIMEOUTS  CommTimeouts;
	CommTimeouts.ReadIntervalTimeout = 0;
	CommTimeouts.ReadTotalTimeoutMultiplier = 0;
	CommTimeouts.ReadTotalTimeoutConstant = 0;
	CommTimeouts.WriteTotalTimeoutMultiplier = 0;
	CommTimeouts.WriteTotalTimeoutConstant = 0;
	if (bIsSuccess)
	{
		bIsSuccess = SetCommTimeouts(m_hComm, &CommTimeouts);
	}

	DCB  dcb;
	if (bIsSuccess)
	{
		//DWORD dwNum = MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, NULL, 0);
		//wchar_t *pwText = new wchar_t[dwNum];
		//if (!MultiByteToWideChar(CP_ACP, 0, szDCBparam, -1, pwText, dwNum))
		//{
		//	bIsSuccess = TRUE;
		//}

		/** 获取当前串口配置参数,并且构造串口DCB参数 */
		bIsSuccess = GetCommState(m_hComm, &dcb) && BuildCommDCB(szDCBparam, &dcb);
		/** 开启RTS flow控制 */
		dcb.fRtsControl = RTS_CONTROL_ENABLE;

		/** 释放内存空间 */
		/*delete[] pwText;*/
	}

	if (bIsSuccess)
	{
		/** 使用DCB参数配置串口状态 */
		bIsSuccess = SetCommState(m_hComm, &dcb);
	}

	/**  清空串口缓冲区 */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** 离开临界段 */
	LeaveCriticalSection(&m_csCommunicationSync);

	return bIsSuccess == TRUE;
}

//初始化串口函数
bool CSerialPort::InitPort(UINT portNo, const LPDCB& plDCB)
{
	/** 打开指定串口,该函数内部已经有临界区保护,上面请不要加保护 */
	if (!openPort(portNo))
	{
		return false;
	}

	/** 进入临界段 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 配置串口参数 */
	if (!SetCommState(m_hComm, plDCB))
	{
		return false;
	}

	/**  清空串口缓冲区 */
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

	/** 离开临界段 */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

//关闭关闭串口
void CSerialPort::ClosePort()
{
	/** 如果有串口被打开，关闭它 */
	if (m_hComm != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hComm);
		m_hComm = INVALID_HANDLE_VALUE;
	}
}

//打开出串口
bool CSerialPort::openPort(UINT portNo)
{
	/** 进入临界段 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 把串口的编号转换为设备名 */
	char szPort[50];
	sprintf_s(szPort, "COM%d", portNo);

	/** 打开指定的串口 */
	m_hComm = CreateFileA(szPort,  /** 设备名,COM1,COM2等 */
		GENERIC_READ | GENERIC_WRITE, /** 访问模式,可同时读写 */
		0,                            /** 共享模式,0表示不共享 */
		NULL,                         /** 安全性设置,一般使用NULL */
		OPEN_EXISTING,                /** 该参数表示设备必须存在,否则创建失败 */
		0,
		0);

	/** 如果打开失败，释放资源并返回 */
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		LeaveCriticalSection(&m_csCommunicationSync);
		return false;
	}

	/** 退出临界区 */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}

//打开监听线程
bool CSerialPort::OpenListenThread()
{
	/** 检测线程是否已经开启了 */
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** 线程已经开启 */
		return false;
	}
	s_bExit = false;
	/** 线程ID */
	UINT threadId;
	/** 开启串口数据监听线程 */
	m_hListenThread = (HANDLE)_beginthreadex(NULL, 0, ListenThread, this, 0, &threadId);
	if (!m_hListenThread)
	{
		return false;
	}
	/** 设置线程的优先级,高于普通线程 */
	if (!SetThreadPriority(m_hListenThread, THREAD_PRIORITY_ABOVE_NORMAL))
	{
		return false;
	}

	return true;
}
//关闭监听线程
bool CSerialPort::CloseListenTread()
{
	if (m_hListenThread != INVALID_HANDLE_VALUE)
	{
		/** 通知线程退出 */
		s_bExit = true;

		/** 等待线程退出 */
		Sleep(10);

		/** 置线程句柄无效 */
		CloseHandle(m_hListenThread);
		m_hListenThread = INVALID_HANDLE_VALUE;
	}
	return true;
}
//获取串口缓冲区的字节数
UINT CSerialPort::GetBytesInCOM()
{
	DWORD dwError = 0;  /** 错误码 */
	COMSTAT  comstat;   /** COMSTAT结构体,记录通信设备的状态信息 */
	memset(&comstat, 0, sizeof(COMSTAT));

	BytesInQue_buffer = 0;
	/** 在调用ReadFile和WriteFile之前,通过本函数清除以前遗留的错误标志 */
	if (ClearCommError(m_hComm, &dwError, &comstat))
	{
		BytesInQue_buffer = comstat.cbInQue; /** 获取在输入缓冲区中的字节数 */
	}

	return BytesInQue_buffer;
}

/*
 * 将字符转换为数值
 * */
int c2i(char ch)
{
	// 如果是数字，则用数字的ASCII码减去48, 如果ch = '2' ,则 '2' - 48 = 2
	if (isdigit(ch))
		return ch - 48;

	// 如果是字母，但不是A~F,a~f则返回
	if (ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z')
		return -1;

	// 如果是大写字母，则用数字的ASCII码减去55, 如果ch = 'A' ,则 'A' - 55 = 10
	// 如果是小写字母，则用数字的ASCII码减去87, 如果ch = 'a' ,则 'a' - 87 = 10
	if (isalpha(ch))
		return isupper(ch) ? ch - 55 : ch - 87;

	return -1;
}

/*
 * 功能：将十六进制字符串转换为整型(int)数值
 * */
int hex2dec(const char* hex)
{
	int len;
	int num = 0;
	int temp;
	int bits;
	int i;

	// 此例中 hex = "1de" 长度为3, hex是main函数传递的
	len = strlen(hex);

	for (i = 0, temp = 0; i < len; i++, temp = 0)
	{
		// 第一次：i=0, *(hex + i) = *(hex + 0) = '1', 即temp = 1
		// 第二次：i=1, *(hex + i) = *(hex + 1) = 'd', 即temp = 13
		// 第三次：i=2, *(hex + i) = *(hex + 2) = 'd', 即temp = 14
		temp = c2i(*(hex + i));
		// 总共3位，一个16进制位用 4 bit保存
		// 第一次：'1'为最高位，所以temp左移 (len - i -1) * 4 = 2 * 4 = 8 位
		// 第二次：'d'为次高位，所以temp左移 (len - i -1) * 4 = 1 * 4 = 4 位
		// 第三次：'e'为最低位，所以temp左移 (len - i -1) * 4 = 0 * 4 = 0 位
		bits = (len - i - 1) * 4;
		temp = temp << bits;

		// 此处也可以用 num += temp;进行累加
		num = num | temp;
	}

	// 返回结果
	return num;
}

bool CSerialPort::receive(int* read_buffer, int start, int time)
{
	// 线程循环,轮询方式读取串口数据  
	int end = clock();
	while (!s_bExit && (end - start) < time)
	{
		UINT BytesInQue = GetBytesInCOM();
		/** 如果串口输入缓冲区中无数据,则休息一会再查询 */
		end = clock();
		if (BytesInQue != _msize(read_buffer) / sizeof(int)-1)
		{
			//Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}

		/** 读取输入缓冲区中的数据并输出显示 */
		unsigned char cRecved = 0x00;
		unsigned char* buffer = new unsigned char[BytesInQue];
		//read_buffer = new unsigned char[BytesInQue];
		int bufferLength = BytesInQue;

		cRecved = 0x00;
		if (ReadChar(buffer, bufferLength) == true)
		{
			
			//cout << "接收的数据是：" << endl;
			do
			{
				//cout << cRecved << endl;
				std::stringstream  ss;
				int currentRead = bufferLength - BytesInQue;
				int tm = buffer[currentRead];
				ss << std::hex << std::setw(2) << std::setfill('0') << tm;
				//ss << " ";
				string a = ss.str();
				string b;
				transform(a.begin(), a.end(), back_inserter(b), ::toupper);
				//cout << b << " ";
				
				read_buffer[currentRead] = hex2dec(b.c_str());// (unsigned char)("0x" + b).c_str();

				continue;
			} while (--BytesInQue);
			
		}
		//cout << endl;
		readFlag = true;
		return true;
	}
	PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
	return false;
}


bool CSerialPort::send(unsigned char* pData)
{
		int* read_buffer = new int[_msize(pData) / sizeof(unsigned char)];
		if (!WriteData(pData, _msize(pData) / sizeof(unsigned char)))
		{
			return false;//发送失败
		}
		return true;
}

//串口监听线程
UINT WINAPI CSerialPort::ListenThread(void* pParam)
{
	/** 得到本类的指针 */
	CSerialPort* pSerialPort = reinterpret_cast<CSerialPort*>(pParam);

	// 线程循环,轮询方式读取串口数据  
	while (!pSerialPort->s_bExit)
	{
		UINT BytesInQue = pSerialPort->GetBytesInCOM();
		/** 如果串口输入缓冲区中无数据,则休息一会再查询 */
		if (BytesInQue == 0)
		{
			//Sleep(SLEEP_TIME_INTERVAL);
			continue;
		}

		/** 读取输入缓冲区中的数据并输出显示 */
		unsigned char cRecved = 0x00;
		unsigned char* buffer = new unsigned char[BytesInQue];
		int bufferLength = BytesInQue;

		cRecved = 0x00;
		if (pSerialPort->ReadChar(buffer, bufferLength) == true)
		{
			do
			{
				cout << cRecved << endl;
				std::stringstream  ss;
				int currentRead = bufferLength - BytesInQue;
				int tm = buffer[currentRead];
				ss << std::hex << std::setw(2) << std::setfill('0') << tm;
				//ss << " ";
				string a = ss.str();
				string b;
				transform(a.begin(), a.end(), back_inserter(b), ::toupper);
				cout << b;
				continue;
			} while (--BytesInQue);
		}

	}

	return 0;
}
//读取串口接收缓冲区中一个字节的数据
//bool CSerialPort::ReadChar(unsigned char &cRecved,int byteNumber)
//{
//	BOOL  bResult = TRUE;
//	DWORD BytesRead = 0;
//	if (m_hComm == INVALID_HANDLE_VALUE)
//	{
//		return false;
//	}
//
//	/** 临界区保护 */
//	EnterCriticalSection(&m_csCommunicationSync);
//
//	/** 从缓冲区读取一个字节的数据 */
//	bResult = ReadFile(m_hComm, &cRecved, byteNumber, &BytesRead, NULL);
//	if ((!bResult))
//	{
//		/** 获取错误码,可以根据该错误码查出错误原因 */
//		DWORD dwError = GetLastError();
//
//		/** 清空串口缓冲区 */
//		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
//		LeaveCriticalSection(&m_csCommunicationSync);
//
//		return false;
//	}
//
//	/** 离开临界区 */
//	LeaveCriticalSection(&m_csCommunicationSync);
//
//	return (BytesRead == 3);
//
//}

bool CSerialPort::ReadChar(unsigned char* cRecved, int byteNumber)
{
	BOOL  bResult = TRUE;
	DWORD BytesRead = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/** 临界区保护 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 从缓冲区读取一个字节的数据 */
	bResult = ReadFile(m_hComm, cRecved, byteNumber, &BytesRead, NULL);
	if ((!bResult))
	{
		/** 获取错误码,可以根据该错误码查出错误原因 */
		DWORD dwError = GetLastError();

		/** 清空串口缓冲区 */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** 离开临界区 */
	LeaveCriticalSection(&m_csCommunicationSync);

	return (BytesRead == byteNumber);

}

// 向串口写数据, 将缓冲区中的数据写入到串口
bool CSerialPort::WriteData(unsigned char* pData, int length)
{
	int* pData1 = new int;
	BOOL   bResult = TRUE;
	DWORD  BytesToSend = 0;
	if (m_hComm == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	/** 临界区保护 */
	EnterCriticalSection(&m_csCommunicationSync);

	/** 向缓冲区写入指定量的数据 */
	bResult = WriteFile(m_hComm,/*文件句柄*/pData,/*用于保存读入数据的一个缓冲区*/ length,/*要读入的字符数*/ &BytesToSend,/*指向实际读取字节数的指针*/ NULL);
	if (!bResult)
	{
		DWORD dwError = GetLastError();
		/** 清空串口缓冲区 */
		PurgeComm(m_hComm, PURGE_RXCLEAR | PURGE_RXABORT);
		LeaveCriticalSection(&m_csCommunicationSync);

		return false;
	}

	/** 离开临界区 */
	LeaveCriticalSection(&m_csCommunicationSync);

	return true;
}


int sendOpenSignal(int port) {
	int result;
	CSerialPort mySerialPort;//首先将之前定义的类实例化
	unsigned char* temp = new unsigned char[4];//动态创建一个数组
	if (!mySerialPort.InitPort(port, CBR_9600, 'N', 8, 1, EV_RXCHAR)){
		std::cout << "初始化失败 !" << std::endl;
		return 1;
	}
	else{
		std::cout << "初始化成功!" << std::endl;
	}
	temp[0] = 0x45; temp[1] = 0x53; temp[2] = 0x02; temp[3] = 0x50;
	result = mySerialPort.send(temp);
	if (result){
		return 0;
	}
	else {
		return -1;
	}
}

int sendCloseSignal(int port) {
	int result;
	CSerialPort mySerialPort;//首先将之前定义的类实例化
	unsigned char* temp = new unsigned char[4];//动态创建一个数组
	if (!mySerialPort.InitPort(port, CBR_9600, 'N', 8, 1, EV_RXCHAR)) {
		std::cout << "初始化失败 !" << std::endl;
		return 1;
	}
	else {
		std::cout << "初始化成功!" << std::endl;
	}
	temp[0] = 0x45; temp[1] = 0x53; temp[2] = 0x01; temp[3] = 0x50;
	result = mySerialPort.send(temp);
	if (result) {
		return 0;
	}
	else {
		return -1;
	}
}

int sendSignalToPLC(int port) {
	int result;
	CSerialPort mySerialPort;//首先将之前定义的类实例化
	unsigned char* temp = new unsigned char[4];//动态创建一个数组
	if (!mySerialPort.InitPort(port, CBR_9600, 'N', 8, 1, EV_RXCHAR)) {
		std::cout << "初始化失败 !" << std::endl;
		return 1;
	}
	else {
		std::cout << "初始化成功!" << std::endl;
	}
	temp[0] = 0x45; temp[1] = 0x53; temp[2] = 0x03; temp[3] = 0x50;
	result = mySerialPort.send(temp);
	if (result) {
		return 0;
	}
	else {
		return -1;
	}
}



#ifndef __COMM_H__
#define __COMM_H__

#include "M3USB.H"
#include "com_class.h"

#define MAX_PACKET_LENGTH 4096
#define MIN_PACKET_LENGTH	64
#define OVER_TIME			10

#define COM_BUFFER_SIZE 1024*1024

 typedef enum
 {
	 ACK_FAIL = 0,
	 ACK_SUCCESS = 1,
	 ACK_TEST = 0xff
 }T_ACK_TYPE;

#pragma pack(1)

typedef struct 
{
	BYTE type;//
	WORD  packet_length;//
	char data[MAX_PACKET_LENGTH];//
}T_TRANSC_PACKET;//

#pragma pack()


class IComm
{
public:
	virtual BOOL Read(BYTE buf[], int size, int *upload_count) = 0;
	virtual BOOL Write(BYTE buf[], int size) = 0;

	virtual BOOL write_transc_packet(BYTE transc_type, BYTE data[], int data_length) = 0;//write_transc_packet
	virtual BYTE read_transc_ack() = 0;//read_transc_ack
};


class CCommUSB: public IComm
{
public:
	CCommUSB();
	~CCommUSB();

public:
	CString		m_dbcc_name;

protected:
	CM3USBC		m_usb;
	BOOL		m_bOpen;

//inherite from IComm
public:
	BOOL Read(BYTE buf[], int size, int *upload_count);//Read
	BOOL Write(BYTE buf[], int size);//Write

	BOOL write_transc_packet(BYTE transc_type, BYTE data[], int data_length);//write_transc_packet
	BYTE read_transc_ack();//read_transc_ack
	
//other
public:
	BOOL Open(CString usb_dbcc_name);//Open
	BOOL Close();//Close
	BOOL isOpen();//isOpen


	BOOL download_data(CString file_path);
};

class CCommCOM : public IComm
{
public:
	CCommCOM();
	CCommCOM(HWND hwnd);
	~CCommCOM();

protected:
	_thread_com	m_com;
	HWND m_hwnd;

public:
	char m_buf[COM_BUFFER_SIZE];//m_buf
	int m_receive_len;//m_receive_len

public:
	BOOL Open(int port, int baud_rate);//Open
	BOOL Close();

	void SetHWND(HWND hwnd);

	BOOL isOpen();

	BOOL SetDCB(int baud_rate, int byte_size = 9, 
		int parity = NOPARITY, int stop_bits = ONESTOPBIT);//SetDCB

//inherite from IComm
public:
	BOOL Read(BYTE buf[], int size, int *upload_count);//Read
	BOOL Write(BYTE buf[], int size);//Write
	
	BOOL write_transc_packet(BYTE transc_type, BYTE data[], int data_length);//write_transc_packet
	BYTE read_transc_ack();//read_transc_ack
	
//other
public:
	int Read(BYTE buf[], int size);//Read
	int Write(char buf[], int size);//Write

	int GetID();

	//Read all current data
	int Read();
};

#endif
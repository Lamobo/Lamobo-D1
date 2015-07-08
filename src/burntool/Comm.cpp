#include "stdafx.h"
#include "Comm.h"

//end_pad
static BYTE end_pad[MIN_PACKET_LENGTH] = {
	0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 
		0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 
		0x50, 0x50, 0x50, 0x52, 0x0, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 
		0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 0x50, 
		0x50, 0x50, 0x50, 0x50, 0x4F, 0x50, 0x13, 0x14
};


//////////////////////////////// CCommUSB /////////////////////////////////


CCommUSB::CCommUSB()
{
	m_bOpen = FALSE;//
}

CCommUSB::~CCommUSB()//
{
}


BOOL CCommUSB::isOpen()//
{
	return m_bOpen;//
}

BOOL CCommUSB::Open(CString usb_dbcc_name)//
{
	if(m_bOpen)//
	{
		Close();//
		m_dbcc_name.Empty();//
		m_bOpen = FALSE;//
	}

	if(m_usb.M3USBOPEN(usb_dbcc_name))
	{
		m_dbcc_name = usb_dbcc_name;//usb_dbcc_name
		
		m_bOpen = TRUE;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CCommUSB::Close()
{
	if(m_usb.M3USBCLOSE())
	{
		m_bOpen = FALSE;//m_bOpen
		return TRUE;//TRUE
	}
	else
	{
		return FALSE;
	}
}

BOOL CCommUSB::Read(BYTE buf[], int size, int *read_count)//Read
{
	UINT upload_count;

	if(!m_usb.M3USB_UPLOAD(buf, size, &upload_count))//M3USB_UPLOAD
	{
		return FALSE;
	}

	*read_count = upload_count;//upload_count

	return TRUE;
}

BOOL CCommUSB::Write(BYTE buf[], int size)//Write
{
	UINT write_len;

	if(!m_usb.M3USB_DOWNLOAD(buf,size, &write_len))//M3USB_DOWNLOAD
	{
		return FALSE;
	}

	if(write_len != size)
	{
		return FALSE;
	}
	return TRUE;
}

BYTE CCommUSB::read_transc_ack()//read_transc_ack
{
	T_TRANSC_PACKET packet;
	UINT	read_length;
	int overtime = 0;

	while(1)
	{
		if( !m_usb.M3USB_UPLOAD((BYTE *)&packet, MIN_PACKET_LENGTH, &read_length) )//M3USB_UPLOAD
		{
			return ACK_FAIL;	
		}

		if(read_length == 0)
		{
			overtime ++;
			if(overtime >= OVER_TIME)
			{
				return ACK_FAIL;
			}
			Sleep(1000);
			continue;
		}

		return (BYTE)packet.data[0];
	}	
}

BOOL CCommUSB::write_transc_packet(BYTE transc_type, BYTE data[], int data_length)//write_transc_packet
{
	T_TRANSC_PACKET packet;
	bool ret;
	UINT real_len, write_len;
	
	packet.type = transc_type;//transc_type

	packet.packet_length = sizeof(packet.type) + sizeof(packet.packet_length) + data_length;//packet_length
	
	memcpy(packet.data, data, data_length);//
	
	write_len = packet.packet_length;//packet_length

	if(write_len % MIN_PACKET_LENGTH != 0)
		write_len = packet.packet_length + (MIN_PACKET_LENGTH - packet.packet_length % MIN_PACKET_LENGTH);
	
	ret = m_usb.M3USB_DOWNLOAD((BYTE *)&packet, write_len, &real_len);//M3USB_DOWNLOAD
	if(!ret)
	{
		return FALSE;		
	}
	ret = m_usb.M3USB_DOWNLOAD(end_pad, MIN_PACKET_LENGTH, &real_len);//M3USB_DOWNLOAD
	if(!ret)
	{
		return FALSE;
	}
	
	return TRUE;
}

BOOL CCommUSB::download_data(CString file_path)
{
	DWORD read_len = 0;//
	UINT usb_write_len = 0;//
	HANDLE hFile = NULL;//
    BOOL bDownloadNandBoot = FALSE;//
    BOOL bfirst = TRUE;//

	BYTE file_data[MAX_PACKET_LENGTH];//
 
	if(file_path.IsEmpty())
        return FALSE;

/*    if(strcmp(file_path, config.nandboot_path) == 0)
    {
		bDownloadNandBoot = TRUE;
	}
*/
    hFile = CreateFile(file_path , GENERIC_READ , FILE_SHARE_READ , NULL , 
			OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);//

	if (hFile == INVALID_HANDLE_VALUE) 
	{ 
		return FALSE;
	}

	while(1)
	{
		BOOL bResult = ReadFile(hFile, file_data, MAX_PACKET_LENGTH,  &read_len, NULL);//
		
		if(!bResult)
		{
			CloseHandle(hFile);

			return FALSE;
		}

/*        if(bDownloadNandBoot && bfirst)
        {
            config_nandboot_parameter(file_data);
            bfirst = FALSE;
        }
*/
		
		int write_len = read_len;
		
		if(write_len % MIN_PACKET_LENGTH != 0)
			write_len = read_len + (MIN_PACKET_LENGTH - read_len % MIN_PACKET_LENGTH);//
		
		//
		if( read_len > 0  && !m_usb.M3USB_DOWNLOAD(file_data, write_len, &usb_write_len))//
		{
			CloseHandle(hFile);
			return FALSE;
		}

//		download_file_len += read_len;
//		PostMessage(get_wnd(), ON_BURNFLASH_MESSAGE, WPARAM(get_id()+100), LPARAM(MESSAGE_DOWNLOAD_FILE_LEN) );
		
		if(read_len < MAX_PACKET_LENGTH)//last packet
		{
			CloseHandle(hFile);
			
			if( !m_usb.M3USB_DOWNLOAD(end_pad, MIN_PACKET_LENGTH, &usb_write_len))//
			{
				return FALSE;
			}
			
			Sleep(200);
			
			if( read_transc_ack() != ACK_SUCCESS)//
			{
				return FALSE;//
			}

			break;
		}
	}

	return true;
}


//////////////////////////////// CCommCOM /////////////////////////////////


CCommCOM::CCommCOM()
{
	m_hwnd = NULL;

	m_receive_len = 0;
	memset(m_buf, 0, sizeof(m_buf));
}

CCommCOM::CCommCOM(HWND hwnd)//
{
	m_hwnd = hwnd;//
}

CCommCOM::~CCommCOM()//
{
}

void CCommCOM::SetHWND(HWND hwnd)
{
	m_hwnd = hwnd;
}

BOOL CCommCOM::isOpen()//
{
	if(m_com.is_open())//
	{
		return TRUE;//
	}
	else
	{
		return FALSE;
	}
}

int CCommCOM::GetID()
{
	return m_com.get_port();//
}

BOOL CCommCOM::Open(int port, int baud_rate)//
{
	if(m_com.is_open())
	{
		m_com.close();
	}

	if(m_com.open(port, baud_rate))
	{
		m_com.set_notify_num(1);//
		m_com.set_hwnd(m_hwnd);//
		return TRUE;//
	}
	else
	{
		return FALSE;//
	}
}

BOOL CCommCOM::SetDCB(int baud_rate, int byte_size, int parity, int stop_bits)
{
	if(m_com.set_dcb(baud_rate, byte_size, parity, stop_bits))
	{
		return TRUE;//
	}
	else
	{
		return FALSE;//
	}
}

BOOL CCommCOM::Close()
{
	m_com.close();//

	return TRUE;
}

BOOL CCommCOM::Read(BYTE buf[], int size)
{
	return m_com.read((char *)buf, size);//
}

BOOL CCommCOM::Write(char buf[], int size)
{
	return m_com.write(buf, size);//
}


int CCommCOM::Read()//
{
	int len;
	int i;
	BYTE buf[200];

	len =Read(buf, 100);

	//when data received is large than buffer size,
	//need to trunk
	if(len + m_receive_len > COM_BUFFER_SIZE)//
	{
		for(i = 0; i < m_receive_len-1024; i++)//
		{
			m_buf[i] = m_buf[i+1024];//
		}

		m_receive_len -= 1024;
	}

	for(i = 0; i < len; i++)
	{
		if(0 == buf[i])
		{
			continue;
		}

		if(i > 0 && '\n'==buf[i] && '\r'!=buf[i-1])
		{
			m_buf[m_receive_len++] = '\r';
		}
	


		m_buf[m_receive_len++] = buf[i];
	}

	m_buf[m_receive_len] = '\0';

	return len;
}


BOOL CCommCOM::Read(BYTE buf[], int size, int *read_count)
{
	int count;

	count = m_com.read((char *)buf, size);

	*read_count = count;

	return TRUE;
}

BOOL CCommCOM::Write(BYTE buf[], int size)
{
	if(m_com.write((char *)buf, size))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

BOOL CCommCOM::write_transc_packet(BYTE transc_type, BYTE data[], int data_length)
{
	return TRUE;	
}

BYTE CCommCOM::read_transc_ack()
{
	return 0;
}
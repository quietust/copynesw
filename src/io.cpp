#include "CopyNESW.h"

#pragma comment(lib, "../src/inpout32.lib")
#pragma comment(lib, "../src/ftd2xx.lib")
#include "inpout32.h"
#include "ftd2xx.h"

static	int sysver = 0;
static	unsigned short baseport = 0;
static	unsigned short ecpoffset = 0;

static	BYTE	shadow = 0x24;
static	BYTE	c;

FT_HANDLE ftHandleA;  //DATA BUS
FT_HANDLE ftHandleB;  //CONTROL BUS
FT_STATUS ftStatus;   //STATUS

char	ROMstring[256];
char	RxBuffer[64];
char	TxBuffer[64];

static int SystemVersion()
{
	OSVERSIONINFOEX osvi;
	BOOL bOsVersionInfoEx;
	ZeroMemory(&osvi,sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (!(bOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *)&osvi)))
	{
		osvi.dwOSVersionInfoSize = sizeof (OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *)&osvi))
			  return 0;
	}
	switch (osvi.dwPlatformId)
	{      
	case VER_PLATFORM_WIN32_NT:	return 2;	break;
	case VER_PLATFORM_WIN32_WINDOWS:return 1;	break;
	}
	return 0;
}

static BOOL	OpenUSB (void)
{
	baseport = 0;
	ftStatus = FT_OpenEx("USB CopyNES A",FT_OPEN_BY_DESCRIPTION,&ftHandleA);   // open data bus
	if (ftStatus != FT_OK)
	{
		// failure - one or both of the devices has not been opened
		MessageBox(topHWnd, "USB Error: Failed to open CopyNES data bus!", "OpenUSB", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	ftStatus = FT_OpenEx("USB CopyNES B",FT_OPEN_BY_DESCRIPTION,&ftHandleB);   //open control bus
	if (ftStatus != FT_OK)
	{
		// failure - one or both of the devices has not been opened 
		MessageBox(topHWnd, "USB Error: Failed to open CopyNES control bus!", "OpenUSB", MB_OK | MB_ICONERROR);
		return FALSE;
	} 
	return TRUE;
}

static	HANDLE pport = INVALID_HANDLE_VALUE;
BOOL	OpenPort (int port, int addr, int ecp)
{
	if (port == -1)
		return OpenUSB();
	char *pname;
	switch (port)
	{
	case 1:	pname = "LPT1";
		break;
	case 2:	pname = "LPT2";
		break;
	case 3:	pname = "LPT3";
		break;
	case 4:	pname = NULL;
		break;
	default:return FALSE;	break;
	}
	if (pname)
	{
		pport = CreateFile(pname,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);
		if (pport == INVALID_HANDLE_VALUE)
		{
			baseport = 0;
			ecpoffset = 0;
			MessageBox(topHWnd,"Parallel port is in use!","CopyNES",MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}
	baseport = addr;
	ecpoffset = ecp;

	sysver = SystemVersion();
	if (sysver == 1)
		return TRUE;
	else if ((sysver == 2) && (IsInpOutDriverOpen()))
		return TRUE;
	else
	{
		if (pport != INVALID_HANDLE_VALUE)
			CloseHandle(pport);
		pport = INVALID_HANDLE_VALUE;
		baseport = 0;
		ecpoffset = 0;
		MessageBox(topHWnd,"Failed to load parallel port driver!","CopyNES",MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

static	void	CloseUSB (void)
{
	FT_Close(ftHandleA);
	FT_Close(ftHandleB);
}

void	ClosePort (void)
{
	if (ParPort == -1)
		return CloseUSB();
	if (!baseport)
		return;
	baseport = 0;
	ecpoffset = 0;
	if (pport != INVALID_HANDLE_VALUE)
		CloseHandle(pport);
	pport = INVALID_HANDLE_VALUE;
}


unsigned char	prData (void)
{
	if (!baseport)
		return 0;
	return (unsigned char)Inp32(baseport);
}
void	pwData (unsigned char data)
{
	if (!baseport)
		return;
	Out32(baseport,data);
}
unsigned char	prStatus (void)
{
	if (!baseport)
		return 0;
	return (unsigned char)Inp32((unsigned short)(baseport+1));
}
void	pwControl (unsigned char data)
{
	if (!baseport)
		return;
	Out32((unsigned short)(baseport+2),data);
}

unsigned char	prECP (void)
{
	if (!baseport)
		return 0;
	return (unsigned char)Inp32((unsigned short)(baseport+ecpoffset+0x2));
}
void	pwECP (unsigned char data)
{
	if (!baseport)
		return;
	Out32((unsigned short)(baseport+ecpoffset+0x2),data);
}

BOOL	ReadByteEx (BYTE &data, int timeout, BOOL warn)
{
	if (ParPort == -1)
	{
		DWORD BytesReceived = 0;
		FT_SetTimeouts(ftHandleA,10000,0);
		ftStatus = FT_Read(ftHandleA,RxBuffer,1,&BytesReceived);
		if (ftStatus == FT_OK)
		{
			if (BytesReceived == 1)
			{
				// FT_Read OK
				data = RxBuffer[0];
				return TRUE;
			}
			else
			{
				// FT_Read Timeout
				if (warn)
					MessageBox(topHWnd, "USB Error: Read Timeout", "ReadByteEx", MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}
		else
		{
			// FT_Read Failed
			if (warn)
				MessageBox(topHWnd, "USB Error: Read Failed", "ReadByteEx", MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}
	else
	{
		BYTE a, b;
		time_t starttime = time(NULL);
		shadow |= 0x20;
		pwControl(shadow);	// set port to input mode
		b = prStatus();		// wait for ACK
		while (!((b ^ c) & 0x20))
		{
			b = prStatus();
			if (time(NULL) > starttime + (time_t)timeout)
			{
				if (warn)
					MessageBox(topHWnd, "Timeout on data transfer!", "ReadByte", MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}
		c = b;
		a = prData();		// read data
		shadow ^= 0x02;
		pwControl(shadow);	// signal byte received
		data = a;
		return TRUE;
	}
}

BOOL	WriteByteEx (BYTE data, int timeout, BOOL warn)
{
	if (ParPort == -1)
	{
		DWORD BytesWritten = 0;
		FT_SetTimeouts(ftHandleA,10000,0);
		TxBuffer[0] = data;
		ftStatus = FT_Write(ftHandleA,TxBuffer,1,&BytesWritten);
		if (ftStatus == FT_OK)
		{
			if (BytesWritten == 1)
			{
				// FT_Read OK
				return TRUE;
			}
			else
			{
				// FT_Write Timeout
				if (warn)
					MessageBox(topHWnd, "USB Error: Write Timeout", "WriteByteEx", MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}
		else
		{
			// FT_Write Failed
			StatusText("FT STATUS = %i", ftStatus);
			MessageBox(topHWnd, "USB Error: Write Failed", "WriteByteEx", MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}
	else
	{
		time_t starttime = time(NULL);
		BYTE b;
		shadow &= 0xDF;
		pwControl(shadow);	// set port to output mode
		pwData(data);		// output data
		shadow ^= 0x01;
		pwControl(shadow);	// signal byte sent
		b = prStatus();		// wait for ACK
		while (!((b ^ c) & 0x10))
		{
			b = prStatus();
			if (time(NULL) > starttime + (time_t)timeout)
			{
				if (warn)
					MessageBox(topHWnd, "Timeout on data transfer!", "WriteByte", MB_OK | MB_ICONERROR);
				return FALSE;
			}
		}
		c = b;
		return TRUE;
	}
}

BOOL WriteBlock (const BYTE* blockdata, int size)
{
	if (ParPort == -1)
	{
		DWORD BytesWritten = 0;
		FT_SetTimeouts(ftHandleA,10000,0);
		ftStatus = FT_Write(ftHandleA, (LPVOID)blockdata, size, &BytesWritten);
		if (ftStatus == FT_OK)
		{ 
			if (BytesWritten == size) 
			{ 
				// FT_Read OK
				return TRUE;
			} 
			else 
			{ 
				// FT_Write Timeout 
				MessageBox(topHWnd, "USB Error: Write Timeout", "WriteBlock", MB_OK | MB_ICONERROR);
				return FALSE;  
			} 
		} 
		else 
		{ 
			// FT_Write Failed 
			StatusText("FT STATUS = %i", ftStatus);
			MessageBox(topHWnd, "USB Error: Write Failed", "WriteBlock", MB_OK | MB_ICONERROR);
			return FALSE;  
		}
	}
	else
	{
		BOOL success;
		for (int i = 0; i < size; i++)
		{
			success = WriteByteEx(blockdata[i], 10, TRUE);
			if (!success)
				return FALSE;
		}
		return TRUE;
	}
}

BOOL	ReadByte (BYTE &data)
{
	return ReadByteEx(data, 10, TRUE);
}
BOOL	WriteByte (BYTE data)
{
	return WriteByteEx(data, 10, TRUE);
}

BOOL	ReadByteSilent (BYTE &data)
{
	return ReadByteEx(data, 10, FALSE);
}
BOOL	WriteByteSilent (BYTE data)
{
	return WriteByteEx(data, 10, FALSE);
}

BOOL	WriteByteAsync (BYTE data)
{
	if (ParPort == -1)
	{
		MessageBox(topHWnd, "ERROR - Code attempted to perform asynchronous write in USB mode", "WriteByteASync", MB_OK | MB_ICONERROR);
		return FALSE;
	}
	else
	{
		shadow &= 0xDF;
		pwControl(shadow);	// set port to output mode
		pwData(data);		// output data
		return TRUE;
	}
}

BOOL ReadByteReady (void)
{
	if (ParPort == -1)
	{
		DWORD EventDWord = 0;
		DWORD RxBytes = 0;
		DWORD TxBytes = 0;
		FT_GetStatus(ftHandleA,&RxBytes,&TxBytes,&EventDWord);
		if (RxBytes > 0)
			return TRUE;
		else
			return FALSE;
	}
	else
	{
		BYTE b;
		b = prStatus();		// wait for ACK
		if (!((b ^ c) & 0x20))
			return FALSE;
		return TRUE;
	}
}

unsigned char	prECP (void);
void	pwECP (unsigned char);

void	InitPort (void)
{
	if (ParPort == -1)
	{
		DWORD modemWord = 0;
		ftStatus = FT_GetModemStatus (ftHandleB, &modemWord);
		if (ftStatus != FT_OK)
		{
			MessageBox(topHWnd, "USB Error: Get Power Status Failed!", "InitPort", MB_OK | MB_ICONERROR);
			return;
		}

		if (modemWord & 0x80)
			MessageBox(topHWnd, "USB CopyNES is OFF - turn on then hit OK", "InitPort", MB_OK | MB_ICONERROR);

		// flush receive/transmit buffers
		ftStatus = FT_Purge (ftHandleA, FT_PURGE_RX | FT_PURGE_TX);
		if (ftStatus != FT_OK)
		{
			MessageBox(topHWnd, "USB Error: Purge A Failed!", "InitPort", MB_OK | MB_ICONERROR);
			return;
		}

		ftStatus = FT_Purge (ftHandleB, FT_PURGE_RX | FT_PURGE_TX);
		if (ftStatus != FT_OK)
		{
			MessageBox(topHWnd, "USB Error: Purge B Failed!", "InitPort", MB_OK | MB_ICONERROR);
			return;
		}
	}
	else
	{
		if ((prECP() & 0x03) == 0x01)
		{
			pwECP(0x34);
			if (prECP() == 0x35)
				pwECP((prECP() & 0x1F) | 0x20);	// try to set ECP port to bidirectional mode
		}
		shadow |= 0x40;	// pull ACK line high - this way, printers won't listen to us
		pwControl(shadow);
		c = prStatus();
	}
}

void	ResetNES (int rtype)
{
	if (ParPort == -1)
	{
 		if (rtype & RESET_PLAYMODE)
		{
			//clr /RTS=1
			ftStatus = FT_ClrRts(ftHandleB);
			if (ftStatus != FT_OK)
			{
				MessageBox(topHWnd, "USB Error: ClrRts Failed!", "ResetNES", MB_OK | MB_ICONERROR);
				return;
			}
		}
		else
		{
			//set /RTS=0
			ftStatus = FT_SetRts(ftHandleB);
			if (ftStatus != FT_OK)
			{
				MessageBox(topHWnd, "USB Error: SetRts Failed!", "ResetNES", MB_OK | MB_ICONERROR);
				return;
			}
		}

		if (!(rtype & RESET_NORESET))
		{
			// pull /RESET low    clear D2
			//set /dtr=0
			ftStatus = FT_ClrDtr(ftHandleB);
			if (ftStatus != FT_OK)
			{
				MessageBox(topHWnd, "USB Error: ClrDtr Failed!", "ResetNES", MB_OK | MB_ICONERROR);
				return;
			}
			Sleep(SLEEP_SHORT);
		}

		// pull /RESET high       set D2
		//clr /dtr=1
		ftStatus = FT_SetDtr(ftHandleB);
		if (ftStatus != FT_OK)
		{
			MessageBox(topHWnd, "USB Error: SetDtr Failed!", "ResetNES", MB_OK | MB_ICONERROR);
			return;
		}
		Sleep(SLEEP_SHORT);
		InitPort();
		Sleep(SLEEP_SHORT);
	}
	else
	{
		if (rtype & RESET_ALTPORT)
			shadow &= 0xF7;
		else	shadow |= 0x08;
		if (rtype & RESET_PLAYMODE)
			shadow &= 0xFE;
		else	shadow |= 0x01;

		if (!(rtype & RESET_NORESET))
		{
			shadow &= 0xFB;
			pwControl(shadow);	// pull /RESET low
			Sleep(SLEEP_SHORT);
		}
		shadow |= 0x04;		// pull /RESET high
		pwControl(shadow);
		Sleep(SLEEP_SHORT);
	}
}

BOOL	WriteCommand (BYTE a, BYTE b, BYTE c, BYTE d, BYTE e)
{
	if (WriteByteSilent(a) && WriteByteSilent(b) && WriteByteSilent(c) && WriteByteSilent(d) && WriteByteSilent(e))
		return TRUE;
	else
	{
		MessageBox(topHWnd, "Timeout on data transfer!", "WriteCommand", MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

BOOL	LoadPlugin (const Plugin *plugin)
{
	if (!WriteCommand(0x4B,0x00,0x04,0x04,0xB4))	// write to CPU space
		return FALSE;
	if (!WriteBlock(plugin->data,1024))
		return FALSE;
	StatusPercent(100);
	strcpy(ROMstring, plugin->romstring.c_str());
	StatusPercent(0);
	Sleep(SLEEP_SHORT);
	return TRUE;
}

BOOL	LoadPlugin (const char *filename)
{
	Plugin *plugin = makePlugin("", filename, -1, "");
	if (!plugin)
	{
		MessageBox(topHWnd,"Failed to open plugin file!","Dump",MB_OK | MB_ICONERROR);
		return FALSE;
	}
	BOOL result = LoadPlugin(plugin);
	delete plugin;
	return result;
}

BOOL	RunCode (void)
{
	return WriteCommand(0x7E,0x00,0x04,0x00,0xE7);
}
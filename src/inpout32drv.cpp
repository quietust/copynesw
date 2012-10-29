#include "StdAfx.h"

#pragma comment(lib, "../src/inpout32.lib")
#pragma comment(lib, "../src/ftd2xx.lib")
#include "inpout32.h"

static	int sysver = 0;
static	unsigned short baseport = 0;
static	unsigned short ecpoffset = 0;

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

extern FT_HANDLE ftHandleA;  //DATA BUS
extern FT_HANDLE ftHandleB;  //CONTROL BUS
extern FT_STATUS ftStatus;   //STATUS

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
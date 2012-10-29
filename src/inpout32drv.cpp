#include "StdAfx.h"

#pragma comment(lib, "../src/inpout32.lib")
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

static	HANDLE pport = INVALID_HANDLE_VALUE;
BOOL	OpenPort (int port, int addr, int ecp)
{
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

void	ClosePort (void)
{
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
	return Inp32(baseport);
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
	return Inp32((unsigned short)(baseport+1));
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
	return Inp32((unsigned short)(baseport+ecpoffset+0x2));
}
void	pwECP (unsigned char data)
{
	if (!baseport)
		return;
	Out32((unsigned short)(baseport+ecpoffset+0x2),data);
}
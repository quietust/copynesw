#include "StdAfx.h"
#include <conio.h>

static	HANDLE hdriver;
static	char path[MAX_PATH];
static	int sysver = 0;
static	unsigned short baseport = 0;

#define IOCTL_READ_PORT_UCHAR	 -1673519100 //CTL_CODE(40000, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_WRITE_PORT_UCHAR	 -1673519096 //CTL_CODE(40000, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

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

static int inst()
{
	SC_HANDLE Mgr, Ser;
	HRSRC hResource;

	GetSystemDirectory(path,sizeof(path));
	hResource = FindResource(hInst,MAKEINTRESOURCE(IDR_DRIVER),"bin");
	if (hResource)
	{
		HGLOBAL binGlob = LoadResource(hInst,hResource);
		if(binGlob)
		{
			void *binData = LockResource(binGlob);
			if (binData)
			{
				HANDLE file;
				strcat(path,"\\Drivers\\hwinterface.sys");
				file = CreateFile(path,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
				if(file)
				{
					DWORD size, written;
					size = SizeofResource(hInst,hResource);
					WriteFile(file, binData, size, &written, NULL);
					CloseHandle(file);
				}
			}
		}
	}
	Mgr = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (Mgr == NULL)
	{				// No permission to create service
		if (GetLastError() == ERROR_ACCESS_DENIED) 
			return 5;	// error access denied
	}
	else	Ser = CreateService(Mgr,"hwinterface","hwinterface",SERVICE_ALL_ACCESS,SERVICE_KERNEL_DRIVER,SERVICE_SYSTEM_START,SERVICE_ERROR_NORMAL,"System32\\Drivers\\hwinterface.sys",NULL,NULL,NULL,NULL,NULL);
	CloseServiceHandle(Mgr);
	CloseServiceHandle(Ser);
	return 0;
}

static int start (void)
{
	SC_HANDLE Mgr, Ser;
	Mgr = OpenSCManager(NULL,NULL,SC_MANAGER_ALL_ACCESS);
	if (Mgr == NULL)
	{	// No permission to create service
		if (GetLastError() == ERROR_ACCESS_DENIED) 
		{
			Mgr = OpenSCManager(NULL,NULL,GENERIC_READ);
			Ser = OpenService(Mgr,"hwinterface",GENERIC_EXECUTE);
			if (Ser)
			{	// we have permission to start the service
				if (!StartService(Ser,0,NULL))
				{
					CloseServiceHandle(Ser);
					return 4;	// we could open the service but unable to start
				}
			}
		}
	}
	else
	{	// Successfuly opened Service Manager with full access
		Ser = OpenService(Mgr,"hwinterface",GENERIC_EXECUTE);
		if (Ser)
		{
			if (!StartService(Ser,0,NULL))
			{
				CloseServiceHandle(Ser);
				return 3;	// opened the Service handle with full access permission, but unable to start
			}
			else
			{
				CloseServiceHandle(Ser);
				return 0;	// started
			}
		}
	}
	return 1;
}

static int Opendriver(void)
{
	hdriver = CreateFile("\\\\.\\hwinterface", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hdriver != INVALID_HANDLE_VALUE)
		return 0;		// successfully opened
	if (start())	// attempt to start
	{		// failed...
		if (inst())	// attempt to install
		{		// failed
			MessageBox(NULL,"Failed to install parallel port I/O driver!","I/O",MB_OK | MB_ICONERROR);
			return 1;
		}
		if (start())
		{
			MessageBox(NULL,"Failed to start parallel port I/O driver!","I/O",MB_OK | MB_ICONERROR);
			return 1;
		}
	}
	hdriver = CreateFile("\\\\.\\hwinterface", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hdriver != INVALID_HANDLE_VALUE)
		return 0;
	return 1;
}

static	HANDLE pport = INVALID_HANDLE_VALUE;
BOOL	OpenPort (int port, int addr)
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
			MessageBox(topHWnd,"Parallel port is in use!","CopyNES",MB_OK | MB_ICONERROR);
			return FALSE;
		}
	}
	baseport = addr;

	sysver = SystemVersion();
	if (sysver == 1)
		return TRUE;
	else if ((sysver == 2) && (!Opendriver()))
		return TRUE;
	else
	{
		if (pport != INVALID_HANDLE_VALUE)
			CloseHandle(pport);
		pport = INVALID_HANDLE_VALUE;
		baseport = 0;
		MessageBox(topHWnd,"Failed to load parallel port driver!","CopyNES",MB_OK | MB_ICONERROR);
		return FALSE;
	}
}

void	ClosePort (void)
{
	if (!baseport)
		return;
	baseport = 0;
	if (pport != INVALID_HANDLE_VALUE)
		CloseHandle(pport);
	pport = INVALID_HANDLE_VALUE;
	if (sysver == 2)
		CloseHandle(hdriver);
}

static unsigned char Inp32 (unsigned short addr)
{
	unsigned int error;
	DWORD BytesReturned;
	unsigned char Buffer[3];

	switch (sysver)
	{
	case 1:	return _inp(addr);	break;
	case 2:	Buffer[0] = LOBYTE(addr);
		Buffer[1] = HIBYTE(addr);
		Buffer[2] = 0;
		error = DeviceIoControl(hdriver,IOCTL_READ_PORT_UCHAR,&Buffer,2,&Buffer,1,&BytesReturned,NULL);
		return Buffer[0];	break;
	}
	return 0;
}

static void Out32 (unsigned short addr, unsigned char data)
{
	unsigned int error;
	DWORD BytesReturned;        
	BYTE Buffer[3];
	switch (sysver)
	{
	case 1:	_outp(addr,data);	break;
	case 2:	Buffer[0] = LOBYTE(addr);
		Buffer[1] = HIBYTE(addr);
		Buffer[2] = data;
		error = DeviceIoControl(hdriver,IOCTL_WRITE_PORT_UCHAR,&Buffer,3,NULL,0,&BytesReturned,NULL);
					break;
	}
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
	return Inp32((unsigned short)(baseport+0x402));
}
void	pwECP (unsigned char data)
{
	if (!baseport)
		return;
	Out32((unsigned short)(baseport+0x402),data);
}

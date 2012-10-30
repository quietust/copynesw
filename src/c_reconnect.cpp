#include "CopyNESW.h"

#define	CMD_NAME	"Reconnect"

extern void EnableMenus (HWND);
extern int FindVersion (void);

BOOL	CMD_RECONNECT (void)
{
	OpenStatus(topHWnd);
	ClosePort();
	StatusText("Reconnecting...");
	if (OpenPort(ParPort, ParAddr, ParECP))
	{
		if (ParPort != -1)
			InitPort();
		ResetNES(RESET_COPYMODE);
		HWVer = FindVersion();
		EnableMenus(topHWnd);
		return TRUE;
	}
	else
	{
		StatusText("Failed!");
		StatusOK();
		HWVer = 0;
		EnableMenus(topHWnd);
		return FALSE;
	}
}

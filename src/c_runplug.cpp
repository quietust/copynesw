#include "CopyNESW.h"

#define	CMD_NAME	"Run Plugin"

BOOL	CMD_RUNPLUG (void)
{
	Plugin *plugin;
	plugin = PromptPlugin(PLUG_UTIL);
	if (plugin == NULL)
		return FALSE;
	OpenStatus(topHWnd);
	InitPort();
	StatusText("Resetting CopyNES...");
	ResetNES(RESET_COPYMODE);

	StatusText("Loading plugin...");
	if (!LoadPlugin(plugin))
	{
		CloseStatus();
		return FALSE;
	}
	StatusText("Initializing plugin...");
	RunCode();
	StatusText("Running plugin - press OK to terminate.");
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

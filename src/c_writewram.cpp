#include "StdAfx.h"
#define	CMD_NAME	"Write SRAM"

BOOL	CMD_WRITEWRAM (void)
{
	PPlugin plugin;
	char filename[MAX_PATH];
	int i, wramsize;
	FILE *WRAM;
	plugin = PromptPlugin(PLUG_WRAM);
	if (plugin == NULL)
		return FALSE;
	if (!PromptFile(topHWnd,"SaveRAM files (*.SAV)\0*.sav\0\0",filename,NULL,Path_WRAM,"Select an SRAM file","sav",FALSE))
		return FALSE;
	OpenStatus(topHWnd);
	StatusText("Initializing parallel port...");
	InitPort();
	StatusText("Resetting CopyNES...");
	ResetNES(RESET_COPYMODE);

	StatusText("Loading plugin...");
	if (!LoadPlugin(plugin->file))
	{
		CloseStatus();
		return FALSE;
	}
	StatusText("Initializing plugin...");
	RunCode();
	WRAM = fopen(filename,"rb");
	fseek(WRAM,0,SEEK_END);
	wramsize = ftell(WRAM);
	StatusText("Uploading WRAM...");
	if (!WriteByte((BYTE)(wramsize >> 8)))
	{
		fclose(WRAM);
		CloseStatus();
		return FALSE;
	}
	fseek(WRAM,0,SEEK_SET);
	for (i = 0; i < wramsize; i++)
	{
		BYTE n;
		fread(&n,1,1,WRAM);
		if (!WriteByte(n))
		{
			fclose(WRAM);
			CloseStatus();
			return FALSE;
		}
		if (!(~i & 0x7F))
			StatusPercent((i*100)/wramsize);
	}
	fclose(WRAM);
	StatusText("Upload complete!");
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

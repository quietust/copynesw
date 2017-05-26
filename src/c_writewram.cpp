#include "CopyNESW.h"

#define	CMD_NAME	"Write SRAM"

BOOL	CMD_WRITEWRAM (void)
{
	Plugin *plugin;
	char filename[MAX_PATH];
	int i, wramsize;
	FILE *WRAM;
	plugin = PromptPlugin(PLUG_WRAM);
	if (plugin == NULL)
		return FALSE;
	if (!PromptFile(topHWnd,"SaveRAM files (*.SAV)\0*.sav\0\0",filename,NULL,Path_WRAM,"Select an SRAM file","sav",FALSE))
		return FALSE;
	WRAM = fopen(filename,"rb");
	if (!WRAM)
	{
		MessageBox(topHWnd,"Unable to open SRAM file!",MSGBOX_TITLE,MB_OK | MB_ICONERROR);
		return FALSE;
	}
	OpenStatus(topHWnd);
	InitPort();
	StatusText("Resetting CopyNES...");
	ResetNES(RESET_COPYMODE);

	StatusText("Loading plugin...");
	if (!LoadPlugin(plugin))
	{
		fclose(WRAM);
		CloseStatus();
		return FALSE;
	}
	StatusText("Initializing plugin...");
	RunCode();
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
	int nblks = wramsize / 1024;
	int nrem = wramsize % 1024;
	BYTE n[1024];

	for (i = 0; i < nblks; i++)
	{
		fread(&n,1024,1,WRAM);
		if (!WriteBlock(n, 1024))
		{
			fclose(WRAM);
			CloseStatus();
			return FALSE;
		}
		StatusPercent((i*100)/(nblks+1));
	}
	if (nrem)
	{
		fread(&n,nrem,1,WRAM);
		if (!WriteBlock(n, nrem))
		{
			fclose(WRAM);
			CloseStatus();
			return FALSE;
		}
	}
	StatusPercent(100);
	fclose(WRAM);
	StatusText("Upload complete!");
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

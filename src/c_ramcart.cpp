#include "StdAfx.h"
#define	CMD_NAME	"RAM Cart"

BOOL	NRAMcart (char *plugin)
{
	char filenes[MAX_PATH];
	FILE *NES;
	int i, PRGsize;
	BYTE PRGamt;
	BYTE header[16];
	BYTE mapper;

	if (!PromptFile(topHWnd,"iNES ROM images (*.NES)\0*.nes\0\0",filenes,NULL,Path_NES,"Select an iNES ROM...","nes",FALSE))
		return FALSE;

	OpenStatus(topHWnd);

	if ((NES = fopen(filenes,"rb")) == NULL)
	{
		MessageBox(topHWnd,"Unable to open file!",MSGBOX_TITLE,MB_OK | MB_ICONERROR);
		CloseStatus();
		return FALSE;
	}
	fread(header,1,16,NES);
	if (memcmp(header,"NES\x1A",4))
	{
		StatusText("Selected file is not an iNES ROM image!");
		StatusOK();
		return FALSE;
	}
	if (header[4] == 2)
	{
		PRGamt = 0;
		PRGsize = 32768;
		StatusText("32KB PRG ROM data located...");
	}
	else if (header[4] == 1)
	{
		PRGamt = 1;
		PRGsize = 16384;
		StatusText("16KB PRG ROM data located...");
	}
	else
	{
		fclose(NES);
		StatusText("Invalid PRG size, must be 16KB or 32KB!");
		StatusOK();
		return FALSE;
	}

	if (header[5] > 1)
		StatusText("More than 8KB of CHR ROM data was detected, sending first 8KB only...");
	else	StatusText("8KB CHR ROM data located...");

	mapper = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);
	if ((mapper != 0) && (MessageBox(topHWnd,"Incorrect iNES mapper detected! Load anyways?",MSGBOX_TITLE,MB_YESNO | MB_ICONQUESTION) == IDNO))
	{
		CloseStatus();
		return FALSE;
	}

	if (header[6] & 1)
		MessageBox(topHWnd,"Please set your cartridge to VERTICAL mirroring.",MSGBOX_TITLE,MB_OK);
	else	MessageBox(topHWnd,"Please set your cartridge to HORIZONTAL mirroring.",MSGBOX_TITLE,MB_OK);

	StatusText("Initializing parallel port...");
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
	Sleep(SLEEP_SHORT);
	if (!WriteByte(PRGamt))
	{
		CloseStatus();
		return FALSE;
	}
	StatusText("Sending PRG data...");
	for (i = 0; i < PRGsize; i++)
	{
		BYTE a;
		fread(&a,1,1,NES);
		if (!WriteByte(a))
		{
			CloseStatus();
			return FALSE;
		}
		if (!(~i & 0xFF))
			StatusPercent((i*100)/PRGsize);
	}
	StatusText("...done!");
	StatusText("Sending CHR data...");
	for (i = 0; i < 8192; i++)
	{
		BYTE a = 0;
		if (header[5])
			fread(&a,1,1,NES);
		if (!WriteByte(a))
		{
			CloseStatus();
			return FALSE;
		}
		if (!(~i & 0xFF))
			StatusPercent((i*100)/8192);
	}
	fclose(NES);
	StatusText("...done!");
	StatusText("Program running - Press OK to exit.");
	ResetNES(RESET_PLAYMODE | RESET_NORESET);
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

BOOL	CNRAMcart (char *plugin)
{
	char filenes[MAX_PATH];
	FILE *NES;
	int i, j;
	BYTE header[16];
	BYTE mapper;
	int maxchr = 4;

	if (!PromptFile(topHWnd,"iNES ROM images (*.NES)\0*.nes\0\0",filenes,NULL,Path_NES,"Select an iNES ROM...","nes",FALSE))
		return FALSE;

	OpenStatus(topHWnd);

	if ((NES = fopen(filenes,"rb")) == NULL)
	{
		MessageBox(topHWnd,"Unable to open file!",MSGBOX_TITLE,MB_OK | MB_ICONERROR);
		CloseStatus();
		return FALSE;
	}
	fread(header,1,16,NES);
	if (memcmp(header,"NES\x1A",4))
	{
		StatusText("Selected file is not an iNES ROM image!");
		StatusOK();
		return FALSE;
	}
	if ((header[4]) && (header[4] <= 2))
		StatusText("%iKB PRG ROM data located...", header[4] * 16);
	else
	{
		fclose(NES);
		StatusText("Invalid PRG size, must be 16KB or 32KB!");
		StatusOK();
		return FALSE;
	}

	mapper = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);
	if (mapper == 0)
		maxchr = 1;

	if (header[5] > maxchr)
		StatusText("More than %iKB of CHR ROM data was detected, sending first %iKB only...", maxchr * 8, maxchr * 8);
	else if (header[5] == 3)
	{
		fclose(NES);
		StatusText("Invalid CHR ROM size (%iKB)!", maxchr * 8);
		StatusOK();
		return FALSE;
	}
	else	StatusText("%iKB CHR ROM data located...", header[5] * 8);

	if ((mapper != 0) && (mapper != 3) && (MessageBox(topHWnd,"Incorrect iNES mapper detected! Load anyways?",MSGBOX_TITLE,MB_YESNO | MB_ICONQUESTION) == IDNO))
	{
		CloseStatus();
		return FALSE;
	}
	if (header[6] & 1)
		MessageBox(topHWnd,"Please set your cartridge to VERTICAL mirroring.",MSGBOX_TITLE,MB_OK);
	else	MessageBox(topHWnd,"Please set your cartridge to HORIZONTAL mirroring.",MSGBOX_TITLE,MB_OK);

	StatusText("Initializing parallel port...");
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
	Sleep(SLEEP_SHORT);

	StatusText("Sending CHR data...");
	fseek(NES,0x10 + header[4] * 16384,SEEK_SET);
	if (!WriteByte(header[5]))
	{
		CloseStatus();
		return FALSE;
	}
	for (i = 0; i < header[5]; i++)
	{
		for (j = 0; j < 8192; j++)
		{
			BYTE a = 0;
			fread(&a,1,1,NES);
			if (!WriteByte(a))
			{
				CloseStatus();
				return FALSE;
			}
			if (!(~j & 0xFF))
				StatusPercent(((i * 8192 + j) * 100) / (header[5] * 8192));
		}
	}
	StatusPercent(100);
	StatusText("...done!");

	StatusText("Sending PRG data...");
	if (!WriteByte(header[4]))
	{
		CloseStatus();
		return FALSE;
	}
	fseek(NES,0x10,SEEK_SET);
	for (i = 0; i < header[4]; i++)
	{
		for (j = 0; j < 16384; j++)
		{
			BYTE a;
			fread(&a,1,1,NES);
			if (!WriteByte(a))
			{
				CloseStatus();
				return FALSE;
			}
			if (!(~j & 0xFF))
				StatusPercent(((i * 16384 + j) * 100) / (header[4] * 16384));
		}
	}
	StatusPercent(100);
	StatusText("...done!");
	
	fclose(NES);
	StatusText("Write protect your cartridge, then press OK to run program...");
	StatusButton();
	if (!WriteByte(0x55) || !WriteByte(0xAA))
	{
		CloseStatus();
		return FALSE;
	}
	StatusText("Program running - Press OK to exit.");
	ResetNES(RESET_PLAYMODE | RESET_NORESET);
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

BOOL	UFROMcart (char *plugin)
{
	char filenes[MAX_PATH];
	FILE *NES;
	int i, j;
	BYTE header[16];
	BYTE mapper;
	BYTE banks;

	if (!PromptFile(topHWnd,"iNES ROM images (*.NES)\0*.nes\0\0",filenes,NULL,Path_NES,"Select an iNES ROM...","nes",FALSE))
		return FALSE;

	OpenStatus(topHWnd);

	if ((NES = fopen(filenes,"rb")) == NULL)
	{
		MessageBox(topHWnd,"Unable to open file!",MSGBOX_TITLE,MB_OK | MB_ICONERROR);
		CloseStatus();
		return FALSE;
	}
	fread(header,1,16,NES);
	if (memcmp(header,"NES\x1A",4))
	{
		fclose(NES);
		StatusText("Selected file is not an iNES ROM image!");
		StatusOK();
		return FALSE;
	}
	if ((header[4] == 1) || (header[4] == 2) || (header[4] == 4) || (header[4] == 8) || (header[4] == 16))
		StatusText("%iKB PRG ROM data located...", header[4] * 16);
	else
	{
		fclose(NES);
		StatusText("Invalid PRG size, must be an even amount between 16KB and 256KB!");
		StatusOK();
		return FALSE;
	}

	mapper = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);

	if (header[5] > 0)
		StatusText("%iKB of CHR ROM data was detected, ignoring...", header[5] * 8);

	if ((mapper != 2) && (MessageBox(topHWnd,"Incorrect iNES mapper detected! Load anyways?",MSGBOX_TITLE,MB_YESNO | MB_ICONQUESTION) == IDNO))
	{
		fclose(NES);
		CloseStatus();
		return FALSE;
	}
	if (header[6] & 1)
		MessageBox(topHWnd,"Please set your cartridge to VERTICAL mirroring.",MSGBOX_TITLE,MB_OK);
	else	MessageBox(topHWnd,"Please set your cartridge to HORIZONTAL mirroring.",MSGBOX_TITLE,MB_OK);

	StatusText("Initializing parallel port...");
	InitPort();
	StatusText("Resetting CopyNES...");
	ResetNES(RESET_COPYMODE);
	StatusText("Loading plugin...");
	if (!LoadPlugin(plugin))
	{
		fclose(NES);
		CloseStatus();
		return FALSE;
	}
	StatusText("Initializing plugin...");
	RunCode();
	Sleep(SLEEP_SHORT);

	StatusText("Erasing Flash ROM...");
	if (!ReadByte(&banks))
	{
		fclose(NES);
		CloseStatus();
		return FALSE;
	}
	for (i = 0; i < banks; i++)
	{
		BYTE a;
		if (!ReadByte(&a))
		{
			fclose(NES);
			CloseStatus();
			return FALSE;
		}
		StatusPercent(100 * i / banks);
	}
	StatusPercent(100);
	StatusText("...done!");

	for (banks = 0; banks < 16; )
	{
		fseek(NES,0x10,SEEK_SET);
		for (i = 0; i < header[4]; i++, banks++)
		{
			for (j = 0; j < 16384; j++)
			{
				BYTE a;
				fread(&a,1,1,NES);
				if (!WriteByte(a))
				{
					fclose(NES);
					CloseStatus();
					return FALSE;
				}
				if (!(~j & 0xFF))
					StatusPercent(((banks * 16384 + j) * 100) / 262144);
			}
		}
	}
	StatusPercent(100);
	StatusText("...done!");
	fclose(NES);

	if (!ReadByte(&banks))
	{
		CloseStatus();
		return FALSE;
	}
	if (banks != 0)
	{
		StatusText("An error occurred while writing to the cartridge!");
		StatusOK();
		return FALSE;
	}

	StatusText("Program running - Press OK to exit.");
	ResetNES(RESET_PLAYMODE | RESET_NORESET);
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

BOOL	CMD_RAMCART (void)
{
	PPlugin plugin;
	// select board name
	plugin = PromptPlugin(PLUG_UPLOAD);
	if (plugin == NULL)
		return FALSE;

	if (plugin->num == 0)
		return NRAMcart(plugin->file);
	else if (plugin->num == 1)
		return CNRAMcart(plugin->file);
	else if (plugin->num == 2)
		return UFROMcart(plugin->file);
	else	return FALSE;
}

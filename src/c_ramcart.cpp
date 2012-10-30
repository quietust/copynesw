#include "CopyNESW.h"

#define	CMD_NAME	"RAM Cart"

BOOL	NRAMcart (Plugin *plugin)
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
		PRGsize = 32;
		StatusText("32KB PRG ROM data located...");
	}
	else if (header[4] == 1)
	{
		PRGamt = 1;
		PRGsize = 16;
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
	else MessageBox(topHWnd,"Please set your cartridge to HORIZONTAL mirroring.",MSGBOX_TITLE,MB_OK);

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
	BYTE a[1024];
	StatusText("Sending PRG data...");
	for (i = 0; i < PRGsize; i++)
	{
		fread(&a,1024,1,NES);
		if (!WriteBlock(a, 1024))
		{
			CloseStatus();
			return FALSE;
		}
		StatusPercent((i*100)/PRGsize);
	}
	StatusText("...done!");
	StatusText("Sending CHR data...");
	memset(a, 0, 1024);
	for (i = 0; i < 8; i++)
	{
		if (header[5])
			fread(&a,1024,1,NES);
		if (!WriteBlock(a, 1024))
		{
			CloseStatus();
			return FALSE;
		}
		StatusPercent((i*100)/8);
	}
	fclose(NES);
	StatusText("...done!");
	StatusText("Program running - Press OK to exit.");
	ResetNES(RESET_PLAYMODE | RESET_NORESET);
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

BOOL	CNRAMcart (Plugin *plugin)
{
	char filenes[MAX_PATH];
	FILE *NES;
	int i;
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
	else MessageBox(topHWnd,"Please set your cartridge to HORIZONTAL mirroring.",MSGBOX_TITLE,MB_OK);

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

	BYTE a[1024];
	for (i = 0; i < header[5] * 8; i++)
	{
		fread(&a,1024,1,NES);
		if (!WriteBlock(a, 1024))
		{
			CloseStatus();
			return FALSE;
		}
		StatusPercent((i * 100) / (header[5] * 8));
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
	for (i = 0; i < header[4] * 16; i++)
	{
		fread(&a,1024,1,NES);
		if (!WriteBlock(a, 1024))
		{
			CloseStatus();
			return FALSE;
		}
		StatusPercent((i * 100) / (header[4] * 16));
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

BOOL	UFROMcart (Plugin *plugin)
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
	else MessageBox(topHWnd,"Please set your cartridge to HORIZONTAL mirroring.",MSGBOX_TITLE,MB_OK);

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
	if (!ReadByte(banks))
	{
		fclose(NES);
		CloseStatus();
		return FALSE;
	}
	for (i = 0; i < banks; i++)
	{
		BYTE a;
		if (!ReadByte(a))
		{
			fclose(NES);
			CloseStatus();
			return FALSE;
		}
		StatusPercent(100 * i / banks);
	}
	StatusPercent(100);
	StatusText("...done!");

	BYTE a[1024];
	for (banks = 0; banks < 16; )
	{
		fseek(NES,0x10,SEEK_SET);
		for (i = 0; i < header[4]; i++, banks++)
		{
			for (j = 0; j < 16; j++)
			{
				fread(&a,1024,1,NES);
				if (!WriteBlock(a, 1024))
				{
					fclose(NES);
					CloseStatus();
					return FALSE;
				}
				StatusPercent(((banks * 16 + j) * 100) / 256);
			}
		}
	}
	StatusPercent(100);
	StatusText("...done!");
	fclose(NES);

	if (!ReadByte(banks))
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

BOOL	PowerPakLitecart (Plugin *plugin)
{
	char filenes[MAX_PATH];
	FILE *NES;
	int i;
	BYTE header[16];
	BYTE mapper;
	int maxchr = 4;
	int maxprg = 0;
	BYTE config = 0;

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

	mapper = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);
	if ((mapper != 0) && (mapper != 1) && (mapper != 2) && (mapper != 3) && 
		(mapper != 7) && (mapper != 11) && (mapper != 34) && (mapper != 66) && 
		(MessageBox(topHWnd,"Incorrect iNES mapper detected! Load anyways?",MSGBOX_TITLE,MB_YESNO | MB_ICONQUESTION) == IDNO))
	{
		CloseStatus();
		return FALSE;
	}

	if (mapper == 0)
	{
		maxchr = 1;    //nrom  =  8KB chr
		maxprg = 2;    //nrom  = 32KB prg
		StatusText("NROM (iNES 0)");
	}
	else if (mapper == 1)
	{
		maxchr = 16;   //mmc1  = 128KB chr
		maxprg = 16;   //mmc1  = 256KB prg
		StatusText("MMC1 (iNES 1)");
	}
	else if (mapper == 2)
	{
		maxchr = 0;    //urom  =  0KB chr
		maxprg = 16;   //urom  = 256KB prg
		StatusText("U*NROM (iNES 2)");
	}
	else if (mapper == 3)
	{
		maxchr = 4;    //cnrom = 32KB chr
		maxprg = 2;    //cnrom = 32KB prg
		StatusText("CNROM (iNES 3)");
	}
	else if (mapper == 7)
	{
		maxchr = 0;    //arom  =  0KB chr   
		maxprg = 16;   //arom  = 256KB prg  
		StatusText("A*ROM (iNES 7)");
	}
	else if (mapper == 11)
	{
		maxchr = 16;   //color = 128KB chr   
		maxprg = 8;    //color = 128KB prg  
		StatusText("ColorDreams (iNES 11)");
	}
	else if (mapper == 34)
	{
		maxchr = 0;    //bnrom =  0KB chr
		maxprg = 8;    //bnrom = 128KB prg
		StatusText("BNROM (iNES 34)");
	}
	else if (mapper == 66)
	{
		maxchr = 4;    //gnrom = 32KB chr 
		maxprg = 8;    //gnrom = 128KB prg   
		StatusText("GNROM (iNES 66)");
	}

	if (header[4] > maxprg)
	{
		fclose(NES);
		StatusText("Invalid PRG size, more than %iKB of PRG ROM data was detected!", maxprg * 8);
		StatusOK();
		return FALSE;
	}
	else	StatusText("%iKB PRG ROM data located...", header[4] * 16);

	if (header[5] > maxchr)
		StatusText("More than %iKB of CHR ROM data was detected, sending first %iKB only...", maxchr * 8, maxchr * 8);
	else	StatusText("%iKB CHR ROM data located...", header[5] * 8);

	InitPort();
	StatusText("Resetting CopyNES...");
	ResetNES(RESET_COPYMODE);
	StatusText("Loading plugin...");
	if (!LoadPlugin(plugin))
	{
		CloseStatus();
		fclose(NES);
		return FALSE;
	}
	StatusText("Initializing plugin...");
	RunCode();
	Sleep(SLEEP_SHORT);

	StatusText("Sending PRG data...");
	if (!WriteByte(header[4]))
	{
		CloseStatus();
		fclose(NES);
		return FALSE;
	}
	fseek(NES,0x10,SEEK_SET);

	BYTE a[1024];

	for (i = 0; i < header[4] * 16; i++)
	{
		fread(a, 1024, 1, NES);
		if (!WriteBlock(a, 1024))
		{
			CloseStatus();
			fclose(NES);
			return FALSE;
		}
		StatusPercent((i * 100) / (header[4] * 16));
	}
	StatusPercent(100);
	StatusText("...done!"); 

	StatusText("Sending CHR data...");
	fseek(NES,0x10 + header[4] * 16384,SEEK_SET);
	if (!WriteByte(header[5]))
	{
		CloseStatus();
		fclose(NES);
		return FALSE;
	}

	for (i = 0; i < header[5] * 8; i++)
	{
		fread(a, 1024, 1, NES);
		if (!WriteBlock(a, 1024))
		{
			CloseStatus();
			fclose(NES);
			return FALSE;
		}
		StatusPercent((i * 100) / (header[5] * 8));
	}
	StatusPercent(100);
	StatusText("...done!");
	fclose(NES);

	///SEND CONFIG PRG BYTE
	config = (header[4] - 1) * 16;
	if (mapper == 0) config = config + 0;
	if (mapper == 1) config = config + 1;  
	if (mapper == 2) config = config + 2;
	if (mapper == 3) config = config + 3;  
	if (mapper == 7) config = config + 4;  
	if (mapper == 11) config = config + 5; 
	if (mapper == 34) config = config + 6; 
	if (mapper == 66) config = config + 7; 

	StatusText("config 1 = %i", config); 

	if (!WriteByte(config))
	{
		CloseStatus();
		return FALSE;
	}
	///SEND CONFIG CHR BYTE
	config = (header[5] - 1);
	if (header[5] == 0)
		config = 32;  //chr ram enable bit
	if (header[6] & 1) //mirror=1
		config = config + 16;

	StatusText("config 2 = %i", config); 

	if (!WriteByte(config))
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

BOOL	PowerPakcart (Plugin *plugin)
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
	if (header[4] == 4)
		StatusText("%iKB PRG ROM data located...", header[4] * 16);
	else
	{
		fclose(NES);
		StatusText("Invalid PRG size, must be 64KB PRG!");
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
	if (!ReadByte(banks))
	{
		fclose(NES);
		CloseStatus();
		return FALSE;
	}
	for (i = 0; i < banks; i++)
	{
		BYTE a;
		if (!ReadByte(a))
		{
			fclose(NES);
			CloseStatus();
			return FALSE;
		}
		StatusPercent(100 * i / banks);
	}
	StatusPercent(100);
	StatusText("...done!");

	StatusText("Sending PRG data...");

	BYTE a[1024];

	// 64KB of data, written 4 times in a row
	for (i=0; i<4; i++)
	{
		fseek(NES,0x10,SEEK_SET); // go to start of prg
		for (j=0; j<64; j++)
		{
			fread(a, 1024, 1, NES);
			if (!WriteBlock(a, 1024))
			{
				CloseStatus();
				fclose(NES);
				return FALSE;
			}
			StatusPercent(((i*64 + j) * 100) / 256);  
		} 
	}

	StatusPercent(100);
	StatusText("...done!"); 
	fclose(NES);

	if (!ReadByte(banks))
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
	ResetNES(RESET_PLAYMODE);
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

BOOL	Glidercart (Plugin *plugin)
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

	// Copied from Powerpak code above - below code assumes 64KB of data
	if (header[4] == 4)
		StatusText("%iKB PRG ROM data located...", header[4] * 16);
	else
	{
		fclose(NES);
		StatusText("Invalid PRG size, must be 64KB PRG!");
		StatusOK();
		return FALSE;
	}

	mapper = ((header[6] & 0xF0) >> 4) | (header[7] & 0xF0);

	if (header[5] > 0)
		StatusText("%iKB of CHR ROM data was detected, ignoring...", header[5] * 8);

	InitPort();
	StatusText("Resetting USB CopyNES...");
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
	if (!ReadByte(banks))
	{
		fclose(NES);
		CloseStatus();
		return FALSE;
	}
	for (i = 0; i < banks; i++)
	{
		BYTE a;
		if (!ReadByte(a))
		{
			fclose(NES);
			CloseStatus();
			return FALSE;
		}
		StatusPercent(100 * i / banks);
	}
	StatusPercent(100);
	StatusText("...done!");

	StatusText("Sending PRG data...");

	BYTE a[1024];

	for (i=0; i<4; i++)
	{
		fseek(NES,0x10,SEEK_SET);  //go to start of prg
		for (j=0; j<64; j++)
		{
			fread(a, 1024, 1, NES);  //write 64KB

			if (!WriteBlock(a, 1024))
			{
				CloseStatus();
				fclose(NES);
				return FALSE;
			}
			StatusPercent(((i*64 + j) * 100) / 256);  
		} 
	}

	StatusPercent(100);
	StatusText("...done!"); 
	fclose(NES);

	if (!ReadByte(banks))
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
	ResetNES(RESET_PLAYMODE);
	StatusOK();
	ResetNES(RESET_COPYMODE);
	return TRUE;
}

BOOL	CMD_RAMCART (void)
{
	Plugin *plugin;
	// select board name
	plugin = PromptPlugin(PLUG_UPLOAD);
	if (plugin == NULL)
		return FALSE;

	if (plugin->num == 0)
		return NRAMcart(plugin);
	else if (plugin->num == 1)
		return CNRAMcart(plugin);
	else if (plugin->num == 2)
		return UFROMcart(plugin);
	else if (plugin->num == 3)
		return PowerPakLitecart(plugin);
	else if (plugin->num == 4)
		return PowerPakcart(plugin);
	else if (plugin->num == 5)
		return Glidercart(plugin);
	else return FALSE;
}

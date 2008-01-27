#include "StdAfx.h"
#include <time.h>

static	BYTE	shadow = 0x24;
static	BYTE	c;

char	ROMstring[256];

BOOL	ReadByteEx (BYTE *data, int timeout, BOOL warn)
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
	*data = a;
	return TRUE;
}

BOOL	WriteByteEx (BYTE data, int timeout, BOOL warn)
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

BOOL	ReadByte (BYTE *data)
{
	return ReadByteEx(data, 10, TRUE);
}
BOOL	WriteByte (BYTE data)
{
	return WriteByteEx(data, 10, TRUE);
}

BOOL	ReadByteSilent (BYTE *data)
{
	return ReadByteEx(data, 10, FALSE);
}
BOOL	WriteByteSilent (BYTE data)
{
	return WriteByteEx(data, 10, FALSE);
}

BOOL	ReadByteAsync (void)
{
	BYTE b;
	b = prStatus();		// wait for ACK
	if (!((b ^ c) & 0x20))
		return FALSE;
	return TRUE;
}

BOOL	WriteByteAsync (BYTE data)
{
	shadow &= 0xDF;
	pwControl(shadow);	// set port to output mode
	pwData(data);		// output data
	return TRUE;
}

unsigned char	prECP (void);
void	pwECP (unsigned char);

void	InitPort (void)
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

void	ResetNES (int rtype)
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

BOOL	LoadPlugin (char *plugin)
{
	int w;
	char filename[MAX_PATH];
	FILE *PLUGIN;
	strcpy(filename,Path_PLUG);
	strcat(filename,plugin);
	if ((PLUGIN = fopen(filename,"rb")) == NULL)
	{
		MessageBox(topHWnd,"Failed to load plugin!","Dump",MB_OK | MB_ICONERROR);
		return FALSE;
	}
	if (!WriteCommand(0x4B,0x00,0x04,0x04,0xB4))	// write to CPU space
	{	// failed to load plugin
		fclose(PLUGIN);
		return FALSE;
	}
	fseek(PLUGIN,128,SEEK_SET);
	for (w = 0; w < 1024; w++)
	{
		BYTE a;
		fread(&a,1,1,PLUGIN);
		if (!WriteByte(a))
		{
			fclose(PLUGIN);
			return FALSE;
		}
		if (!(w & 0x1F))
			StatusPercent((w*100)/1024);
	}
	for (w = 0; !feof(PLUGIN); w++)
		fread(&ROMstring[w],1,1,PLUGIN);
	ROMstring[w] = 0;
	fclose(PLUGIN);
	StatusPercent(0);
	Sleep(SLEEP_SHORT);
	return TRUE;
}

BOOL	RunCode (void)
{
	return WriteCommand(0x7E,0x00,0x04,0x00,0xE7);
}
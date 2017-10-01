# News

## May 3, 2013

The source code for CopyNESW is now available on Github.

## November 10, 2012

Another minor update, this one actually loads plugin data from disk correctly and thus should actually work again.

## November 5, 2012

The download package for CopyNESW has been updated - the INI file now has correct initial paths, a parallel port I/O driver installer has been included (which will automatically prompt for elevation on Windows Vista and later), and it now includes the extra plugin binaries needed for additional RAM cart types.

Additionally, extra download links have been added for the original client (for convenience) and the USB driver (packaged separately because it's rather large and you only need to install it once), and the installation instructions have been updated.

## October 30, 2012

I've uploaded another new build, cleaning up the code a whole lot as well as fixing a bunch of bugs I introduced in the previous build.

## October 29, 2012

With timing totally concidental to my return to the NESdev community, I have released an updated version of CopyNESW which uses an updated inpout32.dll (to add support for Windows 7 64-bit), adds support for custom ECP port offsets (since my 64-bit driver uses base+0x8 instead of base+0x400), and incorporates all of the changes from bunnyboy's USB version to provide a unified client.

## February 4, 2007

As I have ceased involvement with the NESdev community, I have released the source code for the Win32 CopyNES client so that others can continue to update the client. A download link can be found below.

## July 12, 2006

The plugin select dialog has been updated to remember the last plugin selected.

## June 17, 2006

The CNRAM plugin has been updated to fix a bug in which it failed to unmap the CopyNES BIOS from memory.

## June 16, 2006

Added support for Memblers's UNROM Flash cartridge to the RAM Cart command.

Also made the RAM/Flash cartridge type selector use the global plugin select dialog.

## March 4, 2006

Support for custom parallel ports has been added - simply select the port name, then override the address in the configuration dialog. In addition, a program restart is no longer required when changing the port configuration.

I have implemented support for custom plugin categories - if you receive an error when attempting to use the "Run Plugin" or "Write WRAM" options, edit MAKEBASE.BAS (from the standard CopyNES distribution) and make the following changes:

Find the line DATA "*"," ",0,"Test & Dev Use".
Replace it with the following lines:
DATA "*"," ",2,"Test & Dev Use"
DATA "xxxx","xxxx.bin",998,"Custom Plugin"
Locate the "WRAM Readers" and "Nintendo: Famicom" groups.
Between them, insert the following category:
DATA "*"," ",1,"Save RAM Uploaders"
DATA "xxxx","xxxx.bin",998,"Custom writer"
DATA "NES-SxROM","sxram.bin",999,"SxROM (MMC1) SRAM writer"
DATA "NES-TxROM","txram.bin",999,"TxROM (MMC3) SRAM writer"
DATA "NES-ExROM","exram.bin",999,"ExROM (MMC5) SRAM writer"
DATA "NES-HxROM","hxram.bin",999,"HxROM (MMC6) SRAM writer"
Save the file and run it in QBASIC to recreate the file MAPPERS.DAT.
February 28, 2006

Support for the additional NetMos PCI parallel adapter I/O base addresses (E000 and E800) has been added (and the parallel port selection combobox has been resized to show all of them at once).

It is also now possible to unselect the parallel port and use the client offline (without having to wait for the version query to timeout).

## February 18, 2006

CopyNESW now features support for parallel ports operating in ECP mode.

In addition, it also now supports the NetMos High Speed PCI parallel adapter when using I/O base address D800 (so I can use my CopyNES with it).

## January 17, 2006

The VRC7 Tuner module has been implemented, and various bugs have been fixed in other modules (including filename selection in the 'Fix GAR' module).

## January 9, 2006

MicroBug is now mostly functional, though there are still some minor lockup bugs that need to be fixed.

## January 3, 2006

Fixed a bug where cancelling from a 'Select Plugin' dialog would crash the program.

I've also started work on MicroBug - currently, selecting the option will simply display the main dialog (without any functionality).

## January 1, 2006

My CopyNES arrived a few days ago, and I've had the opportunity to fix lots of bugs in the client. BankWatch now works properly, and 'RAM Cart' now supports two suboptions (NROM and CNROM, the latter being for a devcart I made).

'Dump Cart' still has a few issues to iron out (it can crash if you cancel at the wrong places), and 'Run Plugin' currently can only select WRAM writers (though fixing this requires a corresponding update to the QBASIC client).

My next priority will be implementing MicroBug, so keep an eye on this site for further updates.

## December 19, 2005

Added proper support for Custom plugin selection - when prompted to select a plugin which is not in the list, simply select "xxxx" from the first category.
You will then be prompted for a UNIF board name, an iNES mapper number, and the location of the plugin binary - for now, the binary MUST be located within the plugins directory (defaults to PLUGDONE).
This allows you to easily test new plugins without having to recreate MAPPERS.DAT.

Several bugs related to the plugin selection dialog have also been fixed.

## December 4, 2005

Created this page to hold the CopyNES Win32 client and relevant information.

# CopyNESW

## About

CopyNESW is a Windows application designed for interfacing with Kevin Horton's CopyNES, a Nintendo Entertainment System modified to
support dumping cartridges, saving and restoring battery-backed RAM, uploading programs to RAM/Flash cartridges, and real-time debugging.
Nearly all functionality of the original MS-DOS QBASIC client has been implemented (in one form or another).

This client can be run on Windows 2000, Windows XP, Windows Vista, and Windows 7, 32-bit and 64-bit. Both the original parallel port 
CopyNES and USB CopyNES are supported by this client; parallel port mode should work on both 32-bit and 64-bit versions of Windows, though 
USB support has not yet been tested in 64-bit mode and may not be available.

## Downloads

- [Original CopyNES Client](http://kevtris.org/Projects/copynes/copynes.zip), written in QBASIC by Kevin Horton.
- [CopyNESW v3.0](http://www.qmtpro.com/~nes/copynes/copynesw_30.zip), the current build of the Win32 CopyNES client (last updated *Fri, 26 May 2017 21:36:52 +0000*).
- [CopyNESW Sources](https://github.com/quietust/copynesw), for anyone interested in adding their own functionality.
- [FTD2xx Driver](http://www.qmtpro.com/~nes/copynes/usb_driver.zip), must be installed if you are using a USB CopyNES

## Instructions

1. Download the original QBASIC client for CopyNES and extract it to a convenient directory, being sure to restore the directory structure.
2. Download the CopyNESW client and extract it into the COPYNES directory, restoring subdirectories. You should be prompted to overwrite CNRAM.BIN - do so.
3. If you are using a parallel port CopyNES, run install_parport_driver.exe - if you are running Windows Vista or Windows 7, you will be prompted to elevate. If you are using a USB CopyNES, download and install the above FTD2xx Driver if you have not already done so.
4. Run CopyNESW.exe. The menu should appear with only a small number of options enabled.
5. Click on the 'Options' button and select which port to use. If you are using a PCI parallel port card, you may need to specify a custom base address and ECP offset based on the I/O addresses it happens to use.
6. Upon pressing 'OK' (and each time you start the program), a version detection dialog should appear for about a second. If you are using an older CopyNES (with version 1 BIOS), the delay will be longer (3 seconds) and the 'Microbug' option will be disabled. If you are using parallel port mode and receive a warning about bidirectional communication, change your parallel port settings in your BIOS and enable ECP mode.
7. Read the CopyNES QBASIC documentation - a great majority of it is relevant to the Win32 version, and the rest should be fairly intuitive.

## Links

- Kevin Horton's [Official CopyNES page](http://kevtris.org/Projects/copynes/index.html)
- [USB CopyNES](http://www.retrousb.com/product_info.php?cPath=24&products_id=36), whose client is based on CopyNESW.
- [CopyNESW Homepage](http://www.qmtpro.com/~nes/copynes/)

Installing otgTeX Live distributions
=====

The distros can be installed directly to your PlayBook by downloading the .zip archive
that you want to your PlayBook and using otgTeX to install.

To download one of the .zip archives, select one of the links below. You can download it
directly to your PlayBook and then use the "TeX Dist" option in the menu in otgTeX.

Some of standard TeX modules were disabled (notably XeTeX) when the binaries were created.
The system has been tested primarily with pdfTeX, but some other *TeX systems should work.
If you need a system that isn't provided by default, you can download the source and build
it as per the instructions in ../INSTALL or ask nicely for someone to help you.

**NOTE:**
If you want to install a new distro on your PlayBook, you need enough free space
for the distro plus enough for the .zip file containing it. The .zip can be deleted
after installation.

Three variants are included that have different file sizes and contain different
packages based on your needs. They are:

1. otgtexlive-2012
	* Size:			1.66 GB (745 MB zipped)	
	* Contents:		This distro contains a full TeX Live 2012 build 
					(http://www.tug.org/texlive/) without any documentation files.
	* Link:			http://github.com/downloads/breakeyd/otgTeX/otgtexlive-2012.zip
2. otgtexlive-2012basic
	* Size:			155 MB (60 MB zipped)
	* Contents: 	This distro contains all the packages included with the basic
					MacTeX distribution (http://www.tug.org/mactex/).
	* Link:			http://github.com/downloads/breakeyd/otgTeX/otgtexlive-2012basic.zip
3. otgtexlive-2012-BaseFonts
	* Size:			665 MB (225 MB zipped)
	* Contents:		This distro contains a full texlive 2012 build but only with
					the fonts included in the basic MacTeX distribution.
	* Link:			http://github.com/downloads/breakeyd/otgTeX/otgtexlive-2012BaseFonts.zip
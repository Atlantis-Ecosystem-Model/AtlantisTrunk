/*  Open library routines */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include "common.h"
#include "dos2unix.h"
#include "unix2dos.h"
#include <sjwlib.h>
#include <netcdf.h>
#include <atlantisboxmodel.h>

#ifdef _WIN32
#include <process.h>
#endif

#ifdef D2U_UNICODE
void StripDelimiterW(FILE* ipInF, FILE* ipOutF, CFlag *ipFlag, wint_t CurChar)
{
	wint_t TempNextChar;
	/* CurChar is always CR (x0d) */
	/* In normal dos2unix mode put nothing (skip CR). */
	/* Don't modify Mac files when in dos2unix mode. */
	if ( (TempNextChar = d2u_getwc(ipInF, ipFlag->bomtype)) != WEOF) {
		d2u_ungetwc( TempNextChar, ipInF, ipFlag->bomtype); /* put back peek char */
		if ( TempNextChar != 0x0a ) {
			d2u_putwc( CurChar, ipOutF, ipFlag); /* Mac line, put back CR */
		}
	}
	else if ( CurChar == 0x0d ) { /* EOF: last Mac line delimiter (CR)? */
		d2u_putwc( CurChar, ipOutF, ipFlag);
	}
	if (ipFlag->NewLine) { /* add additional LF? */
		d2u_putwc(0x0a, ipOutF, ipFlag);
	}
}
#endif

void StripDelimiter(FILE* ipInF, FILE* ipOutF, CFlag *ipFlag, int CurChar) {
	int TempNextChar;
	/* CurChar is always CR (x0d) */
	/* In normal dos2unix mode put nothing (skip CR). */
	/* Don't modify Mac files when in dos2unix mode. */
	if ((TempNextChar = fgetc(ipInF)) != EOF) {
		ungetc(TempNextChar, ipInF); /* put back peek char */
		if (TempNextChar != '\x0a') {
			fputc(CurChar, ipOutF); /* Mac line, put back CR */
		}
	} else if (CurChar == '\x0d') { /* EOF: last Mac line delimiter (CR)? */
		fputc(CurChar, ipOutF);
	}
	if (ipFlag->NewLine) { /* add additional LF? */
		fputc('\x0a', ipOutF);
	}
}

#ifdef D2U_UNICODE
void AddDOSNewLineW(FILE* ipOutF, CFlag *ipFlag, wint_t CurChar, wint_t PrevChar)
{
	if (ipFlag->NewLine) { /* add additional CR-LF? */
		/* Don't add line ending if it is a DOS line ending. Only in case of Unix line ending. */
		if ((CurChar == 0x0a) && (PrevChar != 0x0d)) {
			d2u_putwc(0x0d, ipOutF, ipFlag);
			d2u_putwc(0x0a, ipOutF, ipFlag);
		}
	}
}
#endif

void AddDOSNewLine(FILE* ipOutF, CFlag *ipFlag, int CurChar, int PrevChar) {
	if (ipFlag->NewLine) { /* add additional CR-LF? */
		/* Don't add line ending if it is a DOS line ending. Only in case of Unix line ending. */
		if ((CurChar == '\x0a') && (PrevChar != '\x0d')) {
			fputc('\x0d', ipOutF);
			fputc('\x0a', ipOutF);
		}
	}
}

/* converts stream ipInF to UNIX format text and write to stream ipOutF
 * RetVal: 0  if success
 *         -1  otherwise
 */
int ConvertDosToUnix(FILE* ipInF, FILE* ipOutF, CFlag *ipFlag, char *progname) {
	int RetVal = 0;
	int TempChar;
	int TempNextChar;
	int *ConvTable;
	int line_nr = 1;
	char *errstr;

	ipFlag->status = 0;

	switch (ipFlag->ConvMode) {
	case CONVMODE_ASCII: /* ascii */
	case CONVMODE_UTF16LE: /* Assume UTF-16LE */
	case CONVMODE_UTF16BE: /* Assume UTF-16BE */
		ConvTable = D2UAsciiTable;
		break;
	case CONVMODE_7BIT: /* 7bit */
		ConvTable = D2U7BitTable;
		break;
	case CONVMODE_437: /* iso */
		ConvTable = D2UIso437Table;
		break;
	case CONVMODE_850: /* iso */
		ConvTable = D2UIso850Table;
		break;
	case CONVMODE_860: /* iso */
		ConvTable = D2UIso860Table;
		break;
	case CONVMODE_863: /* iso */
		ConvTable = D2UIso863Table;
		break;
	case CONVMODE_865: /* iso */
		ConvTable = D2UIso865Table;
		break;
	case CONVMODE_1252: /* iso */
		ConvTable = D2UIso1252Table;
		break;
	default: /* unknown convmode */
		ipFlag->status |= WRONG_CODEPAGE;
		return (-1);
	}
	if ((ipFlag->ConvMode > 1) && (!ipFlag->Quiet)) /* not ascii or 7bit */
	{
		fprintf(stderr, "%s: ", progname);
		fprintf(stderr, _("using code page %d.\n"), ipFlag->ConvMode);
	}

	/* CR-LF -> LF */
	/* LF    -> LF, in case the input file is a Unix text file */
	/* CR    -> CR, in dos2unix mode (don't modify Mac file) */
	/* CR    -> LF, in Mac mode */
	/* \x0a = Newline/Line Feed (LF) */
	/* \x0d = Carriage Return (CR) */

	switch (ipFlag->FromToMode) {
	case FROMTO_DOS2UNIX: /* dos2unix */
		while ((TempChar = fgetc(ipInF)) != EOF) { /* get character */
			if ((ipFlag->Force == 0) && (TempChar < 32) && (TempChar != '\x0a') && /* Not an LF */
			(TempChar != '\x0d') && /* Not a CR */
			(TempChar != '\x09') && /* Not a TAB */
			(TempChar != '\x0c')) { /* Not a form feed */
				RetVal = -1;
				ipFlag->status |= BINARY_FILE;
				if (!ipFlag->Quiet) {
					fprintf(stderr, "%s: ", progname);
					fprintf(stderr, _("Binary symbol 0x%02X found at line %d\n"), TempChar, line_nr);
				}
				break;
			}
			if (TempChar != '\x0d') {
				if (TempChar == '\x0a') /* Count all DOS and Unix line breaks */
					++line_nr;
				if (fputc(ConvTable[TempChar], ipOutF) == EOF) {
					RetVal = -1;
					if (!ipFlag->Quiet) {
						errstr = strerror(errno);
						fprintf(stderr, "%s: ", progname);
						fprintf(stderr, _("can not write to output file: %s\n"), errstr);
					}
					break;
				}
			} else {
				StripDelimiter(ipInF, ipOutF, ipFlag, TempChar);
			}
		}
		break;
	case FROMTO_MAC2UNIX: /* mac2unix */
		while ((TempChar = fgetc(ipInF)) != EOF) {
			if ((ipFlag->Force == 0) && (TempChar < 32) && (TempChar != '\x0a') && /* Not an LF */
			(TempChar != '\x0d') && /* Not a CR */
			(TempChar != '\x09') && /* Not a TAB */
			(TempChar != '\x0c')) { /* Not a form feed */
				RetVal = -1;
				ipFlag->status |= BINARY_FILE;
				if (!ipFlag->Quiet) {
					fprintf(stderr, "%s: ", progname);
					fprintf(stderr, _("Binary symbol 0x%02X found at line %d\n"), TempChar, line_nr);
				}
				break;
			}
			if ((TempChar != '\x0d')) {
				if (TempChar == '\x0a') /* Count all DOS and Unix line breaks */
					++line_nr;
				if (fputc(ConvTable[TempChar], ipOutF) == EOF) {
					RetVal = -1;
					if (!ipFlag->Quiet) {
						errstr = strerror(errno);
						fprintf(stderr, "%s: ", progname);
						fprintf(stderr, _("can not write to output file: %s\n"), errstr);
					}
					break;
				}
			} else {
				/* TempChar is a CR */
				if ((TempNextChar = fgetc(ipInF)) != EOF) {
					ungetc(TempNextChar, ipInF); /* put back peek char */
					/* Don't touch this delimiter if it's a CR,LF pair. */
					if (TempNextChar == '\x0a') {
						fputc('\x0d', ipOutF); /* put CR, part of DOS CR-LF */
						continue;
					}
				}
				if (fputc('\x0a', ipOutF) == EOF) /* MAC line end (CR). Put LF */
				{
					RetVal = -1;
					if (!ipFlag->Quiet) {
						errstr = strerror(errno);
						fprintf(stderr, "%s: ", progname);
						fprintf(stderr, _("can not write to output file: %s\n"), errstr);
					}
					break;
				}
				line_nr++; /* Count all Mac line breaks */
				if (ipFlag->NewLine) { /* add additional LF? */
					fputc('\x0a', ipOutF);
				}
			}
		}
		break;
	default: /* unknown FromToMode */
		;
#if DEBUG
		fprintf(stderr, "%s: ", progname);
		fprintf(stderr, _("program error, invalid conversion mode %d\n"),ipFlag->FromToMode);
		exit(1);
#endif
	}
	return RetVal;
}

/* converts stream ipInF to DOS format text and write to stream ipOutF
 * RetVal: 0  if success
 *         -1  otherwise
 */
int ConvertUnixToDos(FILE* ipInF, FILE* ipOutF, CFlag *ipFlag, char *progname) {
	int RetVal = 0;
	int TempChar;
	int PreviousChar = 0;
	int *ConvTable;
	int line_nr = 1;
	char *errstr;

	ipFlag->status = 0;

	switch (ipFlag->ConvMode) {
	case CONVMODE_ASCII: /* ascii */
	case CONVMODE_UTF16LE: /* Assume UTF-16LE */
	case CONVMODE_UTF16BE: /* Assume UTF-16BE */
		ConvTable = U2DAsciiTable;
		break;
	case CONVMODE_7BIT: /* 7bit */
		ConvTable = U2D7BitTable;
		break;
	case CONVMODE_437: /* iso */
		ConvTable = U2DIso437Table;
		break;
	case CONVMODE_850: /* iso */
		ConvTable = U2DIso850Table;
		break;
	case CONVMODE_860: /* iso */
		ConvTable = U2DIso860Table;
		break;
	case CONVMODE_863: /* iso */
		ConvTable = U2DIso863Table;
		break;
	case CONVMODE_865: /* iso */
		ConvTable = U2DIso865Table;
		break;
	case CONVMODE_1252: /* iso */
		ConvTable = U2DIso1252Table;
		break;
	default: /* unknown convmode */
		ipFlag->status |= WRONG_CODEPAGE;
		return (-1);
	}
	if ((ipFlag->ConvMode > 1) && (!ipFlag->Quiet)) /* not ascii or 7bit */
	{
		fprintf(stderr, "%s: ", progname);
		fprintf(stderr, _("using code page %d.\n"), ipFlag->ConvMode);
	}

	/* LF    -> CR-LF */
	/* CR-LF -> CR-LF, in case the input file is a DOS text file */
	/* \x0a = Newline/Line Feed (LF) */
	/* \x0d = Carriage Return (CR) */

	switch (ipFlag->FromToMode) {
	case FROMTO_UNIX2DOS: /* unix2dos */
		while ((TempChar = fgetc(ipInF)) != EOF) { /* get character */
			if ((ipFlag->Force == 0) && (TempChar < 32) && (TempChar != '\x0a') && /* Not an LF */
			(TempChar != '\x0d') && /* Not a CR */
			(TempChar != '\x09') && /* Not a TAB */
			(TempChar != '\x0c')) { /* Not a form feed */
				RetVal = -1;
				ipFlag->status |= BINARY_FILE;
				if (!ipFlag->Quiet) {
					fprintf(stderr, "%s: ", progname);
					fprintf(stderr, _("Binary symbol 0x%02X found at line %d\n"), TempChar, line_nr);
				}
				break;
			}
			if (TempChar == '\x0a') {
				fputc('\x0d', ipOutF); /* got LF, put extra CR */
			} else {
				if (TempChar == '\x0d') /* got CR */
				{
					if ((TempChar = fgetc(ipInF)) == EOF) /* get next char (possibly LF) */
						TempChar = '\x0d'; /* Read error, or end of file. */
					else {
						fputc('\x0d', ipOutF); /* put CR */
						PreviousChar = '\x0d';
					}
				}
			}
			if (TempChar == '\x0a') /* Count all DOS and Unix line breaks */
				++line_nr;
			if (fputc(ConvTable[TempChar], ipOutF) == EOF) /* put LF or other char */
			{
				RetVal = -1;
				if (!ipFlag->Quiet) {
					errstr = strerror(errno);
					fprintf(stderr, "%s: ", progname);
					fprintf(stderr, _("can not write to output file: %s\n"), errstr);
				}
				break;
			} else {
				AddDOSNewLine(ipOutF, ipFlag, TempChar, PreviousChar);
			}
			PreviousChar = TempChar;
		}
		break;
	case FROMTO_UNIX2MAC: /* unix2mac */
		while ((TempChar = fgetc(ipInF)) != EOF) {
			if ((ipFlag->Force == 0) && (TempChar < 32) && (TempChar != '\x0a') && /* Not an LF */
			(TempChar != '\x0d') && /* Not a CR */
			(TempChar != '\x09') && /* Not a TAB */
			(TempChar != '\x0c')) { /* Not a form feed */
				RetVal = -1;
				ipFlag->status |= BINARY_FILE;
				if (!ipFlag->Quiet) {
					fprintf(stderr, "%s: ", progname);
					fprintf(stderr, _("Binary symbol 0x%02X found at line %d\n"), TempChar, line_nr);
				}
				break;
			}
			if ((TempChar != '\x0a')) /* Not an LF */
			{
				if (fputc(ConvTable[TempChar], ipOutF) == EOF) {
					RetVal = -1;
					if (!ipFlag->Quiet) {
						errstr = strerror(errno);
						fprintf(stderr, "%s: ", progname);
						fprintf(stderr, _("can not write to output file: %s\n"), errstr);
					}
					break;
				}
				PreviousChar = TempChar;
			} else {
				/* TempChar is an LF */
				++line_nr;
				/* Don't touch this delimiter if it's a CR,LF pair. */
				if (PreviousChar == '\x0d') {
					if (fputc('\x0a', ipOutF) == EOF) /* CR,LF pair. Put LF */
					{
						RetVal = -1;
						if (!ipFlag->Quiet) {
							errstr = strerror(errno);
							fprintf(stderr, "%s: ", progname);
							fprintf(stderr, _("can not write to output file: %s\n"), errstr);
						}
						break;
					}
					PreviousChar = TempChar;
					continue;
				}
				PreviousChar = TempChar;
				if (fputc('\x0d', ipOutF) == EOF) /* Unix line end (LF). Put CR */
				{
					RetVal = -1;
					if (!ipFlag->Quiet) {
						errstr = strerror(errno);
						fprintf(stderr, "%s: ", progname);
						fprintf(stderr, _("can not write to output file: %s\n"), errstr);
					}
					break;
				}
				if (ipFlag->NewLine) { /* add additional CR? */
					fputc('\x0d', ipOutF);
				}
			}
		}
		break;
	default: /* unknown FromToMode */
		;
#if DEBUG
		fprintf(stderr, "%s: ", progname);
		fprintf(stderr, _("program error, invalid conversion mode %d\n"),ipFlag->FromToMode);
		exit(1);
#endif
	}
	return RetVal;
}

int Convert_File_Format(char *fileName, char *tempFileName, int lenStr) {

	CFlag *pFlag;
	FILE *inputFile;
	FILE *outputFile;

#ifdef _WIN32
	snprintf(tempFileName, lenStr, "TempFile_%d.txt", _getpid());
#else
	snprintf(tempFileName, lenStr, "TempFile_%d.txt", getpid());
#endif

	if ((inputFile = fopen(fileName, "r")) == NULL)
		return -1;
	outputFile = fopen(tempFileName, "w");

	/* What operating system is this? */
#ifdef _WIN32
	pFlag = (CFlag*)malloc(sizeof(CFlag));
	pFlag->NewFile = 0;
	pFlag->Quiet = 0;
	pFlag->KeepDate = 0;
	pFlag->ConvMode = CONVMODE_ASCII; /* default ascii */
	pFlag->FromToMode = FROMTO_DOS2UNIX; /* default dos2unix */
	pFlag->NewLine = 0;
	pFlag->Force = 0;
	pFlag->Follow = SYMLINK_SKIP;
	pFlag->status = 0;
	pFlag->stdio_mode = 1;
	pFlag->error = 0;

	ConvertUnixToDos(inputFile, outputFile, pFlag, "atlantis");

#else

	pFlag = (CFlag*) malloc(sizeof(CFlag));
	pFlag->NewFile = 0;
	pFlag->Quiet = 0;
	pFlag->KeepDate = 0;
	pFlag->ConvMode = CONVMODE_ASCII; /* default ascii */
	pFlag->FromToMode = FROMTO_UNIX2DOS; /* default unix2dos */
	pFlag->NewLine = 0;
	pFlag->Force = 0;
	pFlag->Follow = SYMLINK_SKIP;
	pFlag->status = 0;
	pFlag->stdio_mode = 1;
	pFlag->error = 0;

	ConvertDosToUnix(inputFile, outputFile, pFlag, "atlantis");

#endif

	fclose(inputFile);
	fclose(outputFile);

	return TRUE;
}

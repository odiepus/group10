#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <math.h>
#include "BitmapReader.h"


// Global Variables for File Data Pointers
BITMAPFILEHEADER *pSrcFileHdr, *pTgtFileHdr;
BITMAPINFOHEADER *pSrcInfoHdr, *pTgtInfoHdr;
RGBQUAD *pSrcColorTable, *pTgtColorTable;
unsigned char *pSrcFile, *pTgtFile, *pSrcData, *pTgtData;
int srcFileSize, tgtFileSize;

unsigned char toCGC[256];
unsigned char toPBC[256];

// default values
unsigned char gNumLSB = 1, gMask = 0xfe, gShift = 7;
float threshold = .3;

// this function builds the canonical gray code array variables
void buildGrayCode()
{
	int i, length, posUp, posDw, cnt;

	length = 1;
	toCGC[0] = 0;
	toPBC[0] = 0;
	cnt = 1;

	while (length < 256)
	{
		posUp = length - 1;
		posDw = length;
		for (i = 0; i < length; i++)
		{
			toCGC[i + posDw] = toCGC[posUp - i] + length;
			toPBC[toCGC[i + posDw]] = cnt++;
		}
		length = length * 2;
	}
	return;
} // buildGrayCode

  // Show the various bitmap header bytes primarily for debugging
void displayFileInfo(char *pFileName,
	BITMAPFILEHEADER *pFileHdr,
	BITMAPINFOHEADER *pFileInfo,
	RGBQUAD *pColorTable,
	unsigned char *pData)
{
	// first two characters in bitmap file should be "BM"
	char *pp = (char *) &(pFileHdr->bfType);
	int numColors, i;
	RGBQUAD *pCT = pColorTable;

	printf("\nFile Info for %s: \n\n", pFileName);

	printf("File Header Info:\n");
	printf("File Type: %c%c\nFile Size:%d\nData Offset:%d\n\n",
		*pp, *(pp + 1), pFileHdr->bfSize, pFileHdr->bfOffBits);

	switch (pFileInfo->biBitCount)
	{
	case 1:
		numColors = 2;
		break;
	case 4:
		numColors = 16;
		break;
	case 8:
		numColors = 256;
		break;
	case 16:
		numColors = 65536;
		break;
	case 24:
		numColors = 16777216;
		break;
	default:
		numColors = 16777216;
	}

	printf("Bit Map Image Info:\n\nSize:%d\nWidth:%d\nHeight:%d\nPlanes:%d\n"
		"Bits/Pixel:%d ==> %d colors\n"
		"Compression:%d\nImage Size:%d\nRes X:%d\nRes Y:%d\nColors:%d\nImportant Colors:%d\n\n",
		pFileInfo->biSize,
		pFileInfo->biWidth,
		pFileInfo->biHeight,
		pFileInfo->biPlanes,
		pFileInfo->biBitCount, numColors,
		pFileInfo->biCompression,
		pFileInfo->biSizeImage,
		pFileInfo->biXPelsPerMeter,
		pFileInfo->biYPelsPerMeter,
		pFileInfo->biClrUsed,
		pFileInfo->biClrImportant);

	//* Display Color Tables if Desired

	//	There are no color tables
	if (pFileInfo->biBitCount > 16) return;

	//	only needed for debugging
	/*printf("Color Table:\n\n");

	for (i = 0; i < numColors; i++)
	{
		printf("R:%02x   G:%02x   B:%02x\n", pCT->rgbRed, pCT->rgbGreen, pCT->rgbBlue);
		pCT++;
	}
	*/

	return;
} // displayFileInfo

  // reads specified bitmap file from disk
unsigned char *readFile(char *fileName, int *fileSize)
{
	FILE *ptrFile;
	unsigned char *pFile;

	ptrFile = fopen(fileName, "rb");	// specify read only and binary (no CR/LF added)

	if (ptrFile == NULL)
	{
		printf("Error in opening file: %s.\n\n", fileName);
		return(NULL);
	}

	*fileSize = filelength(fileno(ptrFile));

	// malloc memory to hold the file, include room for the header and color table
	pFile = (unsigned char *)malloc(*fileSize);

	if (pFile == NULL)
	{
		printf("Memory could not be allocated in readFile.\n\n");
		return(NULL);
	}

	// Read in complete file
	// buffer for data, size of each item, max # items, ptr to the file
	fread(pFile, sizeof(unsigned char), *fileSize, ptrFile);
	fclose(ptrFile);
	return(pFile);
} // readFile

  // writes modified bitmap file to disk
  // gMask used to determine the name of the file
int writeFile(unsigned char *pFile, int fileSize, char *fileName)
{
	FILE *ptrFile;
	char newFileName[256], msk[4];
	int x;

	// convert the mask value to a string
	sprintf(msk, "%02x", gMask);

	// make a new filename based upon the original
	strcpy(newFileName, fileName);

	// remove the .bmp (assumed)
	x = (int)strlen(newFileName) - 4;
	newFileName[x] = 0;

	strcat(newFileName, "_mask_");
	strcat(newFileName, msk);	// name indicates which bit plane(s) was/were saved

								// add the .bmp back to the file name
	strcat(newFileName, ".bmp");

	// open the new file, MUST set binary format (text format will add line feed characters)
	ptrFile = fopen(newFileName, "wb+");
	if (ptrFile == NULL)
	{
		printf("Error opening new file for writing.\n\n");
		return(FAILURE);
	}

	// write the file
	x = (int)fwrite(pFile, sizeof(unsigned char), fileSize, ptrFile);

	// check for success
	if (x != fileSize)
	{
		printf("Error writing file %s.\n\n", newFileName);
		return(FAILURE);
	}
	fclose(ptrFile); // close file
	return(SUCCESS);
} // writeFile


  // conjugate message data if not above threshold
void conjugateBlkMap(int index) {
}

// prints help hide message to the screen
void printHelpHide()
{
	printf("StegoProject: Hiding Mode:\n");
	printf("Usage: StegoProject.exe -h 'source filename' 'target filename' ['threshold']\n\n");
	printf("\tsource filename:\tThe name of the bitmap file to hide.\n");
	printf("\ttarget filename:\tThe name of the bitmap file to conceal within the source.\n");
	printf("\tthreshold:\t\tThe number of bits to hide, range is (.3 - .5).\n");
	printf("\t\tIf not specified .3 bits will be used as the default.\n\n");
	return;
} // printHelpHide

  // prints help extract message to the screen
void printHelpExtract()
{
	printf("StegoProject: Extracting Mode:\n");
	printf("Usage: StegoProject.exe -e 'stego filename' ['threshold']\n\n");
	printf("\tstego filename:\t\tThe name of the file in which a bitmap may be hidden.\n");
	printf("\tthreshold:\t\tThe number of bits to hide, range is (.3 - .5).\n");
	printf("\t\tIf not specified .3 bits will be used as the default.\n\n");
	return;
} // printHelpExtract


int main(int argc, char *argv[])
{
	int x;

	if (argc < 3 || argc > 5)
	{
		printHelpHide();
		printHelpExtract();
		return FAILURE;
	}

	// initialize gray code conversion table
	buildGrayCode();

	// get the number of bits to use for data hiding or data extracting
	// if not specified, default to one
	if ((strcmp(argv[1], "-h") == 0 && argc == 5) || (strcmp(argv[1], "-e") == 0 && argc == 4))
	{
		//assigns the threshold for hiding/extracting and assigns a default if not entered or invalid range
		if (strcmp(argv[1], "-h") == 0)
			threshold = atof(argv[4]);
		else if (strcmp(argv[1], "-e") == 0)
			threshold = atof(argv[3]);

		if (threshold < .3 || threshold > .5)
		{
			threshold = .3;
			printf("The number specified for Threshold was invalid, using the default value of '.3'.\n\n");
		}
	}

	// read the source file
	pSrcFile = readFile(argv[2], &srcFileSize);
	if (pSrcFile == NULL) return -1;

	// Set up pointers to various parts of the source file
	pSrcFileHdr = (BITMAPFILEHEADER *)pSrcFile;
	pSrcInfoHdr = (BITMAPINFOHEADER *)(pSrcFile + sizeof(BITMAPFILEHEADER));
	// pointer to first RGB color palette, follows file header and bitmap header
	pSrcColorTable = (RGBQUAD *)(pSrcFile + sizeof(BITMAPFILEHEADER) + pSrcInfoHdr->biSize);
	// file header indicates where image data begins
	pSrcData = pSrcFile + pSrcFileHdr->bfOffBits;

	// for debugging purposes, show file info on the screen
	displayFileInfo(argv[2], pSrcFileHdr, pSrcInfoHdr, pSrcColorTable, pSrcData);


	// read the target file
	if (strcmp(argv[1], "-h") == 0)
	{
		pTgtFile = readFile(argv[3], &tgtFileSize);
		if (pTgtFile == NULL) return -1;

		// Set up pointers to various parts of file
		pTgtFileHdr = (BITMAPFILEHEADER *)pTgtFile;
		pTgtInfoHdr = (BITMAPINFOHEADER *)(pTgtFile + sizeof(BITMAPFILEHEADER));
		pTgtColorTable = (RGBQUAD *)(pTgtFile + sizeof(BITMAPFILEHEADER) + pTgtInfoHdr->biSize);
		pTgtData = pTgtFile + pTgtFileHdr->bfOffBits;

		// for debugging purposes, show file info on the screen
		displayFileInfo(argv[3], pTgtFileHdr, pTgtInfoHdr, pTgtColorTable, pTgtData);



		// write the file to disk
		//x = writeFile(pTgtFile, pTgtFileHdr->bfSize, argv[3]);
	}

	return -1;
}
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

// function borrowed from Professor John Ortiz BPCS code
// reads specified bitmap file from disk
unsigned char *readFile(char *fileName, int *fileSize)
{
	FILE *ptrFile;
	unsigned char *pFile;

	ptrFile = fopen(fileName, "rb");	// specify read only and binary (no CR/LF added)

	if(ptrFile == NULL)
	{
		printf("Error in opening file: %s.\n\n", fileName);
		return(NULL);
	}

	*fileSize = filelength( fileno( ptrFile ) );

	// malloc memory to hold the file, include room for the header and color table
	pFile = (unsigned char *) malloc(*fileSize);

	if(pFile == NULL)
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


// function borrowed from Professor John Ortiz BPCS code
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
	x = (int) strlen(newFileName) - 4;
	newFileName[x] = 0;

	strcat(newFileName, "_mask_");
	strcat(newFileName, msk);	// name indicates which bit plane(s) was/were saved

	// add the .bmp back to the file name
	strcat(newFileName, ".bmp");

	// open the new file, MUST set binary format (text format will add line feed characters)
	ptrFile = fopen(newFileName, "wb+");
	if(ptrFile == NULL)
	{
		printf("Error opening new file for writing.\n\n");
		return(FAILURE);
	}

	// write the file
	x = (int) fwrite(pFile, sizeof(unsigned char), fileSize, ptrFile);

	// check for success
	if(x != fileSize)
	{
		printf("Error writing file %s.\n\n", newFileName);
		return(FAILURE);
	}
	fclose(ptrFile); // close file
	return(SUCCESS);
} // writeFile



// this function builds the canonical gray code array variables
void buildGrayCode()
{
	int i, length, posUp, posDw, cnt;

	length = 1;
	toCGC[0] = 0;
	toPBC[0] = 0;
	cnt = 1;

	while(length < 256)
	{
		posUp = length - 1;
		posDw = length;
		for(i = 0; i < length; i++)
		{
			toCGC[i + posDw] = toCGC[posUp - i] + length;
			toPBC[toCGC[i + posDw]] = cnt++;
		}
		length = length * 2;
	}
	return;
} // buildGrayCode


// conjugate message data if not above threshold
void conjugateBlkMap(int index){
}

// prints help message to the screen
void printHelp()
{
	printf("Usage: Steg_BPSC'source filename' 'target filename' [bits to use] \n\n");
	printf("Where 'source filename' is the name of the bitmap file to hide.\n");
	printf("Where 'target filename' is the name of the bitmap file to conceal the source.\n");
	printf("To extract data from the source, name the target file \"ex\".\n");
	printf("To bit slice the source, name the target file \"bs\".\n");
	printf("The number of bits to hide or extract, range is (1 - 7).\n");
	printf("If not specified, 1 bit is used as the default.\n\n");
	return;
} // printHelp


// Parameters are used to indicate the input file and available options
void main(int argc, char *argv[])
{
	int x;

	if(argc < 3 || argc > 4)
	{
		printHelp();
		return;
	}

	// read the target file
	pTgtFile = readFile(argv[2], &tgtFileSize);
	if(pTgtFile == NULL) return;

    return 0;
}


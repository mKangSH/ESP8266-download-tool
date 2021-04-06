#pragma warning(disable: 6054; disable: 6031)

#define SYNC_SIZE 46

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "ESP8266_SW_ROM_command.h"

int main()
{
	wchar_t COMPortMask[20] = L"\\\\.\\";
	inputComPort(COMPortMask);

	HANDLE hMasterCOM = CreateFile(COMPortMask, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, 0);
	PurgeComm(hMasterCOM, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);

	DCB dcbMasterInitState;
	GetCommState(hMasterCOM, &dcbMasterInitState);

	DCB dcbMaster = dcbMasterInitState;

	dcbMaster.BaudRate = CBR_57600;
	dcbMaster.Parity = NOPARITY;
	dcbMaster.ByteSize = 8;
	dcbMaster.StopBits = ONESTOPBIT;

	SetCommState(hMasterCOM, &dcbMaster);


	DWORD dwWritten = 0;
	BYTE* checkSum = (BYTE*)calloc(0x208, 1);
	
	if (writeData(hMasterCOM, Sync, SYNC_SIZE, &dwWritten) == false)
	{
		printf("Sync command error");
		exit(1);
	}

	if (writeData(hMasterCOM, Sync, SYNC_SIZE, &dwWritten) == false)
	{
		printf("Sync command error");
		exit(1);
	}	

	char fileName[MAX_PATH] = "factory_WROOM-02N_portChangedByJ.Min.bin";
	//inputFileName(fileName);
	FILE* fp = fopen((const char*)fileName, "rb");

	if (fp == NULL)
	{
		printf("can't open file\n");
		exit(1);
	}

	checkSumCalculate(fp, checkSum);
	
	fseek(fp, 0, SEEK_SET);
	memBegin(hMasterCOM, &dwWritten);
	memDataByFile(hMasterCOM, &dwWritten, fp, checkSum);

	if (writeData(hMasterCOM, MEM_END, 18, &dwWritten) == false)
	{
		printf("mem_end write error");
		exit(1);
	}

	fseek(fp, 0, SEEK_SET);
	flashBegin(hMasterCOM, &dwWritten);
	ReadPacket(hMasterCOM);
	flashDataByFile(hMasterCOM, &dwWritten, fp, checkSum);

	if(writeData(hMasterCOM, FLASH_END, 14, &dwWritten) == false)
	{
		printf("flash_end write error");
		exit(1);
	}

	fclose(fp);
	CloseHandle(hMasterCOM);
	hMasterCOM = INVALID_HANDLE_VALUE;
	
	free(checkSum);

	return 0;
}

/*
	if (readData(hMasterCOM, readCommand, 100, &dwRead, 200) == true)
	{
		for (int i = 0; i < 150; i++)
		{
			printf("%02x\t", readCommand[i]);
		}
	}
*/


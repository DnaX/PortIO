// D.Iracà 14/02/2005 last version

#include "diio.h" // attenzione!! qui ci sono le definizioni dei codici IOCTL !!!

#include <stdio.h>

HANDLE IODevice;

BOOL OpenIO(char *);
BOOL CloseIO(void);
BYTE Inp(UINT addr);
void Outp(BYTE val, UINT addr);
BOOL SetInterruptHandlerFunction(ULONG CallBack, unsigned int vector);
unsigned long GetIntCount(void);




BOOL OpenIO(char *DriverName)
{
	char buff[80];
	
	strcpy(buff, "\\\\.\\"); strcat(buff , DriverName);

	IODevice = CreateFile(buff, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	if (IODevice == INVALID_HANDLE_VALUE)
	{
		printf("Can't open "DRIVERNAME" device!! error code %d\n", GetLastError());
		return FALSE;
	}

	return TRUE;
}


BOOL CloseIO() { return(CloseHandle(IODevice)); }



BYTE Inp(UINT addr)
{
 DWORD junk;

// DeviceIoControl codice IOCTL_IN: ingresso un long con l'indirizzo di lettura;
// ritorna un long nei cui 8 bit bassi c'è il byte letto.
 if(DeviceIoControl(IODevice, IOCTL_IN, (PVOID)&addr, sizeof(addr), (PVOID)&addr, sizeof(addr), &junk, NULL)) 
	 return((BYTE)(addr & 0xff));
 return(0);
}


// DeviceIoControl codice IOCTL_OUT: ingresso un long con l'indirizzo di scrittura nei 16 bit meno 
// significativi e il valore da scrivere in testa nei bit più significativi
void Outp(BYTE val, UINT addr)
{
	DWORD junk;
	ULONG tmp = addr | ((ULONG)val << 16);

	DeviceIoControl(IODevice, IOCTL_OUT, (PVOID)&tmp, sizeof(tmp), NULL, 0, &junk, NULL);
}


BOOL SetInterruptHandlerFunction(ULONG CallBack, unsigned int vector)
{
	ULONG tmp = CallBack;
	DWORD junk;

	if(!DeviceIoControl(IODevice, IOCTL_SETCALLBACK, (PVOID)&tmp, sizeof(tmp), NULL, 0, &junk, NULL))
		return FALSE;
	
	tmp = vector;

	if(!DeviceIoControl(IODevice, IOCTL_SETISAVECTOR, (PVOID)&tmp, sizeof(tmp), NULL, 0, &junk, NULL))
		return FALSE;

	return TRUE;
}



unsigned long GetIntCount()
{
	DWORD junk;
	DWORD IntCount = 0;
	
	DeviceIoControl(IODevice, IOCTL_READINTCOUNT, NULL, 0, (PVOID)&IntCount, sizeof(IntCount), &junk, NULL);

	return IntCount;
}



/*
BOOL DeviceIoControl(
  HANDLE hDevice,              // handle to device of interest
  DWORD dwIoControlCode,       // control code of operation to perform
  LPVOID lpInBuffer,           // pointer to buffer to supply input data
  DWORD nInBufferSize,         // size, in bytes, of input buffer
  LPVOID lpOutBuffer,          // pointer to buffer to receive output data
  DWORD nOutBufferSize,        // size, in bytes, of output buffer
  LPDWORD lpBytesReturned,     // pointer to variable to receive byte count
  LPOVERLAPPED lpOverlapped    // pointer to structure for asynchronous operation
);
*/

// D.Iracà 14/02/2005 last version

#include <windows.h>

#define DRIVERNAME "ISAKerPlug"

extern HANDLE IODevice;

extern BOOL OpenIO(char *);
extern BOOL CloseIO(void);
extern BYTE Inp(UINT addr);
extern void Outp(BYTE val, UINT addr);

extern BOOL SetInterruptHandlerFunction(ULONG InterruptHandler, unsigned int vector);
extern unsigned long GetIntCount(void);

#include <winioctl.h>

#define IOCTL_IN                    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_OUT                   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READINTCOUNT          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_READINTFLAG           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SETCALLBACK           CTL_CODE(FILE_DEVICE_UNKNOWN, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SETISAVECTOR          CTL_CODE(FILE_DEVICE_UNKNOWN, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define	SET_INTERRUPT_HANDLER_FUNCTION(InterruptHandler, vector) SetInterruptHandlerFunction((ULONG) InterruptHandler, (unsigned int )vector)

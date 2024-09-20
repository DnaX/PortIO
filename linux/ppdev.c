#include <stdio.h>
#include <unistd.h>
/*#include <asm/io.h>*/
#include <linux/ioctl.h>
#include <linux/parport.h>
#include <linux/ppdev.h>
#include <fcntl.h>

#define SPPDATAPORT    0x378
#define SPPSTATUSPORT  (SPPDATAPORT + 1)
#define SPPCONTROLPORT (SPPDATAPORT + 2)

#define OUTPUTENABLE 0x02
#define OUTPUTLATCH  0x04

struct ppdev_frob_struct frob;

int main(int argc, char *argv[])
{
  int fd, mode;
  unsigned char status, data;

/* 1. get the file descriptor for the parallel port */
  fd = open("/dev/parport0",O_RDWR);
  if (fd == -1)
  {
    perror("open");
    exit(1);
  }

/* 2. request access to the port */
  if (ioctl(fd,PPCLAIM))
  {
    perror("PPCLAIM");
    close(fd);
    exit(1);
  }

/* 3. configure the port for SPP mode */
  mode = IEEE1284_MODE_COMPAT;
  if (ioctl(fd, PPNEGOT, &mode))
  {
    perror ("PPNEGOT");
    close (fd);
    return 1;
  }

/* 4. assert the latch's OUTPUTENABLE signal */
  frob.mask = OUTPUTENABLE ;
  frob.val = OUTPUTENABLE;
  ioctl(fd, PPFCONTROL, &frob);

/* 5. put the command line argument 1 on the data bus */
  ioctl(fd,PPWDATA,0xff);



/* 6. toggle the OUTPUTLATCH signal to latch data */
  frob.mask = OUTPUTENABLE | OUTPUTLATCH ;
  frob.val = OUTPUTENABLE | OUTPUTLATCH;
  ioctl(fd, PPFCONTROL, &frob);

    sleep(10);

frob.mask = OUTPUTENABLE ;
  frob.val = OUTPUTENABLE;
  ioctl(fd, PPFCONTROL, &frob);

/* 7. release the port */
  ioctl (fd, PPRELEASE);

/* 8. close the device file */
  close(fd);
}


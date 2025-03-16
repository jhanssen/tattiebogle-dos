#include "atapi.h"
#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <i86.h>

#define INCL_LBA48      0        // set to 1 to include 48-bit LBA

#define INCL_PCI_DMA    1        // set to 1 to include PCI DMA

extern "C" {
#include <ATAIO.H>
} // extern "C"

#define BUFFER_SIZE 4096
static unsigned char buffer[BUFFER_SIZE];
static unsigned char far* bufferPtr;

static unsigned char cdb[16];
static unsigned char far* cdbPtr;

static void ClearTrace(void)
{
    // clear the command history and low level traces
    trc_cht_dump0();     // zero the command history
    trc_llt_dump0();     // zero the low level trace
}

static void pause(void)
{
    // clear any queued up keys
    while (kbhit())
    {
        if (getch() == 0)
            getch();
    }
    // pause until key hit
    printf("Press any key to continue...\n");
    while (!kbhit())
        /* do nothing */ ;
    // clear any queued up keys
    while (kbhit())
    {
        if (getch() == 0)
            getch();
    }
}

ATAPI* ATAPI::sInstance = NULL;

ATAPI::ATAPI()
    : mDev(-1)
{
}

ATAPI::~ATAPI()
{
}

bool ATAPI::initialize(Controller controller, Device device)
{
    if (sInstance == 0) {
        bufferPtr = (unsigned char far*)buffer;
        cdbPtr = (unsigned char far*)cdb;

        reg_buffer_size = BUFFER_SIZE;
        tmr_time_out = 20;

        sInstance = new ATAPI();

        int priSec = (controller == PRIMARY) ? 0 : 1;
        int dev = (device == MASTER) ? 0 : 1;

        int cmdBase  = priSec ? 0x170 : 0x1f0;
        int ctrlBase = priSec ? 0x370 : 0x3f0;
        int irqNum = priSec ? 15 : 14;

        pio_set_iobase_addr(cmdBase, ctrlBase, 0);
        int numDev = reg_config();

        if (numDev < 1) {
            return false;
        }

        ClearTrace();
        int rc = reg_reset(0, dev);
        if (rc) {
            return false;
        }

        // do an ATAPI Identify command in LBA mode
        ClearTrace();

        memset(buffer, 0, sizeof(buffer));
        rc = reg_pio_data_in_lba28(
            dev, CMD_IDENTIFY_DEVICE_PACKET,
            0, 0,
            0L,
            FP_SEG(bufferPtr), FP_OFF(bufferPtr),
            1L, 0 );
        if (rc) {
            return false;
        }

        ClearTrace();

        sInstance->mDev = dev;
    }
    return true;
}

void ATAPI::destroy()
{
    delete sInstance;
    sInstance = NULL;
}

long ATAPI::sendPacket(const unsigned char* cdb, int cdbLen)
{
    cdbPtr = (unsigned char far*)cdb;
    memset(buffer, 0, sizeof(buffer));
    int rc = reg_packet(
        mDev,
        cdbLen, FP_SEG(cdbPtr), FP_OFF(cdbPtr),
        0,
        4096, FP_SEG(bufferPtr), FP_OFF(bufferPtr),
        0L    // lba for tracing
        );
    if (rc) {
        return -1;
    }
    return reg_cmd_info.totalBytesXfer;
}

void ATAPI::printError()
{
    int lc = 0;
    unsigned char * cp;

    printf("ERROR !\n");

    // display the command error information
    trc_err_dump1();           // start
    while (1)
    {
        cp = trc_err_dump2();   // get and display a line
        if (cp == NULL)
            break;
        printf("* %s\n", cp);
    }
    pause();

    // display the command history
    trc_cht_dump1();           // start
    while (1)
    {
        cp = trc_cht_dump2();   // get and display a line
        if (cp == NULL)
            break;
        printf("* %s\n", cp);
        lc ++ ;
        if (!(lc & 0x000f))
            pause();
    }

    // display the low level trace
    trc_llt_dump1();           // start
    while (1)
    {
        cp = trc_llt_dump2();   // get and display a line
        if (cp == NULL)
            break;
        printf("* %s\n", cp);
        lc ++ ;
        if (!(lc & 0x000f))
            pause();
    }

    if (lc & 0x000f)
        pause();
}

const unsigned char* ATAPI::responseData()
{
    return buffer;
}

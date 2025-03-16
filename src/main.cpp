#include <conio.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <i86.h>

#define INCL_LBA48      0        // set to 1 to include 48-bit LBA

#define INCL_PCI_DMA    1        // set to 1 to include PCI DMA

extern "C" {
#include <ATAIO.H>
} // extern "C"

static void pause( void )

{

    // clear any queued up keys
    while ( kbhit() )
    {
        if ( getch() == 0 )
            getch();
    }
    // pause until key hit
    printf( "Press any key to continue...\n" );
    while ( ! kbhit() )
        /* do nothing */ ;
    // clear any queued up keys
    while ( kbhit() )
    {
        if ( getch() == 0 )
            getch();
    }
}

void ShowAll( void )

{
    int lc = 0;
    unsigned char * cp;

    printf( "ERROR !\n" );

    // display the command error information
    trc_err_dump1();           // start
    while ( 1 )
    {
        cp = trc_err_dump2();   // get and display a line
        if ( cp == NULL )
            break;
        printf( "* %s\n", cp );
    }
    pause();

    // display the command history
    trc_cht_dump1();           // start
    while ( 1 )
    {
        cp = trc_cht_dump2();   // get and display a line
        if ( cp == NULL )
            break;
        printf( "* %s\n", cp );
        lc ++ ;
        if ( ! ( lc & 0x000f ) )
            pause();
    }

    // display the low level trace
    trc_llt_dump1();           // start
    while ( 1 )
    {
        cp = trc_llt_dump2();   // get and display a line
        if ( cp == NULL )
            break;
        printf( "* %s\n", cp );
        lc ++ ;
        if ( ! ( lc & 0x000f ) )
            pause();
    }

    if ( lc & 0x000f )
        pause();
}

void ClearTrace( void )

{

    // clear the command history and low level traces
    trc_cht_dump0();     // zero the command history
    trc_llt_dump0();     // zero the low level trace
}

#define BUFFER_SIZE 4096
unsigned char buffer[BUFFER_SIZE];
unsigned char far * bufferPtr;

unsigned char cdb[16];
unsigned char far * cdbPtr;

const char * devTypeStr[]
= { "NO DEVICE", "UNKNOWN TYPE", "ATA", "ATAPI" };

int main(int, char**)
{
    // initialize far pointer to the I/O buffer
    bufferPtr = (unsigned char far *) buffer;

    // tell ATADRVR how big the buffer is
    reg_buffer_size = BUFFER_SIZE;

    // set the ATADRVR command timeout (in seconds)
    tmr_time_out = 20;

    // initialize far pointer to the CBD buffer
    cdbPtr = (unsigned char far *) cdb;

    // priSec = 0 for primary, 1 for secondary
    int priSec = 1;
    // dev = 0 for master, 1 for slave
    int dev = 0;

    int ndx;

    int cmdBase  = priSec ? 0x170 : 0x1f0;
    int ctrlBase = priSec ? 0x370 : 0x3f0;
    int irqNum = priSec ? 15 : 14;

    pio_set_iobase_addr( cmdBase, ctrlBase, 0 );
    int numDev = reg_config();

    printf( "hey3 Found %d devices, dev 0 is %s, dev 1 is %s.\n",
            numDev,
            devTypeStr[ reg_config_info[0] ],
            devTypeStr[ reg_config_info[1] ] );

    if ( numDev < 1 )
        ShowAll();
    pause();

    //---------- Try some commands in "polling" mode

    printf( "Polling mode...\n" );

    // do an ATA soft reset (SRST) and return the command block
    // regs for device 0 in struct reg_cmd_info
    ClearTrace();
    printf( "Soft Reset...\n" );
    int rc = reg_reset( 0, dev );
    if ( rc )
        ShowAll();

    // do an ATAPI Identify command in LBA mode
    ClearTrace();
    printf( "ATAPI Identify, polling...\n" );
    memset( buffer, 0, sizeof( buffer ) );
    rc = reg_pio_data_in_lba28(
        dev, CMD_IDENTIFY_DEVICE_PACKET,
        0, 0,
        0L,
        FP_SEG( bufferPtr ), FP_OFF( bufferPtr ),
        1L, 0 );
    if ( rc )
        ShowAll();
    else
    {
        // you get to add the code here to display all the ID data
        // display the first 16 bytes read
        printf( "   data read %02X%02X%02X%02X %02X%02X%02X%02X "
                "%02X%02X%02X%02X %02X%02X%02X%02X\n",
                buffer[ 0], buffer[ 1], buffer[ 2], buffer[ 3],
                buffer[ 4], buffer[ 5], buffer[ 6], buffer[ 7],
                buffer[ 8], buffer[ 9], buffer[10], buffer[11],
                buffer[12], buffer[13], buffer[14], buffer[15] );
    }

    ClearTrace();
    printf("ATAPI Inquiry\n");
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = 0x12;    // Inquiry command code
    cdb[4] = 36;      // allocation length
    memset(buffer, 0, sizeof(buffer));
    rc = reg_packet(
        dev,
        12, FP_SEG(cdbPtr), FP_OFF(cdbPtr),
        0,
        4096, FP_SEG(bufferPtr), FP_OFF(bufferPtr),
        0L    // lba for tracing
        );

    pause();
    if ( rc )
        ShowAll();
    else
    {
        printf("inquiry result length %d\n", reg_cmd_info.totalBytesXfer);
        for (int i = 8; i < 16; ++i) {
            printf("%c", buffer[i]);
        }
        printf("\n");
    }

    pause();

    /*
    // do an ATAPI Read TOC command and display the TOC data
    ClearTrace();
    printf( "ATAPI CD-ROM Read TOC, polling...\n" );
    memset( cdb, 0, sizeof( cdb ) );
    cdb[0] = 0x43;    // command code
    cdb[1] = 0x02;    // MSF flag
    cdb[7] = 0x10;    // allocation length
    cdb[8] = 0x00;    //    of 4096
    cdb[9] = 0x80;    // TOC format
    memset( buffer, 0, sizeof( buffer ) );
    rc = reg_packet(
        dev,
        12, FP_SEG( cdbPtr ), FP_OFF( cdbPtr ),
        0,
        4096, FP_SEG( bufferPtr ), FP_OFF( bufferPtr ),
        0L    // lba for tracing
        );
    if ( rc )
        ShowAll();
    else
    {
        // and here too I'll give you some help looking at the TOC data
        // again, check the number of bytes transferred (notice
        // how every SCSI like command is different here)
        if ( reg_cmd_info.totalBytesXfer
             !=
             ( 2U + ( buffer[0] * 256U ) + buffer[1] )
            )
            printf( "Number of bytes transferred (%ld) does not match \n"
                    "2 + bytes 0-1 in the TOC data received (%u) ! \n",
                    reg_cmd_info.totalBytesXfer,
                    2U + ( buffer[0] * 256U ) + buffer[1] );
        // display the TOC data
        printf( "First Session=%02X, Last Session=%02X\n",
                buffer[2], buffer[3] );
        printf( "TOC entries (11 bytes each)... \n" );
        rc = ( ( buffer[0] * 256U ) + buffer[1] - 2U ) / 11;
        ndx = 4;
        while ( rc > 0 )
        {
            printf( "   %02X %02X %02X "
                    "%02X %02x %02X "
                    "%02X %02X %02X "
                    "%02X %02x \n",
                    buffer[ndx+0], buffer[ndx+1], buffer[ndx+2],
                    buffer[ndx+3], buffer[ndx+4], buffer[ndx+5],
                    buffer[ndx+6], buffer[ndx+7], buffer[ndx+8],
                    buffer[ndx+9], buffer[ndx+10], buffer[ndx+11]
                );
            rc -- ;
            ndx = ndx + 11;
        }
        pause();
    }
    */

    return 0;
}

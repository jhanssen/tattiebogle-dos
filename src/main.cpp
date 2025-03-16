#include "atapi.h"
#include <stdio.h>
#include <string.h>

static unsigned char cdb[16];

int main(int, char**)
{
    if (!ATAPI::initialize(ATAPI::SECONDARY, ATAPI::MASTER)) {
        printf("ATAPI initialization failed\n");
        ATAPI::instance()->printError();
        ATAPI::destroy();
        return 1;
    }

    ATAPI* atapi = ATAPI::instance();

    printf("ATAPI Inquiry\n");
    memset(cdb, 0, sizeof(cdb));
    cdb[0] = 0x12;    // Inquiry command code
    cdb[4] = 36;      // allocation length

    long responseLength = atapi->sendPacket(cdb);
    if (responseLength == -1) {
        printf("ATAPI sendPacket failed\n");
        atapi->printError();
        ATAPI::destroy();
        return 2;
    }

    const unsigned char* buffer = atapi->responseData();
    printf("inquiry result length %d\n", responseLength);
    for (int i = 8; i < 16; ++i) {
        printf("%c", buffer[i]);
    }
    printf("\n");

    return 0;
}

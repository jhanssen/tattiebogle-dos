#include "atapi.h"
#include "args.h"
#include "ini.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static unsigned char cdb[16];

// "MICE    "
static unsigned char tattiebogle[8] = {
    0x4D, 0x49, 0x43, 0x45, 0x20, 0x20, 0x20, 0x20
};

int main(int argc, char** argv)
{
    Args args(argc, argv);

    if (args.hasArg("h") || args.hasArg("?") || args.hasArg("help")) {
        printf("Usage: cdemu [/C <controller>] [/D <device>]\n");
        printf("  /C <controller>  'S' for secondary, default is primary\n");
        printf("  /D <device>      'S' for slave, default is master\n");
        return 0;
    }

    Ini ini("cdemu.ini");

    ATAPI::Controller controller = ATAPI::PRIMARY;
    ATAPI::Device device = ATAPI::MASTER;

    const char* cini = ini.asString("cdemu", "controller");
    if (cini != NULL && tolower(cini[0]) == 's') {
        controller = ATAPI::SECONDARY;
    }

    const char* dini = ini.asString("cdemu", "device");
    if (dini != NULL && tolower(dini[0]) == 's') {
        device = ATAPI::SLAVE;
    }

    const char* carg = args.argAsString("c");
    if (carg != NULL && tolower(carg[0]) == 's') {
        controller = ATAPI::SECONDARY;
    }

    const char* darg = args.argAsString("d");
    if (darg != NULL && tolower(darg[0]) == 's') {
        device = ATAPI::SLAVE;
    }

    printf("Using %s controller, %s device\n",
        controller == ATAPI::PRIMARY ? "primary" : "secondary",
        device == ATAPI::MASTER ? "master" : "slave");

    if (!ATAPI::initialize(controller, device)) {
        printf("ATAPI initialization failed\nPossibly no ATAPI drive present\n");
        ATAPI::instance()->printError();
        ATAPI::destroy();
        return 1;
    }

    ATAPI* atapi = ATAPI::instance();

    memset(cdb, 0, sizeof(cdb));
    cdb[0] = 0x12;    // Inquiry command code
    cdb[4] = 36;      // allocation length

    long responseLength = atapi->sendPacket(cdb, 12);
    if (responseLength == -1) {
        printf("ATAPI inqury failed\n");
        atapi->printError();
        ATAPI::destroy();
        return 2;
    } else if (responseLength != 36) {
        printf("ATAPI inqury failed\nExpected response length 36, got %ld\n", responseLength);
        ATAPI::destroy();
        return 2;
    }

    const unsigned char* buffer = atapi->responseData();
    for (int i = 0; i < 8; ++i) {
        if (buffer[i + 8] != tattiebogle[i]) {
            printf("No tattiebogle ATAPI device found, got \"");
            for (int j = 0; j < 8; ++j) {
                printf("%c", buffer[j + 8]);
            }
            printf("\"\n");
            ATAPI::destroy();
            return 3;
        }
    }
    printf("\n");

    ATAPI::destroy();
    return 0;
}

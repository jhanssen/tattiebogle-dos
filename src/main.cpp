#include "atapi.h"
#include "args.h"
#include "ini.h"
#include <ctype.h>
#include <direct.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>

static unsigned char cdb[16];

// "MICE    "
static unsigned char tattiebogle[8] = {
    0x4D, 0x49, 0x43, 0x45, 0x20, 0x20, 0x20, 0x20
};

static char* programDirectory(char* argv0)
{
    // first, see if there path is relative or not
    // msdos uses backslashes instead of forward slashes

    // is there a drive letter in the path
    bool isDrive = false;
    bool isAbsolute = false;

    const int len = strlen(argv0);
    if (len > 2 && argv0[1] == ':') {
        isDrive = true;
        if (len > 3 && argv0[2] == '\\') {
            // it's a drive letter
            isAbsolute = true;
        }
    } else if (len > 0 && argv0[0] == '\\') {
        isAbsolute = true;
    }

    // find the last backslash, if any
    int backslash = -1;
    for (int i = strlen(argv0) - 1; i >= 0; --i) {
        if (argv0[i] == '\\') {
            backslash = i;
            break;
        }
    }

    //printf("programDirectory: argv0 '%s' isDrive %d, isAbsolute %d, backslash %d\n", argv0, isDrive, isAbsolute, backslash);

    // seems like argv0 is always absolute?
    if (isAbsolute) {
        if (backslash != -1) {
            char* dir = new char[backslash + 2];
            strncpy(dir, argv0, backslash + 1);
            dir[backslash + 1] = 0;
            return dir;
        }
        // should not happen
        return NULL;
    }

    char buffer[512];
    // if we have a drive letter, chdir to the drive
    if (isDrive) {
        buffer[0] = argv0[0];
        buffer[1] = ':';
        buffer[2] = 0;
        chdir(buffer);
    }

    char* basepath = getcwd(buffer, sizeof(buffer));
    const int baselen = strlen(basepath);

    const int start = isDrive ? 2 : 0;
    // append up until the last backslash, if any
    if (backslash != -1) {
        for (int i = start; i <= backslash; ++i) {
            if (baselen + i - start < sizeof(buffer)) {
                buffer[baselen + i - start] = argv0[i];
            } else {
                return NULL;
            }
        }
        buffer[baselen + backslash + 1 - start] = 0;
    }

    char* dir = new char[baselen + backslash + 2];
    strncpy(dir, buffer, baselen + backslash + 1);
    dir[baselen + backslash + 1] = 0;
    return dir;
}

int main(int argc, char** argv)
{
    Args args(argc, argv);

    if (args.hasArg("h") || args.hasArg("?") || args.hasArg("help")) {
        printf("Usage: cdemu [/C <controller>] [/D <device>]\n");
        printf("  /C <controller>  'P' for primary, 'S' for secondary, default is primary\n");
        printf("  /D <device>      'M' for master, 'S' for slave, default is master\n");
        return 0;
    }

    char iniFilePath[512];
    const char* iniFile = "cdemu.ini";
    char* programDir = programDirectory(argv[0]);
    if (programDir != NULL) {
        snprintf(iniFilePath, sizeof(iniFilePath), "%s%s", programDir, iniFile);
        iniFile = iniFilePath;
        delete[] programDir;
    }

    Ini ini(iniFile);

    ATAPI::Controller controller = ATAPI::PRIMARY;
    ATAPI::Device device = ATAPI::MASTER;

    // check ini file first
    const char* cini = ini.asString("cdemu", "controller");
    if (cini != NULL) {
        if (strcasecmp(cini, "primary") == 0) {
            controller = ATAPI::PRIMARY;
        } else if (strcasecmp(cini, "secondary") == 0) {
            controller = ATAPI::SECONDARY;
        } else {
            printf("Invalid controller in ini file: '%s'\n", cini);
            return 1;
        }
    }

    const char* dini = ini.asString("cdemu", "device");
    if (dini != NULL) {
        if (strcasecmp(dini, "master") == 0) {
            device = ATAPI::MASTER;
        } else if (strcasecmp(dini, "slave") == 0) {
            device = ATAPI::SLAVE;
        } else {
            printf("Invalid device in ini file: '%s'\n", dini);
            return 1;
        }
    }

    // command line args override ini so they come after
    const char* carg = args.argAsString("c");
    if (carg != NULL) {
        const int clen = strlen(carg);
        if (clen == 1 && tolower(carg[0]) == 'p') {
            controller = ATAPI::PRIMARY;
        } else if (clen == 1 && tolower(carg[0]) == 's') {
            controller = ATAPI::SECONDARY;
        } else {
            printf("Invalid controller: '%s'\n", carg);
            return 1;
        }
    }

    const char* darg = args.argAsString("d");
    if (darg != NULL) {
        const int dlen = strlen(darg);
        if (dlen == 1 && tolower(darg[0]) == 'm') {
            device = ATAPI::MASTER;
        } else if (dlen == 1 && tolower(darg[0]) == 's') {
            device = ATAPI::SLAVE;
        } else {
            printf("Invalid device: '%s'\n", darg);
            return 1;
        }
    }

    printf("Using %s controller, %s device\n",
        controller == ATAPI::PRIMARY ? "primary" : "secondary",
        device == ATAPI::MASTER ? "master" : "slave");

    if (!ATAPI::initialize(controller, device)) {
        printf("ATAPI initialization failed\nPossibly no ATAPI drive present\n");
        ATAPI::instance()->printError();
        ATAPI::destroy();
        return 2;
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
        return 3;
    } else if (responseLength != 36) {
        printf("ATAPI inqury failed\nExpected response length 36, got %ld\n", responseLength);
        ATAPI::destroy();
        return 3;
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
            return 4;
        }
    }
    printf("\n");

    ATAPI::destroy();
    return 0;
}

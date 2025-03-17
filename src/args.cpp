#include "Args.h"
#include <ctype.h>
#include <limits.h>
#include <stdlib.h>

Args::Args(int argc, char** argv)
{
    int isOption = 0;
    for (int i = 0; i < argc; ++i) {
        switch (argv[i][0]) {
        case '-':
        case '/':
            // option
            if (isOption > 0) {
                // previous arg was an option with no value
                mArgs.insert(Arg());
                Arg& arg = mArgs[mArgs.entries() - 1];

                // lower case the name
                arg.name = strdup(&argv[i - 1][isOption]);
                for (char* p = (char*)arg.name; *p; ++p) {
                    *p = tolower(*p);
                }
                arg.value = NULL;
                arg.valueInt = INT_MAX;
            }
            isOption = argv[i][1] == '-' ? 2 : 1;
            break;
        default:
            // free argument?
            if (isOption > 0) {
                // no, option arg
                mArgs.insert(Arg());
                Arg& arg = mArgs[mArgs.entries() - 1];

                // lower case the name
                arg.name = strdup(&argv[i - 1][isOption]);
                for (char* p = (char*)arg.name; *p; ++p) {
                    *p = tolower(*p);
                }
                arg.value = argv[i];
                arg.valueInt = INT_MAX;

                isOption = 0;
            } else {
                // yes, free arg
                mFreeArgs.insert(FreeArg());
                FreeArg& arg = mFreeArgs[mArgs.entries() - 1];

                arg.value = argv[i];
            }
            break;
        }
    }

    if (isOption > 0) {
        // last arg was an option with no value
        mArgs.insert(Arg());
        Arg& arg = mArgs[mArgs.entries() - 1];

        // lower case the name
        arg.name = strdup(&argv[argc - 1][isOption]);
        for (char* p = (char*)arg.name; *p; ++p) {
            *p = tolower(*p);
        }
        arg.value = NULL;
        arg.valueInt = INT_MAX;
    }
}

Args::~Args()
{
    // free all names
    for (int i = 0; i < mArgs.entries(); ++i) {
        free((void*)mArgs[i].name);
    }
}

int Args::freeArgs() const
{
    return mFreeArgs.entries();
}

const char* Args::freeArg(int index) const
{
    if (index < 0 || index >= mFreeArgs.entries()) {
        return NULL;
    }
    return mFreeArgs[index].value;
}

bool Args::hasArg(const char* name) const
{
    Arg arg;
    arg.name = name;
    return mArgs.contains(arg) != 0;
}

int Args::argAsInt(const char* name) const
{
    Arg arg;
    arg.name = name;
    int index = mArgs.index(arg);
    if (index == -1 || mArgs[index].value == NULL) {
        return INT_MAX;
    }
    if (mArgs[index].valueInt == INT_MAX) {
        Arg* mutableArg = const_cast<Arg*>(&mArgs[index]);
        mutableArg->valueInt = atoi(mArgs[index].value);
    }
    return mArgs[index].valueInt;
}

const char* Args::argAsString(const char* name) const
{
    Arg arg;
    arg.name = name;
    int index = mArgs.index(arg);
    if (index == -1) {
        return NULL;
    }
    return mArgs[index].value;
}

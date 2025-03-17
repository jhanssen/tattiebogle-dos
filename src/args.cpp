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
                arg.name = &argv[i - 1][isOption];
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
                arg.name = &argv[i - 1][isOption];
                arg.value = argv[i];
                arg.valueInt = INT_MAX;

                isOption = 0;
            } else {
                // yes, free arg
                mFreeArgs.insert(FreeArg());
                FreeArg& arg = mFreeArgs[mArgs.entries() - 1];

                arg.value = argv[i];
                arg.valueInt = INT_MAX;
            }
            break;
        }
    }

    if (isOption > 0) {
        // last arg was an option with no value
        mArgs.insert(Arg());
        Arg& arg = mArgs[mArgs.entries() - 1];

        // lower case the name
        arg.name = &argv[argc - 1][isOption];
        arg.value = NULL;
        arg.valueInt = INT_MAX;
    }
}

Args::~Args()
{
}

int Args::freeArgs() const
{
    return mFreeArgs.entries();
}

const char* Args::freeArgAsString(int index) const
{
    if (index < 0 || index >= mFreeArgs.entries()) {
        return NULL;
    }
    return mFreeArgs[index].value;
}

int Args::freeArgAsInt(int index) const
{
    if (index < 0 || index >= mFreeArgs.entries()) {
        return NULL;
    }
    if (mFreeArgs[index].valueInt == INT_MAX) {
        FreeArg* mutableArg = const_cast<FreeArg*>(&mFreeArgs[index]);
        char* end;
        long val = strtol(mFreeArgs[index].value, &end, 10);
        if (*end == '\0') {
            mutableArg->valueInt = static_cast<int>(val);
        } else {
            mutableArg->valueInt = INT_MIN;
        }
    }
    return mFreeArgs[index].valueInt != INT_MIN ? mFreeArgs[index].valueInt : INT_MAX;
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
        char* end;
        long val = strtol(mArgs[index].value, &end, 10);
        if (*end == '\0') {
            mutableArg->valueInt = static_cast<int>(val);
        } else {
            mutableArg->valueInt = INT_MIN;
        }
    }
    return mArgs[index].valueInt != INT_MIN ? mArgs[index].valueInt : INT_MAX;
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

bool Args::Arg::operator==(const Arg& other) const
{
    return !strcasecmp(name, other.name);
}

bool Args::FreeArg::operator==(const FreeArg& other) const
{
    return !strcasecmp(value, other.value);
}

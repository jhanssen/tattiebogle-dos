#ifndef ARGS_H
#define ARGS_H

#include <strings.h>
#include <wcvector.h>

class Args
{
public:
    Args(int argc, char** argv);
    ~Args();

    int freeArgs() const;
    int findFreeArg(const char* name) const;
    const char* freeArgAsString(int index) const;
    int freeArgAsInt(int index) const;

    bool hasArg(const char* name) const;
    int argAsInt(const char* name) const;
    const char* argAsString(const char* name) const;

private:
    struct Arg {
        const char* name;
        const char* value;
        int valueInt;

        bool operator==(const Arg& other) const;
    };

    struct FreeArg {
        const char* value;
        int valueInt;

        bool operator==(const FreeArg& other) const;
    };

    WCValOrderedVector<Arg> mArgs;
    WCValOrderedVector<FreeArg> mFreeArgs;
};

#endif // ARGS_H

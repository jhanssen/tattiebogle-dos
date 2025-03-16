#ifndef ARGS_H
#define ARGS_H

#include <string.h>
#include <wcvector.h>

class Args
{
public:
    Args(int argc, char** argv);
    ~Args();

    int freeArgs() const;
    const char* freeArg(int index) const;

    bool hasArg(const char* name) const;
    int argAsInt(const char* name) const;
    const char* argAsString(const char* name) const;

private:
    struct Arg {
        const char* name;
        const char* value;
        int valueInt;

        bool operator==(const Arg& other) const {
            return !strcmp(name, other.name);
        }
    };

    struct FreeArg {
        const char* value;

        bool operator==(const FreeArg& other) const {
            return !strcmp(value, other.value);
        }
    };

    WCValOrderedVector<Arg> mArgs;
    WCValOrderedVector<FreeArg> mFreeArgs;
};

#endif // ARGS_H

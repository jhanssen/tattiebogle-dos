#ifndef INI_H
#define INI_H

#include <strings.h>
#include <wcvector.h>

class Ini
{
public:
    Ini(const char* filename);
    ~Ini();

    const char* asString(const char* key) const;
    int asInt(const char* key) const;

    const char* asString(const char* section, const char* key) const;
    int asInt(const char* section, const char* key) const;

private:
    void parse();

private:
    struct Entry {
        const char* key;
        const char* value;
        int valueInt;

        bool operator==(const Entry& other) const;
    };

    struct Section {
        const char* name;
        WCValOrderedVector<Entry> entries;

        bool operator==(const Section& other) const;
    };

    const Entry* findEntry(const char* section, const char* key) const;

    WCValOrderedVector<Section> mSections;
    char* mContents;
};

#endif

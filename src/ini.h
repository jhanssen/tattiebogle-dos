#ifndef INI_H
#define INI_H

#include <string.h>
#include <wcvector.h>

class Ini
{
public:
    Ini(const char* filename);
    ~Ini();

    const char* asString(const char* section, const char* key) const;
    int asInt(const char* section, const char* key) const;

private:
    void parse();

private:
    struct Entry {
        const char* key;
        const char* value;
        int valueInt;

        bool operator==(const Entry& other) const {
            return !strcmp(key, other.key);
        }
    };

    struct Section {
        const char* name;
        WCValOrderedVector<Entry> entries;

        bool operator==(const Section& other) const {
            return (name == NULL && other.name == NULL) || !strcmp(name, other.name);
        }
    };

    const Entry* findEntry(const char* section, const char* key) const;

    WCValOrderedVector<Section> mSections;
    char* mContents;
};

#endif

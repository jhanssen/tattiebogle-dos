#include "ini.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

Ini::Ini(const char* filename)
    : mContents(NULL)
{
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    mContents = new char[size + 1];
    fread(mContents, 1, static_cast<size_t>(size), f);
    mContents[size] = 0;
    fclose(f);

    parse();
}

Ini::~Ini()
{
    delete[] mContents;
}

void Ini::parse()
{
    // parse mContents into lines, stripping whitespace as we go
    // then parse lines into sections and keys

    // push a global section
    Section globalSection;
    globalSection.name = NULL;
    mSections.insert(globalSection);

    char* prev = mContents;
    char* nl = strchr(mContents, '\n');
    while (nl != NULL) {
        // process line from prev to nl
        // first, trim whitespace
        *nl = 0;
        char* line = prev;
        while (*line == ' ' || *line == '\t') {
            line++;
        }
        char* end = nl - 1;
        while (end > line && (*end == ' ' || *end == '\t' || *end == '\r')) {
            end--;
        }
        *(end + 1) = 0;
        prev = nl + 1;

        // check if this is a section
        if (*line == '[' && *(end) == ']') {
            // this is a section
            *end = 0;
            Section section;
            section.name = line + 1;
            mSections.insert(section);
        } else {
            // this is a key=value pair
            char* eq = strchr(line, '=');
            if (eq != NULL) {
                char* origEq = eq;

                // trim whitespace before the '='
                while (*(eq - 1) == ' ' || *(eq - 1) == '\t') {
                    eq--;
                }
                *eq = 0;

                const char* key = line;
                const char* value = origEq + 1;
                // trim whitespace after the '='
                while (*value == ' ' || *value == '\t') {
                    value++;
                }

                // add entry to the current section
                Entry entry;
                entry.key = key;
                entry.value = value;
                entry.valueInt = INT_MAX;

                assert(mSections.entries() > 0);
                mSections[mSections.entries() - 1].entries.insert(entry);
            }
        }
    }
}

const Ini::Entry* Ini::findEntry(const char* section, const char* key) const
{

    Section searchSection;
    searchSection.name = section;
    int sectionIndex = mSections.index(searchSection);
    if (sectionIndex == -1) {
        return NULL;
    }
    const Section& sec = mSections[sectionIndex];

    Entry searchEntry;
    searchEntry.key = key;
    int entryIndex = sec.entries.index(searchEntry);
    if (entryIndex == -1) {
        return NULL;
    }
    return &sec.entries[entryIndex];
}

const char* Ini::asString(const char* section, const char* key) const
{
    const Entry* entry = findEntry(section, key);
    if (entry == NULL) {
        return NULL;
    }
    return entry->value;
}

const char* Ini::asString(const char* key) const
{
    return asString(NULL, key);
}

int Ini::asInt(const char* section, const char* key) const
{
    const Entry* entry = findEntry(section, key);
    if (entry == NULL) {
        return INT_MAX;
    }
    if (entry->valueInt == INT_MAX) {
        Entry* mutableEntry = const_cast<Entry*>(entry);
        char* end;
        long val = strtol(entry->value, &end, 10);
        if (*end == '\0') {
            mutableEntry->valueInt = static_cast<int>(val);
        } else {
            mutableEntry->valueInt = INT_MIN;
        }
    }
    return entry->valueInt != INT_MIN ? entry->valueInt : INT_MAX;
}

int Ini::asInt(const char* key) const
{
    return asInt(NULL, key);
}

bool Ini::Entry::operator==(const Entry& other) const
{
    return !strcasecmp(key, other.key);
}

bool Ini::Section::operator==(const Section& other) const
{
    return (name == NULL && other.name == NULL) || !strcasecmp(name, other.name);
}

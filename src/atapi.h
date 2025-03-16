#ifndef ATAPI_H
#define ATAPI_H

class ATAPI
{
public:
    ~ATAPI();

    enum Controller {
        PRIMARY,
        SECONDARY
    };

    enum Device {
        MASTER,
        SLAVE
    };

    static bool initialize(Controller controller, Device device);
    static void destroy();

    static ATAPI* instance();

    long sendPacket(const unsigned char* cdb);
    void printError();

    const unsigned char* responseData();

private:
    ATAPI();

    int mDev;

    static ATAPI* sInstance;
};

inline ATAPI* ATAPI::instance()
{
    return sInstance;
}

#endif // ATAPI_H

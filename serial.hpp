#ifndef SERIAL_HPP
#define SERIAL_HPP

#ifdef WIN32
#include <windows.h>
#endif
#include <functional>
#include <stdint.h>

class Serial
{
public:
    Serial(const char *options, std::function<void(uint8_t)> read);
    void write(uint8_t byte);
    void event_loop();
private:
    std::function<void(uint8_t)> read;
#ifdef WIN32
    HANDLE hComm;
#else
    int pty;
#endif
};

#endif

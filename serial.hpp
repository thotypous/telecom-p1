#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <functional>
#include <stdint.h>

class Serial
{
public:
    Serial(const char *options, std::function<void(uint8_t)> rx);
    void tx(uint8_t byte);
    void event_loop();
private:
    std::function<void(uint8_t)> rx;
#ifdef WIN32
#else
    int pty;
#endif
};

#endif

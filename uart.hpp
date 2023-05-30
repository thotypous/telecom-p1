#ifndef UART_HPP
#define UART_HPP

#include <vector>
#include <deque>
#include "config.hpp"

typedef void (*UartRxCallback)(unsigned int byte);

class UART_RX
{
public:
    UART_RX(UartRxCallback get_byte) :get_byte(get_byte) {}
    void put_samples(std::vector<unsigned int> &buffer);
private:
    UartRxCallback get_byte;
};

class UART_TX
{
public:
    void put_byte(unsigned int byte);
    void get_samples(std::vector<unsigned int> &buffer);
private:
    std::deque<unsigned int> samples;
    void put_bit(unsigned int bit);
};

#endif

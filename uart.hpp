#ifndef UART_HPP
#define UART_HPP

#include <deque>

#include "config.hpp"

class UART_RX
{
};

class UART_TX
{
public:
    void put_byte(unsigned int byte);
    void get_samples(unsigned int *buffer, unsigned int nsamples);
private:
    std::deque<unsigned int> samples;
    void put_bit(unsigned int bit);
};

#endif

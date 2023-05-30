#ifndef UART_HPP
#define UART_HPP

#include <vector>
#include <deque>
#include "config.hpp"

class UART_RX
{
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

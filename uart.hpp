#ifndef UART_HPP
#define UART_HPP

#include <functional>
#include <vector>
#include <deque>
#include <mutex>
#include <stdint.h>
#include "config.hpp"

class UART_RX
{
public:
    UART_RX(std::function<void(uint8_t)> get_byte) :get_byte(get_byte) {}
    void put_samples(std::vector<unsigned int> &buffer);
private:
    std::function<void(uint8_t)> get_byte;
};

class UART_TX
{
public:
    void put_byte(uint8_t byte);
    void get_samples(std::vector<unsigned int> &buffer);
private:
    std::deque<unsigned int> samples;
    std::mutex samples_mutex;
    void put_bit(unsigned int bit);
};

#endif

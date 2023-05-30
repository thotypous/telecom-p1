#include "uart.hpp"
#include "config.hpp"

void UART_RX::put_samples(std::vector<unsigned int> &buffer)
{
    // seu código aqui
}

void UART_TX::put_byte(uint8_t byte)
{
    put_bit(0);  // start bit
    for (int i = 0; i < 8; i++) {
        put_bit(byte & 1);
        byte >>= 1;
    }
    put_bit(1);  // stop bit
}

void UART_TX::get_samples(std::vector<unsigned int> &buffer)
{
    std::vector<unsigned int>::size_type i = 0;
    while (!samples.empty() && i < buffer.size()) {
        buffer[i++] = samples.front();
        samples.pop_front();
    }
    while (i < buffer.size()) {
        // idle
        buffer[i++] = 1;
    }
}

void UART_TX::put_bit(unsigned int bit)
{
    for (int i = 0; i < SAMPLES_PER_SYMBOL; i++) {
        samples.push_back(bit);
    }
}

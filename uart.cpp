#include "uart.hpp"
#include "config.hpp"


void UART_TX::put_byte(unsigned int byte) {
    put_bit(0);  // start bit
    for (int i = 0; i < 8; i++) {
        put_bit(byte & 1);
        byte >>= 1;
    }
    put_bit(1);  // stop bit
}

void UART_TX::get_samples(unsigned int *buffer, unsigned int nsamples) {
    while (!samples.empty() && nsamples > 0) {
        *buffer++ = samples.front();
        samples.pop_front();
        --nsamples;
    }
    while (nsamples > 0) {
        // idle
        *buffer++ = 1;
        --nsamples;
    }
}

void UART_TX::put_bit(unsigned int bit)
{
    for (int i = 0; i < SAMPLES_PER_SYMBOL; i++) {
        samples.push_back(bit);
    }
}

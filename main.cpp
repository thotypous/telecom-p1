#include <iostream>
#include "config.hpp"
#include "uart.hpp"

int main(int argc, char **argv) {
    UART_TX uart_tx;
    uart_tx.put_byte('H');
    for (int i = 0; i < 16; i++) {
        unsigned int samples[1024];
        uart_tx.get_samples(samples, 1024);
        for (int j = 0; j < 1024; j++) {
            std::cout << samples[j] << ", ";
        }
    }
    return 0;
}

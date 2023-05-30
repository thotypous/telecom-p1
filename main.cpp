#include <iostream>
#include "config.hpp"
#include "uart.hpp"

int main(int argc, char **argv) {
    UART_TX uart_tx;
    uart_tx.put_byte('H');
    for (int i = 0; i < 16; i++) {
        std::vector<unsigned int> samples(1024, 0);
        uart_tx.get_samples(samples);
        for (int j = 0; j < samples.size(); j++) {
            std::cout << samples[j] << ", ";
        }
    }
    return 0;
}

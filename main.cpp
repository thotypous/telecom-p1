#include <iostream>
#include <getopt.h>
#include "uart.hpp"
#include "serial.hpp"

int main(int argc, char **argv) {
    UART_TX uart_tx;
    Serial serial("", [&uart_tx](uint8_t b){ uart_tx.put_byte(b); });
    serial.event_loop();
    return 0;
}

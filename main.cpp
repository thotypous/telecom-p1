#include <iostream>
#include <getopt.h>
#include <string.h>
#include <numbers>
#include "uart.hpp"
#include "serial.hpp"
#include "v21.hpp"

void usage(const char *prog)
{
    std::cerr << prog << " [-s serial_opts] [-l] [-r record_dev] [-p playback_dev] call|ans" << std::endl;
}

int main(int argc, char **argv)
{
    int opt;
    const char *serial_opts = NULL;
    while ((opt = getopt(argc, argv, "s:lr:p:")) != -1) {
        switch (opt) {
        case 's':
            serial_opts = optarg;
            break;
        case 'l':
            break;
        case 'r':
            break;
        case 'p':
            break;
        default:
            usage(argv[0]);
            exit(1);
        }
    }

    if (optind != argc - 1) {
        usage(argv[0]);
        exit(1);
    }

    float tx_omega0, tx_omega1, rx_omega0, rx_omega1;
    if (!strcmp(argv[optind], "call")) {
        tx_omega0 = 2*std::numbers::pi*(1080 + 100);
        tx_omega1 = 2*std::numbers::pi*(1080 - 100);
        rx_omega0 = 2*std::numbers::pi*(1750 + 100);
        rx_omega1 = 2*std::numbers::pi*(1750 - 100);
    }
    else if (!strcmp(argv[optind], "ans")) {
        rx_omega0 = 2*std::numbers::pi*(1080 + 100);
        rx_omega1 = 2*std::numbers::pi*(1080 - 100);
        tx_omega0 = 2*std::numbers::pi*(1750 + 100);
        tx_omega1 = 2*std::numbers::pi*(1750 - 100);
    }
    else {
        usage(argv[0]);
        exit(1);
    }

    UART_TX uart_tx;
    Serial serial(serial_opts, [&uart_tx](uint8_t b){ uart_tx.put_byte(b); });
    UART_RX uart_rx([&serial](uint8_t b){ serial.tx(b); });
    V21_RX v21_rx(rx_omega1, rx_omega0);
    V21_TX v21_tx(tx_omega1, tx_omega0);

    serial.event_loop();

    return 0;
}

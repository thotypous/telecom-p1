#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <pty.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include "serial.hpp"

Serial::Serial(const char *options, std::function<void(uint8_t)> rx)
    :read(rx)
{
    (void)options;  // não usado na versão Linux

    int pty, slave_fd;
    if (openpty(&pty, &slave_fd, NULL, NULL, NULL) < 0) {
        perror("openpty");
        exit(1);
    }
    this->pty = pty;

    struct termios t;
    if (tcgetattr(pty, &t) < 0) {
        perror("tcgetattr");
        exit(1);
    }

    cfmakeraw(&t);
    t.c_ispeed = B115200;
    t.c_ospeed = B115200;

    if (tcsetattr(pty, 0, &t) < 0) {
        perror("tcsetattr");
        exit(1);
    }

    char pty_name[256];
    ttyname_r(slave_fd, pty_name, sizeof(pty_name));
    close(slave_fd);
    std::cerr << "criado porto serial em " << pty_name << std::endl;
}

void Serial::write(uint8_t byte)
{
    ::write(pty, &byte, 1);
}

void Serial::event_loop()
{
    while (true) {
        uint8_t byte;
        int res = ::read(pty, &byte, 1);
        if (res < 0) {
            // ignora o EIO que acontece enquanto a outra ponta não conecta à pty
            if (errno != EIO) {
                perror("read");
            }
            usleep(100000);
        }
        else if (res == 1) {
            this->read(byte);
        }
    }
}

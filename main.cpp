#include <iostream>
#include <getopt.h>
#include <string.h>
#include <numbers>
#include <RtAudio.h>
#include <functional>
#include <vector>
#include "config.hpp"
#include "uart.hpp"
#include "serial.hpp"
#include "v21.hpp"

void usage(const char *prog)
{
    std::cerr << prog << " [-s serial_opts] [-l] [-i input_dev] [-o output_dev] call|ans" << std::endl;
}

int rtaudio_callback(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
    auto f = (std::function<int(void*,void*,unsigned int,double,RtAudioStreamStatus)> *)userData;
    return (*f)(outputBuffer, inputBuffer, nBufferFrames, streamTime, status);
}

int main(int argc, char **argv)
{
    RtAudio dev;

    int audio_in_dev = -1;
    int audio_out_dev = -1;

    int opt;
    const char *serial_opts = NULL;
    while ((opt = getopt(argc, argv, "s:li:o:")) != -1) {
        switch (opt) {
        case 's':
            serial_opts = optarg;
            break;
        case 'l':
            for (unsigned int i = 0; i < dev.getDeviceCount(); i++) {
                auto info = dev.getDeviceInfo(i);
                std::cout << i << " " << info.name;
                if (info.isDefaultInput) {
                    std::cout << " (default in)";
                }
                if (info.isDefaultOutput) {
                    std::cout << " (default out)";
                }
                std::cout << std::endl;
            }
            exit(0);
            break;
        case 'i':
            audio_in_dev = atoi(optarg);
            break;
        case 'o':
            audio_out_dev = atoi(optarg);
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

    for (unsigned int i = 0; i < dev.getDeviceCount(); i++) {
        auto info = dev.getDeviceInfo(i);
        if (audio_in_dev < 0 && info.isDefaultInput) {
            audio_in_dev = (int)i;
        }
        if (audio_out_dev < 0 && info.isDefaultOutput) {
            audio_out_dev = (int)i;
        }
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
    UART_RX uart_rx([&serial](uint8_t b){ serial.write(b); });
    V21_RX v21_rx(rx_omega1, rx_omega0);
    V21_TX v21_tx(tx_omega1, tx_omega0);

    RtAudio::StreamParameters in_params = {
        .deviceId = (unsigned int)audio_in_dev,
        .nChannels = 1,
        .firstChannel = 0,
    };
    RtAudio::StreamParameters out_params = {
        .deviceId = (unsigned int)audio_out_dev,
        .nChannels = 1,
        .firstChannel = 0,
    };
    unsigned int bufferFrames = SAMPLING_RATE / 40;

    std::function<int(void*,void*,unsigned int,double,RtAudioStreamStatus)> rtaudio_closure =
        [&uart_tx, &v21_tx, &v21_rx, &uart_rx]
        (void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status)
    {
        if (status == RTAUDIO_INPUT_OVERFLOW) {
            std::cerr << "audio input overflow!" << std::endl;
        }
        else if (status == RTAUDIO_OUTPUT_UNDERFLOW) {
            std::cerr << "audio output underflow!" << std::endl;
        }

        unsigned int digital_buffer[nBufferFrames];
        
        uart_tx.get_samples(digital_buffer, nBufferFrames);
        v21_tx.modulate(digital_buffer, (float *)outputBuffer, nBufferFrames);

        v21_rx.demodulate((const float *)inputBuffer, digital_buffer, nBufferFrames);
        uart_rx.put_samples(digital_buffer, nBufferFrames);

        return 0;
    };

    dev.openStream(&out_params, &in_params, RTAUDIO_FLOAT32, SAMPLING_RATE, &bufferFrames, rtaudio_callback, (void *)&rtaudio_closure);
    dev.startStream();

    serial.event_loop();

    return 0;
}

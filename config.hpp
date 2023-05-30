#ifndef CONFIG_HPP
#define CONFIG_HPP

constexpr int SAMPLING_RATE = 48000;
constexpr int BAUD_RATE = 300;

constexpr int SAMPLES_PER_SYMBOL = SAMPLING_RATE / BAUD_RATE;

#endif

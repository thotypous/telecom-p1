#ifndef V21_HPP
#define V21_HPP

#include <vector>
#include "config.hpp"

class V21_RX
{
public:
    V21_RX(float omega_mark, float omega_space) :omega_mark(omega_mark),omega_space(omega_space) {};
    void demodulate(std::vector<float> &in_analog_samples, std::vector<unsigned int> &out_digital_samples);
private:
    float omega_mark, omega_space;
};

class V21_TX
{
public:
    V21_TX(float omega_mark, float omega_space) :omega_mark(omega_mark),omega_space(omega_space),phase(0.f) {};
    void modulate(std::vector<unsigned int> &in_digital_samples, std::vector<float> &out_analog_samples);
private:
    float omega_mark, omega_space;
    float phase;
};

#endif

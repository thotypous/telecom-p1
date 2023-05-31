#ifndef V21_HPP
#define V21_HPP

#include "config.hpp"

class V21_RX
{
public:
    V21_RX(float omega_mark, float omega_space) :omega_mark(omega_mark),omega_space(omega_space) {};
    void demodulate(const float *in_analog_samples, unsigned int *out_digital_samples, unsigned int n);
private:
    float omega_mark, omega_space;
};

class V21_TX
{
public:
    V21_TX(float omega_mark, float omega_space) :omega_mark(omega_mark),omega_space(omega_space),phase(0.f) {};
    void modulate(const unsigned int *in_digital_samples, float *out_analog_samples, unsigned int n);
private:
    float omega_mark, omega_space;
    float phase;
};

#endif

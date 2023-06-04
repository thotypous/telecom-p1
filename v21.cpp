#include <math.h>
#include <numbers>
#include "v21.hpp"

void V21_RX::demodulate(const float *in_analog_samples, unsigned int n)
{
    unsigned int digital_samples[n];

    // substitua o loop abaixo, que gera um sinal UART constantemente ocioso,
    // pelo seu código
    for (int i = 0; i < n; i++) {
        digital_samples[i] = 1;
    }

    get_digital_samples(digital_samples, n);
}

void V21_TX::modulate(const unsigned int *in_digital_samples, float *out_analog_samples, unsigned int n)
{
    while (n--) {
        *out_analog_samples++ = sin(phase);
        phase += (*in_digital_samples++ ? omega_mark : omega_space) * SAMPLING_PERIOD;

        // evita que phase cresça indefinidamente, o que causaria perda de precisão
        phase = remainder(phase, 2*std::numbers::pi);
    }
}

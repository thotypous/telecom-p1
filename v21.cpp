#include <math.h>
#include <numbers>
#include "v21.hpp"

void V21_RX::demodulate(std::vector<float> &in_analog_samples, std::vector<unsigned int> &out_digital_samples)
{
    // seu código aqui
}

void V21_TX::modulate(std::vector<unsigned int> &in_digital_samples, std::vector<float> &out_analog_samples)
{
    auto inp = in_digital_samples.begin();
    auto outp = out_analog_samples.begin();

    while (inp != in_digital_samples.end() && outp != out_analog_samples.end()) {
        *outp = sin(phase);
        phase += (*inp ? omega_mark : omega_space) * SAMPLING_PERIOD;

        // evita que phase cresça indefinidamente, o que causaria perda de precisão
        phase = remainder(phase, 2*std::numbers::pi);

        ++inp; ++outp;
    }

}

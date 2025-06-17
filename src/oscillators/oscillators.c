#include "oscillators.h"

#include <math.h>
#include <stdlib.h>

#include "../utils/constant.h"

double generate_sine(double phase) {
    return sin(phase);
}

double generate_square(double phase) {
    return sin(phase) > 0.0 ? 1.0 : -1.0;
}

double generate_sawtooth(double phase) {
    return 2.0 * (phase / (2.0 * M_PI) - floor(phase / (2.0 * M_PI) + 0.5));
}

double generate_triangle(double phase) {
    double saw = generate_sawtooth(phase);
    return saw > 0.0 ? 2.0 * saw - 1.0 : -2.0 * saw - 1.0;
}

double generate_noise(void) {
    return ((double)rand() / RAND_MAX) * 2.0 - 1.0;
}

double generate_waveform(WaveType type, double phase) {
    switch (type) {
        case WAVE_SINE:     return generate_sine(phase);
        case WAVE_SQUARE:   return generate_square(phase);
        case WAVE_SAWTOOTH: return generate_sawtooth(phase);
        case WAVE_TRIANGLE: return generate_triangle(phase);
        case WAVE_NOISE:    return generate_noise();
        default:            return generate_sine(phase);
    }
}

double phase_increment(double frequency, double sample_rate) {
    return 2.0 * M_PI * frequency / sample_rate;
}

double wrap_phase(double phase) {
    while (phase >= 2.0 * M_PI) {
        phase -= 2.0 * M_PI;
    }
    while (phase < 0.0) {
        phase += 2.0 * M_PI;
    }
    return phase;
}

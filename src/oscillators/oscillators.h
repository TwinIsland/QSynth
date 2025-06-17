#pragma once


typedef enum {
    WAVE_NONE = 0,
    WAVE_SINE,
    WAVE_SQUARE,
    WAVE_SAWTOOTH,
    WAVE_TRIANGLE,
    WAVE_NOISE,
} WaveType;


// Waveform generation functions
double generate_sine(double phase);
double generate_square(double phase);
double generate_sawtooth(double phase);
double generate_triangle(double phase);
double generate_noise(void);
double generate_waveform(WaveType type, double phase);

// Phase utilities
double phase_increment(double frequency, double sample_rate);
double wrap_phase(double phase);

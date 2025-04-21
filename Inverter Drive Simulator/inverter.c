#include "inverter.h"
#include <math.h>

static double vdc = 400.0;
static double freq = 50.0;
static double mod_index = 0.8;

void set_inverter_params(double vdc_in, double freq_in, double mod_index_in) {
    vdc = vdc_in;
    freq = freq_in;
    mod_index = mod_index_in;
}

double calculate_vll(void) {
    // V_L-L = m_a * V_dc * sqrt(3)/sqrt(2)
    return mod_index * vdc * sqrt(3.0) / sqrt(2.0);
}

void get_pwm_waveform(double *waveform, int samples, double time) {
    double period = (freq > 0) ? 1.0 / freq : 1.0;
    double t_step = period / samples;
    for (int i = 0; i < samples; i++) {
        double t = time + i * t_step;
        waveform[i] = mod_index * vdc * sin(2.0 * M_PI * freq * t);
    }
}

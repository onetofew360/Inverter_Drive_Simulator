#ifndef INVERTER_H
#define INVERTER_H

void set_inverter_params(double vdc, double freq, double mod_index);
double calculate_vll(void);
void get_pwm_waveform(double *waveform, int samples, double time);

#endif

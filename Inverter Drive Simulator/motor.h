#ifndef MOTOR_H
#define MOTOR_H

void set_motor_params(double rated_voltage, double rated_current, double rated_freq, double rated_rpm);
void set_drive_params(double ramp_up, double ramp_down, double max_freq, double min_freq);
void update_motor(double target_freq, int is_forward, double ramp_up, double ramp_down);
double get_motor_current(void);
double get_motor_speed(void);
double get_motor_torque(void);
double get_motor_temp(void);

#endif

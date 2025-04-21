#include "motor.h"
#include <math.h>

static double rated_voltage = 400.0;
static double rated_current = 10.0;
static double rated_freq = 50.0;
static double rated_rpm = 1500.0;
static double ramp_up = 2.0;
static double ramp_down = 2.0;
static double max_freq = 100.0;
static double min_freq = 10.0;
static double current_speed = 0.0;
static double current_freq = 0.0;
static double torque = 0.0;
static double current = 0.0;
static double temp = 25.0;

void set_motor_params(double voltage, double current, double freq, double rpm) {
    rated_voltage = voltage;
    rated_current = current;
    rated_freq = freq;
    rated_rpm = rpm;
}

void set_drive_params(double ramp_up_in, double ramp_down_in, double max_freq_in, double min_freq_in) {
    ramp_up = ramp_up_in;
    ramp_down = ramp_down_in;
    max_freq = max_freq_in;
    min_freq = min_freq_in;
}

void update_motor(double target_freq, int is_forward, double ramp_up, double ramp_down) {
    target_freq = fmax(min_freq, fmin(max_freq, target_freq));
    double target_speed = (target_freq / rated_freq) * rated_rpm * (is_forward ? 1.0 : -1.0);
    double accel = rated_rpm / ramp_up;
    double decel = rated_rpm / ramp_down;
    if (current_speed < target_speed) {
        current_speed = fmin(current_speed + accel * 0.05, target_speed);
    } else {
        current_speed = fmax(current_speed - decel * 0.05, target_speed);
    }
    current_freq = fabs(current_speed / rated_rpm) * rated_freq;
    torque = rated_current * rated_voltage / rated_rpm * fabs(current_speed);
    current = rated_current * (current_freq / rated_freq);
    temp += current * 0.01;
    temp = fmax(25.0, temp);
}

double get_motor_current(void) {
    return current;
}

double get_motor_speed(void) {
    return current_speed;
}

double get_motor_torque(void) {
    return torque;
}

double get_motor_temp(void) {
    return temp;
}

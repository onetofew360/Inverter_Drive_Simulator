#ifndef FAULT_H
#define FAULT_H

typedef enum {
    FAULT_NONE,
    FAULT_OVERCURRENT,
    FAULT_UNDERVOLTAGE,
    FAULT_OVERTEMP
} FaultType;

void check_faults(double voltage, double current, double temp);
void trigger_fault(FaultType fault);
void reset_faults(void);
int has_fault(void);
const char *get_fault_status(void);

#endif

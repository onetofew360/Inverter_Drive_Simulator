#include "fault.h"
#include <string.h>

static FaultType current_fault = FAULT_NONE;
static char fault_status[256] = "";

void check_faults(double voltage, double current, double temp) {
    if (current_fault != FAULT_NONE) return;
    if (current > 20.0) {
        trigger_fault(FAULT_OVERCURRENT);
    } else if (voltage < 300.0) {
        trigger_fault(FAULT_UNDERVOLTAGE);
    } else if (temp > 80.0) {
        trigger_fault(FAULT_OVERTEMP);
    }
}

void trigger_fault(FaultType fault) {
    current_fault = fault;
    switch (fault) {
        case FAULT_OVERCURRENT:
            strcpy(fault_status, "Fault: Overcurrent");
            break;
        case FAULT_UNDERVOLTAGE:
            strcpy(fault_status, "Fault: Undervoltage");
            break;
        case FAULT_OVERTEMP:
            strcpy(fault_status, "Fault: Overtemperature");
            break;
        default:
            fault_status[0] = '\0';
    }
}

void reset_faults(void) {
    current_fault = FAULT_NONE;
    fault_status[0] = '\0';
}

int has_fault(void) {
    return current_fault != FAULT_NONE;
}

const char *get_fault_status(void) {
    return fault_status;
}

# Inverter Drive Simulator

## Overview

The **Inverter Drive Simulator** is a GTK+ 4.0-based application developed in Code::Blocks with MSYS2 on Windows, designed to simulate a variable frequency drive (VFD) controlling an induction motor. It provides a graphical user interface (GUI) for configuring motor and drive parameters, simulating input signals, monitoring motor behavior, injecting faults, and visualizing real-time plots. The application uses a dark theme for improved visibility and is structured to mimic real-world VFD operations with simplified models for educational purposes.

## Application Structure

The simulator is organized into modular C files, each handling specific functionality:

- **main.c**: Initializes the GTK application, sets the dark theme (`gtk-application-prefer-dark-theme`), and activates the main window.
- **gui.c/h**: Manages the GUI layout using a `GtkGrid` with controls on the left (motor parameters, drive settings, input signals, control panel, fault inputs, outputs) and plots on the right (voltage, current, frequency, torque, speed).
- **inverter.c/h**: Handles inverter calculations, including line-to-line voltage (`V_L-L`) and PWM waveform generation.
- **motor.c/h**: Simulates an induction motor with V/f control, calculating speed, torque, current, and temperature.
- **fault.c/h**: Detects and manages faults (overcurrent, undervoltage, overtemperature) with manual injection and reset capabilities.
- **waveform.c/h**: Renders five real-time plots using `GtkDrawingArea` and Cairo, currently displaying a placeholder voltage waveform for all parameters.

## GUI Layout

The GUI is divided into two main sections within a `GtkGrid`:

- **Left Columns (0–3)**:
  - **Status Label**: Displays "Running", "Stopped", or "Faulted" with direction (Forward/Reverse).
  - **Motor Parameters**: Text entries for rated voltage (V), current (A), frequency (Hz), and RPM.
  - **Drive Settings**: Entries for ramp up/down times (s), max/min frequency (Hz), and a combo box for control mode (V/f only, with placeholders for vector control, sensorless vector, DTC).
  - **Input Signals**: Slider for speed reference (0–100%), Forward/Reverse buttons.
  - **Control Panel**: Run, Stop, Reset buttons, and a keypad entry for parameter updates (currently sets rated voltage).
  - **Fault Inputs**: Checkboxes for manual fault injection (overcurrent, undervoltage, overtemperature).
  - **Output Labels**: Show calculated V_L-L, current, speed, torque, frequency, and fault/error messages.
- **Right Column (4)**:
  - **Plot Area**: Five stacked plots (voltage, current, frequency, torque, speed) rendered with Cairo, each with a grid, waveform, and label. Currently, all use a placeholder voltage waveform.

The window size is 1200x800 pixels, with the plot area set to 600x600 pixels for vertical stacking of graphs. The dark theme ensures a dark background with light text and bright plot colors (blue waveform, light gray grid).

## Simulation Logic

The simulation runs in a timed loop (50 ms interval) triggered by the "Run" button and managed in `gui.c` (`update_simulation` function). Key steps include:

1. **Input Collection**:

   - Reads motor parameters, drive settings, speed reference, and fault checkboxes from GUI widgets.
   - Parameters: Rated voltage (1–10,000 V), current (1–1,000 A), frequency (1–1,000 Hz), RPM (1–10,000), ramp up/down (0–60 s), max/min frequency (0–1,000 Hz, min ≤ max).
   - Speed reference: Slider value (0–100%) scales to max frequency.

2. **Input Validation**:

   - Checks for valid ranges (e.g., `rated_voltage > 0`, `max_freq > min_freq`).
   - Displays error messages in the error label (e.g., "Invalid Rated Voltage (1–10000 V)") and stops simulation if invalid.

3. **Motor and Inverter Update**:

   - Updates motor parameters (`motor.c`: `set_motor_params`) and drive settings (`set_drive_params`).
   - Sets inverter parameters (`inverter.c`: `set_inverter_params`) with rated voltage, target frequency (from speed reference), and fixed modulation index (0.8).
   - Updates motor state (`update_motor`) with target frequency, direction, and ramp times.

4. **Fault Handling**:

   - Checks automatic faults (`fault.c`: `check_faults`) based on thresholds: current &gt; 20 A, voltage &lt; 300 V, temperature &gt; 80°C.
   - Applies manual faults if checkboxes are active (`trigger_fault`).
   - Stops simulation and updates status/error labels if a fault occurs.

5. **Output Calculation**:

   - Calculates V_L-L (`calculate_vll`), motor current, speed, torque, and frequency (`get_motor_*` functions).
   - Updates output label with formatted values (e.g., "V_L-L: 392.30 V\\nCurrent: 8.00 A\\n...").
   - Queues plot redraw (`gtk_widget_queue_draw`).

6. **Plot Rendering**:

   - Draws five plots in `waveform.c` (`draw_waveform`), each with a grid, waveform, and label.
   - Currently uses a placeholder voltage waveform (`get_pwm_waveform`) scaled to max values (e.g., 1.1 \* V_dc for voltage).

The simulation stops when:

- "Stop" button is clicked (`is_running = FALSE`).
- A fault is detected or injected (`has_fault()`).
- Invalid inputs are entered.

The "Reset" button clears faults, unchecks fault boxes, and updates the status.

## Key Calculations

The simulator uses simplified models for inverter and motor behavior, focusing on V/f control. Below are the main calculations:

### Inverter Calculations (`inverter.c`)

- **Line-to-Line Voltage (V_L-L)**:
  - Formula: `V_L-L = m_a * V_dc * sqrt(3) / sqrt(2)`
  - `m_a` (modulation index) is fixed at 0.8.
  - `V_dc` is the rated voltage from motor parameters.
  - Example: For V_dc = 400 V, `V_L-L = 0.8 * 400 * sqrt(3) / sqrt(2) ≈ 392.30 V`.
- **PWM Waveform**:
  - Generates a sinusoidal waveform: `V(t) = m_a * V_dc * sin(2π * freq * t)`.
  - `freq` is the target frequency (speed reference \* max frequency).
  - Samples: 100 points over one period (`1/freq`).

### Motor Calculations (`motor.c`)

- **Speed**:
  - Target speed: `(target_freq / rated_freq) * rated_rpm * (is_forward ? 1 : -1)`.
  - Applies ramping: Acceleration = `rated_rpm / ramp_up`, deceleration = `rated_rpm / ramp_down`.
  - Updates every 50 ms: `current_speed += accel * 0.05` or `-decel * 0.05`, clamped to target.
  - Example: For target_freq = 50 Hz, rated_freq = 50 Hz, rated_rpm = 1500, speed = 1500 RPM (forward).
- **Frequency**:
  - Derived from speed: `current_freq = |current_speed / rated_rpm| * rated_freq`.
- **Torque**:
  - Simplified (constant torque load): `torque = rated_current * rated_voltage / rated_rpm * |current_speed|`.
  - Example: For rated_current = 10 A, rated_voltage = 400 V, rated_rpm = 1500, speed = 1500 RPM, `torque ≈ 106.67 Nm`.
- **Current**:
  - Proportional to frequency: `current = rated_current * (current_freq / rated_freq)`.
  - Example: For current_freq = 40 Hz, rated_freq = 50 Hz, rated_current = 10 A, `current = 8 A`.
- **Temperature**:
  - Increases with current: `temp += current * 0.01` per cycle, minimum 25°C (ambient).
  - Example: For current = 10 A, temp increases by 0.1°C every 50 ms.

### Fault Detection (`fault.c`)

- **Overcurrent**: Triggered if `current > 20 A`.
- **Undervoltage**: Triggered if `voltage < 300 V`.
- **Overtemperature**: Triggered if `temp > 80°C`.
- Manual faults override automatic checks when checkboxes are active.

## Parameters

The application handles the following user-configurable parameters, validated in `gui.c`:

| Parameter | Range | Default | Description |
| --- | --- | --- | --- |
| Rated Voltage (V) | 1–10,000 | 400 | Motor rated voltage (V_dc for inverter). |
| Rated Current (A) | 1–1,000 | 10 | Motor rated current. |
| Rated Frequency (Hz) | 1–1,000 | 50 | Motor rated frequency. |
| Rated RPM | 1–10,000 | 1500 | Motor rated speed. |
| Ramp Up Time (s) | 0–60 | 2 | Time to reach rated speed. |
| Ramp Down Time (s) | 0–60 | 2 | Time to stop from rated speed. |
| Max Frequency (Hz) | 0–1,000 (&gt; min) | 100 | Maximum output frequency. |
| Min Frequency (Hz) | 0–max_freq | 10 | Minimum output frequency. |
| Speed Reference (%) | 0–100 | 50 | Scales target frequency (min to max). |
| Control Mode | V/f (others N/A) | V/f | Motor control mode (V/f implemented). |
| Direction | Forward/Reverse | Forward | Motor rotation direction. |
| Faults (Overcurrent, etc.) | On/Off | Off | Manual fault injection. |
| Keypad Value | Any number | N/A | Updates rated voltage (simplified). |

- **Validation**: Invalid inputs (e.g., negative voltage, max_freq &lt; min_freq) display error messages and halt simulation.
- **Keypad**: Currently sets rated voltage; a full menu system is planned.

## Simulation Features

- **Input Signals**:
  - Speed reference slider scales target frequency (`speed_ref * max_freq / 100`).
  - Forward/Reverse buttons toggle motor direction (`is_forward`).
  - Run/Stop/Reset buttons control simulation state and fault clearing.
- **Motor Behavior**:
  - V/f control: Voltage and frequency are proportional (`V/f = constant`).
  - Ramps speed based on user-defined ramp times.
  - Simulates constant torque load (simplified).
- **Graphical Monitoring**:
  - Five plots (600x600 pixels total, 80 pixels per plot) show voltage, current, frequency, torque, and speed.
  - Placeholder: All plots use the PWM voltage waveform scaled to max values (e.g., 1.1 \* V_dc for voltage).
  - Future: Integrate real motor data for accurate plots.
- **Fault Handling**:
  - Automatic detection based on thresholds.
  - Manual injection via checkboxes.
  - Faults stop simulation and display status (e.g., "Fault: Overcurrent").
- **Control Panel**:
  - Mimics a VFD panel with Run, Stop, Reset, Forward/Reverse buttons.
  - Keypad entry simulates parameter editing (limited to voltage).

## Limitations and Future Enhancements

- **Plots**: Currently use a placeholder voltage waveform. Need integration with motor data for current, torque, speed, and frequency.
- **Motor Model**: Simplified V/f control; lacks slip, rotor flux, or advanced dynamics (e.g., vector control, DTC).
- **Control Modes**: Only V/f implemented; others are placeholders.
- **Load**: Assumes constant torque; variable or user-defined loads are not supported.
- **Faults**: Limited to three types; no short circuit or stall simulation.
- **Keypad**: Updates only rated voltage; needs a full menu system.
- **PWM Waveform**: Sinusoidal approximation; true PWM requires carrier wave modulation.
- **Performance**: Multiple plots may slow rendering on low-end systems.

## Usage Notes

- **Testing**: Use inputs like 600 V, 60 Hz, 1800 RPM, 50% speed ref to verify simulation. Inject faults and reset to test fault handling.
- **Console**: `g_print` statements (e.g., "Run button clicked") aid debugging. Remove for production.
- **Dark Theme**: Ensures visibility with dark background and bright plot colors.
- **Window Layout**: Controls on the left, plots on the right for intuitive operation.

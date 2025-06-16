# Smart Roomba MCU Application

This is the microcontroller firmware for the Smart Roomba project, built on the Zephyr RTOS.

## Overview

The Smart Roomba MCU application controls the low-level hardware functions of the robotic vacuum cleaner, including:
- Motor control for movement and suction
- Sensor data collection (distance, bumpers, etc.)
- Communication with the Raspberry Pi main controller
- Battery management
- Safety systems

## Hardware Setup

### Prerequisites

- STM32-based development board (or compatible)
- Development tools for flashing and debugging
- Power supply and connection cables

## Software Setup

### Prerequisites

| Tools    | Version |
| -------- | ------- |
| CMake    | 3.28.3  |
| Python   | 3.12.3  |
| Device Tree Compiler (DTC)   | 1.7.0   |
| West     | Latest  |
| Zephyr SDK | Latest |

### Building the Application

1. **Initialize the workspace** (if not already done):
   ```bash
   cd /home/jacky/git/zephyr-workspace
   west init --local smart-roomba/mcu
   west update
   ```

2. **Build for your target board**:
   ```bash
   cd smart-roomba/mcu
   west build -b <your_board_name>
   ```
   
   For example, to build for an STM32 Discovery board:
   ```bash
   west build -b stm32f4_disco
   ```

3. **Flash to hardware**:
   ```bash
   west flash
   ```

4. **Build for emulation** (testing):
   ```bash
   west build -b qemu_cortex_m3
   west build -t run
   ```

### Configuration

- **prj.conf**: Contains Kconfig options for enabling/disabling features
- **app.overlay**: Device tree overlay for hardware-specific configurations
- **CMakeLists.txt**: Build system configuration

### Application Structure

```
mcu/
├── CMakeLists.txt          # Build configuration
├── prj.conf               # Kconfig settings
├── app.overlay            # Device tree overlay
├── VERSION                # Version information
├── west.yml              # West manifest
├── README.md             # This file
└── src/
    └── main.c            # Main application code
```

### Development

The application currently implements a basic LED blink example with logging. 
You can extend it by adding:

- Motor control drivers
- Sensor interfaces (I2C, SPI, ADC)
- Communication protocols (UART, CAN, etc.)
- Real-time control algorithms
- Power management features

### General Style
Try to follow Linux kernel coding style. See [link](https://docs.zephyrproject.org/latest/contribute/style/code.html). Also, [general coding style](https://docs.zephyrproject.org/latest/contribute/style/index.html).


### How to Setup Debugging?
Copy `.vscode` to your workspace. There should be `launch.json`, `openocd.cfg`, and `tasks.json` files.

#### DEBUG vs. CONFIG_DEBUG
`CONFIG_DEBUG` is a definition provided by Zephyr OS. The way this is used in the project is for Zephyr kernel/system-level debugging. For any application-level debugging, use `DEBUG`.

### Building
Debug build
```
west build -- -DCMAKE_BUILD_TYPE=Debug
```

Release build  
```
west build -- -DCMAKE_BUILD_TYPE=Release
```
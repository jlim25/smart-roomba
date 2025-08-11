# Smart Roomba MCU Application

This is the microcontroller firmware for the Smart Roomba project, built on the Zephyr RTOS (4.1.0).

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

## Environment Setup
A Linux system is recommended to work on this project.
1. Modify the user's groups to include dialout.
   ```bash
   sudo usermod -a -G dialout $USER
   ```

### Building the Application

1. **Initialize the workspace** (if not already done):
   ```bash
   cd /home/jacky/git/zephyr-workspace
   west init --local smart-roomba/mcu
   west update
   source .venv/bin/activate
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
├── prj.conf                # Kconfig settings / build-time feature flags
├── app.overlay             # Device tree overlay (describes the hardware)
├── VERSION                 # Version information
├── west.yml                # West manifest
├── README.md               # This file
├── VERSION
└── src/
    └── main.c              # Main application code
└── lib/                    # Custom library to be used via kconfig
    └── inc/
    └── src/
└── boards/                 # Hardware config files
    └── stm32f4_disco.conf
└── build/                  # Build artificats (not tracked)
└── conf/                   # Contains debug and release build configurations
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
1. Copy `.vscode` to your workspace. There should be `launch.json`, `openocd.cfg`, `settings.json` and `tasks.json` files.

2. Open `settings.json` and modify the path to Zephyr SDK if it is different from the default location. 

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

## Testing
To run testing, Twister (comes with Zephyr) is used. Test units are structured based on modules.
```
west twister -T tests/<module_name>
```
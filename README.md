# STM32 Scaled Hardware PWM LED Breathing Engine

A precise, resource-optimized embedded firmware implementing a hardware-scaled Pulse Width Modulation (PWM) dimming engine to execute a controlled LED breathing animation cycle on the **ARM Cortex-M3 (STM32F103RB)**.

## 🚀 Optimized Hardware Register Configurations

Rather than managing extremely large raw tick windows in software, this iteration introduces hardware-level clock prescaling. By dividing the peripheral clock directly down to a manageable frequency profile, we maintain a flicker-free output while keeping step counts highly predictable.

### Core Metrics & Waveform Constraints
* **Master Timer Clock Base:** 26 MHz
* **Prescaler Division:** 49 (`Init.Prescaler = 49`, resulting in a 520 kHz counting rate)
* **Auto-Reload Period:** 479 (`Init.Period = 480 - 1`)
* **Resulting Carrier PWM Frequency:** **1,083.33 Hz**
* **Luminance Resolution:** 0 to 479 discrete brightness levels

---

## 📌 Control Logic & Timeline Workflow

The fading logic runs within a dual-state stepping layout in the main loop context. With an interval delay of 1000ms and a step size of 50 ticks, the transition completes a full phase transition every ~10 seconds:
while(1) {
    // 1. Fade Up State: Increments duty cycle to max threshold (479) over ~10 seconds
    while (brightness < htimer2.Init.Period) {
        brightness += 50;
        __HAL_TIM_SET_COMPARE(&htimer2, TIM_CHANNEL_1, brightness);
        HAL_Delay(1000);
    }

    // 2. Fade Down State: Decrements duty cycle back to 0% over ~10 seconds
    while (brightness > 0) {
        brightness -= 50;
        __HAL_TIM_SET_COMPARE(&htimer2, TIM_CHANNEL_1, brightness);
        HAL_Delay(1000);
    }
}

## 💻 How To Run & Verify

### 1. Hardware Connections
* **Connect the Anode** (long leg) of a standard LED to microcontroller pin **PA0**.
* **Connect a $220\ \Omega$ or $330\ \Omega$ current-limiting resistor** in series from the LED's **Cathode** (short leg) to any **GND** pin on the Nucleo board.

### 2. Compilation and Flashing
* **Clone this repository** and open the project inside **STM32CubeIDE**.
* **Clean and build the project** (`Ctrl + B`) to ensure zero errors or warnings.
* **Connect the Nucleo-64 board** to your PC via a mini-USB cable.
* **Click Run or Debug** (`F11`) to flash the compiled binary onto the microcontroller.

### 3. Visual Verification
* Once booted, observe the LED executing a highly structured, 20-second complete loop cycle. 
* It will gracefully step up from absolute darkness to full brightness over 10 seconds, and smoothly transition back down to dark over the next 10 seconds.

### 4. Logic Analyzer / Oscilloscope Inspection (Optional)
* Hook up a digital logic analyzer channel to pin **PA0** and attach the ground probe to **GND**.
* Set your analyzer sampling rate to at least **10 MS/s**.
* You will capture a stable **1,083.33 Hz square wave** where the pulse width dynamically widens by 50-tick steps every second during the fade-up phase, and shrinks during the fade-down phase.

# SeaTalk1-to-SignalK-SensESP-v3
SeaTalk 1 to Signal K Bridge (SensESP v3)
A robust, low-cost solution to bridge legacy Raymarine SeaTalk 1 data to a Signal K server using an ESP32. This project is specifically designed for SensESP version 3.x.
Overview
Raymarine's SeaTalk 1 (ST1) is a 9-bit serial protocol running at 4800 baud. Integrating it with modern ESP32 microcontrollers often fails due to library incompatibilities or signal inversion issues.
This project solves these problems by using:
1.	Standard 8-bit UART with Gap Synchronization: A 15ms silence detection method to reliably identify the start of messages without requiring complex 9-bit hardware drivers.
2.	"Hardware Rectified" Signal Logic: A specific wiring method that simplifies the software and ensures high signal integrity.
Supported Instruments
•	Autopilot (ST5000+, ST4000+): Magnetic Heading, Rudder Angle, Autopilot State (Standby/Auto/Wind/Track).
•	Tridata (ST60): Depth below transducer, Speed Through Water (STW), Water Temperature, Trip/Total Log.
•	Wind (ST60): Apparent Wind Angle (AWA), Apparent Wind Speed (AWS).
Hardware Configuration
Components
•	ESP32 (e.g., DevKit V1).
•	Optocoupler Module (PC817 or similar).
The "Breakthrough" Wiring
To avoid complex software inversion and ensure the UART "Idle" state is correctly seen as HIGH (3.3V), use the following input wiring:
Input (SeaTalk side):
•	SeaTalk Red (+12V) → Optocoupler Input (+)
•	SeaTalk Yellow (Data) → Optocoupler Input (-)
•	Note: This configuration means the opto-LED only flashes when data pulls the line to 0V, effectively "rectifying" the logic for the ESP32.
Output (ESP32 side):
•	VCC → ESP32 3.3V
•	GND → ESP32 GND
•	Signal (OUT) → ESP32 GPIO 4
•	Pull-up: A 10kΩ resistor between GPIO 4 and 3.3V is required (many modules have this built-in as R2).
Software Features
•	SensESP v3 Ready: Fully compatible with the latest Signal K framework for ESP32.
•	Uptime Heartbeat: Sends a heartbeat signal to Signal K to verify the bridge is alive.
•	Robust Math: Implements the Thomas Knauf SeaTalk formulas for high-precision data conversion (e.g., preventing common heading errors).
Installation
1.	Install PlatformIO in VS Code.
2.	Clone this repository.
3.	Edit the main.cpp to set your Signal K server IP address.
4.	Upload to your ESP32.
5.	In the Signal K Dashboard, approve the new device request.
PlatformIO Configuration
Ensure your platformio.ini includes the following flags for SensESP v3:
codeIni
build_unflags = -std=gnu++11
build_flags =
    -std=gnu++17
lib_ldf_mode = deep+
Credits
•	Thomas Knauf: For his legendary documentation of the SeaTalk 1 protocol.
•	SensESP Community: For the excellent Signal K framework.



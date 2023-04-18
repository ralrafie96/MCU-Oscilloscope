# MCU-Oscilloscope

A system to adequately acquire 10 – 15 seconds of a time varying signal ranging between 0 – 3.3V using Matlab via the MSP-EXP430f5529

- Samples and digitizes a single channel analog signal at 1.25 kHz at 12-bits resolution

– Transmits the digitized signal via the msp430f5529’s UART at adequate speed to enable capture and reconstruction of the signal within Matlab.

– Uses the MSP430’s low power mode to put the microcontroller (MCU) to sleep when it is not capturing or transmitting data (interrupt driven).

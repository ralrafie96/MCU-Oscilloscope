#include "driverlib.h"
#include <msp430.h>

extern volatile unsigned int buffer1 = 0x00;
extern volatile unsigned int buffer2 = 0x00;
extern volatile unsigned int buffer3 = 0x00;
extern volatile unsigned int buffer4 = 0x00;

extern volatile unsigned int sensorValue1 = 0x0000;
extern volatile unsigned int sensorValue2 = 0x0000;

extern volatile unsigned int firstSensorLoaded = 0;

void initUART(void);
void initADC(void);

int main(void)
{
    //Stop WatchDog Timer
    WDTCTL = WDTPW | WDTHOLD;

    //Initialize ADC Pins and UART
    initUART();
    initADC();

    GPIO_setAsPeripheralModuleFunctionInputPin(
            GPIO_PORT_P6,
            GPIO_PIN0
            );

    while(1)
    {
    //Enable/Start sampling and conversion
            /*
             * Base address of ADC12_A Module
             * Start the conversion into memory buffer 0
             * Use the single-channel, single-conversion mode
             */
            ADC12_A_startConversion(ADC12_A_BASE,
                    ADC12_A_MEMORY_0,
                    ADC12_A_SINGLECHANNEL);

            //Enter LPM3, interrupt enabled
            __bis_SR_register(LPM3_bits +GIE);
            __delay_cycles(200);
    }
}

void initADC(void)
{
    //Initialize the ADC12_A Module
    /*
     * Base address of ADC12_A Module
     * Use internal ADC12_A bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider of 1
     */
    ADC12_A_init(ADC12_A_BASE,
            ADC12_A_SAMPLEHOLDSOURCE_SC,
            ADC12_A_CLOCKSOURCE_ADC12OSC,
            ADC12_A_CLOCKDIVIDER_1);

    ADC12_A_enable(ADC12_A_BASE);

    /*
     * Base address of ADC12_A Module
     * For memory buffers 0-7 sample/hold for 64 clock cycles
     * For memory buffers 8-15 sample/hold for 4 clock cycles (default)
     * Disable Multiple Sampling
     */
    ADC12_A_setupSamplingTimer(ADC12_A_BASE,
            ADC12_A_CYCLEHOLD_64_CYCLES,
            ADC12_A_CYCLEHOLD_4_CYCLES,
            ADC12_A_MULTIPLESAMPLESDISABLE);

    //Configure Memory Buffer
    /*
     * Base address of the ADC12_A Module
     * Configure memory buffer 0
     * Map input A0 to memory buffer 0
     * Vref+ = AVcc
     * Vr- = AVss
     * Memory buffer 0 is not the end of a sequence
     */
    ADC12_A_configureMemoryParam ADC12param =
    {
     ADC12_A_MEMORY_0,
     ADC12_A_INPUT_A0,
     ADC12_A_VREFPOS_AVCC,
     ADC12_A_VREFNEG_AVSS,
     ADC12_A_NOTENDOFSEQUENCE
    };
//    param.memoryBufferControlIndex = ADC12_A_MEMORY_0;
//    param.inputSourceSelect = ADC12_A_INPUT_A0;
//    param.positiveRefVoltageSourceSelect = ADC12_A_VREFPOS_AVCC;
//    param.negativeRefVoltageSourceSelect = ADC12_A_VREFPOS_AVCC;
//    param.endOfSequence = ADC12_A_NOTENDOFSEQUENCE;

    ADC12_A_configureMemory(ADC12_A_BASE ,&ADC12param);

    //Enable memory buffer 0 interrupt
    ADC12_A_clearInterrupt(ADC12_A_BASE,
            ADC12IFG0);
    ADC12_A_enableInterrupt(ADC12_A_BASE,
            ADC12IE0);
}

void initUART(void)
{
    P3SEL |= BIT3+BIT4;                       // P3.3,4 = USCI_A0 TXD/RXD

    //USCI Initial Parameters
    USCI_A_UART_initParam UART_Params =
    {
     USCI_A_UART_CLOCKSOURCE_SMCLK,
     9, //Clock Prescalar
     0, //FirstModReg: UCBRFx
     1, //SecondModReg: UCBRSx
     USCI_A_UART_NO_PARITY,
     USCI_A_UART_LSB_FIRST,
     USCI_A_UART_ONE_STOP_BIT,
     USCI_A_UART_MODE,
     USCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION
    };

    USCI_A_UART_init (USCI_A0_BASE, &UART_Params);

    //Enable USCI_A_UART module for operation
    USCI_A_UART_enable (USCI_A0_BASE);

    //Enable Receive Interrupt
    USCI_A_UART_enableInterrupt (USCI_A0_BASE, UCRXIE);
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(ADC12_VECTOR)))
#endif
void ADC12_A_ISR (void)
{
    switch (__even_in_range(ADC12IV,34)){
        case  0: break;   //Vector  0:  No interrupt
        case  2: break;   //Vector  2:  ADC overflow
        case  4: break;   //Vector  4:  ADC timing overflow
        case  6:          //Vector  6:  ADC12IFG0

            if (firstSensorLoaded == 0)
                {
                    sensorValue1 = ADC12_A_getResults(ADC12_A_BASE, ADC12_A_MEMORY_0);
//                    sensorValue1 = 0x0ABC;
                    firstSensorLoaded = 1;
                } else {
                    sensorValue2 = ADC12_A_getResults(ADC12_A_BASE, ADC12_A_MEMORY_0);
//                    sensorValue2 = 0x0123;
                    firstSensorLoaded = 0;
                    UCA0IFG = 0x03;    //USCI A0 Interrupt Flags Register
                }

                 __bic_SR_register_on_exit(LPM0_bits);
                 break;
        case  8: break;   //Vector  8:  ADC12IFG1
        case 10: break;   //Vector 10:  ADC12IFG2
        case 12: break;   //Vector 12:  ADC12IFG3
        case 14: break;   //Vector 14:  ADC12IFG4
        case 16: break;   //Vector 16:  ADC12IFG5
        case 18: break;   //Vector 18:  ADC12IFG6
        case 20: break;   //Vector 20:  ADC12IFG7
        case 22: break;   //Vector 22:  ADC12IFG8
        case 24: break;   //Vector 24:  ADC12IFG9
        case 26: break;   //Vector 26:  ADC12IFG10
        case 28: break;   //Vector 28:  ADC12IFG11
        case 30: break;   //Vector 30:  ADC12IFG12
        case 32: break;   //Vector 32:  ADC12IFG13
        case 34: break;   //Vector 34:  ADC12IFG14
        default: break;
    }
}

// Takes RX value, converts it, sends it to TX value, (received by Serial Breakout Board)
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A0_VECTOR))) USCI_A0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;                                                                                 // Vector 0 - no interrupt
  case 2:                                                                                       // Vector 2 - RXIFG
    while (!USCI_A_UART_getInterruptStatus(USCI_A0_BASE, USCI_A_UART_TRANSMIT_INTERRUPT_FLAG)); // USCI_A0 TX buffer ready?

    buffer1 = (uint8_t)sensorValue1;
    buffer2 = (uint8_t)(((sensorValue1>>4)&0x00F0) | (sensorValue2&0x000F));
    buffer3 = (uint8_t)(sensorValue2>>4);
    buffer4 = 0x0A;

    USCI_A_UART_transmitData (USCI_A0_BASE, buffer1);                                           // TX -> buffer characters
    USCI_A_UART_transmitData (USCI_A0_BASE, buffer2);
    USCI_A_UART_transmitData (USCI_A0_BASE, buffer3);
    USCI_A_UART_transmitData (USCI_A0_BASE, buffer4);
//    UCRXIFG = 0; //Turn on USCI Receive Interrupt Flag
    break;
  case 4:break;                                                                                 // Vector 4 - TXIFG
  default: break;
  }
}

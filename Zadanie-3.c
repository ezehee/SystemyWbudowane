// PIC24FJ128GA010 Configuration Bit Settings

// 'C' source line config statements

// CONFIG2
#pragma config POSCMOD = XT             // Primary Oscillator Select (XT Oscillator mode selected)
#pragma config OSCIOFNC = ON            // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSDCMD           // Clock Switching and Monitor (Clock switching and Fail-Safe Clock Monitor are disabled)
#pragma config FNOSC = PRI              // Oscillator Select (Primary Oscillator (XT, HS, EC))
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = ON              // Watchdog Timer Enable (Watchdog Timer is enabled)
#pragma config ICS = PGx2               // Comm Channel Select (Emulator/debugger uses EMUC2/EMUD2)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

#include <xc.h>
#include "libpic30.h"
#include "adc.h"
#include "buttons.h"

#define PRZEKROCZENIE_ADC 512
#define CZAS_MRUGANIA 10 // 10 * 500ms = 5s

void init(void) {
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    ADC_ChannelEnable(ADC_CHANNEL_POTENTIOMETER);

    TRISA = 0x0000;
    TRISD |= (1 << 6);
}

int main(void) {
    init();

    unsigned int adc_value;
    char alarm = 0;
    char mruganie = 0;
    char licznik_mrugania = 0;
    char poprzedni_stan_przycisku = 1;

    while (1) {
        adc_value = ADC_Read10bit(ADC_CHANNEL_POTENTIOMETER);
        if (adc_value == 0xFFFF) {
            continue;
        }
        
        unsigned char stan_przycisku = PORTDbits.RD6;
        
        // Jesli alarm jest wlaczony i przycisk jest wcisniety to resetuje sie alarm i czysci diody
        if (alarm && poprzedni_stan_przycisku == 1 && stan_przycisku == 0 && adc_value < PRZEKROCZENIE_ADC) {
            alarm = 0;
            mruganie = 0;
            licznik_mrugania = 0;
            LATA = 0x00;
        }

        // Jesli nie ma alarmu i potencjometr przekroczy polowe zakresu alarm sie zalacza
        if (!alarm && adc_value >= PRZEKROCZENIE_ADC) {
            alarm = 1;
            mruganie = 1;
            licznik_mrugania = 0;
        }

        if (alarm) {
            if (mruganie) {
                LATA = (licznik_mrugania % 2) ? 0x01 : 0x00;
                licznik_mrugania++;
                if (licznik_mrugania >= CZAS_MRUGANIA) {
                    mruganie = 0;
                    LATA = 0xFF;
                }
            } else {
                LATA = 0xFF;
            }
        }
        
        poprzedni_stan_przycisku = stan_przycisku;
        
        __delay32(2000000);
    }

    return 0;
}

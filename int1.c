#include "system.h"

void INT1Init(void)
{
    IPC0bits.IC1IP = 7;     /* Highest priority */
    IFS1bits.INT1IF = 0;
    IEC1bits.INT1IE = 0;
    INTCON2bits.INT1EP = 0; /* Rising edge */
    IFS1bits.INT1IF = 0;
    IEC1bits.INT1IE = 1;
}

void __attribute__((interrupt,auto_psv)) _INT1Interrupt()
{
    IFS1bits.INT1IF = 0;

    if(!time_out)
    {
        TMR1 = 0;
        IFS0bits.T1IF = 0;
        IEC0bits.T1IE = 1;

        steps++;

        time_out = 1;

        T1CONbits.TON = 1;
    }
    else
    {
        asm("nop");
        asm("nop");
    }

    step_int_cnt++;
}

/*****************************************************************************
 *
 * Explorer 16 Development Board Demo Program.
 * Modified for PIC24FJ64GA004 family with PPS.
 *
 *****************************************************************************
 * FileName:        PIC24ExplDemo.c
 * Dependencies:    system.h
 * Processor:       PIC24
 * Compiler:       	MPLAB C30 v3.00 or later
 * Linker:          MPLAB LINK30
 * Company:         Microchip Technology Incorporated
 *
 *****************************************************************************/

#include "system.h"
#include "lis3dh_driver.h"
// Setup configuration bits
_CONFIG1(WDTPS_PS1 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & ICS_PGx1 & GWRP_OFF & GCP_OFF & JTAGEN_OFF)
_CONFIG2(POSCMOD_NONE & I2C1SEL_PRI & IOL1WAY_OFF & OSCIOFNC_OFF & FCKSM_CSDCMD & FNOSC_FRCPLL & PLL96MHZ_ON & PLLDIV_NODIV & IESO_ON)   // OSCIOFNC_ON: get RA3 as digital I/O
_CONFIG3(WPFP_WPFP0 & SOSCSEL_SOSC & WUTSEL_LEG & WPDIS_WPDIS & WPCFG_WPCFGDIS & WPEND_WPENDMEM)   // SOSCSEL_IO: get RA4 and RB4 as digital I/O
_CONFIG4(DSWDTPS_DSWDTPS3 & DSWDTOSC_LPRC & RTCOSC_SOSC & DSBOREN_OFF & DSWDTEN_OFF)

void LIS3DH_SPIWrite(unsigned char add, unsigned char dat){
    PORTB &= 0xFEFF;
//    address &= 0x7F;

    SPIWrite(add);
    while(SPISTAT & 0x02);
    SPIWrite(dat);
    while(SPISTAT & 0x02);

    while(!(IFS0bits.SPI1IF));
    IFS0bits.SPI1IF = 0;

    PORTB |= 0x0100;
}

void LIS3DH_SPIRead(unsigned char add){
    PORTB &= 0xFEFF;
//    address &= 0x7F;

    add |= 0x80;

    SPIWrite(add);
    while(SPISTAT & 0x02);
    SPIWrite(0xFF);
    while(SPISTAT & 0x02);

    while(!(IFS0bits.SPI1IF));
    IFS0bits.SPI1IF = 0;

    PORTB |= 0x0100;
}

int main(void)
{
    unsigned char response;
    AxesRaw_t data;

    ioMap();
    lockIO();

    // Setup SPI to communicate to EEPROM
    SPIInit();

  //Inizialize MEMS Sensor
  response = LIS3DH_SetMode(LIS3DH_POWER_DOWN);
  //set PowerMode
  response = LIS3DH_SetMode(LIS3DH_NORMAL);
  response = LIS3DH_Reboot();
//  LIS3DH_WriteReg(LIS3DH_CTRL_REG5, 0x80);
  //set ODR (turn ON device)
  response = LIS3DH_SetODR(LIS3DH_ODR_100Hz);
  //set Fullscale
  response = LIS3DH_SetFullScale(LIS3DH_FULLSCALE_2);
  //set axis Enable
//  response = LIS3DH_SetAxis(LIS3DH_X_ENABLE);
  response = LIS3DH_SetAxis(LIS3DH_X_ENABLE | LIS3DH_Y_ENABLE | LIS3DH_Z_ENABLE);
//  response = LIS3DH_SetAxis(LIS3DH_Z_ENABLE);
//  response = LIS3DH_SetAxis(LIS3DH_Z_ENABLE | LIS3DH_X_ENABLE);
#if 1
  //Initialize interrupt
  //write 09h into CTRL_REG2
  response = LIS3DH_HPFClickEnable(MEMS_DISABLE);
  response = LIS3DH_HPFAOI1Enable(MEMS_ENABLE);
  response = LIS3DH_HPFAOI2Enable(MEMS_DISABLE);
  response = LIS3DH_SetHPFMode(LIS3DH_HPM_NORMAL_MODE_RES);
  response = LIS3DH_SetHPFCutOFF(LIS3DH_HPFCF_1);
  response = LIS3DH_SetFilterDataSel(MEMS_ENABLE);
  //write 40h into CTRL_REG3
  response = LIS3DH_SetInt1Pin(LIS3DH_I1_INT1_ON_PIN_INT1_ENABLE);
  //write 08h into CTRL_REG5
  response = LIS3DH_Int1LatchEnable(MEMS_DISABLE);
  //write 10h into INT1_THS
  response = LIS3DH_SetInt1Threshold(0x18);
  //write 00h into INT1_DURATION
  response = LIS3DH_SetInt1Duration(0x00);

  //read HP_FILTER_RESET
  response = LIS3DH_ReadReg(LIS3DH_REFERENCE_REG);
  //write 2Ah into INT1_CFG
  response = LIS3DH_SetIntConfiguration(0x2A);
#endif
    while (1)
    {
        response = LIS3DH_GetAccAxesRaw(&data);
        asm("nop");
        if(PORTB & 0x0200)
        {
            unsigned char temp;

            response = LIS3DH_GetInt1Src(&temp);
            response = LIS3DH_ReadReg(LIS3DH_REFERENCE_REG);
            response = LIS3DH_SetIntConfiguration(0x2A);
        }
    }// End of while(1)...
}// End of main()...


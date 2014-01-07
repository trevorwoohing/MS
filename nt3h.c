/*
 * Copyright (c) Woohing Studios Ltd.
 *
 * This software is the property of Woohing Studios and may
 * not be copied or reproduced otherwise than on to a single hard disk for
 * backup or archival purposes.  The source code is confidential
 * information and must not be disclosed to third parties or used without
 * the express written permission of Woohing Studios.
 *
 *  @file      <file name>
 *  @brief     <A description here>
 *  @author    <Author>
 *  @date      <Date>
 *
 */


/*************************************************************************
 * Include Statements (relative path)
 *************************************************************************/
#include <i2c.h>
#include "nt3h.h"


/*************************************************************************
 * Private Constants
 *************************************************************************/
#define SYSCLK (80000000)
#define PBCLK (SYSCLK/2)
#define FSCK 50000
#define BRG_VAL  ((PBCLK/2/FSCK)-2)


#define I2C_BRG 157 // 100kHz I2C

#define SLAVE_ADRESS     0xAA
#define WR_CMD_BITMASK   0x00
#define RD_CMD_BITMASK   0x01


/*************************************************************************
 * Private macros
 *************************************************************************/


/*************************************************************************
 *   Private typedefs and enums
 *************************************************************************/


/*************************************************************************
 * Private Variables (File Local Variables)
 *************************************************************************/


/*************************************************************************
 * Exported Variables initialisation
 *************************************************************************/

/* TODO: this function should be move to i2c2 object */
static void i2c2_initialisation (void)
{
    I2C2CONbits.I2CEN = 0; //Disable I2C

    //************ I2C interrupt configuration ******************************************************
	//ConfigIntI2C2(MI2C_INT_OFF); //Disable I2C interrupt

    IEC3bits.MI2C2IE = 0; //Disable I2C interrupt


    //***************** I2C2 configuration **********************************************************
    /**********************************************************************************************
    *
    *		I2C2 enabled
    *		continue I2C module in Idle mode
    *		IPMI mode not enabled
    *		I2CADD is 7-bit address
    *		Disable Slew Rate Control for 100KHz
    *		Enable SM bus specification
    *		Disable General call address
    *		Baud @ 8MHz = 39 into I2CxBRG
    **********************************************************************************************/

    I2C2CONbits.I2CEN = 1; //Enable I2C
    I2C2CONbits.A10M = 0; //7-bit slave address
    I2C2CONbits.DISSLW = 1; //Disable slew rate control for high speed I2C (100kHz)
//    I2C2CONbits.DISSLW = 0; //Enable slew rate control for high speed I2C (400kHz)
    I2C2BRG = I2C_BRG; //Set baud rate

}



void nt3h_Initialise (void)
{
    uint8_t sessionData[NT3H_BLOCk_SIZE];

    i2c2_initialisation();

    nt3h_ReadBlock (0x7A ,sessionData, NT3H_BLOCk_SIZE);
    asm("nop");//breakpoint
    asm("nop");//breakpoint
    asm("nop");//breakpoint
    asm("nop");//breakpoint

    sessionData[0] |= 0x02;//SRAM_MIRROR_ON_OFF = 1
    sessionData[2] = 1;
    nt3h_WriteBlock (0x7A ,sessionData, NT3H_BLOCk_SIZE);
    asm("nop");//breakpoint
    asm("nop");//breakpoint
    asm("nop");//breakpoint
    asm("nop");//breakpoint

}

/*************************************************************************
 * Private Function Prototypes
 *************************************************************************/


/*************************************************************************
 * Exported Functions
 *************************************************************************/
/*************************************************************************
 *  @brief      <A description here>
 *
 *  @param      <argument1 first argument>
 *  @param      <argument2 second argument>
 *
 *  @return     <return value>
 *
 *************************************************************************/
void nt3h_WriteBlock (uint8_t block, uint8_t* pData, uint8_t nBytes)
{
    bool_t success = TRUE;
    StartI2C2();    //Send the Start Bit
    IdleI2C2();        //Wait to complete

    /* send the device address */
    MasterWriteI2C2( SLAVE_ADRESS|WR_CMD_BITMASK);
    IdleI2C2();        //Wait to complete
    if( I2C2STATbits.ACKSTAT )  //if 1 then slave has not acknowledge the data.
        success = FALSE;

    /* send the device address */
    MasterWriteI2C2(block);
    IdleI2C2();        //Wait to complete
    if( I2C2STATbits.ACKSTAT )  //if 1 then slave has not acknowledge the data.
        success = FALSE;

    while( nBytes )
    {
        MasterWriteI2C2( *pData++);
        IdleI2C2();        //Wait to complete
        nBytes--;
        //ACKSTAT is 0 when slave acknowledge,
        //if 1 then slave has not acknowledge the data.
        if( I2C2STATbits.ACKSTAT )
        {
            success = FALSE;
            break;
        }
    }

    /* Send the Stop condition */
    StopI2C2();
    IdleI2C2();    //Wait to complete
}


void nt3h_ReadBlock  (uint8_t block, uint8_t* pData, uint8_t nBytes)
{
    /**************************************************************************
    Send device address and set receive mode
    **************************************************************************/
    I2C2CONbits.SEN = 1;  //Send start bit

    while(I2C2CONbits.SEN);  //Wait until Start sequence is completed

    I2C2TRN = (uint8_t)(0xAA);//Write Slave address and set master for transmission
    while(I2C2STATbits.TBF); //Wait till address is transmitted
    while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

    while(I2C2STATbits.ACKSTAT); //Wait for ACK from slave

	while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

    /**************************************************************************
    Write register address byte
    **************************************************************************/
    I2C2TRN = (uint8_t)block; //Write register address to transmit buffer
    while(I2C2STATbits.TBF); //Wait till address is transmitted
    while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state
    while(I2C2STATbits.ACKSTAT); //Wait for ACK from slave

    //Send repeat start bit
    I2C2CONbits.RSEN = 1;  //Send repeat start bit

    while(I2C2CONbits.RSEN);  //Wait until repeat start sequence is completed

    //Set master for reception
    I2C2TRN = (uint8_t)(0xAB);//Write Slave address and set master for reception
    while(I2C2STATbits.TBF); //Wait till address is transmitted
    while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

    while(I2C2STATbits.ACKSTAT); //Wait for ACK from slave

	while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

    /**************************************************************************
    Receive register value
    **************************************************************************/



    while (nBytes)
    {
        nBytes--;

        I2C2CONbits.RCEN = 1; //Enable master recieve mode
        while(I2C2CONbits.RCEN);
        I2C2STATbits.I2COV = 0;
        *pData++ = I2C2RCV;//Copy received data to local variable

        I2C2CONbits.ACKDT = 0; //give the ack
        I2C2CONbits.ACKEN = 1;

        while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
              I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state
    }



    I2C2CONbits.PEN = 1; //Send stop bit
    while(I2C2CONbits.PEN); //Wait until stop sequence is completed
    while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

}

#define TX_TEST_DATA    0xF0

void nt3h_Debug2 (uint8_t block, uint8_t nBytes)
{
    uint8_t RegisterValue = 0;
    /**************************************************************************
    Send device address and set receive mode
    **************************************************************************/
    I2C2CONbits.SEN = 1;  //Send start bit

    while(I2C2CONbits.SEN);  //Wait until Start sequence is completed

    I2C2TRN = (uint8_t)(SLAVE_ADRESS|WR_CMD_BITMASK);//Write Slave address and set master for transmission
    while(I2C2STATbits.TBF); //Wait till address is transmitted
    while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

    while(I2C2STATbits.ACKSTAT); //Wait for ACK from slave

	while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

    /**************************************************************************
    Write register address byte
    **************************************************************************/
    I2C2TRN = (uint8_t)block; //Write register address to transmit buffer
    while(I2C2STATbits.TBF); //Wait till address is transmitted
    while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state
    while(I2C2STATbits.ACKSTAT); //Wait for ACK from slave

    //Send repeat start bit
    I2C2CONbits.RSEN = 1;  //Send repeat start bit

    while(I2C2CONbits.RSEN);  //Wait until repeat start sequence is completed

    //Set master for reception
    I2C2TRN = (uint8_t)(SLAVE_ADRESS|RD_CMD_BITMASK);//Write Slave address and set master for reception
    while(I2C2STATbits.TBF); //Wait till address is transmitted
    while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

    while(I2C2STATbits.ACKSTAT); //Wait for ACK from slave

	while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

    /**************************************************************************
    Receive register value
    **************************************************************************/
    


    while (nBytes)
    {
        nBytes--;
        
        I2C2CONbits.RCEN = 1; //Enable master recieve mode
        while(I2C2CONbits.RCEN);
        I2C2STATbits.I2COV = 0;
        RegisterValue = I2C2RCV;//Copy received data to local variable

        I2C2CONbits.ACKDT = 0; //give the ack
        I2C2CONbits.ACKEN = 1;

        while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
              I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state
    }



    I2C2CONbits.PEN = 1; //Send stop bit
    while(I2C2CONbits.PEN); //Wait until stop sequence is completed
    while(I2C2CONbits.SEN || I2C2CONbits.RSEN || I2C2CONbits.PEN || I2C2CONbits.RCEN ||
          I2C2CONbits.ACKEN || I2C2STATbits.TRSTAT);//Wait for I2C idle state

}


void nt3h_Debug (void)
{
    uint8_t showDebug;
    uint8_t nBytes = 16;
    // START
    StartI2C2();
    IdleI2C2();

    // Control Byte
    MasterWriteI2C2(0xAA); //Write Slave address and set master for transmission
    IdleI2C2();

    MasterWriteI2C2(0x0); //Write Slave address and set master for transmission
    IdleI2C2();

    RestartI2C2();
    IdleI2C2();
    
    MasterWriteI2C2(0xAA); //Write Slave address and set master for transmission
    IdleI2C2();

    while( nBytes )
    {
        showDebug = MasterReadI2C2();
        nBytes--;
        asm("nop");//breakpoint
        asm("nop");//breakpoint
        AckI2C2();
        IdleI2C2();
    }
    // STOP
    StopI2C2(); // Create Stop Sequence
    IdleI2C2();

}



#ifdef REFERENCE
void nt3h_ReadBlock  (uint8_t block, uint8_t* pData, uint8_t nBytes)
{
    bool_t success = TRUE;
    uint8_t showDebug;
    StartI2C2();    //Send the Start Bit
    IdleI2C2();        //Wait to complete

    /* send the device address */
    MasterWriteI2C2 (SLAVE_ADRESS|WR_CMD_BITMASK);
    IdleI2C2();        //Wait to complete
    if( I2C2STATbits.ACKSTAT )  //if 1 then slave has not acknowledge the data.
        success = FALSE;

    /* send the block address */
    MasterWriteI2C2(block);
    IdleI2C2();        //Wait to complete
    if( I2C2STATbits.ACKSTAT )  //if 1 then slave has not acknowledge the data.
        success = FALSE;

    /* Send the Stop condition */
    StopI2C2();
    IdleI2C2();    //Wait to complete

    StartI2C2();    //Send the Start Bit
    IdleI2C2();        //Wait to complete

    MasterWriteI2C2( SLAVE_ADRESS|RD_CMD_BITMASK);
    IdleI2C2();        //Wait to complete
    if( I2C2STATbits.ACKSTAT )  //if 1 then slave has not acknowledge the data.
        success = FALSE;

    while( nBytes )
    {

      //  *pData++ = MasterReadI2C2();
        showDebug = MasterReadI2C2();
        *pData++ = showDebug;


        IdleI2C2();        //Wait to complete

        nBytes--;
        //ACKSTAT is 0 when slave acknowledge,
        //if 1 then slave has not acknowledge the data.
        if( I2C2STATbits.ACKSTAT )
        {
            success = FALSE;
            break;
        }
    }

    /* Send the Stop condition */
    StopI2C2();
    IdleI2C2();    //Wait to complete
        asm("nop");//breakpoint
        asm("nop");//breakpoint
}
#endif//REFERENCE
/*************************************************************************
 *  Private Functions
 *************************************************************************/
/*************************************************************************
 *  @brief      <A description here>
 *
 *  @param      <argument1 first argument>
 *  @param      <argument2 second argument>
 *
 *  @return     <return value>
 *
 *************************************************************************/



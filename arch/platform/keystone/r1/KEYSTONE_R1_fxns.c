/*
 * Copyright (c) 2018, Texas Instruments Incorporated
 * Copyright (c) 2018, This. Is. IoT. - https://thisisiot.io
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * ======== CC1352R1_LAUNCHXL_fxns.c ========
 *  This file contains the board-specific initialization functions, and
 *  RF callback function for antenna switching.
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/ioc.h)
#include DeviceFamily_constructPath(driverlib/cpu.h)
#include <ti/drivers/rf/RF.h>
#include <ti/drivers/pin/PINCC26XX.h>

#include "Board.h"
#include "lib/sensors.h"
#include "batmon-sensor.h"

 /*---------------------------------------------------------------------------*/
 /* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "Board"
#define LOG_LEVEL LOG_LEVEL_NONE
 /*---------------------------------------------------------------------------*/

extern void SX126xIoInit(void);

/*
 *  ======== CC1352R1_LAUNCHXL_sendExtFlashByte ========
 */
void CC1352R1_LAUNCHXL_sendExtFlashByte(PIN_Handle pinHandle, uint8_t byte)
{
    uint8_t i;

    /* SPI Flash CS */
    PIN_setOutputValue(pinHandle, IOID_20, 0);

    for (i = 0; i < 8; i++) {
        PIN_setOutputValue(pinHandle, IOID_10, 0);  /* SPI Flash CLK */

        /* SPI Flash MOSI */
        PIN_setOutputValue(pinHandle, IOID_9, (byte >> (7 - i)) & 0x01);
        PIN_setOutputValue(pinHandle, IOID_10, 1);  /* SPI Flash CLK */

        /*
         * Waste a few cycles to keep the CLK high for at
         * least 45% of the period.
         * 3 cycles per loop: 8 loops @ 48 Mhz = 0.5 us.
         */
        CPUdelay(8);
    }

    PIN_setOutputValue(pinHandle, IOID_10, 0);  /* CLK */
    PIN_setOutputValue(pinHandle, IOID_20, 1);  /* CS */

    /*
     * Keep CS high at least 40 us
     * 3 cycles per loop: 700 loops @ 48 Mhz ~= 44 us
     */
    CPUdelay(700);
}

/*
 *  ======== CC1352R1_LAUNCHXL_wakeUpExtFlash ========
 */
void CC1352R1_LAUNCHXL_wakeUpExtFlash(void)
{
    PIN_Config extFlashPinTable[] = {
        /* SPI Flash CS */
        IOID_20 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL |
                PIN_INPUT_DIS | PIN_DRVSTR_MED,
        PIN_TERMINATE
    };
    PIN_State extFlashPinState;
    PIN_Handle extFlashPinHandle = PIN_open(&extFlashPinState, extFlashPinTable);

    /*
     *  To wake up we need to toggle the chip select at
     *  least 20 ns and ten wait at least 35 us.
     */

    /* Toggle chip select for ~20ns to wake ext. flash */
    PIN_setOutputValue(extFlashPinHandle, IOID_20, 0);
    /* 3 cycles per loop: 1 loop @ 48 Mhz ~= 62 ns */
    CPUdelay(1);
    PIN_setOutputValue(extFlashPinHandle, IOID_20, 1);
    /* 3 cycles per loop: 560 loops @ 48 Mhz ~= 35 us */
    CPUdelay(560);

    PIN_close(extFlashPinHandle);
}

/*
 *  ======== CC1352R1_LAUNCHXL_shutDownExtFlash ========
 */
void CC1352R1_LAUNCHXL_shutDownExtFlash(void)
{
    /*
     *  To be sure we are putting the flash into sleep and not waking it,
     *  we first have to make a wake up call
     */
    CC1352R1_LAUNCHXL_wakeUpExtFlash();

    PIN_Config extFlashPinTable[] = {
        /* SPI Flash CS*/
        IOID_20 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_HIGH | PIN_PUSHPULL |
                PIN_INPUT_DIS | PIN_DRVSTR_MED,
        /* SPI Flash CLK */
        IOID_10 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL |
                PIN_INPUT_DIS | PIN_DRVSTR_MED,
        /* SPI Flash MOSI */
        IOID_9 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL |
                PIN_INPUT_DIS | PIN_DRVSTR_MED,
        /* SPI Flash MISO */
        IOID_8 | PIN_INPUT_EN | PIN_PULLDOWN,
        PIN_TERMINATE
    };
    PIN_State extFlashPinState;
    PIN_Handle extFlashPinHandle = PIN_open(&extFlashPinState, extFlashPinTable);

    uint8_t extFlashShutdown = 0xB9;

    CC1352R1_LAUNCHXL_sendExtFlashByte(extFlashPinHandle, extFlashShutdown);

    PIN_close(extFlashPinHandle);
}

/*
 *  ======== Board_initHook ========
 *  Called by Board_init() to perform board-specific initialization.
 */
void Board_initHook()
{
    CC1352R1_LAUNCHXL_shutDownExtFlash();

    SX126xIoInit();

    SENSORS_ACTIVATE(batmon_sensor);
}


/*
 * ======== rfDriverCallback ========
 * This is an implementation for the CC1352R1 launchpad which uses a
 * single signal for antenna switching.
 *
 */
void rfDriverCallback(RF_Handle client, RF_GlobalEvent events, void *arg)
{
    /* Note this is execute in a context that won't allow UART messages. */
    (void)client;
    RF_RadioSetup* setupCommand = (RF_RadioSetup*)arg;

    if ((events & RF_GlobalEventRadioSetup) &&
            (setupCommand->common.commandNo == CMD_PROP_RADIO_DIV_SETUP)) {

        //LOG_DBG("CC1352 Sub-GHz antenna selected\n");
        KEYSTONE_R1_setAntennaMeshRadio();
    }
    else if (events & RF_GlobalEventRadioPowerDown) {
        //LOG_DBG("CC1352 Sub-GHz antenna de-selected\n");
        /* Disable antenna switch to save current */
        KEYSTONE_R1_unsetAntennaMeshRadio();
    }
}


/* Get the platform battery level for the LoRaMac stack */
uint8_t BoardGetBatteryLevel(void)
{
    int value;

    value = batmon_sensor.value(BATMON_SENSOR_TYPE_VOLT);

    return (uint8_t)value;

}
/*
*  NCTRL  CTRL
*  LOW    LOW     ?
*  LOW    HIGH    RF1   SX1262
*  HIGH   LOW     RF2   CC1352
*  HIGH   HIGH    RF1   SX1262
*/
void KEYSTONE_R1_setAntennaMeshRadio(void)
{
    /* Bring CTRL LOW first, then NCTRL HIGH  */
    PINCC26XX_setOutputValue(Board_RF_SUB1GHZ_CTRL, 0);
    PINCC26XX_setOutputValue(Board_RF_SUB1GHZ_NCTRL, 1);
}

/*
*  NCTRL  CTRL
*  LOW    LOW     ?
*  LOW    HIGH    RF1   SX1262
*  HIGH   LOW     RF2   CC1352
*  HIGH   HIGH    RF1   SX1262
*/
void KEYSTONE_R1_unsetAntennaMeshRadio(void)
{
    /* Bring CTRL and NCTRL LOW  */
    PINCC26XX_setOutputValue(Board_RF_SUB1GHZ_CTRL, 0);
    PINCC26XX_setOutputValue(Board_RF_SUB1GHZ_NCTRL, 0);
}

/*
*  NCTRL  CTRL
*  LOW    LOW     ?
*  LOW    HIGH    RF1   SX1262
*  HIGH   LOW     RF2   CC1352
*  HIGH   HIGH    RF1   SX1262
*/
void KEYSTONE_R1_setAntennaLoRaRadio(void)
{
    /* Bring NCTRL LOW first, then CTRL HIGH  */
    PINCC26XX_setOutputValue(Board_RF_SUB1GHZ_NCTRL, 0);
    PINCC26XX_setOutputValue(Board_RF_SUB1GHZ_CTRL, 1);
}

/*
*  NCTRL  CTRL
*  LOW    LOW     ?
*  LOW    HIGH    RF1   SX1262
*  HIGH   LOW     RF2   CC1352
*  HIGH   HIGH    RF1   SX1262
*/
void KEYSTONE_R1_unsetAntennaLoRaRadio(void)
{
    /* Bring NCTRL then CTRL LOW  */
    PINCC26XX_setOutputValue(Board_RF_SUB1GHZ_NCTRL, 0);
    PINCC26XX_setOutputValue(Board_RF_SUB1GHZ_CTRL, 0);
}

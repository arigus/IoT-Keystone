/*
 * Copyright (c) 2018, Texas Instruments Incorporated - http://www.ti.com/
 * Copyright (c) 2018, This. Is. IoT.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/**
 * \addtogroup sensortag-peripherals
 * @{
 *
 * \defgroup sensortag-opt-sensor SensorTag Optical Sensor
 *
 *        Due to the time required for the sensor to startup, this driver is
 *        meant to be used in an asynchronous fashion. The caller must first
 *        activate the sensor by calling SENSORS_ACTIVATE(). This will trigger
 *        the sensor's startup sequence, but the call will not wait for it to
 *        complete so that the CPU can perform other tasks or drop to a low
 *        power mode.
 *
 *        Once the reading and conversion are complete, the driver will
 *        generate a sensors_changed event.
 *
 *        We use single-shot readings. In this mode, the hardware
 *        automatically goes back to its shutdown mode after the conversion
 *        is finished. However, it will still respond to I2C operations, so
 *        the last conversion can still be read out.
 *
 *        In order to take a new reading, the caller has to use
 *        SENSORS_ACTIVATE again.
 *
 *        The sensor can be identified as being present on
 *        the bus, along with the manufacturer and device IDs,
 *        by polling status(OPT_3001_IDENTIFY).
 * @{
 *
 * \file
 *        Header file for the Keystone OPT-3001 light sensor.
 * \author
 *        Edvard Pettersen <e.pettersen@ti.com>
 *        Evan Ross <evan@thisisiot.io>
 */
/*---------------------------------------------------------------------------*/
#ifndef OPT_3001_SENSOR_H_
#define OPT_3001_SENSOR_H_
/*---------------------------------------------------------------------------*/
#include "contiki.h"
/*---------------------------------------------------------------------------*/
#include "board-conf.h"
/*---------------------------------------------------------------------------*/
#if BOARD_SENSORS_ENABLE
#if (TI_I2C_CONF_ENABLE == 0) || (TI_I2C_CONF_I2C0_ENABLE == 0)
#error "The OPT-3001 requires the I2C driver (TI_I2C_CONF_ENABLE = 1)"
#endif
#endif
/*---------------------------------------------------------------------------*/
#define OPT_3001_READING_ERROR  -1
#define OPT_3001_IDENTIFY       135
/*---------------------------------------------------------------------------*/
typedef enum {
  OPT_3001_STATUS_DISABLED,
  OPT_3001_STATUS_STANDBY,
  OPT_3001_STATUS_BOOTING,
  OPT_3001_STATUS_ACTIVE,
  OPT_3001_STATUS_DATA_READY,
  OPT_3001_STATUS_I2C_ERROR,
} OPT_3001_STATUS;
/*---------------------------------------------------------------------------*/
extern const struct sensors_sensor opt_3001_sensor;
/*---------------------------------------------------------------------------*/
#endif /* OPT_3001_SENSOR_H_ */
/*---------------------------------------------------------------------------*/
/**
 * @}
 * @}
 */

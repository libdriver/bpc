/**
 * Copyright (c) 2015 - present LibDriver All rights reserved
 * 
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE. 
 *
 * @file      driver_bpc_interface.h
 * @brief     driver bpc interface header file
 * @version   1.0.0
 * @author    Shifeng Li
 * @date      2025-12-30
 *
 * <h3>history</h3>
 * <table>
 * <tr><th>Date        <th>Version  <th>Author      <th>Description
 * <tr><td>2025/12/30  <td>1.0      <td>Shifeng Li  <td>first upload
 * </table>
 */

#ifndef DRIVER_BPC_INTERFACE_H
#define DRIVER_BPC_INTERFACE_H

#include "driver_bpc.h"

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup bpc_interface_driver bpc interface driver function
 * @brief    bpc interface driver modules
 * @ingroup  bpc_driver
 * @{
 */

/**
 * @brief     interface timestamp read
 * @param[in] *t pointer to a bpc_time structure
 * @return    status code
 *            - 0 success
 *            - 1 read failed
 * @note      none
 */
uint8_t bpc_interface_timestamp_read(bpc_time_t *t);

/**
 * @brief     interface delay ms
 * @param[in] ms time
 * @note      none
 */
void bpc_interface_delay_ms(uint32_t ms);

/**
 * @brief     interface print format data
 * @param[in] fmt format data
 * @note      none
 */
void bpc_interface_debug_print(const char *const fmt, ...);

/**
 * @brief     interface receive callback
 * @param[in] *data pointer to a bpc_t structure
 * @note      none
 */
void bpc_interface_receive_callback(bpc_t *data);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

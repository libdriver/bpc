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
 * @file      driver_bpc.h
 * @brief     driver bpc header file
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

#ifndef DRIVER_BPC_H
#define DRIVER_BPC_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif

/**
 * @defgroup bpc_driver bpc driver function
 * @brief    bpc driver modules
 * @{
 */

/**
 * @addtogroup bpc_basic_driver
 * @{
 */

/**
 * @brief bpc max start range definition
 */
#ifndef BPC_MAX_START_RANGE
    #define BPC_MAX_START_RANGE        0.20f        /**< 20% */
#endif

/**
 * @brief bpc max range definition
 */
#ifndef BPC_MAX_RANGE
    #define BPC_MAX_RANGE        0.20f        /**< 20% */
#endif

/**
 * @brief bpc status enumeration definition
 */
typedef enum
{
    BPC_STATUS_OK            = 0x00,        /**< ok */
    BPC_STATUS_PARITY_ERR    = 0x01,        /**< parity error */
    BPC_STATUS_FRAME_INVALID = 0x02,        /**< frame invalid */
} bpc_status_t;

/**
 * @brief bpc structure definition
 */
typedef struct bpc_s
{
    uint8_t status;       /**< status */
    uint16_t year;        /**< year */
    uint8_t month;        /**< month */
    uint8_t day;          /**< day */
    uint8_t week;         /**< week */
    uint8_t hour;         /**< hour */
    uint8_t minute;       /**< minute */
    uint8_t second;       /**< second */
} bpc_t;

/**
 * @brief bpc time structure definition
 */
typedef struct bpc_time_s
{
    uint64_t s;         /**< second */
    uint32_t us;        /**< microsecond */
} bpc_time_t;

/**
 * @brief bpc decode structure definition
 */
typedef struct bpc_decode_s
{
    bpc_time_t t;        /**< timestamp */
    uint32_t diff_us;    /**< diff us */
} bpc_decode_t;

/**
 * @brief bpc handle structure definition
 */
typedef struct bpc_handle_s
{
    uint8_t (*timestamp_read)(bpc_time_t *t);               /**< point to a timestamp_read function address */
    void (*delay_ms)(uint32_t ms);                          /**< point to a delay_ms function address */
    void (*debug_print)(const char *const fmt, ...);        /**< point to a debug_print function address */
    void (*receive_callback)(bpc_t *data);                  /**< point to a receive_callback function address */
    uint8_t inited;                                         /**< inited flag */
    bpc_decode_t decode[76];                                /**< decode buffer */
    uint16_t decode_len;                                    /**< decode length */
    bpc_time_t last_time;                                   /**< last time */
    uint8_t decode_offset;                                  /**< decode offset */
    uint8_t decode_valid;                                   /**< decode valid */
    uint8_t trace_valid;                                    /**< trace valid */
} bpc_handle_t;

/**
 * @brief bpc information structure definition
 */
typedef struct bpc_info_s
{
    char chip_name[32];                /**< chip name */
    char manufacturer_name[32];        /**< manufacturer name */
    char interface[8];                 /**< chip interface name */
    float supply_voltage_min_v;        /**< chip min supply voltage */
    float supply_voltage_max_v;        /**< chip max supply voltage */
    float max_current_ma;              /**< chip max current */
    float temperature_min;             /**< chip min operating temperature */
    float temperature_max;             /**< chip max operating temperature */
    uint32_t driver_version;           /**< driver version */
} bpc_info_t;

/**
 * @}
 */

/**
 * @defgroup bpc_link_driver bpc link driver function
 * @brief    bpc link driver modules
 * @ingroup  bpc_driver
 * @{
 */

/**
 * @brief     initialize bpc_handle_t structure
 * @param[in] HANDLE pointer to a bpc handle structure
 * @param[in] STRUCTURE bpc_handle_t
 * @note      none
 */
#define DRIVER_BPC_LINK_INIT(HANDLE, STRUCTURE)               memset(HANDLE, 0, sizeof(STRUCTURE))

/**
 * @brief     link timestamp_read function
 * @param[in] HANDLE pointer to a bpc handle structure
 * @param[in] FUC pointer to a timestamp_read function address
 * @note      none
 */
#define DRIVER_BPC_LINK_TIMESTAMP_READ(HANDLE, FUC)          (HANDLE)->timestamp_read = FUC

/**
 * @brief     link delay_ms function
 * @param[in] HANDLE pointer to a bpc handle structure
 * @param[in] FUC pointer to a delay_ms function address
 * @note      none
 */
#define DRIVER_BPC_LINK_DELAY_MS(HANDLE, FUC)                (HANDLE)->delay_ms = FUC

/**
 * @brief     link debug_print function
 * @param[in] HANDLE pointer to a bpc handle structure
 * @param[in] FUC pointer to a debug_print function address
 * @note      none
 */
#define DRIVER_BPC_LINK_DEBUG_PRINT(HANDLE, FUC)             (HANDLE)->debug_print = FUC

/**
 * @brief     link receive_callback function
 * @param[in] HANDLE pointer to a bpc handle structure
 * @param[in] FUC pointer to a receive_callback function address
 * @note      none
 */
#define DRIVER_BPC_LINK_RECEIVE_CALLBACK(HANDLE, FUC)        (HANDLE)->receive_callback = FUC

/**
 * @}
 */

/**
 * @defgroup bpc_basic_driver bpc basic driver function
 * @brief    bpc basic driver modules
 * @ingroup  bpc_driver
 * @{
 */

/**
 * @brief      get chip's information
 * @param[out] *info pointer to a bpc info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t bpc_info(bpc_info_t *info);

/**
 * @brief     irq handler
 * @param[in] *handle pointer to a bpc handle structure
 * @return    status code
 *            - 0 success
 *            - 1 run failed
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t bpc_irq_handler(bpc_handle_t *handle);

/**
 * @brief     initialize the chip
 * @param[in] *handle pointer to a bpc handle structure
 * @return    status code
 *            - 0 success
 *            - 1 gpio initialization failed
 *            - 2 handle is NULL
 *            - 3 linked functions is NULL
 * @note      none
 */
uint8_t bpc_init(bpc_handle_t *handle);

/**
 * @brief     close the chip
 * @param[in] *handle pointer to a bpc handle structure
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t bpc_deinit(bpc_handle_t *handle);

/**
 * @}
 */

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif

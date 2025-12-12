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
 * @file      driver_bpc_basic.c
 * @brief     driver bpc basic source file
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

#include "driver_bpc_basic.h"

static bpc_handle_t gs_handle;          /**< bpc handle */
static int8_t gs_time_zone = 0;          /**< local zone */

/**
 * @brief  basic irq
 * @return status code
 *         - 0 success
 *         - 1 run failed
 * @note   none
 */
uint8_t bpc_basic_irq_handler(void)
{
    if (bpc_irq_handler(&gs_handle) != 0)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief     basic example init
 * @param[in] *callback pointer to an irq callback address
 * @return    status code
 *            - 0 success
 *            - 1 init failed
 * @note      none
 */
uint8_t bpc_basic_init(void (*callback)(bpc_t *data))
{
    uint8_t res;
    
    /* link interface function */
    DRIVER_BPC_LINK_INIT(&gs_handle, bpc_handle_t);
    DRIVER_BPC_LINK_TIMESTAMP_READ(&gs_handle, bpc_interface_timestamp_read);
    DRIVER_BPC_LINK_DELAY_MS(&gs_handle, bpc_interface_delay_ms);
    DRIVER_BPC_LINK_DEBUG_PRINT(&gs_handle, bpc_interface_debug_print);
    DRIVER_BPC_LINK_RECEIVE_CALLBACK(&gs_handle, callback);
    
    /* init */
    res = bpc_init(&gs_handle);
    if (res != 0)
    {
        bpc_interface_debug_print("bpc: init failed.\n");
       
        return 1;
    }
    
    return 0;
}

/**
 * @brief  basic example deinit
 * @return status code
 *         - 0 success
 *         - 1 deinit failed
 * @note   none
 */
uint8_t bpc_basic_deinit(void)
{
    if (bpc_deinit(&gs_handle) != 0)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief     basic example set the local time zone
 * @param[in] zone local time zone
 * @return    status code
 *            - 0 success
 * @note      none
 */
uint8_t bpc_basic_set_timestamp_time_zone(int8_t zone)
{
    gs_time_zone = zone;
    
    return 0;
}

/**
 * @brief      basic example get the local time zone
 * @param[out] *zone pointer to a local time zone buffer
 * @return     status code
 *             - 0 success
 * @note       none
 */
uint8_t bpc_basic_get_timestamp_time_zone(int8_t *zone)
{
    *zone = gs_time_zone;
    
    return 0;
}

/**
 * @brief      basic example convert the time to a unix timestamp
 * @param[in]  *t pointer to a bpc structure
 * @param[out] *timestamp pointer to a unix timestamp buffer
 * @return     status code
 *             - 0 success
 *             - 1 get timestamp failed
 * @note       none
 */
uint8_t bpc_basic_convert_timestamp(bpc_t *t, time_t *timestamp)
{
    struct tm timeptr;
    
    timeptr.tm_year = t->year - 1900;
    timeptr.tm_mon = t->month - 1;
    timeptr.tm_wday = t->week; 
    timeptr.tm_mday = t->day;
    timeptr.tm_hour = t->hour;
    timeptr.tm_min = t->minute;
    timeptr.tm_sec = t->second;
    
    /* make time */
    *timestamp = mktime(&timeptr) - gs_time_zone * 3600;
    
    return 0;
}

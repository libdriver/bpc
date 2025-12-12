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
 * @file      driver_bpc_receive_test.c
 * @brief     driver bpc receive test source file
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

#include "driver_bpc_receive_test.h"

static bpc_handle_t gs_handle;        /**< bpc handle */
static volatile uint8_t gs_flag;      /**< flag */

/**
 * @brief  receive test irq
 * @return status code
 *         - 0 success
 *         - 1 run failed
 * @note   none
 */
uint8_t bpc_receive_test_irq_handler(void)
{
    if (bpc_irq_handler(&gs_handle) != 0)
    {
        return 1;
    }
    
    return 0;
}

/**
 * @brief     interface receive callback
 * @param[in] *data pointer to a bpc_t structure
 * @note      none
 */
static void a_receive_callback(bpc_t *data)
{
    const char week[][10] = 
    {
        "Sunday",
        "Monday",
        "Tuesday",
        "Wednesday",
        "Thursday",
        "Friday",
        "Saturday",
        "Invalid",
    };
    
    switch (data->status)
    {
        case BPC_STATUS_OK :
        {
            bpc_interface_debug_print("bpc: irq ok.\n");
            bpc_interface_debug_print("bpc: time is %04d-%02d-%02d %02d:%02d:%02d %s.\n", 
                                      data->year, data->month, data->day,
                                      data->hour, data->minute, data->second,
                                      week[data->week > 7 ? 7 : data->week]);
            gs_flag = 1;
            
            break;
        }
        case BPC_STATUS_PARITY_ERR :
        {
            bpc_interface_debug_print("bpc: irq parity error.\n");
            
            break;
        }
        case BPC_STATUS_FRAME_INVALID :
        {
            bpc_interface_debug_print("bpc: irq frame invalid.\n");
            
            break;
        }
        default :
        {
            bpc_interface_debug_print("bpc: irq unknown status.\n");
            
            break;
        }
    }
}

/**
 * @brief     receive test
 * @param[in] times test times
 * @return    status code
 *            - 0 success
 *            - 1 test failed
 * @note      none
 */
uint8_t bpc_receive_test(uint32_t times)
{
    uint8_t res;
    uint16_t timeout;
    uint32_t i;
    bpc_info_t info;
    
    /* link interface function */
    DRIVER_BPC_LINK_INIT(&gs_handle, bpc_handle_t);
    DRIVER_BPC_LINK_TIMESTAMP_READ(&gs_handle, bpc_interface_timestamp_read);
    DRIVER_BPC_LINK_DELAY_MS(&gs_handle, bpc_interface_delay_ms);
    DRIVER_BPC_LINK_DEBUG_PRINT(&gs_handle, bpc_interface_debug_print);
    DRIVER_BPC_LINK_RECEIVE_CALLBACK(&gs_handle, a_receive_callback);
    
    /* get information */
    res = bpc_info(&info);
    if (res != 0)
    {
        bpc_interface_debug_print("bpc: get info failed.\n");
       
        return 1;
    }
    else
    {
        /* print chip info */
        bpc_interface_debug_print("bpc: chip is %s.\n", info.chip_name);
        bpc_interface_debug_print("bpc: manufacturer is %s.\n", info.manufacturer_name);
        bpc_interface_debug_print("bpc: interface is %s.\n", info.interface);
        bpc_interface_debug_print("bpc: driver version is %d.%d.\n", info.driver_version / 1000, (info.driver_version % 1000) / 100);
        bpc_interface_debug_print("bpc: min supply voltage is %0.1fV.\n", info.supply_voltage_min_v);
        bpc_interface_debug_print("bpc: max supply voltage is %0.1fV.\n", info.supply_voltage_max_v);
        bpc_interface_debug_print("bpc: max current is %0.2fmA.\n", info.max_current_ma);
        bpc_interface_debug_print("bpc: max temperature is %0.1fC.\n", info.temperature_max);
        bpc_interface_debug_print("bpc: min temperature is %0.1fC.\n", info.temperature_min);
    }
    
    /* init */
    res = bpc_init(&gs_handle);
    if (res != 0)
    {
        bpc_interface_debug_print("bpc: init failed.\n");
       
        return 1;
    }
    
    /* start receive test */
    bpc_interface_debug_print("bpc: start receive test.\n");
    
    /* loop */
    for (i = 0; i < times; i++)
    {
        /* 60s timeout */
        timeout = 6000;
        
        /* init 0 */
        gs_flag = 0;
        
        /* check timeout */
        while (timeout != 0)
        {
            /* check the flag */
            if (gs_flag != 0)
            {
                break;
            }
            
            /* timeout -- */
            timeout--;
            
            /* delay 10ms */
            bpc_interface_delay_ms(10);
        }
        
        /* check the timeout */
        if (timeout == 0)
        {
            /* receive timeout */
            bpc_interface_debug_print("bpc: receive timeout.\n");
            (void)bpc_deinit(&gs_handle);
                
            return 1;
        }
    }
    
    /* finish receive test */
    bpc_interface_debug_print("bpc: finish receive test.\n");
    (void)bpc_deinit(&gs_handle);
    
    return 0;
}

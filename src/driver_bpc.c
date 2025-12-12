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
 * @file      driver_bpc.c
 * @brief     driver bpc source file
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

#include "driver_bpc.h"
#include <stdlib.h>

/**
 * @brief chip information definition
 */
#define CHIP_NAME                 "China BPC"        /**< chip name */
#define MANUFACTURER_NAME         "China"            /**< manufacturer name */
#define SUPPLY_VOLTAGE_MIN        2.7f               /**< chip min supply voltage */
#define SUPPLY_VOLTAGE_MAX        5.5f               /**< chip max supply voltage */
#define MAX_CURRENT               1.5f               /**< chip max current */
#define TEMPERATURE_MIN           -40.0f             /**< chip min operating temperature */
#define TEMPERATURE_MAX           125.0f             /**< chip max operating temperature */
#define DRIVER_VERSION            1000               /**< driver version */

/**
 * @brief frame check definition
 */
#define BPC_CHECK_START_FRAME_MIN        (1600 * 1000)       /**< start min frame */
#define BPC_CHECK_START_FRAME_MAX        (1900 * 1000)       /**< start max frame */
#define BPC_CHECK_FRAME_TIME             (1000 * 1000)       /**< frame time */
#define BPC_CHECK_DATA_0                 (100 * 1000)        /**< data 0 */
#define BPC_CHECK_DATA_1                 (200 * 1000)        /**< data 1 */
#define BPC_CHECK_DATA_2                 (300 * 1000)        /**< data 2 */
#define BPC_CHECK_DATA_3                 (400 * 1000)        /**< data 3 */

/**
 * @brief     check the frame time
 * @param[in] check checked time
 * @param[in] t standard time
 * @return    status code
 *            - 0 success
 *            - 1 checked failed
 * @note      none
 */
static inline uint8_t a_check_frame(uint32_t check, uint32_t t)
{
    if (abs((int)((int)check - (int)t)) > (int)((float)(t) * BPC_MAX_RANGE))        /* check the time */
    {
        return 1;                                                                   /* check failed */
    }
    else
    {
        return 0;                                                                   /* success return 0 */
    }
}

/**
 * @brief     check the frame time
 * @param[in] check checked time
 * @param[in] t standard time
 * @return    status code
 *            - 0 success
 *            - 1 checked failed
 * @note      none
 */
static inline uint8_t a_check_frame2(uint32_t check, uint32_t remain_check, uint32_t t)
{
    uint32_t total;
    
    total = check + remain_check;                                                           /* get the total time */
    if (((float)(total) < (float)(BPC_CHECK_FRAME_TIME) * (1.0f - BPC_MAX_RANGE)) ||
        ((float)(total) > (float)(BPC_CHECK_FRAME_TIME) * (1.0f + BPC_MAX_RANGE)))          /* check range */
    {
        return 1;                                                                           /* check failed */
    }
    
    if (abs((int)((int)check - (int)t)) > (int)((float)(t) * BPC_MAX_RANGE))                /* check the time */
    {
        return 1;                                                                           /* check failed */
    }
    else
    {
        return 0;                                                                           /* success return 0 */
    }
}

/**
 * @brief     check the start frame time
 * @param[in] check checked time
 * @param[in] t standard time
 * @return    status code
 *            - 0 success
 *            - 1 checked failed
 * @note      none
 */
static inline uint8_t a_check_start_frame(uint32_t check)
{
    if (((float)(check) > (float)(BPC_CHECK_START_FRAME_MIN) * (1.0f - BPC_MAX_START_RANGE)) &&
        ((float)(check) < (float)(BPC_CHECK_START_FRAME_MAX) * (1.0f + BPC_MAX_START_RANGE)))        /* check range */
    {
        return 0;                                                                                    /* success return 0 */ 
    }
    else
    {
        return 1;                                                                                    /* check failed */
    }
}

/**
 * @brief     bpc trace decode
 * @param[in] *handle pointer to a bpc handle structure
 * @note      none
 */
static void a_bpc_trace_decode(bpc_handle_t *handle)
{
    int64_t diff;
    
    diff = (int64_t)((int64_t)handle->decode[0].t.s -
           (int64_t)handle->last_time.s) * 1000000 + 
           (int64_t)((int64_t)handle->decode[0].t.us -
           (int64_t)handle->last_time.us);               /* diff time */
    if (a_check_start_frame((uint32_t)diff) == 0)        /* check diff time */
    {
        handle->decode_offset = 0;                       /* init 0 */
        handle->decode_valid = 1;                        /* set valid */
        handle->trace_valid = 0;                         /* set invalid */
    }
    else
    {
        handle->decode_len = 0;                          /* clear the buffer */
        handle->decode_offset = 0;                       /* init 0 */
        handle->decode_valid = 0;                        /* set invalid */
        handle->trace_valid = 0;                         /* set invalid */
    }
}

/**
 * @brief     bpc start decode
 * @param[in] *handle pointer to a bpc handle structure
 * @note      none
 */
static void a_bpc_start_decode(bpc_handle_t *handle)
{
    uint16_t i;
    uint16_t len;
    
    len = handle->decode_len - 1;                                                 /* len - 1 */
    for (i = 0; i < len; i++)                                                     /* diff all time */
    {
        int64_t diff;
        
        diff = (int64_t)((int64_t)handle->decode[i + 1].t.s -
               (int64_t)handle->decode[i].t.s) * 1000000 + 
               (int64_t)((int64_t)handle->decode[i + 1].t.us -
               (int64_t)handle->decode[i].t.us);                                  /* diff time */
        handle->decode[i].diff_us = (uint32_t)diff;                               /* save the time diff */
        if (a_check_start_frame((uint32_t)handle->decode[i].diff_us) == 0)        /* check start frame */
        {
            handle->decode_valid = 1;                                             /* set valid */
            handle->decode_offset = (uint8_t)(i + 1);                             /* save offset */
        }
    }
}

/**
 * @brief     bpc data decode
 * @param[in] *handle pointer to a bpc handle structure
 * @param[in] diff_us diff time in us
 * @param[in] *data pointer to a data buffer
 * @note      none
 */
static uint8_t a_bpc_data_decode(bpc_handle_t *handle, uint32_t diff_us, uint8_t *data)
{
    if (a_check_frame(diff_us, BPC_CHECK_DATA_0) == 0)                    /* check diff */
    {
        *data = 0;                                                        /* set data 0 */
    }
    else
    {
        if (a_check_frame(diff_us, BPC_CHECK_DATA_1) == 0)                /* check start diff */
        {
            *data = 1;                                                    /* set data 1 */
        }
        else
        {
            if (a_check_frame(diff_us, BPC_CHECK_DATA_2) == 0)            /* check start diff */
            {
                *data = 2;                                                /* set data 2 */
            }
            else
            {
                if (a_check_frame(diff_us, BPC_CHECK_DATA_3) == 0)        /* check start diff */
                {
                    *data = 3;                                            /* set data 3 */
                }
                else
                {
                    return 1;                                             /* return error */
                }
            }
        }
    }
    
    return 0;                                                             /* success return 0 */
}

/**
 * @brief     bpc data decode
 * @param[in] *handle pointer to a bpc handle structure
 * @param[in] diff_us diff time in us
 * @param[in] remain_diff_us remain diff time in us
 * @param[in] *data pointer to a data buffer
 * @note      none
 */
static uint8_t a_bpc_data_decode2(bpc_handle_t *handle, uint32_t diff_us, uint32_t remain_diff_us, uint8_t *data)
{
    if (a_check_frame2(diff_us, remain_diff_us, BPC_CHECK_DATA_0) == 0)                    /* check diff */
    {
        *data = 0;                                                                         /* set data 0 */
    }
    else
    {
        if (a_check_frame2(diff_us, remain_diff_us, BPC_CHECK_DATA_1) == 0)                /* check start diff */
        {
            *data = 1;                                                                     /* set data 1 */
        }
        else
        {
            if (a_check_frame2(diff_us, remain_diff_us, BPC_CHECK_DATA_2) == 0)            /* check start diff */
            {
                *data = 2;                                                                 /* set data 2 */
            }
            else
            {
                if (a_check_frame2(diff_us, remain_diff_us, BPC_CHECK_DATA_3) == 0)        /* check start diff */
                {
                    *data = 3;                                                             /* set data 3 */
                }
                else
                {
                    return 1;                                                              /* return error */
                }
            }
        }
    }
    
    return 0;                                                                              /* success return 0 */
}

/**
 * @brief     bpc decode
 * @param[in] *handle pointer to a bpc handle structure
 * @note      none
 */
static void a_bpc_decode(bpc_handle_t *handle)
{
    if (handle->decode_len - handle->decode_offset >= 38)                                       /* if the min length */
    {
        uint8_t res;
        uint8_t p1;
        uint8_t p2;
        uint8_t p3;
        uint8_t p4;
        uint8_t temp0;
        uint8_t temp1;
        uint8_t temp2;
        uint8_t count;
        uint16_t i;
        uint16_t len;
        uint16_t ind;
        uint16_t parity;
        bpc_t data = {0};
        
        len = handle->decode_len - 1;                                                           /* len - 1 */
        for (i = 0; i < len; i++)                                                               /* diff all time */
        {
            int64_t diff;
            
            diff = (int64_t)((int64_t)handle->decode[i + 1].t.s -
                   (int64_t)handle->decode[i].t.s) * 1000000 + 
                   (int64_t)((int64_t)handle->decode[i + 1].t.us -
                   (int64_t)handle->decode[i].t.us);                                            /* diff time */
            handle->decode[i].diff_us = (uint32_t)diff;                                         /* save the time diff */
        }
        
        ind = handle->decode_offset;                                                            /* set start index */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &p1);                /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        if (p1 == 0)                                                                            /* 19 second */
        {
            data.second = 19;                                                                   /* set 19s */
        }
        else if (p1 == 1)                                                                       /* 39 second */
        {
            data.second = 39;                                                                   /* set 39s */
        }
        else if (p1 == 2)                                                                       /* 59 second */
        {
            data.second = 59;                                                                   /* set 59s */
        }
        else                                                                                    /* invalid */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &p2);                /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp0);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp1);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        data.hour = (temp0 << 2) | temp1;                                                       /* set hour */
        
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp0);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp1);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp2);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        data.minute = ((uint8_t)temp0 << 4) | ((uint8_t)temp1 << 2) | temp2;                    /* set minute */
        
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp0);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp1);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        data.week = (temp0 << 2) | temp1;                                                       /* set week */
        
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &p3);                /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        
        count = 0;                                                                              /* init 0 */
        parity = p1;                                                                            /* set p1 */
        while (parity != 0)                                                                     /* check 0 */
        {
            count += parity & 0x1;                                                              /* add 1 */
            parity >>= 1;                                                                       /* right shift */
        }
        parity = p2;                                                                            /* set p2 */
        while (parity != 0)                                                                     /* check 0 */
        {
            count += parity & 0x1;                                                              /* add 1 */
            parity >>= 1;                                                                       /* right shift */
        }
        parity = data.hour;                                                                     /* set hour */
        while (parity != 0)                                                                     /* check 0 */
        {
            count += parity & 0x1;                                                              /* add 1 */
            parity >>= 1;                                                                       /* right shift */
        }
        parity = data.minute;                                                                   /* set minute */
        while (parity != 0)                                                                     /* check 0 */
        {
            count += parity & 0x1;                                                              /* add 1 */
            parity >>= 1;                                                                       /* right shift */
        }
        parity = data.week;                                                                     /* set week */
        while (parity != 0)                                                                     /* check 0 */
        {
            count += parity & 0x1;                                                              /* add 1 */
            parity >>= 1;                                                                       /* right shift */
        }
        
        if (p3 == 0)                                                                            /* even, am */
        {
            if ((count % 2) != 0)                                                               /* check even */
            {
                data.status = BPC_STATUS_PARITY_ERR;                                            /* frame invalid */
                data.year = 0;                                                                  /* set 0 */
                data.month = 0;                                                                 /* set 0 */
                data.day = 0;                                                                   /* set 0 */
                data.week = 0;                                                                  /* set 0 */
                data.hour = 0;                                                                  /* set 0 */
                data.minute = 0;                                                                /* set 0 */
                data.second = 0;                                                                /* set 0 */
                if (handle->receive_callback != NULL)                                           /* not null */
                {
                    handle->receive_callback(&data);                                            /* run the callback */
                }
                
                handle->decode_len = 0;                                                         /* clear the buffer */
                handle->decode_offset = 0;                                                      /* init 0 */
                handle->decode_valid = 0;                                                       /* set invalid */
                handle->trace_valid = 0;                                                        /* set invalid */
                
                return;                                                                         /* return */
            }
        }
        else if (p3 == 1)                                                                       /* odd, am */
        {
            if ((count % 2) == 0)                                                               /* check even */
            {
                data.status = BPC_STATUS_PARITY_ERR;                                            /* frame invalid */
                data.year = 0;                                                                  /* set 0 */
                data.month = 0;                                                                 /* set 0 */
                data.day = 0;                                                                   /* set 0 */
                data.week = 0;                                                                  /* set 0 */
                data.hour = 0;                                                                  /* set 0 */
                data.minute = 0;                                                                /* set 0 */
                data.second = 0;                                                                /* set 0 */
                if (handle->receive_callback != NULL)                                           /* not null */
                {
                    handle->receive_callback(&data);                                            /* run the callback */
                }
                
                handle->decode_len = 0;                                                         /* clear the buffer */
                handle->decode_offset = 0;                                                      /* init 0 */
                handle->decode_valid = 0;                                                       /* set invalid */
                handle->trace_valid = 0;                                                        /* set invalid */
                
                return;                                                                         /* return */
            }
        }
        else if (p3 == 2)                                                                       /* even, pm */
        {
            if ((count % 2) != 0)                                                               /* check even */
            {
                data.status = BPC_STATUS_PARITY_ERR;                                            /* frame invalid */
                data.year = 0;                                                                  /* set 0 */
                data.month = 0;                                                                 /* set 0 */
                data.day = 0;                                                                   /* set 0 */
                data.week = 0;                                                                  /* set 0 */
                data.hour = 0;                                                                  /* set 0 */
                data.minute = 0;                                                                /* set 0 */
                data.second = 0;                                                                /* set 0 */
                if (handle->receive_callback != NULL)                                           /* not null */
                {
                    handle->receive_callback(&data);                                            /* run the callback */
                }
                
                handle->decode_len = 0;                                                         /* clear the buffer */
                handle->decode_offset = 0;                                                      /* init 0 */
                handle->decode_valid = 0;                                                       /* set invalid */
                handle->trace_valid = 0;                                                        /* set invalid */
                
                return;                                                                         /* return */
            }
            data.hour += 12;                                                                    /* add 12h */
        }
        else                                                                                    /* odd, pm */
        {
            if ((count % 2) == 0)                                                               /* check even */
            {
                data.status = BPC_STATUS_PARITY_ERR;                                            /* frame invalid */
                data.year = 0;                                                                  /* set 0 */
                data.month = 0;                                                                 /* set 0 */
                data.day = 0;                                                                   /* set 0 */
                data.week = 0;                                                                  /* set 0 */
                data.hour = 0;                                                                  /* set 0 */
                data.minute = 0;                                                                /* set 0 */
                data.second = 0;                                                                /* set 0 */
                if (handle->receive_callback != NULL)                                           /* not null */
                {
                    handle->receive_callback(&data);                                            /* run the callback */
                }
                
                handle->decode_len = 0;                                                         /* clear the buffer */
                handle->decode_offset = 0;                                                      /* init 0 */
                handle->decode_valid = 0;                                                       /* set invalid */
                handle->trace_valid = 0;                                                        /* set invalid */
                
                return;                                                                         /* return */
            }
            data.hour += 12;                                                                    /* add 12h */
        }
        
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp0);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp1);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp2);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        data.day = ((uint8_t)temp0 << 4) | ((uint8_t)temp1 << 2) | temp2;                       /* set day */
        
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp0);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp1);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        data.month = (temp0 << 2) | temp1;                                                      /* set month */
        
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp0);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp1);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        res = a_bpc_data_decode2(handle, (uint32_t)handle->decode[ind].diff_us,
                                (uint32_t)handle->decode[ind + 1].diff_us, &temp2);             /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        ind += 2;                                                                               /* index + 2 */
        data.year = ((uint8_t)temp0 << 4) | ((uint8_t)temp1 << 2) | temp2;                      /* set day */
        
        res = a_bpc_data_decode(handle, (uint32_t)handle->decode[ind].diff_us, &p4);            /* get data */
        if (res != 0)                                                                           /* check the result */
        {
            data.status = BPC_STATUS_FRAME_INVALID;                                             /* frame invalid */
            data.year = 0;                                                                      /* set 0 */
            data.month = 0;                                                                     /* set 0 */
            data.day = 0;                                                                       /* set 0 */
            data.week = 0;                                                                      /* set 0 */
            data.hour = 0;                                                                      /* set 0 */
            data.minute = 0;                                                                    /* set 0 */
            data.second = 0;                                                                    /* set 0 */
            if (handle->receive_callback != NULL)                                               /* not null */
            {
                handle->receive_callback(&data);                                                /* run the callback */
            }
            
            handle->decode_len = 0;                                                             /* clear the buffer */
            handle->decode_offset = 0;                                                          /* init 0 */
            handle->decode_valid = 0;                                                           /* set invalid */
            handle->trace_valid = 0;                                                            /* set invalid */
            
            return;                                                                             /* return */
        }
        
        count = 0;                                                                              /* init 0 */
        parity = data.day;                                                                      /* set day */
        while (parity != 0)                                                                     /* check 0 */
        {
            count += parity & 0x1;                                                              /* add 1 */
            parity >>= 1;                                                                       /* right shift */
        }
        parity = data.month;                                                                    /* set month */
        while (parity != 0)                                                                     /* check 0 */
        {
            count += parity & 0x1;                                                              /* add 1 */
            parity >>= 1;                                                                       /* right shift */
        }
        parity = data.year;                                                                     /* set year */
        while (parity != 0)                                                                     /* check 0 */
        {
            count += parity & 0x1;                                                              /* add 1 */
            parity >>= 1;                                                                       /* right shift */
        }
        if (p4 == 0)                                                                            /* even */
        {
            if ((count % 2) != 0)                                                               /* check even */
            {
                data.status = BPC_STATUS_PARITY_ERR;                                            /* frame invalid */
                data.year = 0;                                                                  /* set 0 */
                data.month = 0;                                                                 /* set 0 */
                data.day = 0;                                                                   /* set 0 */
                data.week = 0;                                                                  /* set 0 */
                data.hour = 0;                                                                  /* set 0 */
                data.minute = 0;                                                                /* set 0 */
                data.second = 0;                                                                /* set 0 */
                if (handle->receive_callback != NULL)                                           /* not null */
                {
                    handle->receive_callback(&data);                                            /* run the callback */
                }
                
                handle->decode_len = 0;                                                         /* clear the buffer */
                handle->decode_offset = 0;                                                      /* init 0 */
                handle->decode_valid = 0;                                                       /* set invalid */
                handle->trace_valid = 0;                                                        /* set invalid */
                
                return;                                                                         /* return */
            }
            data.year += 2000;                                                                  /* add 2000 */
        }
        else if (p4 == 1)                                                                       /* odd */
        {
            if ((count % 2) == 0)                                                               /* check even */
            {
                data.status = BPC_STATUS_PARITY_ERR;                                            /* frame invalid */
                data.year = 0;                                                                  /* set 0 */
                data.month = 0;                                                                 /* set 0 */
                data.day = 0;                                                                   /* set 0 */
                data.week = 0;                                                                  /* set 0 */
                data.hour = 0;                                                                  /* set 0 */
                data.minute = 0;                                                                /* set 0 */
                data.second = 0;                                                                /* set 0 */
                if (handle->receive_callback != NULL)                                           /* not null */
                {
                    handle->receive_callback(&data);                                            /* run the callback */
                }
                
                handle->decode_len = 0;                                                         /* clear the buffer */
                handle->decode_offset = 0;                                                      /* init 0 */
                handle->decode_valid = 0;                                                       /* set invalid */
                handle->trace_valid = 0;                                                        /* set invalid */
                
                return;                                                                         /* return */
            }
            data.year += 2000;                                                                  /* add 2000 */
        }
        else if (p4 == 2)                                                                       /* even, year add */
        {
            if ((count % 2) != 0)                                                               /* check even */
            {
                data.status = BPC_STATUS_PARITY_ERR;                                            /* frame invalid */
                data.year = 0;                                                                  /* set 0 */
                data.month = 0;                                                                 /* set 0 */
                data.day = 0;                                                                   /* set 0 */
                data.week = 0;                                                                  /* set 0 */
                data.hour = 0;                                                                  /* set 0 */
                data.minute = 0;                                                                /* set 0 */
                data.second = 0;                                                                /* set 0 */
                if (handle->receive_callback != NULL)                                           /* not null */
                {
                    handle->receive_callback(&data);                                            /* run the callback */
                }
                
                handle->decode_len = 0;                                                         /* clear the buffer */
                handle->decode_offset = 0;                                                      /* init 0 */
                handle->decode_valid = 0;                                                       /* set invalid */
                handle->trace_valid = 0;                                                        /* set invalid */
                
                return;                                                                         /* return */
            }
            data.year = ((uint16_t)1 << 6);                                                     /* add year */
            data.year += 2000;                                                                  /* add 2000 */
        }
        else                                                                                    /* odd, year add */
        {
            if ((count % 2) == 0)                                                               /* check even */
            {
                data.status = BPC_STATUS_PARITY_ERR;                                            /* frame invalid */
                data.year = 0;                                                                  /* set 0 */
                data.month = 0;                                                                 /* set 0 */
                data.day = 0;                                                                   /* set 0 */
                data.week = 0;                                                                  /* set 0 */
                data.hour = 0;                                                                  /* set 0 */
                data.minute = 0;                                                                /* set 0 */
                data.second = 0;                                                                /* set 0 */
                if (handle->receive_callback != NULL)                                           /* not null */
                {
                    handle->receive_callback(&data);                                            /* run the callback */
                }
                
                handle->decode_len = 0;                                                         /* clear the buffer */
                handle->decode_offset = 0;                                                      /* init 0 */
                handle->decode_valid = 0;                                                       /* set invalid */
                handle->trace_valid = 0;                                                        /* set invalid */
                
                return;                                                                         /* return */
            }
            data.year = ((uint16_t)1 << 6);                                                     /* add year */
            data.year += 2000;                                                                  /* add 2000 */
        }
        
        if (data.week == 7)                                                                     /* sunday is 7 in old version */
        {
            data.week = 0;                                                                      /* set 0 */
        }
        data.status = BPC_STATUS_OK;                                                            /* set ok */
        if (handle->receive_callback != NULL)                                                   /* not null */
        {
            handle->receive_callback(&data);                                                    /* run the callback */
        }

        handle->decode_len = 0;                                                                 /* clear the buffer */
        handle->decode_offset = 0;                                                              /* init 0 */
        handle->decode_valid = 0;                                                               /* set invalid */
        handle->trace_valid = 1;                                                                /* set valid */
    }
}

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
uint8_t bpc_irq_handler(bpc_handle_t *handle)
{
    uint8_t res;
    int64_t diff;
    bpc_time_t t;
    
    if (handle == NULL)                                              /* check handle */
    {
        return 2;                                                    /* return error */
    }
    if (handle->inited != 1)                                         /* check handle initialization */
    {
        return 3;                                                    /* return error */
    }
    
    res = handle->timestamp_read(&t);                                /* timestamp read */
    if (res != 0)                                                    /* check result */
    {
        handle->debug_print("bpc: timestamp read failed.\n");        /* timestamp read failed */
        
        return 1;                                                    /* return error */
    }
    diff = (int64_t)((int64_t)t.s - 
           (int64_t)handle->last_time.s) * 1000000 + 
           (int64_t)((int64_t)t.us - 
           (int64_t)handle->last_time.us);                           /* now - last time */
    if (diff - (int64_t)3000000L >= 0)                               /* if over 3s, force reset */
    {
        handle->decode_len = 0;                                      /* reset the decode */
        handle->decode_offset = 0;                                   /* set 0 */
        handle->decode_valid = 0;                                    /* set invalid */
        handle->trace_valid = 0;                                     /* set invalid */
    }
    if (handle->decode_len >= 76)                                    /* check the max length */
    {
        handle->decode_len = 0;                                      /* reset the decode */
        handle->decode_offset = 0;                                   /* set 0 */
        handle->decode_valid = 0;                                    /* set invalid */
        handle->trace_valid = 0;                                     /* set invalid */
    }
    handle->decode[handle->decode_len].t.s = t.s;                    /* save s */
    handle->decode[handle->decode_len].t.us = t.us;                  /* save us */
    handle->decode_len++;                                            /* length++ */
    if (handle->trace_valid != 0)                                    /* check trace valid */
    {
        a_bpc_trace_decode(handle);                                  /* try to trace decode */
    }
    if (handle->decode_len >= 38)                                    /* check the end length */
    {
        if (handle->decode_valid != 0)                               /* check decode valid */
        {
            a_bpc_decode(handle);                                    /* try to decode */
        }
        else
        {
            a_bpc_start_decode(handle);                              /* try to decode starter */
        }
    }
    handle->last_time.s = t.s;                                       /* save last time */
    handle->last_time.us = t.us;                                     /* save last time */
    
    return 0;                                                        /* success return 0 */
}

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
uint8_t bpc_init(bpc_handle_t *handle)
{
    uint8_t res;
    bpc_time_t t;
    
    if (handle == NULL)                                                 /* check handle */
    {
        return 2;                                                       /* return error */
    }
    if (handle->debug_print == NULL)                                    /* check debug_print */
    {
        return 3;                                                       /* return error */
    }
    if (handle->timestamp_read == NULL)                                 /* check timestamp_read */
    {
        handle->debug_print("bpc: timestamp_read is null.\n");          /* timestamp_read is null */
        
        return 3;                                                       /* return error */
    }
    if (handle->delay_ms == NULL)                                       /* check delay_ms */
    {
        handle->debug_print("bpc: delay_ms is null.\n");                /* delay_ms is null */
        
        return 3;                                                       /* return error */
    }
    if (handle->receive_callback == NULL)                               /* check receive_callback */
    {
        handle->debug_print("bpc: receive_callback is null.\n");        /* receive_callback is null */
        
        return 3;                                                       /* return error */
    }
    
    res = handle->timestamp_read(&t);                                   /* timestamp read */
    if (res != 0)                                                       /* check result */
    {
        handle->debug_print("bpc: timestamp read failed.\n");           /* timestamp read failed */
        
        return 1;                                                       /* return error */
    }
    handle->last_time.s = t.s;                                          /* save last time */
    handle->last_time.us = t.us;                                        /* save last time */
    handle->decode_len = 0;                                             /* init 0 */
    handle->decode_offset = 0;                                          /* init 0 */
    handle->decode_valid = 0;                                           /* set invalid */
    handle->trace_valid = 0;                                            /* set invalid */
    handle->inited = 1;                                                 /* flag inited */
    
    return 0;                                                           /* success return 0 */
}

/**
 * @brief     close the chip
 * @param[in] *handle pointer to a bpc handle structure
 * @return    status code
 *            - 0 success
 *            - 2 handle is NULL
 *            - 3 handle is not initialized
 * @note      none
 */
uint8_t bpc_deinit(bpc_handle_t *handle)
{
    if (handle == NULL)             /* check handle */
    {
        return 2;                   /* return error */
    }
    if (handle->inited != 1)        /* check handle initialization */
    {
        return 3;                   /* return error */
    }
    
    handle->inited = 0;             /* flag close */
    
    return 0;                       /* success return 0 */
}

/**
 * @brief      get chip's information
 * @param[out] *info pointer to a bpc info structure
 * @return     status code
 *             - 0 success
 *             - 2 handle is NULL
 * @note       none
 */
uint8_t bpc_info(bpc_info_t *info)
{
    if (info == NULL)                                               /* check handle */
    {
        return 2;                                                   /* return error */
    }
    
    memset(info, 0, sizeof(bpc_info_t));                            /* initialize bpc info structure */
    strncpy(info->chip_name, CHIP_NAME, 32);                        /* copy chip name */
    strncpy(info->manufacturer_name, MANUFACTURER_NAME, 32);        /* copy manufacturer name */
    strncpy(info->interface, "GPIO", 8);                            /* copy interface name */
    info->supply_voltage_min_v = SUPPLY_VOLTAGE_MIN;                /* set minimal supply voltage */
    info->supply_voltage_max_v = SUPPLY_VOLTAGE_MAX;                /* set maximum supply voltage */
    info->max_current_ma = MAX_CURRENT;                             /* set maximum current */
    info->temperature_max = TEMPERATURE_MAX;                        /* set minimal temperature */
    info->temperature_min = TEMPERATURE_MIN;                        /* set maximum temperature */
    info->driver_version = DRIVER_VERSION;                          /* set driver version */
    
    return 0;                                                       /* success return 0 */
}

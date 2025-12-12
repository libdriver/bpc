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
 * @file      main.c
 * @brief     main source file
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
#include "driver_bpc_receive_test.h"
#include "shell.h"
#include "clock.h"
#include "delay.h"
#include "gpio.h"
#include "uart.h"
#include "getopt.h"
#include <stdlib.h>

/**
 * @brief global var definition
 */
uint8_t g_buf[256];                                   /**< uart buffer */
volatile uint16_t g_len;                              /**< uart buffer length */
volatile uint8_t g_flag;                              /**< interrupt flag */
uint8_t (*g_gpio_irq)(void) = NULL;                   /**< gpio irq */
extern uint8_t bpc_interface_timer_init(void);        /**< timer init function */

/**
 * @brief exti 0 irq
 * @note  none
 */
void EXTI0_IRQHandler(void)
{
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_0);
}

/**
 * @brief     gpio exti callback
 * @param[in] pin gpio pin
 * @note      none
 */
void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
    if (pin == GPIO_PIN_0)
    {
        /* run the gpio riq */
        if (g_gpio_irq != NULL)
        {
            g_gpio_irq();
        }
    }
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
            time_t timestamp;
            
            bpc_interface_debug_print("bpc: irq ok.\n");
            bpc_interface_debug_print("bpc: time is %04d-%02d-%02d %02d:%02d:%02d %s.\n", 
                                      data->year, data->month, data->day,
                                      data->hour, data->minute, data->second,
                                      week[data->week > 7 ? 7 : data->week]);
            (void)bpc_basic_convert_timestamp(data, &timestamp);
            bpc_interface_debug_print("bpc: timestamp is %d.\n", timestamp);
            g_flag = 1;
            
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
 * @brief     bpc full function
 * @param[in] argc arg numbers
 * @param[in] **argv arg address
 * @return    status code
 *            - 0 success
 *            - 1 run failed
 *            - 5 param is invalid
 * @note      none
 */
uint8_t bpc(uint8_t argc, char **argv)
{
    int c;
    int longindex = 0;
    char short_options[] = "hipe:t:";
    struct option long_options[] =
    {
        {"help", no_argument, NULL, 'h'},
        {"information", no_argument, NULL, 'i'},
        {"port", no_argument, NULL, 'p'},
        {"example", required_argument, NULL, 'e'},
        {"test", required_argument, NULL, 't'},
        {"times", required_argument, NULL, 1},
        {NULL, 0, NULL, 0},
    };
    char type[33] = "unknown";
    uint32_t times = 3;
    
    /* if no params */
    if (argc == 1)
    {
        /* goto the help */
        goto help;
    }

    /* init 0 */
    optind = 0;

    /* parse */
    do
    {
        /* parse the args */
        c = getopt_long(argc, argv, short_options, long_options, &longindex);

        /* judge the result */
        switch (c)
        {
            /* help */
            case 'h' :
            {
                /* set the type */
                memset(type, 0, sizeof(char) * 33);
                snprintf(type, 32, "h");

                break;
            }

            /* information */
            case 'i' :
            {
                /* set the type */
                memset(type, 0, sizeof(char) * 33);
                snprintf(type, 32, "i");

                break;
            }

            /* port */
            case 'p' :
            {
                /* set the type */
                memset(type, 0, sizeof(char) * 33);
                snprintf(type, 32, "p");

                break;
            }

            /* example */
            case 'e' :
            {
                /* set the type */
                memset(type, 0, sizeof(char) * 33);
                snprintf(type, 32, "e_%s", optarg);

                break;
            }

            /* test */
            case 't' :
            {
                /* set the type */
                memset(type, 0, sizeof(char) * 33);
                snprintf(type, 32, "t_%s", optarg);

                break;
            }

            /* running times */
            case 1 :
            {
                /* set the times */
                times = atol(optarg);

                break;
            }

            /* the end */
            case -1 :
            {
                break;
            }

            /* others */
            default :
            {
                return 5;
            }
        }
    } while (c != -1);

    /* run the function */
    if (strcmp("t_read", type) == 0)
    {
        uint8_t res;
        
        /* gpio init */
        res = gpio_interrupt_init();
        if (res != 0)
        {
            return 1;
        }
        
        /* set the irq */
        g_gpio_irq = bpc_receive_test_irq_handler;
        
        /* run the receive test */
        res = bpc_receive_test(times);
        if (res != 0)
        {
            (void)gpio_interrupt_deinit();
            g_gpio_irq = NULL;
        }
        
        /* gpio deinit */
        (void)gpio_interrupt_deinit();
        g_gpio_irq = NULL;
        
        return 0;
    }
    else if (strcmp("e_read", type) == 0)
    {
        uint8_t res;
        uint32_t i;
        uint16_t timeout;
        
        /* gpio init */
        res = gpio_interrupt_init();
        if (res != 0)
        {
            return 1;
        }
        
        /* set the irq */
        g_gpio_irq = bpc_basic_irq_handler;
        
        /* basic init */
        res = bpc_basic_init(a_receive_callback);
        if (res != 0)
        {
            (void)gpio_interrupt_deinit();
            g_gpio_irq = NULL;
        }
        
        /* set time zone */
        res = bpc_basic_set_timestamp_time_zone(8);
        if (res != 0)
        {
            (void)gpio_interrupt_deinit();
            g_gpio_irq = NULL;
        }
        
        /* loop */
        for (i = 0; i < times; i++)
        {
            /* output */
            bpc_interface_debug_print("%d/%d\n", i + 1, times);
            
            /* 60s timeout */
            timeout = 6000;
            
            /* init 0 */
            g_flag = 0;
            
            /* check timeout */
            while (timeout != 0)
            {
                /* check the flag */
                if (g_flag != 0)
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
                (void)bpc_basic_deinit();
                (void)gpio_interrupt_deinit();
                g_gpio_irq = NULL;
                
                return 1;
            }
        }
        
        /* basic deinit */
        (void)bpc_basic_deinit();
        
        /* gpio deinit */
        (void)gpio_interrupt_deinit();
        g_gpio_irq = NULL;
        
        return 0;
    }
    else if (strcmp("h", type) == 0)
    {
        help:
        bpc_interface_debug_print("Usage:\n");
        bpc_interface_debug_print("  bpc (-i | --information)\n");
        bpc_interface_debug_print("  bpc (-h | --help)\n");
        bpc_interface_debug_print("  bpc (-p | --port)\n");
        bpc_interface_debug_print("  bpc (-t read | --test=read) [--times=<num>]\n");
        bpc_interface_debug_print("  bpc (-e read | --example=read) [--times=<num>]\n");
        bpc_interface_debug_print("\n");
        bpc_interface_debug_print("Options:\n");
        bpc_interface_debug_print("  -e <read>, --example=<read>    Run the driver example.\n");
        bpc_interface_debug_print("  -h, --help                     Show the help.\n");
        bpc_interface_debug_print("  -i, --information              Show the chip information.\n");
        bpc_interface_debug_print("  -p, --port                     Display the pin connections of the current board.\n");
        bpc_interface_debug_print("  -t <read>, --test=<read>       Run the driver test.\n");
        bpc_interface_debug_print("      --times=<num>              Set the running times.([default: 3])\n");

        return 0;
    }
    else if (strcmp("i", type) == 0)
    {
        bpc_info_t info;

        /* print bpc info */
        bpc_info(&info);
        bpc_interface_debug_print("bpc: chip is %s.\n", info.chip_name);
        bpc_interface_debug_print("bpc: manufacturer is %s.\n", info.manufacturer_name);
        bpc_interface_debug_print("bpc: interface is %s.\n", info.interface);
        bpc_interface_debug_print("bpc: driver version is %d.%d.\n", info.driver_version / 1000, (info.driver_version % 1000) / 100);
        bpc_interface_debug_print("bpc: min supply voltage is %0.1fV.\n", info.supply_voltage_min_v);
        bpc_interface_debug_print("bpc: max supply voltage is %0.1fV.\n", info.supply_voltage_max_v);
        bpc_interface_debug_print("bpc: max current is %0.2fmA.\n", info.max_current_ma);
        bpc_interface_debug_print("bpc: max temperature is %0.1fC.\n", info.temperature_max);
        bpc_interface_debug_print("bpc: min temperature is %0.1fC.\n", info.temperature_min);

        return 0;
    }
    else if (strcmp("p", type) == 0)
    {
        /* print pin connection */
        bpc_interface_debug_print("bpc: INT connected to GPIOB PIN0.\n");

        return 0;
    }
    else
    {
        return 5;
    }
}

/**
 * @brief main function
 * @note  none
 */
int main(void)
{
    uint8_t res;

    /* stm32f407 clock init and hal init */
    clock_init();

    /* delay init */
    delay_init();

    /* uart init */
    uart_init(115200);
    
    /* timer init */
    bpc_interface_timer_init();
    
    /* shell init && register bpc function */
    shell_init();
    shell_register("bpc", bpc);
    uart_print("bpc: welcome to libdriver bpc.\n");
    
    while (1)
    {
        /* read uart */
        g_len = uart_read(g_buf, 256);
        if (g_len != 0)
        {
            /* run shell */
            res = shell_parse((char *)g_buf, g_len);
            if (res == 0)
            {
                /* run success */
            }
            else if (res == 1)
            {
                uart_print("bpc: run failed.\n");
            }
            else if (res == 2)
            {
                uart_print("bpc: unknown command.\n");
            }
            else if (res == 3)
            {
                uart_print("bpc: length is too long.\n");
            }
            else if (res == 4)
            {
                uart_print("bpc: pretreat failed.\n");
            }
            else if (res == 5)
            {
                uart_print("bpc: param is invalid.\n");
            }
            else
            {
                uart_print("bpc: unknown status code.\n");
            }
            uart_flush();
        }
        delay_ms(100);
    }
}

[English](/README.md) | [ 简体中文](/README_zh-Hans.md) | [繁體中文](/README_zh-Hant.md) | [日本語](/README_ja.md) | [Deutsch](/README_de.md) | [한국어](/README_ko.md)

<div align=center>
<img src="/doc/image/logo.svg" width="400" height="150"/>
</div>

## LibDriver BPC

[![MISRA](https://img.shields.io/badge/misra-compliant-brightgreen.svg)](/misra/README.md) [![API](https://img.shields.io/badge/api-reference-blue.svg)](https://www.libdriver.com/docs/bpc/index.html) [![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](/LICENSE)

BPC 解码是指通过电子设备接收并解析由中国科学院国家授时中心发布的低频电波授时信号的过程。该信号发射频率为 68.5kHz，旨在为电波表、时钟及各类计时设备提供高精度的标准北京时间自动对时。

LibDriver BPC是LibDriver推出的BPC全功能驱动，该驱动提供BPC解码功能并且它符合MISRA标准。

### 目录

  - [说明](#说明)
  - [安装](#安装)
  - [使用](#使用)
    - [example basic](#example-basic)
  - [文档](#文档)
  - [贡献](#贡献)
  - [版权](#版权)
  - [联系我们](#联系我们)

### 说明

/src目录包含了LibDriver BPC的源文件。

/interface目录包含了LibDriver BPC与平台无关的GPIO总线模板。

/test目录包含了LibDriver BPC驱动测试程序，该程序可以简单的测试芯片必要功能。

/example目录包含了LibDriver BPC编程范例。

/doc目录包含了LibDriver BPC离线文档。

/datasheet目录包含了BPC数据手册。

/project目录包含了常用Linux与单片机开发板的工程样例。所有工程均采用shell脚本作为调试方法，详细内容可参考每个工程里面的README.md。

/misra目录包含了LibDriver MISRA代码扫描结果。

### 安装

参考/interface目录下与平台无关的GPIO总线模板，完成指定平台的GPIO总线驱动。

将/src目录，您使用平台的接口驱动和您开发的驱动加入工程，如果您想要使用默认的范例驱动，可以将/example目录加入您的工程。

### 使用

您可以参考/example目录下的编程范例完成适合您的驱动，如果您想要使用默认的编程范例，以下是它们的使用方法。

#### example basic

```C
#include "driver_bpc_basic.h"

volatile uint8_t g_flag;
uint8_t (*g_gpio_irq)(void) = NULL;
uint8_t res;
uint32_t i;
uint16_t timeout;

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

...
    
/* loop */
for (i = 0; i < 3; i++)
{
    /* output */
    bpc_interface_debug_print("%d/%d\n", i + 1, 3);

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
    
    ...
}

...
    
/* basic deinit */
(void)bpc_basic_deinit();

/* gpio deinit */
(void)gpio_interrupt_deinit();
g_gpio_irq = NULL;

return 0;
```

### 文档

在线文档: [https://www.libdriver.com/docs/bpc/index.html](https://www.libdriver.com/docs/bpc/index.html)。

离线文档: /doc/html/index.html。

### 贡献

请参考CONTRIBUTING.md。

### 版权

版权 (c) 2015 - 现在 LibDriver 版权所有

MIT 许可证（MIT）

特此免费授予任何获得本软件副本和相关文档文件（下称“软件”）的人不受限制地处置该软件的权利，包括不受限制地使用、复制、修改、合并、发布、分发、转授许可和/或出售该软件副本，以及再授权被配发了本软件的人如上的权利，须在下列条件下：

上述版权声明和本许可声明应包含在该软件的所有副本或实质成分中。

本软件是“如此”提供的，没有任何形式的明示或暗示的保证，包括但不限于对适销性、特定用途的适用性和不侵权的保证。在任何情况下，作者或版权持有人都不对任何索赔、损害或其他责任负责，无论这些追责来自合同、侵权或其它行为中，还是产生于、源于或有关于本软件以及本软件的使用或其它处置。

### 联系我们

请联系lishifenging@outlook.com。
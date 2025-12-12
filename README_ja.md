[English](/README.md) | [ 简体中文](/README_zh-Hans.md) | [繁體中文](/README_zh-Hant.md) | [日本語](/README_ja.md) | [Deutsch](/README_de.md) | [한국어](/README_ko.md)

<div align=center>
<img src="/doc/image/logo.svg" width="400" height="150"/>
</div>

## LibDriver BPC

[![MISRA](https://img.shields.io/badge/misra-compliant-brightgreen.svg)](/misra/README.md) [![API](https://img.shields.io/badge/api-reference-blue.svg)](https://www.libdriver.com/docs/bpc/index.html) [![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](/LICENSE)

BPCデコードとは、中国科学院国家時刻サービスセンターが発行する低周波電波時刻サービス信号を電子機器で受信・解析するプロセスを指します。信号の伝送周波数は68.5kHzで、電波計器、時計、各種計時装置に高精度の北京標準時自動同期を提供することを目的としています。

LibDriver BPCは、LibDriverがリリースしたBPC用のフル機能ドライバです。BPCデコード機能と追加機能を提供します。LibDriverはMISRAに準拠しています。

### 目次

  - [説明](#説明)
  - [インストール](#インストール)
  - [使用](#使用)
    - [example basic](#example-basic)
  - [ドキュメント](#ドキュメント)
  - [貢献](#貢献)
  - [著作権](#著作権)
  - [連絡して](#連絡して)

### 説明

/ srcディレクトリには、LibDriver BPCのソースファイルが含まれています。

/ interfaceディレクトリには、LibDriver BPC用のプラットフォームに依存しないGPIOバステンプレートが含まれています。

/ testディレクトリには、チップの必要な機能を簡単にテストできるLibDriver BPCドライバーテストプログラムが含まれています。

/ exampleディレクトリには、LibDriver BPCプログラミング例が含まれています。

/ docディレクトリには、LibDriver BPCオフラインドキュメントが含まれています。

/ datasheetディレクトリには、BPCデータシートが含まれています。

/ projectディレクトリには、一般的に使用されるLinuxおよびマイクロコントローラー開発ボードのプロジェクトサンプルが含まれています。 すべてのプロジェクトは、デバッグ方法としてシェルスクリプトを使用しています。詳細については、各プロジェクトのREADME.mdを参照してください。

/ misraはLibDriver misraコードスキャン結果を含む。

### インストール

/ interfaceディレクトリにあるプラットフォームに依存しないGPIOバステンプレートを参照して、指定したプラットフォームのGPIOバスドライバを完成させます。

/src ディレクトリ、プラットフォームのインターフェイス ドライバー、および独自のドライバーをプロジェクトに追加します。デフォルトのサンプル ドライバーを使用する場合は、/example ディレクトリをプロジェクトに追加します。

### 使用

/example ディレクトリ内のサンプルを参照して、独自のドライバーを完成させることができます。 デフォルトのプログラミング例を使用したい場合の使用方法は次のとおりです。

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

### ドキュメント

オンラインドキュメント: [https://www.libdriver.com/docs/bpc/index.html](https://www.libdriver.com/docs/bpc/index.html)。

オフラインドキュメント: /doc/html/index.html。

### 貢献

CONTRIBUTING.mdを参照してください。

### 著作権

著作権（c）2015-今 LibDriver 全著作権所有

MITライセンス（MIT）

このソフトウェアおよび関連するドキュメントファイル（「ソフトウェア」）のコピーを取得した人は、無制限の使用、複製、変更、組み込み、公開、配布、サブライセンスを含む、ソフトウェアを処分する権利を制限なく付与されます。ソフトウェアのライセンスおよび/またはコピーの販売、および上記のようにソフトウェアが配布された人の権利のサブライセンスは、次の条件に従うものとします。

上記の著作権表示およびこの許可通知は、このソフトウェアのすべてのコピーまたは実体に含まれるものとします。

このソフトウェアは「現状有姿」で提供され、商品性、特定目的への適合性、および非侵害の保証を含むがこれらに限定されない、明示または黙示を問わず、いかなる種類の保証もありません。 いかなる場合も、作者または著作権所有者は、契約、不法行為、またはその他の方法で、本ソフトウェアおよび本ソフトウェアの使用またはその他の廃棄に起因または関連して、請求、損害、またはその他の責任を負わないものとします。

### 連絡して

お問い合わせくださいlishifenging@outlook.com。
[English](/README.md) | [ 简体中文](/README_zh-Hans.md) | [繁體中文](/README_zh-Hant.md) | [日本語](/README_ja.md) | [Deutsch](/README_de.md) | [한국어](/README_ko.md)

<div align=center>
<img src="/doc/image/logo.svg" width="400" height="150"/>
</div>

## LibDriver BPC

[![MISRA](https://img.shields.io/badge/misra-compliant-brightgreen.svg)](/misra/README.md) [![API](https://img.shields.io/badge/api-reference-blue.svg)](https://www.libdriver.com/docs/bpc/index.html) [![License](https://img.shields.io/badge/license-MIT-brightgreen.svg)](/LICENSE) 

Die BPC-Dekodierung bezeichnet den Empfang und die Analyse von Niederfrequenz-Funkzeitsignalen des Nationalen Zeitzentrums der Chinesischen Akademie der Wissenschaften mittels elektronischer Geräte. Die Sendefrequenz beträgt 68,5 kHz und dient der hochpräzisen automatischen Synchronisierung von Funkzählern, Uhren und anderen Zeitmessgeräten mit der Pekinger Standardzeit.

LibDriver BPC ist ein umfassender Treiber für BPC, entwickelt von LibDriver. Er bietet BPC-Dekodierung und zusätzliche Funktionen. LibDriver ist MISRA-konform.

### Inhaltsverzeichnis

  - [Anweisung](#Anweisung)
  - [Installieren](#Installieren)
  - [Nutzung](#Nutzung)
    - [example basic](#example-basic)
  - [Dokument](#Dokument)
  - [Beitrag](#Beitrag)
  - [Lizenz](#Lizenz)
  - [Kontaktieren Sie uns](#Kontaktieren-Sie-uns)

### Anweisung

/src enthält LibDriver BPC-Quelldateien.

/interface enthält die plattformunabhängige Vorlage LibDriver BPC GPIO.

/test enthält den Testcode des LibDriver BPC-Treibers und dieser Code kann die erforderliche Funktion des Chips einfach testen.

/example enthält LibDriver BPC-Beispielcode.

/doc enthält das LibDriver BPC-Offlinedokument.

/Datenblatt enthält BPC-Datenblatt.

/project enthält den allgemeinen Beispielcode für Linux- und MCU-Entwicklungsboards. Alle Projekte verwenden das Shell-Skript, um den Treiber zu debuggen, und die detaillierten Anweisungen finden Sie in der README.md jedes Projekts.

/misra enthält die Ergebnisse des LibDriver MISRA Code Scans.

### Installieren

Verweisen Sie auf eine plattformunabhängige GPIO-Schnittstellenvorlage und stellen Sie Ihren Plattform-GPIO-Treiber fertig.

Fügen Sie das Verzeichnis /src, den Schnittstellentreiber für Ihre Plattform und Ihre eigenen Treiber zu Ihrem Projekt hinzu. Wenn Sie die Standardbeispieltreiber verwenden möchten, fügen Sie das Verzeichnis /example zu Ihrem Projekt hinzu.

### Nutzung

Sie können auf die Beispiele im Verzeichnis /example zurückgreifen, um Ihren eigenen Treiber zu vervollständigen. Wenn Sie die Standardprogrammierbeispiele verwenden möchten, erfahren Sie hier, wie Sie diese verwenden.

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

### Dokument

Online-Dokumente: [https://www.libdriver.com/docs/bpc/index.html](https://www.libdriver.com/docs/bpc/index.html).

Offline-Dokumente: /doc/html/index.html.

### Beitrag

Bitte beachten Sie CONTRIBUTING.md.

### Lizenz

Urheberrechte © (c) 2015 - Gegenwart LibDriver Alle Rechte vorbehalten



Die MIT-Lizenz (MIT)



Hiermit wird jeder Person kostenlos die Erlaubnis erteilt, eine Kopie zu erhalten

dieser Software und zugehörigen Dokumentationsdateien (die „Software“) zu behandeln

in der Software ohne Einschränkung, einschließlich, aber nicht beschränkt auf die Rechte

zu verwenden, zu kopieren, zu modifizieren, zusammenzuführen, zu veröffentlichen, zu verteilen, unterzulizenzieren und/oder zu verkaufen

Kopien der Software und Personen, denen die Software gehört, zu gestatten

dazu eingerichtet werden, unter folgenden Bedingungen:



Der obige Urheberrechtshinweis und dieser Genehmigungshinweis müssen in allen enthalten sein

Kopien oder wesentliche Teile der Software.



DIE SOFTWARE WIRD "WIE BESEHEN" BEREITGESTELLT, OHNE JEGLICHE GEWÄHRLEISTUNG, AUSDRÜCKLICH ODER

STILLSCHWEIGEND, EINSCHLIESSLICH, ABER NICHT BESCHRÄNKT AUF DIE GEWÄHRLEISTUNG DER MARKTGÄNGIGKEIT,

EIGNUNG FÜR EINEN BESTIMMTEN ZWECK UND NICHTVERLETZUNG VON RECHTEN DRITTER. IN KEINEM FALL DARF DAS

AUTOREN ODER URHEBERRECHTSINHABER HAFTEN FÜR JEGLICHE ANSPRÜCHE, SCHÄDEN ODER ANDERE

HAFTUNG, OB AUS VERTRAG, DELIKT ODER ANDERWEITIG, ENTSTEHEND AUS,

AUS ODER IM ZUSAMMENHANG MIT DER SOFTWARE ODER DER VERWENDUNG ODER ANDEREN HANDLUNGEN MIT DER

SOFTWARE.

### Kontaktieren Sie uns

Bitte senden Sie eine E-Mail an lishifenging@outlook.com.
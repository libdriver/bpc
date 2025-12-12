### 1. Chip

#### 1.1 Chip Info

Chip Name: STM32F407ZGT6.

Extern Oscillator: 8MHz.

GPIO Pin: INT PB0.

### 2. Development and Debugging

#### 2.1 Integrated Development Environment

LibDriver provides both Keil and IAR integrated development environment projects.

MDK is the Keil ARM project and your Keil version must be 5 or higher.Keil ARM project needs STMicroelectronics STM32F4 Series Device Family Pack and you can download from https://www.keil.com/dd2/stmicroelectronics/stm32f407zgtx.

EW is the IAR ARM project and your IAR version must be 9 or higher.

#### 2.2 Serial Port Parameter

Baud Rate: 115200.

Data Bits : 8.

Stop Bits: 1.

Parity: None.

Flow Control: None.

#### 2.3 Serial Port Assistant

We use '\n' to wrap lines.If your serial port assistant displays exceptions (e.g. the displayed content does not divide lines), please modify the configuration of your serial port assistant or replace one that supports '\n' parsing.

### 3. BPC

#### 3.1 Command Instruction

1. Show bpc chip and driver information.

    ```shell
    bpc (-i | --information)  
    ```

2. Show bpc help.

    ```shell
    bpc (-h | --help)        
    ```

3. Show bpc pin connections of the current board.

    ```shell
    bpc (-p | --port)              
    ```

4. Run bpc read test, num means test times.

    ```shell
    bpc (-t read | --test=read) [--times=<num>]
    ```

5. Run bpc read function, num means read times.

    ```shell
    bpc (-e read | --example=read) [--times=<num>]   
    ```
    
#### 3.2 Command Example

```shell
bpc -i

bpc: chip is China BPC.
bpc: manufacturer is China.
bpc: interface is GPIO.
bpc: driver version is 1.0.
bpc: min supply voltage is 2.7V.
bpc: max supply voltage is 5.5V.
bpc: max current is 1.50mA.
bpc: max temperature is 125.0C.
bpc: min temperature is -40.0C.
```

```shell
bpc -p

bpc: INT connected to GPIOB PIN0.
```

```shell
bpc -t read --times=3

bpc: chip is China BPC.
bpc: manufacturer is China.
bpc: interface is GPIO.
bpc: driver version is 1.0.
bpc: min supply voltage is 2.7V.
bpc: max supply voltage is 5.5V.
bpc: max current is 1.50mA.
bpc: max temperature is 125.0C.
bpc: min temperature is -40.0C.
bpc: start receive test.
bpc: irq ok.
bpc: time is 2025-12-12 20:52:59 Friday.
bpc: irq ok.
bpc: time is 2025-12-12 20:53:19 Friday.
bpc: irq ok.
bpc: time is 2025-12-12 20:53:39 Friday.
bpc: finish receive test.
```

```shell
bpc -e read --times=3

1/3
bpc: irq ok.
bpc: time is 2025-12-12 20:55:39 Friday.
2/3
bpc: irq ok.
bpc: time is 2025-12-12 20:55:59 Friday.
3/3
bpc: irq ok.
bpc: time is 2025-12-12 20:56:19 Friday.
```

```shell
bpc -h

Usage:
  bpc (-i | --information)
  bpc (-h | --help)
  bpc (-p | --port)
  bpc (-t read | --test=read) [--times=<num>]
  bpc (-e read | --example=read) [--times=<num>]

Options:
  -e <read>, --example=<read>    Run the driver example.
  -h, --help                     Show the help.
  -i, --information              Show the chip information.
  -p, --port                     Display the pin connections of the current board.
  -t <read>, --test=<read>       Run the driver test.
      --times=<num>              Set the running times.([default: 3])
```

### 1. Board

#### 1.1 Board Info

Board Name: Raspberry Pi 4B.

GPIO Pin: INT GPIO17.

### 2. Install

#### 2.1 Dependencies

Install the necessary dependencies.

```shell
sudo apt-get install libgpiod-dev pkg-config cmake -y
```

#### 2.2 Makefile

Build the project.

```shell
make
```

Install the project and this is optional.

```shell
sudo make install
```

Uninstall the project and this is optional.

```shell
sudo make uninstall
```

#### 2.3 CMake

Build the project.

```shell
mkdir build && cd build 
cmake .. 
make
```

Install the project and this is optional.

```shell
sudo make install
```

Uninstall the project and this is optional.

```shell
sudo make uninstall
```

Test the project and this is optional.

```shell
make test
```

Find the compiled library in CMake. 

```cmake
find_package(bpc REQUIRED)
```

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
./bpc -i

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
./bpc -p

bpc: INT connected to GPIO17(BCM).
```

```shell
./bpc -t read --times=3

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
bpc: time is 2025-12-12 21:42:39 Friday.
bpc: irq ok.
bpc: time is 2025-12-12 21:42:59 Friday.
bpc: irq ok.
bpc: time is 2025-12-12 21:43:19 Friday.
bpc: finish receive test.
```

```shell
./bpc -e read --times=3

1/3
bpc: irq ok.
bpc: time is 2025-12-12 21:45:39 Friday.
2/3
bpc: irq ok.
bpc: time is 2025-12-12 21:45:59 Friday.
3/3
bpc: irq frame invalid.
bpc: irq ok.
bpc: time is 2025-12-12 21:46:39 Friday.
```

```shell
./bpc -h

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

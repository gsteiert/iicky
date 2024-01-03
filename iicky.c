#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "tusb.h"

#define QWIIC_SDA_PIN  22
#define QWIIC_SCL_PIN  23

#define COMMAND_BUFFER_LENGTH  127
#define I2C_BUFFER_LENGTH       64

const char *delimiters = ", \t";

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}

void bus_scan(i2c_inst_t *i2c) {
    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
}

int nextInt() {
  char * nptr = strtok(NULL, delimiters);
  if (nptr == NULL) {
    return -1;
  } else {
    char * endptr = NULL;
    unsigned long ul = strtoul(nptr, &endptr, 0);
    if (nptr == endptr) {
      return -1;
    } else {
      return ul;
    }
  }
}

void runI2C() {
  uint8_t i2cSrc[I2C_BUFFER_LENGTH];
  uint8_t i2cDst[I2C_BUFFER_LENGTH];
  int extendedAddr = nextInt();
  if (extendedAddr > 0) {
    i2c_inst_t * i2cBus = (extendedAddr > 0xFF) ? i2c1 : i2c0;
    uint8_t i2cAddr = extendedAddr >> 1;
    int i2cRead = 0;
    if (extendedAddr & 0x1) { i2cRead = nextInt(); }
    int i2cWrite = 0;
    int writeData = nextInt();
    while (writeData >= 0) {
      i2cSrc[i2cWrite] = writeData;
      i2cWrite += 1;
      writeData = nextInt();
    }
    if (i2cWrite > 0) {
      i2cWrite = i2c_write_blocking(i2cBus, i2cAddr, i2cSrc, i2cWrite, false);
      printf("Wrote %i bytes\n", i2cWrite);
    }
    if (i2cRead > 0) {
      i2cRead = i2c_read_blocking(i2cBus, i2cAddr, i2cDst, i2cRead, false);
      if (i2cRead >0) {
        printf("Read: ");
        for (int i; i < i2cRead; i++) {
          printf(" 0x%X", i2cDst[i]);
        }
        printf("\n");
      } else {
        printf("Read error: %i\n", i2cRead);
      }
    }

  } else {
    printf("Bad I2C Command\n");
  }

}

void doIt(char *buf){
  char *command = strtok(buf, delimiters);
  switch (command[0]) {
    case 'S':
    case 's':
      bus_scan(i2c0);
      bus_scan(i2c1);
      break;
    case '?':
    case 'H':
    case 'h':
      printf("Help Command\n");
      break;
    case 'I':
    case 'i':
      runI2C();
      break;
    default:
      printf("Unknown command:  %s\n", command);
  }

}

int main() {

  bi_decl(bi_program_description("This is a stupidly terse I2C bridge terminal."));

  stdio_init_all();
  sleep_ms(50);

#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
#warning i2c/bus_scan example requires a board with I2C pins
    puts("Default I2C pins were not defined");
#else
    // This example will use I2C0 on the default SDA and SCL pins (GP4, GP5 on a Pico)
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);
    gpio_set_function(QWIIC_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(QWIIC_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(QWIIC_SDA_PIN);
    gpio_pull_up(QWIIC_SCL_PIN);
    // Make the I2C pins available to picotool
    i2c_init(i2c0, 100 * 1000);
    i2c_init(i2c1, 100 * 1000);

    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
    bi_decl(bi_2pins_with_func(QWIIC_SDA_PIN, QWIIC_SCL_PIN, GPIO_FUNC_I2C));

#endif
  // wait for something on STDIN before scanning to make sure the terminal is open
    while (!tud_cdc_connected()) {
      sleep_ms(50);
    }
    printf("\nIICKY Terminal\n");

  char cmd[COMMAND_BUFFER_LENGTH +1];

  cmd[0] = 0;

  printf("Starting main loop...\n");
  while (1) {
    gets(cmd);
    printf("cmd:  %s\n", cmd);
    doIt(cmd);
    sleep_ms(5);
  }
}
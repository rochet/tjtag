#ifndef BUSPIRATE_H_
#define BUSPIRATE_H_

#include <stdint.h>



#define ERR  -1
#define BBIO 0x00
#define SPI 0x01
#define I2C 0x02
#define ART  0x03
#define OWD  0x04
#define RAW  0X05

#define BP_CS 0
#define BP_MISO 1
#define BP_CLK 2
#define BP_MOSI 3
#define BP_AUX 4


int BP_EnableBinary(int);

#endif /* BUSPIRATE_H_ */

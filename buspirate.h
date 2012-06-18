#ifndef BUSPIRATE_H_
#define BUSPIRATE_H_

#include <stdint.h>



#define ERR		-1
#define BBIO	0x00
#define SPI		0x01
#define I2C		0x02
#define ART		0x03
#define OWD		0x04
#define RAW		0X05
#define OOCD	0x06

#define BP_CS	0
#define BP_MISO	1
#define BP_CLK	2
#define BP_MOSI	3
#define BP_AUX	4

enum {
    OOCD_FEATURE_LED=0x01,
    OOCD_FEATURE_VREG=0x02,
    OOCD_FEATURE_TRST=0x04,
    OOCD_FEATURE_SRST=0x08,
    OOCD_FEATURE_PULLUP=0x10
};

enum {
    OOCD_MODE_HIZ=0,
    OOCD_MODE_JTAG=1,
    OOCD_MODE_JTAG_OD=2, // open-drain outputs
};

int BP_EnableBinary(int);
void BP_DisableBinary(int fd);
int BP_EnableOCD(int fd);
void BP_OCDMode(int fd, int mode);
void BP_OCDFeature(int fd, int feature, int value);
int BP_OCDTapShift(int fd, unsigned char *tdodata, const unsigned char *tdidata, const unsigned char *tmsdata, int bitcount);

#endif /* BUSPIRATE_H_ */

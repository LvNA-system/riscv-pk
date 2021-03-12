#include <stdio.h>
#include <stdint.h>
#include <mtrap.h>

#define FREQ (100 * 1000 * 1000)
#define BAUD 115200
#define UART_BASE 	0x12000000

#define REG8(add)  *((volatile unsigned char *)  (add))
#define REG16(add) *((volatile unsigned short *) (add))
#define REG32(add) *((volatile unsigned int*)  (add))  //rv64

/*****the uart register is little-endian and 4 byte-align******/

#define RB_THR_OFF  0x00 + 0x00
#define IER_OFF     0x04 + 0x00
#define IIR_FCR_OFF 0x08 + 0x00
#define LCR_OFF     0x0c + 0x00
#define MCR_OFF     0x10 + 0x00
#define LSR_OFF     0x14 + 0x00
#define MSR_OFF     0x18 + 0x00
#define CDRl_OFF    0x00 + 0x00
#define CDRh_OFF    0x04 + 0x01

#define DLAB_EN   0x83
#define DLAB_DIS  0x03

#define DL 0x001b
#define DL_1 0x1b
#define DL_2 0x00

static inline void uart_set_baud() {
  //unsigned int DL = FREQ/(BAUD*16);
  //unsigned int DL_1 = DL & 0x000000ff;   //LSB of DL 
  //unsigned int DL_2 = DL & 0x0000ff00;   //MSB of DL
  //REG8(UART_BASE+LCR_OFF)  = 0x33221100 | DLAB_EN; //enable DLAB
  //REG(UART_BASE+CDRl_OFF) = 0x44332200 | DL_1; //dl[`UART_DL1]=0x1b
  //REG32(UART_BASE+CDRh_OFF) = 0x44330011 | DL_2; //dl[`UART_DL2]=0x00
  //REG32(UART_BASE+LCR_OFF)  = 0x33221100 | DLAB_DIS; //disable DLAB

  uint32_t dl = (FREQ + (BAUD * 16) / 2) / (BAUD * 16);
  uint8_t lo = dl & 0xff;
  uint8_t hi = (dl >> 8) & 0xff;

  REG8(UART_BASE+0x0c+0x03) = 0x83; //enable DLAB
  REG8(UART_BASE+0x00+0x00) = lo; //dl[`UART_DL1]=0x1b
  REG8(UART_BASE+0x04+0x01) = hi; //dl[`UART_DL2]=0x00
  REG8(UART_BASE+0x0c+0x03) = 0x03; //disable DLAB
}

void __am_uartlite_putchar(char ch) {
  if (ch == '\n') __am_uartlite_putchar('\r');

  while(!(REG8(UART_BASE+LSR_OFF) & 0x20));
  //wait until transmitter FIFO is empty
  //judge condition LSR[5]==1
  REG8(UART_BASE+RB_THR_OFF) = ch;

}

int __am_uartlite_getchar() {
  //judge condition LSR[0]==1
  if ((REG8(UART_BASE+LSR_OFF) & 0x1)) {
    return (unsigned int)REG8(UART_BASE+RB_THR_OFF);
  }
  return -1;
}

void __am_init_uartlite(void) {
  uart_set_baud();
  printm("Hello, BeiHai\n");
  printm("freq = %dMHz\n", FREQ / 1000 / 1000);
}

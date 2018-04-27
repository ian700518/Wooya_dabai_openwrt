#ifndef __UART_H_
#define __UART_H_

#define DEV_UART "/dev/ttyS1"

int uart_initial(char *dev, int baudrate, int bits, int parity, int stopbits);

#endif /* !__UART_H_ */

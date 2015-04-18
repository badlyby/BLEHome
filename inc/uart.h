//=============================================================================
// 文件名 : uart.h
// 功能  : 设置串口
// 历史版本 :
//   1.0 : 2009年5月26日 王忠磊
//=============================================================================

#ifndef __UART_H__
#define __UART_H__

#ifdef __cplusplus
extern "C" {
#endif

int set_uart(const char *dev, int speed, int databits, int stopbits, int parity);
void clean_uart(int fuart);
#ifdef __cplusplus
}
#endif
#endif  //__UART_H__

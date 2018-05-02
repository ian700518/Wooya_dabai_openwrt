#ifndef __BLUETOOTH_H_
#define __BLUETOOTH_H_

#define Direct_Out 1
#define Direct_In 0

#define GPIO_Swbtn_Num 11
#define GPIO_Wakeup_Num 3
#define GPIO_Reset_Num 2
#define GPIO_P04_Num 1
#define GPIO_P15_Num 0
#define GPIO_P20_Num 20
#define GPIO_P24_Num 21
#define GPIO_Ean_Num 18

#define GPIOCTRL_0 0x10000600
#define GPIODATA_0 0x10000620

enum
{
    normal_mode=0,
    WEE_mode,

};

#endif /* !__BLUETOOTH_H_ */

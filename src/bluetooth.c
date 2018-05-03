#include <stdio.h>      /*標準輸入輸出定義*/
#include <stdlib.h>     /*標準函數庫定義*/
#include <unistd.h>     /*Unix 標準函數定義*/
#include <errno.h>      /*錯誤號定義*/
#include <memory.h>
#include "bluetooth.h"

void send_command(char *command, char *resulte, int resulte_length)
{
    FILE *fp;

    fp = popen(command, "r");
    if(resulte != NULL)
    {
        fgets(resulte, resulte_length, fp);
    }
    pclose(fp);
}


int SetGpioReg(int reg, int gpio_num, int val)
{
    if(val)
    {
        reg |= (val << gpio_num);
    }
    else
    {
        reg &= ~((val | 1) << gpio_num);
    }
    return reg;
}

int PinSetForBMModule(void)
{
    int reg, addr;
    char command[64], resulte[32];
    FILE *fp;

    // set GPIO0~32 ctrlol register
    sprintf(command, "devmem 0x%08x", GPIOCTRL_0);
    send_command(command, resulte, sizeof(resulte));
    reg = strtoul(resulte + 2, NULL, 16);
    // set swbtn/wakeup/reset/p20/p24/ean pin to out mode and p04/p15 pin in mode
    // swbtn -> pin 11
    // wakeup -> pin 3
    // reset -> pin 2
    // p20 -> pin 20
    // p24 -> pin 21
    // ean -> pin 18
    // p04 -> pin 1
    // p15 -> pin 0

    reg |= (Direct_Out << GPIO_Swbtn_Num);
    reg |= (Direct_Out << GPIO_Wakeup_Num);
    reg |= (Direct_Out << GPIO_Reset_Num);
    reg |= (Direct_Out << GPIO_P20_Num);
    reg |= (Direct_Out << GPIO_P24_Num);
    reg |= (Direct_Out << GPIO_Ean_Num);
    reg &= ~((Direct_In | 1) << GPIO_P04_Num);
    reg &= ~((Direct_In | 1) << GPIO_P15_Num);
    sprintf(command, "devmem 0x%x 32 0x%08x", GPIOCTRL_0, reg);
    send_command(command, NULL, NULL);
    // set all pin to 0
    sprintf(command, "devmem 0x%08x", GPIODATA_0);
    send_command(command, resulte, sizeof(resulte));
    reg = strtoul(resulte + 2, NULL, 16);
    reg = SetGpioReg(reg, GPIO_Wakeup_Num, 0);
    reg = SetGpioReg(reg, GPIO_Ean_Num, 0);
    reg = SetGpioReg(reg, GPIO_P20_Num, 0);
    reg = SetGpioReg(reg, GPIO_P24_Num, 0);
    reg = SetGpioReg(reg, GPIO_Swbtn_Num, 0);
    reg = SetGpioReg(reg, GPIO_Reset_Num, 0);
    sprintf(command, "devmem 0x%08x 32 0x%08x", GPIODATA_0, reg);
    send_command(command, NULL, NULL);
    sprintf(command, "devmem 0x%08x", GPIODATA_0);
    send_command(command, resulte, sizeof(resulte));
    reg = strtoul(resulte + 2, NULL, 16);
}

int BMModule_initial(int OPmode)
{
    FILE *fp;
    char resulte[64], command[64];
    int reg;

    sprintf(command, "devmem 0x%08x", GPIODATA_0);
    send_command(command, resulte, sizeof(resulte));
    reg = strtoul(resulte + 2, NULL, 16);
    reg = SetGpioReg(reg, GPIO_Wakeup_Num, 1);
    switch(OPmode)
    {
        case normal_mode:
        default:
            reg = SetGpioReg(reg, GPIO_Ean_Num, 0);
            reg = SetGpioReg(reg, GPIO_P20_Num, 1);
            reg = SetGpioReg(reg, GPIO_P24_Num, 1);
            break;

        case WEE_mode:
            reg = SetGpioReg(reg, GPIO_Ean_Num, 0);
            reg = SetGpioReg(reg, GPIO_P20_Num, 0);
            reg = SetGpioReg(reg, GPIO_P24_Num, 1);
            break;
    }
    reg = SetGpioReg(reg, GPIO_Swbtn_Num, 1);
    sprintf(command, "devmem 0x%08x 32 0x%08x", GPIODATA_0, reg);
    //printf("BMModule_initial command : %s\n", command);
    usleep(40000);
    reg = SetGpioReg(reg, GPIO_Reset_Num, 1);
    sprintf(command, "devmem 0x%08x 32 0x%08x", GPIODATA_0, reg);
    send_command(command, NULL, NULL);
    usleep(450000);
}

// add test program by ian at 2018-04-25
#include <stdio.h>      /*標準輸入輸出定義*/
#include <stdlib.h>     /*標準函數庫定義*/
#include <unistd.h>     /*Unix 標準函數定義*/
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>      /*檔控制定義*/
#include <termios.h>    /*PPSIX 終端控制定義*/
#include <errno.h>      /*錯誤號定義*/
#include <memory.h>
#include <sys/time.h>
#include "uart.h"


int main()
{
    int fd, ct = 0;
    char buf[512];

    printf("This is dabai test program!\n");
    fd = uart_initial(DEV_UART, 57600, 8, 'N', 1);
    if(fd < 0)
        return -1;
    memset(buf, 0, sizeof(buf));

    while(1)
    {
        // read serial port data
        ct = uart_read(fd, buf);
        if(ct > 0)
        {
            ct = 0;
            printf("buf : %s\n", buf);
        }
    }
    return 0;
}

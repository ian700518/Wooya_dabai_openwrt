// add test program by ian at 2018-04-25
#include <stdio.h>      /*標準輸入輸出定義*/
#include <stdlib.h>     /*標準函數庫定義*/
#include <unistd.h>     /*Unix 標準函數定義*/
#include <errno.h>      /*錯誤號定義*/
#include <memory.h>
#include "uart.h"

/*将字符串s中出现的字符c删除*/
void squeeze(char s[],int c)
{
    int i,j;

    for (i = 0, j = 0; s[i] != '\0'; i++)
    {
        if (s[i] != c)
        {
            s[j++] = s[i];
        }
    }
    s[j] = '\0';    //这一条语句千万不能忘记，字符串的结束标记
}

int check_data_format(char *buf, int bytenum, char *buf2)
{
    char *tmp, mac_tmp[24], path[36];
    int check_ret, i;
    int index, type;
    FILE *fp1;

    // check data profile : first char is "{" and last char is "}"
    if((strchr(buf, '{') == NULL) || (strchr(buf, '}') == NULL))
    {
        printf("Data profile is error\n");
        sprintf(buf2, "Data profile is error\n");
        return -99;
    }
    if((tmp = strstr(buf, "index")) != NULL)
    {
        index = atoi(tmp+8);
        //printf("index is %d\n", index);
    }
    switch(index)
    {
        // set Host parameter. like ssid / store id ...
        case 1:
            break;

        // device like phone / powerband ... send device information to host.
        case 2:
            if(strstr(buf, "type") == NULL)
            {
                printf("Data not inclue operate system type\n");
                sprintf(buf2, "Data not inclue operate system type\n");
                return -98;
            }
            else if(strstr(buf, "userId") == NULL)
            {
                printf("Data not inclue userId data\n");
                sprintf(buf2, "Data not inclue userId data\n");
                return -97;
            }
            else if(strstr(buf, "account") == NULL)
            {
                printf("Data not inclue account data\n");
                sprintf(buf2, "Data not inclue account data\n");
                return -96;
            }
            else if(strstr(buf, "mac") == NULL)
            {
                printf("Data not inclue mac data\n");
                sprintf(buf2, "Data not inclue mac data\n");
                return -95;
            }
            if(strstr(buf, "android") != NULL)
                type = 0;
            else if(strstr(buf, "iOS") != NULL)
                type = 1;
            tmp = strstr(buf, "mac");
            //printf("tmp : %s\n", tmp);
            break;
    }
    switch(type)
    {
        case 0: // android system
            memcpy(mac_tmp, tmp+6, 17);
            //printf("1. mac_tmp : %s\n", mac_tmp);
            squeeze(mac_tmp, ':');
            //printf("2. mac_tmp : %s\n", mac_tmp);
            sprintf(path, "/DaBai/%s.json", mac_tmp);
            //printf("device json file %s\n", path);
            //printf("check_data_format buf is : %s\n", buf);
            fp1 = fopen(path, "w+");
            if(fp1 != NULL)
            {
                fprintf(fp1, "%s", buf);
                fclose(fp1);
            }
            else
            {
                printf("device jason %s open error\n", path);
            }
            break;

        case 1: // iOS system
            break;
    }
    sprintf(buf2, "Receive Data is OK\n");
    return 0;
}

int main()
{
    int fd, recct = 0, writect = 0;
    char buf[512];
    char buf2[512];
    char bytec[512];
    int byte = 0;
    FILE *fp;

    printf("This is dabai test program!\n");
    fd = uart_initial(DEV_UART, 57600, 8, 'N', 1);
    if(fd < 0)
        return -1;
    memset(buf, "", strlen(buf));
    //printf("buf strlen : %d\n", strlen(buf));

    while(1)
    {
        // read serial port data
        recct = uart_read(fd, buf);
        if(recct > 0)
        {
            check_data_format(buf, recct, buf2);
            if(strcmp(buf2, "Receive Data is OK\n") == 0)
            {
                //printf("buf2 : %s\n", buf2);
                fp = fopen("/DaBai/timg.jpg", "r");
                if(fp != NULL)
                {
                    //printf("read jpg file success\n");
                    while(1)
                    {
                        if(fgets(bytec, 512, fp) != NULL)
                        {
                            uart_write(fd, bytec);
                        }
                        else
                            break;
                    }
                }
            }
            recct = 0;
            //printf("buf : %s\n", buf);
            memset(buf, "", strlen(buf));
        }
    }
    return 0;
}

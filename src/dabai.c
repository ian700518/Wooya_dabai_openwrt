// add test program by ian at 2018-04-25
#include <stdio.h>      /*標準輸入輸出定義*/
#include <stdlib.h>     /*標準函數庫定義*/
#include <unistd.h>     /*Unix 標準函數定義*/
#include <errno.h>      /*錯誤號定義*/
#include <memory.h>
#include "uart.h"
#include "bluetooth.h"

/* read string */
void readstr(char src[], char des[])
{
    int i = 0;

    while(src[i] != '\"')
    {
        des[i] = src[i];
        i++;
    }
    des[i] = '\0';
}

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
    char *tmp, mac_tmp[24], path[36], ucicommand[64];
    int check_ret, i;
    int type = 0;
    FILE *fp1, *fp2;

    // check data profile : first char is "{" and last char is "}"
    if((strchr(buf, '{') == NULL) || (strchr(buf, '}') == NULL))
    {
        //printf("Data profile is error\n");
        //sprintf(buf2, "Data profile is error\n");
        return Err_Profile;
    }

    // check data is include index value
    if((tmp = strstr(buf, "index")) != NULL)
    {
        CmdIndex = atoi(tmp + strlen("index\":\""));
    }
    else
    {
        return Err_Index;
    }

    switch(CmdIndex)
    {
        // set Host parameter. like ssid / store id ...
        case 1:
            // check SSID
            if((tmp = strstr(buf, "SSID")) != NULL)
            {
                //printf("SSID buf : %s\nstrlen is %d\n", tmp, strlen("SSID\":\""));
                readstr(tmp + strlen("SSID\":\""), StaSsid);
                //printf("SSID : %s\n", StaSsid);
            }
            else
            {
                return Err_Ssid;
            }

            // check Passord
            if((tmp = strstr(buf, "PASSWORD")) != NULL)
            {
                //printf("PASSWORD buf : %s\nstrlen is %d\n", tmp, strlen("PASSWORD\":\""));
                readstr(tmp + strlen("PASSWORD\":\""), StaPassword);
                //printf("PASSWORD : %s\n", StaPassword);
            }
            else
            {
                return Err_Password;
            }

            // check encryption
            if((tmp = strstr(buf, "ENCRYPTION")) != NULL)
            {
                //printf("ENCRYPTION buf : %s\nstrlen is %d\n", tmp, strlen("ENCRYPTION\":\""));
                readstr(tmp + strlen("ENCRYPTION\":\""), StaEncryption);
                //printf("ENCRYPTION : %s\n", StaEncryption);
            }
            else
            {
                return Err_Encrytpion;
            }

            // check store Id
            if((tmp = strstr(buf, "StoreId")) != NULL)
            {
                //printf("StoreId buf : %s\nstrlen is %d\n", tmp, strlen("StoreId\":\""));
                readstr(tmp + strlen("StoreId\":\""), DBStoreId);
                //printf("StoreId : %s\n", DBStoreId);
            }
            else
            {
                return Err_Dbstoreid;
            }
            // set Host ssid for ap mode
            sprintf(ucicommand, "uci set wireless.ap.ssid=\"DaBai_%s\"", DBStoreId);
            send_command(ucicommand, NULL, NULL);

            // set Store wifi ssid to Host device
            sprintf(ucicommand, "uci set wireless.sta.ssid=\"%s\"", StaSsid);
            send_command(ucicommand, NULL, NULL);

            // set Store wifi ssid to Host device
            sprintf(ucicommand, "uci set wireless.sta.key=\"%s\"", StaPassword);
            send_command(ucicommand, NULL, NULL);

            // set Store wifi ssid to Host device
            sprintf(ucicommand, "uci set wireless.sta.encryption=\"%s\"", StaEncryption);
            send_command(ucicommand, NULL, NULL);

            // set Host device to apsta mode
            send_command("wifi_mode apsta", NULL, NULL);

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
                type = 1;
            else if(strstr(buf, "iOS") != NULL)
                type = 2;
            tmp = strstr(buf, "mac");
            //printf("tmp : %s\n", tmp);
            switch(type)
            {
                case 1: // android system
                    memcpy(mac_tmp, tmp+6, 17);
                    squeeze(mac_tmp, ':');
                    sprintf(path, "/DaBai/device-%s.json", mac_tmp);
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

                case 2: // iOS system
                    break;
            }
            break;
    }
    sprintf(buf2, "Receive Data is OK\n");
    return 0;
}

int main()
{
    int fd, recct = 0;
    char buf[512];
    char buf2[64];
    char *tmp;
    FILE *fp;
    int i, reg, data_length;

    // set AGPIO_CFG
    send_command("devmem 0x1000003C", buf, sizeof(buf));
    reg = strtoul(buf + 2, NULL, 16);
    reg |= 0x000E000F;
    sprintf(buf, "devmem 0x1000003c 32 0x%08x", reg);
    send_command(buf, NULL, NULL);
    send_command("devmem 0x1000003C", buf, sizeof(buf));
    usleep(1000);

    PinSetForBMModule();
    usleep(1000);
    BMModule_initial(normal_mode);

    fd = uart_initial(DEV_UART, 57600, 8, 'N', 1);
    if(fd < 0)
        return -1;
    memset(buf, "", strlen(buf));

    printf("This is dabai test program!\n");
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
                switch(CmdIndex)
                {
                    case 1:
                        uart_write(fd, buf2, strlen(buf2));
                        break;

                    case 2:
                        fp = fopen("/DaBai/timg.jpg", "r");
                        if(fp != NULL)
                        {
                            fseek(fp, 0, SEEK_END); //定位到文件末
                            data_length = ftell(fp);
                            rewind(fp);
                            char *jpgbuf = (char *)malloc(FiletottyByte);
                            i = 0;
                            while(!feof(fp))
                            {
                                i = fread(jpgbuf, 1, FiletottyByte, fp);
                                uart_write(fd, jpgbuf, i);
                            }
                            free(jpgbuf);
                            fclose(fp);
                        }
                        break;
                }

            }
            recct = 0;
            //printf("buf : %s\n", buf);
            memset(buf, "", strlen(buf));
        }
    }
    return 0;
}

// add test program by ian at 2018-04-25
#include <stdio.h>      /*標準輸入輸出定義*/
#include <stdlib.h>     /*標準函數庫定義*/
#include <unistd.h>     /*Unix 標準函數定義*/
#include <errno.h>      /*錯誤號定義*/
#include <memory.h>
#include <time.h>
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

// client device information write into online list
int write_online_list(struct ClientDev client)
{
    FILE *fp;
    char OldStr[256], NewStr[256];
    int length;

    fp = fopen("DaBai/Onlist.txt", "a+");
    if(fp)
    {
        sprintf(NewStr, "%s-%s-%s-%s %ld\n", client.DevType, client.DevMac, client.DevUserId, client.DevAccount, time(NULL));
        length = strlen(client.DevType) + strlen(client.DevMac) + strlen(client.DevUserId) + strlen(client.DevAccount);
        while(!feof(fp))
        {
            // check device at list already, if device has at list return otherwise write into onlist.txt
            if(fgets(OldStr, sizeof(OldStr), fp) != NULL)
            {
                if(strncmp(OldStr, NewStr, length) == 0)
                {
                    fclose(fp);
                    return 1;
                }
            }
        }
        fprintf(fp, NewStr);
        fclose(fp);
        return 0;
    }
    else
    {
        return -1;
    }
}
/*
int create_device_json(struct ClientDev client)
{
    FILE *fp;

    fp = fopen(SendtoClientFile, "w+");
    if(fp != NULL)
    {

    }
    else
    {
        return -1;
    }
}
*/

int check_data_format(char *buf, int bytenum, char *buf2)
{
    char *tmp, mac_tmp[24], path[36], ucicommand[64];
    int check_ret, i;
    int type = 0;
    FILE *fp1, *fp2;
    struct ClientDev CDev;

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
            // cheeck commnad include device type information
            if((tmp = strstr(buf, "type")) != NULL)
            {
                readstr(tmp + strlen("type\":\""), CDev.DevType);
                printf("DevType : %s\n", CDev.DevType);
            }
            else
            {
                return Err_Type;
            }

            // check command include UserId
            if((tmp = strstr(buf, "userId")) != NULL)
            {
                readstr(tmp + strlen("userId\":\""), CDev.DevUserId);
                printf("DevUserId : %s\n", CDev.DevUserId);
            }
            else
            {
                return Err_Userid;
            }

            // check command include account
            if((tmp = strstr(buf, "account")) != NULL)
            {
                readstr(tmp + strlen("account\":\""), CDev.DevAccount);
                printf("DevAccount : %s\n", CDev.DevAccount);
            }
            else
            {
                return Err_Account;
            }

            // check command include device mac address
            if((tmp = strstr(buf, "mac")) != NULL)
            {
                readstr(tmp + strlen("mac\":\""), CDev.DevMac);
                printf("DevMac : %s\n", CDev.DevMac);
            }
            else
            {
                return Err_Mac;
            }
            write_online_list(CDev);
            if(strcmp(CDev.DevType, "android") == 0)
                TypeIdx = 1;
            else if(strcmp(CDev.DevType, "iOS") == 0)
                TypeIdx = 2;
            //create_device_json(CDev);
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
    char *jpgbuf;
    unsigned char prebuf[6], suffixbuf[2];

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
                            jpgbuf = (char *)malloc(FiletottyByte);

                            // for iOS system start
                            if(TypeIdx == 2)
                            {
                                prebuf[0] = 0x55;
                                prebuf[1] = 0xAA;
                                prebuf[2] = (char)((data_length >> 24) & 0x000000FF);
                                prebuf[3] = (char)((data_length >> 16) & 0x000000FF);
                                prebuf[4] = (char)((data_length >> 8) & 0x000000FF);
                                prebuf[5] = (char)(data_length & 0x000000FF);
                                for(i=0; i<6; i++)
                                    printf("prebuf : 0x%x\n", prebuf[i]);
                                uart_write(fd, prebuf, 4);
                            }
                            // for iOS system end
                            i = 0;
                            while(!feof(fp))
                            {
                                i = fread(jpgbuf, 1, FiletottyByte, fp);
                                uart_write(fd, jpgbuf, i);
                            }
                            // for iOS system start
                            if(TypeIdx == 2)
                            {
                                suffixbuf[0] = 0xAA;
                                suffixbuf[1] = 0x55;
                                for(i=0; i<2; i++)
                                    printf("suffixbuf : 0x%x\n", suffixbuf[i]);
                                uart_write(fd, suffixbuf, 2);
                            }
                            // for iOS system end
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

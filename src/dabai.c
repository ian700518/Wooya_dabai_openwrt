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

int open_uart(char *dev)
{
    int fd = open(dev, O_RDWR | O_NOCTTY | O_NDELAY);
    if(fd)
        return fd;
    else
    {
        perror("Can't Open Serial Port");
        return -1;
    }
}

/**
*@brief  設置串口通信速率
*@param  fd     類型 int  打開串口的文件控制碼
*@param  speed  類型 int  串口速度
*@return  void
*/
int speed_arr[] = {B1000000, B921600, B576000, B460800, B230400, B115200, B57600, B38400, B19200, B9600, B4800, B2400, B1200};
int name_arr[] = {1000000, 921600, 576000, 460800, 230400, 115200, 57600, 38400,  19200,  9600,  4800,  2400,  1200};
void set_speed(int fd, int speed)
{
    int i;
    int status;
    struct termios Opt;
    tcgetattr(fd, &Opt);
    for(i= 0;i < (sizeof(speed_arr) / sizeof(int)); i++)
    {
         if(speed == name_arr[i])
         {
             tcflush(fd, TCIOFLUSH);
             cfsetispeed(&Opt, speed_arr[i]);
             cfsetospeed(&Opt, speed_arr[i]);
             status = tcsetattr(fd, TCSANOW, &Opt);
             if(status != 0)
             {
                      perror("tcsetattr fd1");
                      return;
             }
             tcflush(fd,TCIOFLUSH);
         }
    }
}

/**
*@brief   設置串口資料位元，停止位元和效驗位
*@param  fd     類型  int  打開的串口文件控制碼
*@param  databits 類型  int 資料位元   取值 為 7 或者8
*@param  stopbits 類型  int 停止位   取值為 1 或者2
*@param  parity  類型  int  效驗類型 取值為N,E,O,,S
*/
int set_Parity(int fd, int databits, int stopbits, int parity)
{
        struct termios options;
        if(tcgetattr(fd,&options) != 0)
        {
            perror("SetupSerial 1");
            return -1;
        }
        options.c_cflag &= ~CSIZE;
        switch(databits) /*設置數據位元數*/
        {
            case 7:
                options.c_cflag |= CS7;
                break;
            case 8:
                options.c_cflag |= CS8;
                break;
            default:
                fprintf(stderr,"Unsupported data size\n");
                return -1;
        }
        switch(parity)
        {
            case 'n':
            case 'N':
                options.c_cflag &= ~PARENB;   /* Clear parity enable */
                options.c_iflag &= ~INPCK;     /* Enable parity checking */
                break;
            case 'o':
            case 'O':
                options.c_cflag |= (PARODD | PARENB); /* 設置為奇效驗*/
                options.c_iflag |= INPCK;             /* Disnable parity checking */
                break;
            case 'e':
            case 'E':
                options.c_cflag |= PARENB;     /* Enable parity */
                options.c_cflag &= ~PARODD;   /* 轉換為偶效驗*/
                options.c_iflag |= INPCK;       /* Disnable parity checking */
                break;
            case 'S':
            case 's':  /*as no parity*/
                options.c_cflag &= ~PARENB;
                options.c_cflag &= ~CSTOPB;break;
            default:
                     fprintf(stderr,"Unsupported parity\n");
                     return -1;
        }
        /* 設置停止位*/
        switch(stopbits)
        {
            case 1:
                options.c_cflag &= ~CSTOPB;
                break;
            case 2:
                options.c_cflag |= CSTOPB;
                break;
            default:
                fprintf(stderr,"Unsupported stop bits\n");
                return -1;
        }
        /* Set input parity option */
        if(parity != 'n')
            options.c_iflag |= INPCK;
        options.c_lflag  &= ~(ICANON | ECHO | ECHOE | ISIG);  /*Input*/
        options.c_oflag  &= ~OPOST;   /*Output*/
        tcflush(fd,TCIFLUSH);
        options.c_cc[VTIME] = 0; /* 設置超時15 seconds*/
        options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
        if(tcsetattr(fd,TCSANOW,&options) != 0)
        {
            perror("SetupSerial 3");
            return -1;
        }
        return 0;
}

void send_message(int fd, int mode)
{
    char buf[512];
    int buf_ct, nwrite;

    switch(mode)
    {
        case 99:
            sprintf(buf, "Receive OK\n");
            buf_ct = sizeof("Receive OK\n");
            break;

        default:
            sprintf(buf, "Receive Error\n");
            buf_ct = sizeof("Receive Error\n");
            break;
    }
    nwrite = write(fd, buf, buf_ct);
    memset(buf, 0, buf_ct);
    usleep(buf_ct*200);
}

int main()
{
    int fd, fd2;
    int nread, nwrite;
    char buff[512];
    fd_set readfd;
    struct timeval timeout;
    char *dev="/dev/ttyS1";
    char r_flag = 0;
    int i, ret;

    printf("This is dabai test program!\n");
    fd = open_uart(dev);
    if(fd > 0)
        set_speed(fd, 57600);
    else
    {
        printf("Can't Open Serial Port!\n");
        exit(0);
    }

    if(set_Parity(fd, 8, 1,'N') == -1)
    {
        printf("Set Parity Error\n");
        exit(1);
    }
    timeout.tv_sec = 0;
    timeout.tv_usec = 200;
    FD_ZERO(&readfd);
    FD_SET(fd,&readfd);
    while(1)
    {
        ret = select(fd+1, &readfd, NULL, NULL, &timeout);
        if(ret < 0)
            printf("Select errot\n");
        else if(ret)
        {
            if(FD_ISSET(fd, &readfd))
            {
                nread = read(fd, buff, 512);
                if(nread)
                {
                    usleep(nread*200);
                    r_flag = 1;
                }
            }
            if(r_flag)
            {
                r_flag = 0;
                fd2 = open("/DaBai/config.json", O_RDWR | O_CREAT | O_TRUNC, 0775);
                nwrite = write(fd2, buff, nread);
                usleep(nwrite * 200);
                close(fd2);
                memset(buff, 0, sizeof(buff));
                send_message(fd, 99);
                nread = 0;
            }
        }
        /*
        while((nread = read(fd, buff, 512)) > 0)
        {
            printf("Receive Data : %s", buff);
            printf("1. Receive Byte : %d\n", nread);
            usleep(nread * 200);
            fd2 = open("/DaBai/config.json", O_RDWR | O_CREAT | O_TRUNC, 0775);
            nwrite = write(fd2, buff, nread);
            usleep(nread * 200);
            close(fd2);
            memset(buff, 0, sizeof(buff));
            r_flag = 1;
        }
        if(r_flag)
        {
            r_flag = 0;
            sprintf(buff, "Receive OK\n");
            printf("buff Byte : %d\n", sizeof("Receive OK\n"));
            nwrite = write(fd, buff, sizeof("Receive OK\n"));
            printf("Send Byte : %d\n", nwrite);
            nread = 0;
            memset(buff, 0, sizeof(buff));
            usleep(sizeof("Receive OK\n") * 200);
        }
        */
    }
    return 0;
}

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

int uart_initial(char *dev, int baudrate, int bits, int parity, int stopbits)
{
    int fd;

    fd = open_uart(dev);
    if(fd > 0)
    {
        set_speed(fd, baudrate);
        set_Parity(fd, bits, stopbits, parity);
        return fd;
    }
    return -1;
}

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

int uart_read(int fd, char *buf)
{
    struct timeval timeout;
    char tmp[512];
    fd_set readfd;
    int select_ret, readct = 0, nread = 0, ret;

    memset(tmp, 0, sizeof(tmp));
    while(1)
    {
        timeout.tv_sec = 0;
        timeout.tv_usec = 5000;
        FD_ZERO(&readfd);
        FD_SET(fd,&readfd);
        ret = select(fd+1, &readfd, NULL, NULL, &timeout);
        if(ret < 0)
        {
            printf("Select errot\n");
            ret = -1;
            break;
        }
        else if(ret == 0)
        {
            if(readct != 0 && nread == 0)
            {
                printf("Receive Data Finish\n");
                memcpy(buf, tmp, readct);
                send_message(fd, 99);
                ret = readct;
                break;
            }
            else if(readct >= sizeof(tmp))
            {
                printf("Receive count overflow\n");
                ret = -1;
                break;
            }
            else
            {
                ret = -1;
                break;
            }
        }
        else
        {
            if(FD_ISSET(fd, &readfd))
            {
                while((nread = read(fd, tmp+readct, 512)) > 0)
                {
                    readct += nread;
                }
            }
        }
    }
    return ret;
}

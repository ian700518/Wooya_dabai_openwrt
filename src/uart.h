#ifndef __UART_H_
#define __UART_H_

#define DEV_UART "/dev/ttyS1"
#define DaBai_passwd "lingshi508"
#define FiletottyByte 4096
#define SendtoClientFile "DaBai/ToClient.json"

#define RW_ByteTime ((10/(57600/10000))+1)*100
//int uart_initial(char *dev, int baudrate, int bits, int parity, int stopbits);

char StaSsid[128];
char StaPassword[64];
char StaEncryption[16];
char DBStoreId[128];
int CmdIndex;
int TypeIdx;

struct ClientDev
{
    char DevType[16];
    char DevMac[64];
    char DevAccount[64];
    char DevUserId[64];
};

enum
{
    //Err_none=0,
    Err_Profile=-1,
    Err_Index,
    Err_Ssid,
    Err_Password,
    Err_Encrytpion,
    Err_Dbstoreid,
    Err_Type,
    Err_Userid,
    Err_Account,
    Err_Mac,
};

#endif /* !__UART_H_ */

#include <stdio.h>
#include <windows.h>
#include "init.c"

int main(){
    HANDLE hFile;
    DWORD dwReturn = 0;
    char    *buff;
    FILE    *fd;
    int i, payload[2], w, h, d, buffsz;
    HDC ScreenDC;

    if( !(ScreenDC = GetDC(NULL)) ){
      printf("couldn't get screendc\n");
      return -1;
    }
    w = GetDeviceCaps(ScreenDC, HORZRES);
    h = GetDeviceCaps(ScreenDC, VERTRES);
    d = GetDeviceCaps(ScreenDC, BITSPIXEL)/8;
    ReleaseDC(NULL, ScreenDC);
    printf("%i\t%i\t%i\n", w, h, d);
    buffsz = w*h*d;
    if( (buff = (char *)malloc(buffsz)) == 0 ){
        printf("Error allocating\n");
        return -1;
    }

    if( (hFile = CreateFile("\\\\.\\ElectricEye",
                            GENERIC_READ, 0, NULL,
                            OPEN_EXISTING, 0, NULL)) == 0 ){
        printf("failed to open device\n");
        return -1;
    }

    GetFBInfo(payload);

    printf("sending ioctl\n");
    DeviceIoControl(hFile, IOCTL_EEYE_INITFB, payload,
		    2*sizeof(int), NULL, 0, &dwReturn, NULL);
    printf("reading\n");
    ReadFile(hFile, buff, buffsz, &dwReturn, NULL);
    CloseHandle(hFile);
    /*
    if(dwReturn == 0){
        printf("Error reading:%i\n", (int) dwReturn);
        return -1;
    }
*/
    fd = fopen("./out.rgb","wb+");
    fwrite(buff, sizeof(char), buffsz, fd);

    fclose(fd);
    return 0;

}

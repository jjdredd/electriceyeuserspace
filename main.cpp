#include <stdio.h>
#include <windows.h>
#include "init.c"
#include "mkimg.c"

int main(){
    HANDLE hFile;
    DWORD dwReturn = 0;
    char    *buff, *buff_rgb_conv, *chk_buff;
    FILE    *fd;
    int i, w, h, d, buffsz, k;
    PAYLOAD pl;
    HDC ScreenDC, MemDC;
    HBITMAP hbmp, hbmpold;

    if( !(ScreenDC = GetDC(NULL)) ){
      printf("couldn't get screendc\n");
      return -1;
    }
    w = GetDeviceCaps(ScreenDC, HORZRES);
    h = GetDeviceCaps(ScreenDC, VERTRES);
    d = GetDeviceCaps(ScreenDC, BITSPIXEL)/8;
    printf("%i\t%i\t%i\n", w, h, d);
    buffsz = w*h*d;
    if( (chk_buff = (char *)malloc(buffsz)) == 0 ){
        printf("Error allocating\n");
        return -1;
    }
    MemDC = CreateCompatibleDC(ScreenDC);
    hbmp = CreateCompatibleBitmap(ScreenDC, w, h);
    hbmpold = (HBITMAP)SelectObject(MemDC, hbmp);
    BitBlt(MemDC, 0, 0, w, h, ScreenDC, 0, 0, SRCCOPY);
    hbmp = (HBITMAP)SelectObject(MemDC, hbmpold);
    if( GetBitmapBits(hbmp, buffsz, chk_buff) != buffsz){
      printf("Couldn't get bitmap bits\n");
      return -1;
    }
    memcpy(pl.begin, chk_buff, 10);
    memcpy(pl.end, chk_buff + buffsz - 10, 10);
    DeleteObject(hbmp);
    free(chk_buff);
    ReleaseDC(NULL, ScreenDC);
    DeleteDC(MemDC);

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

    GetFBInfo(&pl);

    printf("sending ioctl\n");
    DeviceIoControl(hFile, IOCTL_EEYE_INITFB, &pl,
                    sizeof(PAYLOAD), NULL, 0, &dwReturn, NULL);
    printf("reading\n");
    ReadFile(hFile, buff, buffsz, &dwReturn, NULL);
    CloseHandle(hFile);
    /*
    if(dwReturn == 0){
        printf("Error reading:%i\n", (int) dwReturn);
        return -1;
    }
*/
    fd = fopen("./out.jpeg","wb+");
    printf("converting and writing\n");
    if( d == 4 ){
            printf("removing a %i\n", buffsz*3/4);
      if( (buff_rgb_conv = (char *)malloc(buffsz*3/4)) == 0 ){
        printf("Error allocating\n");
        return -1;
      }
      //bgra to rgb
      for(i = 0, k = 0; i < buffsz; i += 4, k +=3 ){
            buff_rgb_conv[k+2] = buff[i];
            buff_rgb_conv[k+1] = buff[i+1];
            buff_rgb_conv[k] = buff[i+2];
        }
      mkimg(buff_rgb_conv, w, h, fd);
    }
    else{
     for(i = 0; i < buffsz; i += 3){
        k = buff[i];
        buff[i] = buff[i+2];
        buff[i+2] = k;
	 }
     mkimg(buff, w, h, fd);
    }
    fclose(fd);
    free(buff);
    if(buff_rgb_conv) free(buff_rgb_conv);
    return 0;

}

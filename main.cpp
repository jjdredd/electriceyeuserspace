#include <windows.h>
#include <stdio.h>
#define buffsz  1024*768*4
#define buff2sz  1024*768*3
int main(){
    HANDLE hFile;
    DWORD dwReturn = 0;
    char    *buff, *buff2;
    FILE    *fd;
    int i, k;

    if( (buff = (char *)malloc(buffsz)) == 0 ){
        printf("Error allocating\n");
        return -1;
    }

    if( (buff2 = (char *)malloc(buff2sz)) == 0 ){
        printf("Error allocating\n");
        return -1;
    }

    if( (hFile = CreateFile("\\\\.\\ElectricEye",
                            GENERIC_READ, 0, NULL,
                            OPEN_EXISTING, 0, NULL)) == 0 ){
        printf("failed to open device\n");
        return -1;
    }

    ReadFile(hFile, buff, buffsz, &dwReturn, NULL);
    CloseHandle(hFile);
    /*
    if(dwReturn == 0){
        printf("Error reading:%i\n", (int) dwReturn);
        return -1;
    }
*/

    for(i = 0; i < buffsz; i += 4){
      buff2[k] = buff[i];
      buff2[k+1] = buff[i+1];
      buff2[k+2] = buff[i+2];
      k += 3;
    }
    printf("Writing\n");
    fd = fopen("./out.rgb","wb+");
    fwrite(buff2, sizeof(char), 3*buffsz/4, fd);

    fclose(fd);
    return 0;

}

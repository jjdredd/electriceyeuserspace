#include <stdio.h>
#include <windows.h>
#include <setupapi.h>
#include <devguid.h>
#include <regstr.h>
#include <ddk/cfgmgr32.h>

int main( int argc, char *argv[ ], char *envp[ ] ){
  HDEVINFO hDevInfo;
  SP_DEVINFO_DATA DeviceInfoData;
  DWORD i;
  GUID ClassGuid = {0x4d36e968, 0xe325, 0x11ce, {0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18} };

  LOG_CONF plc0;
  RES_DES rd, rd1;
  PMEM_RESOURCE  MemRes = 0;
  ULONG ResDataSize;
  DWORD FBPhysAddr = 0, FBSz = 0;
  CONFIGRET cfgret;

  // Create a HDEVINFO with all present devices.
  hDevInfo = SetupDiGetClassDevs(&ClassGuid, 0, // Enumerator
				 0, DIGCF_PRESENT);
  if (hDevInfo == INVALID_HANDLE_VALUE) return 1;
  // Enumerate through all devices in Set.
  DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
  for (i=0;SetupDiEnumDeviceInfo(hDevInfo,i,&DeviceInfoData);i++){
    //not needed vvvvvvv
    DWORD DataT;
    LPTSTR buffer = NULL;
    DWORD buffersize = 0;
    while (!SetupDiGetDeviceRegistryProperty(hDevInfo, &DeviceInfoData,
					     SPDRP_DEVICEDESC, &DataT,
					     (PBYTE)buffer, buffersize,
					     &buffersize)){
      if (GetLastError() == ERROR_INSUFFICIENT_BUFFER){
	if (buffer) LocalFree(buffer);
	buffer = (char *)LocalAlloc(LPTR,buffersize * 2);
      }
      else  break;
    }

    printf("Detected:[%s]\n",buffer);
    if (buffer) LocalFree(buffer);
    //not needed ^^^^

    if( ( cfgret = CM_Get_First_Log_Conf(&plc0, DeviceInfoData.DevInst,
			  BOOT_LOG_CONF) ) != CR_SUCCESS )
            printf("failed to get empty log conf:\t0x%x\n", cfgret); //BOOT_LOG_CONF BASIC_LOG_CONF???

    rd = (RES_DES)plc0;
    while( ( cfgret = CM_Get_Next_Res_Des(&rd1, rd, ResType_Mem, NULL, 0) )
                == CR_SUCCESS){
      CM_Get_Res_Des_Data_Size(&ResDataSize, rd1, 0);
      if(MemRes) LocalFree(MemRes);
      MemRes = (PMEM_RESOURCE)LocalAlloc(LPTR, ResDataSize);
      CM_Get_Res_Des_Data(rd1, MemRes, ResDataSize, 0);
/*
    printf("0x%x\t0x%x\n",MemRes->MEM_Header.MD_Alloc_End,
                MemRes->MEM_Header.MD_Alloc_Base);
*/
      if( FBSz
	  < (MemRes->MEM_Header.MD_Alloc_End
	     - MemRes->MEM_Header.MD_Alloc_Base)){
	FBPhysAddr = MemRes->MEM_Header.MD_Alloc_Base;
	FBSz = MemRes->MEM_Header.MD_Alloc_End
	  - MemRes->MEM_Header.MD_Alloc_Base;
      }
      CM_Free_Res_Des_Handle(rd);
      rd = rd1;
    }
  }
  printf("Best guess: FBPhysAddr = 0x%x\t FBSz = 0x%x\n", FBPhysAddr, FBSz);
  //printf("0x%x\n", cfgret);
  //  Cleanup
  LocalFree(MemRes);
  CM_Free_Res_Des_Handle(rd1);
  CM_Free_Log_Conf_Handle(plc0);
  SetupDiDestroyDeviceInfoList(hDevInfo);
  return 0;
}

// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             //  �� Windows ͷ�ļ����ų�����ʹ�õ���Ϣ

// Windows ͷ�ļ�:
#include <windows.h>
#include <winioctl.h>
#include <shlwapi.h>
#include <dbt.h>
#include <shlwapi.h>

// C ����ʱͷ�ļ�
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#define  FILE_ATTR_VOLUMEID 0x08

#define  HX_SECTOR_SIZE       512           //�����ֽ���
#define  HX_KERNEL_START      2             //HX�ں˴����ʼ������ַ
#define  HX_KERNEL_SIZE       (8+128+1120)  //HX�ں�ռ�������ռ�
#define  HX_HDD_RESERVED      204800        //HX�����ռ���������(U��ʹ�ã�
#define  HX_BOOTSEC_MPS       0x1BE         //����������һ����������ʼ��ַ

//����Ӳ������
#define  HX_VDISK_RESERVED            8000           //����Ӳ�̱����ռ���������
#define  HX_VDISK_SECTOR             (204800*2)     //����Ӳ������Ĭ������(200MB)
#define  HX_VDISK_CLINDER_PER_TRACK  255            //
#define  HX_VDISK_TRACK_PER_SECTOR   63
#define  HX_VDISK_REDUN_SIZE         (1024*1024*64) //������̿հ������С
    

//fat32��ض���
#define  HX_FAT32_SECPERCLUS  16            //fat32�ļ�ϵͳÿ����������
#define  HX_FAT32_FATSIZE     16145         //fat���С(��λ����)
#define  HX_FAT32_RESERVED    32            //fat32������������
#define  HX_FAT32_BLANK       100           //fat32ʣ����������


//�������̶���
#define  PROCESS_MAKE_BOOT	  1
#define  PROCESS_MAKE_KERNEL  2
#define  PROCESS_MAKE_FAT32   3 
#define  PROCESS_INIT_DEIVCE  4
#define  PROCESS_INIT_VHD	  5
#define  PROCESS_IMPORT_VHD	  6

//��������ֵ
#define  RET_COMPLETE		  100
#define  RET_USB_ERR         -100
#define  RET_BOOTSEC_ERR     -200
#define  RET_KERNEL_ERR      -300
#define  RET_FAT32_ERR       -400
#define  RET_IMPORT_ERR      -800



#define  FAT32_FMT_FLAGE       "FAT32 FORMAT OK" //�Զ����Fat32��ʽ����־����ֹ���¸�ʽ���Ա��ʡʱ��
#define  FAT32_FMT_SEC         2                 //Fat32��ʽ����־����λ��


#define  WM_MAKE_PROCESS   (WM_USER+1001)
#define  USB_NO_FOUND      L"û�з���USB�豸�����������USB�豸"


//g_nUsbSectors = (UINT64)pg.Cylinders.LowPart*(UINT64)pg.TracksPerCylinder*(UINT64)pg.SectorsPerTrack;
//�豸��Ϣ
typedef struct DISK_INFO
{
		UINT    nCylinders;
		UINT    nTracksPerCylinder;
		UINT    nSectorsPerTrack;
		UINT64  nTotalSector;
		UINT64  nTotalSize;

}__DISK_INFO;

typedef struct MAIN_PATION_INFO
{
	BYTE    bActiveFlage;
	BYTE    bStartClinder;
	BYTE    bStartTrack;
	BYTE    bStartSector;

	BYTE    bPationType;
	BYTE    bEndClinder;
	BYTE    bEndTrack;
	BYTE    bEndSector;

	UINT    nBeforSectors;
	UINT    nPationSectors;
	
}__MAIN__INFO;

typedef struct VHD_INFO 
{
	UINT64   cookie;
	UINT     Features;
	UINT     ffv;
	UINT64   offset;
	UINT     time;
	UINT     creat_app;
	UINT     creat_ver;
	UINT     creat_os;
	UINT64   orgin_size;
	UINT64   current_size;
	UINT     disk_geo;
	UINT     disk_type;
	UINT     checksum;
	CHAR     unique_id[16];
	CHAR     reserved[428];

}_VHD_INFO;


//�豸��ʽ������
BOOL APIENTRY Fat32Format(HANDLE  hDiskDrive,UINT nSectorCount,UINT nReserved);


//�õ�Ŀ¼�ܴ�С
UINT64 APIENTRY GetDirSize(LPCTSTR pSrcDir);

//�����ļ���Vhd�ļ�
DWORD APIENTRY ImportFilesToVhd(LPCTSTR pVhdFile,LPCTSTR pSrcDir,DWORD dwPartitionSatrt);


INT   APIENTRY GetModelPath(HMODULE hModul,LPTSTR pModelPath,INT nBufLen);
TCHAR APIENTRY GetDriveFromMask(UINT nMask) ; 
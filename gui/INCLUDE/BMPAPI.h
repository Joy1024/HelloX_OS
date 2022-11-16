//***********************************************************************/
//    Author                    : sh
//    Original Date             : 14,04 2009
//    Module Name               : BMPAPI.h
//    Module Funciton           : Declare the BMP struct  
//								  Declare 5 functions define in BMPAPI.cpp
//
//    Last modified Author      : sh
//    Last modified Date        : 18,04 2009
//    Last modified Content     :
//                                1.
//                                2.
//    Lines number              : 
//***********************************************************************/

#ifndef __BMPAPI_H_
#define __BMPAPI_H_

#ifndef __VESA_H__
#include "..\INCLUDE\VESA.H"
#endif

//Modified by garry at 2009.04.22.
#ifndef __VIDEO_H__
#include "..\INCLUDE\VIDEO.H"
#endif


//#define DATA_TYPE_INT


#ifdef DATA_TYPE_INT
typedef int IMGDATATYPE; 
#else
typedef BYTE IMGDATATYPE; 
#endif

//Revised by garry in 2009.04.22,replace the float point number 0.5 by
//multiplying 2 and then divided by 2.
#define ROUND(a)		(((a) < 0) ? (int)(((a) * 2 - 1)/2) : (int)(((a) * 2 + 1)/2)) 
#define BOUND(a, b, c)	((a) < (b) ? (b) : ((a) > (c) ? (c) : (a)))
#define  BytesPerLine(BmWidth, BmBitCnt)	(((BmBitCnt) * (BmWidth) + 31) / 32 * 4)
//#define RGB(r,g,b) ((((DWORD)r) << 16) + (((DWORD)g) << 8) + ((DWORD)b))


typedef struct 
{
	unsigned short	type;
	unsigned long	size;
	unsigned short	reserved1;
	unsigned short	reserved2;
	unsigned long	OffBits;
} BMPFILEHEADER; // 14 bytes   note: sizeof(BMPFILEHEADER) == 16  (padding 2 bytes) 

/************************************************************************************************
   
   BMP�ļ�ͷ˵��(һ��14���ֽ�)��

   type��			���ݵ�ַΪ0�������ڱ�־�ļ���ʽ����ֵΪ0x4d42(ʮ����19778)(���ַ�����BM��)��
					��ʾ��ͼ���ļ�ΪBMP�ļ���
   size��			���ݵ�ַΪ2����ָ�����BMP�ļ���С�����ֽ�Ϊ��λ��
   reservedl��		���ݵ�ַΪ6����BMP�ļ��ı����֣���ֵ����Ϊ0�� 
   reserved2��		���ݵ�ַΪ8����BMP�ļ��ı����֣���ֵ����Ϊ0�� 
   OffBits��		���ݵ�ַΪ10����Ϊ���ļ�ͷ��ʵ�ʵ�λͼ���ݵ�ƫ���ֽ�������ǰ������(BMP��
					��ͷ��BMP��Ϣͷ����ɫ��)�ĳ���֮�ͣ����ֽ�Ϊ��λ��

 ************************************************************************************************/


typedef struct 
{
	unsigned long	size;
	long			width;
	long			height;
	unsigned short	planes;
	unsigned short	BitCount;
	unsigned long	compression;
	unsigned long	SizeImage;
	long			XPelsPerMeter;
	long			YPelsPerMeter;
	unsigned long	ClrUsed;
	unsigned long	ClrImportant;
} BMPINFOHEADER; // 40 bytes

/************************************************************************************************
 
   BMP��Ϣͷ˵��(һ��40���ֽ�)��

   size��			���ݵ�ַΪ14����ָ��λͼ��Ϣͷ�ĳ��ȣ���ֵΪ40��
   width��			���ݵ�ַΪ18����ָ��ͼ��Ŀ�ȣ���λ�����ء�  
�� height��			���ݵ�ַΪ22����ָ��ͼ��ĸ߶ȣ���λ�����ء���height��ȡֵΪ������ �����
					λͼΪbottom��up���͵�DIBλͼ������λͼԭ��Ϊ���½ǡ���height��ȡֵΪ ������
					�����λͼΪtop��down���͵�DIBλͼ������λͼԭ��Ϊ���Ͻǡ���һ��λͼ�����У�
					height�ֶε�ȡֵΪ������
   planes��			���ݵ�ַΪ26��������Ŀ���豸�ļ��𣬱���Ϊ1��
   BitCount��		���ݵ�ַΪ28����ȷ��ÿ����������Ҫ��λ����ֵΪ1��ʾ�ڰ׶�ɫͼ��ֵ4Ϊ��ʾ16
					ɫͼ��ֵ8Ϊ��ʾ256ɫͼ��ֵΪ24��ʾ���ɫͼ��
   compression��	���ݵ�ַΪ30��������bottom��up����λͼ��ѹ������(ע�⣺bottom��down����λͼ
					���ܽ���ѹ������)�������ȡֵ���京��ֱ�Ϊ�������ֶε�ȡֵΪBI��RGB�����ʾ
					�ļ��ڵ�ͼ������û�о���ѹ�����������ֶε�ȡֵΪBI��RLE8�����ʾ��ѹ����ͼ
					��������256ɫ�����õ�ѹ��������RLE8�������ֶε�ȡֵΪBI��RLE4�����ʾ��ѹ��
					��ͼ��������16ɫ�����õ�ѹ��������RLE4�������ֶε�ȡֵΪBI��BITFIELDS�����
					��ͼ���ļ��ڵ�����û�о���ѹ������������ɫ���ɷֱ��ʾÿ�����ص�ĺ졢�̡�
					����ԭɫ��˫����ɡ�BMP�ļ���ʽ�ڴ���ɫ�������ɫͼ��ʱ������ͼ�����ݶ�ô
					�Ӵ󣬶�����ͼ�����ݽ����κ�ѹ������
   Sizelmage��		���ݵ�ַΪ34����������BMP��ͼ������ռ�õĿռ��С����ͼ���ļ�����BI��RGBλͼ��
					����ֶε�ֵ��������Ϊ0��
�� XPelsPerMeter��	���ݵ�ַΪ38����ÿ��������Ϊ��λ������λͼĿ���豸ˮƽ�Լ���ֱ����ķֱ��ʡ�
					Ӧ�ó�����Ը���XPelsPerMeter�ֶε�ֵ����Դλͼ����ѡ���뵱ǰ�豸�ص���ƥ��
					��λͼ��
�� YPelsPerMeter��	���ݵ�ַΪ42����ÿ��������Ϊ��λ������λͼĿ���豸ˮƽ�Լ���ֱ����ķֱ��ʡ�
					Ӧ�ó�����Ը���YPelsPerMeter�ֶε�ֵ����Դλͼ����ѡ���뵱ǰ�豸�ص���ƥ��
					��λͼ��
   ClrUsed��		���ݵ�ַΪ46��������λͼʵ��ʹ�õ���ɫ���е���ɫ��ַ����������ֶε�ȡֵΪ0��
					�����λͼ�õ�����ɫΪ2��BitCount���ݣ�����BitCount�ֶε�ȡֵ��compression
					��ָ����ѹ��������ء����磺���ͼ��Ϊ16ɫ�������ֶε�ȡֵΪ10�������λͼ
					��ʹ����12����ɫ��������ֶε�ȡֵ���㣬����BitCount�ֶε�ȡֵС��16�������
					��ָ��ͼ������豸��������ȡ��ʵ����ɫ������biBitCount�ֶε�ȡֵ���ڻ��ߵ���
					16������ֶ�ָ��ʹWindow ϵͳ��ɫ��ﵽ�������ܵ���ɫ���С��
�� Clrlmportant��	���ݵ�ַΪ50��������λͼ��ʾ��������Ҫ��ɫ�ı�ַ���������ֶε�ȡֵΪ0�����ʾ
					����ʹ�õ���ɫ������Ҫ��ɫ�� 

 ************************************************************************************************/


typedef struct 
{
	BMPFILEHEADER BmpFileHeader;
	BMPINFOHEADER BmpInfoHeader;
} BMPHEADER;

/************************************************************************************************
   
	 BMPͷ�����ļ�ͷ����Ϣͷ��һ��54���ֽڡ� 

 ************************************************************************************************/


typedef struct 
{
	unsigned char	blue;
	unsigned char	green;
	unsigned char	red;
	unsigned char	reserved;
} hcRGBQUAD;

/************************************************************************/
/*	 �ڴ�˵����BMPͷ��Щ��Ϣ��û����ã����ϵĽṹ��ֻΪ����׼BMP��д����չ֮��.
	 ʵ�ʵ����õ���Ϣ�洢�������BMPIMAGE�У��������ڲ���                                                                      */
/************************************************************************/


typedef struct 
{
	//int CompNum;
	int height;
	int width;
	int BitCount;//λ���
	BYTE *ColorMap;//��ɫ��
	BYTE *ColorIndex;//λ���<=8��BMP������
	int IndexNum;//��Ϊλ���<8��BMP����ֵ����1byte������Ҫ����ͼ�������ֽ���
	IMGDATATYPE *DataB, *DataG, *DataR;//������ʾ����ɫ����ֵ��bpp<=8ʱ��ΪNULL
} BMPIMAGE;


//#ifdef __cplusplus
//#if __cplusplus
//extern "C"{
//#endif
//#endif /* __cplusplus */ 
	//remember fopen and fclose to operator FILE *

	/*extern BMPIMAGE *BmpRead(FILE* InBmp);
	extern void BmpWrite(FILE* OutBmp, BMPIMAGE *image);
	extern BMPIMAGE* ImageAlloc(int height, int width);//, int CompNum);
	extern void ImageDealloc(BMPIMAGE* image);	
	extern void BmpShow(__VIDEO *pVideo, int x, int y, BMPIMAGE *image);
	
	//other functions
	extern void Bmp2Txt(BMPIMAGE *image, FILE *OutTxt);
	extern void Get16Num(unsigned char *orin, char *str);*/
	void BmpShowArray(__VIDEO* pVideo, int x, int y, int height, int width, BYTE *DataB, BYTE *DataG, BYTE *DataR);
	//data declare
	extern char NUM16[16];
	extern BYTE DataBlue[196];
	extern BYTE DataGreen[196];
	extern BYTE DataRed[196];

	//extern __VIDEO Video;
//#ifdef __cplusplus
//#if __cplusplus
//}
//#endif
//#endif /* __cplusplus */ 



#endif
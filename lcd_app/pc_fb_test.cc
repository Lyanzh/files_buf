#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/ioctl.h>

int main (int argc, char **argv)
{
	int fp=0;
	struct fb_var_screeninfo vinfo;
	struct fb_fix_screeninfo finfo;

	char *fbp;
	int screensize;
	int x, y;
	int location;
	
	fp = open ("/dev/fb0", O_RDWR);
	if (fp < 0){
		printf("Error : Can not open framebuffer device/n");
		exit(1);
	}


	if (ioctl(fp,FBIOGET_FSCREENINFO,&finfo)){
		printf("Error reading fixed information/n");
		exit(2);
	}

	if (ioctl(fp,FBIOGET_VSCREENINFO,&vinfo)){
		printf("Error reading variable information/n");
		exit(3);
	}


	printf("The mem is :0x%x\n",finfo.smem_len);
	printf("The line_length is :%d\n",finfo.line_length);
	printf("The xres is :%d\n",vinfo.xres);
	printf("The yres is :%d\n",vinfo.yres);
	printf("bits_per_pixel is :%d\n",vinfo.bits_per_pixel);

	screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
	/*����ǰ�fp��ָ���ļ��дӿ�ʼ��screensize��С�����ݸ�ӳ��������õ�һ��ָ�����ռ��ָ��*/
	fbp =(char *) mmap (0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
	if ((int) fbp == -1)
	{
		printf ("Error: failed to map framebuffer device to memory./n");
		exit (4);
	}
	/*�������뻭�ĵ��λ������,(0��0)������Ļ���Ͻ�*/
	for (y = 0; y < 100; y++) {
		for (x = 0; x < 100; x++) {
			location = x * (vinfo.bits_per_pixel / 8) + y  *  finfo.line_length;

			*(fbp + location) = 100;  /* ��ɫ��ɫ�� */  /*ֱ�Ӹ�ֵ���ı���Ļ��ĳ�����ɫ*/
			*(fbp + location + 1) = 15; /* ��ɫ��ɫ��*/
			*(fbp + location + 2) = 200; /* ��ɫ��ɫ��*/
			*(fbp + location + 3) = 0;  /* �Ƿ�͸��*/
		}
	}
	
	munmap (fbp, screensize); /*���ӳ��*/

	close (fp);
}


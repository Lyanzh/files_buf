#include <stdio.h>
#include <malloc.h>

typedef unsigned char TUInt8; // [0..255]

struct TARGB32      //32 bit color
{
    TUInt8  B,G,R,A;          // A is alpha
};

struct TPicRegion  //一块颜色数据区的描述，便于参数传递
{
    TARGB32*    pdata;         //颜色数据首地址
    long        byte_width;    //一行数据的物理宽度(字节宽度)；
                //abs(byte_width)有可能大于等于width*sizeof(TARGB32);
    long        width;         //像素宽度
    long        height;        //像素高度
};

//那么访问一个点的函数可以写为：
inline TARGB32& Pixels(const TPicRegion& pic, const long x, const long y)
{
    return ( (TARGB32*)((TUInt8*)pic.pdata+pic.byte_width*y) )[x];
}

void PicZoom0(const TPicRegion& Dst, const TPicRegion& Src)
{
	unsigned long x = 0;
	unsigned long y = 0;
	unsigned long srcx;
	unsigned long srcy;

	float ratex;
	float ratey;

	if ((0 == Dst.width) || (0 == Dst.height) ||
		(0 == Src.width) || (0 == Src.height))
		return;

	ratex = Src.width / Dst.width;
	ratey = Src.height / Dst.height; 

	for (y = 0; y < Dst.height; ++y)
	{
		srcy = (unsigned long)(y * ratey);
		for (x = 0; x < Dst.width; ++x)
		{
			srcx = (unsigned long)(x * ratex);
			Pixels(Dst, x, y) = Pixels(Src, srcx, srcy);
		}
	}
}

void PicZoom3_Table(const TPicRegion& Dst,const TPicRegion& Src)
{
	unsigned long x = 0;
	unsigned long y = 0;
	unsigned long srcx;
	unsigned long srcy;

	unsigned long dst_width;
	unsigned long* SrcX_Table;
	
	if ((0 == Dst.width) || (0 == Dst.height) ||
		(0 == Src.width) || (0 == Src.height))
		return;
	
	dst_width = Dst.width;
	SrcX_Table = (unsigned long*)malloc(sizeof(unsigned long) * dst_width);//new unsigned long[dst_width];
    for (x = 0; x < dst_width; ++x)//生成表 SrcX_Table
    {
        SrcX_Table[x] = (x * Src.width / Dst.width);
    }

    TARGB32* pDstLine = Dst.pdata;
    for (y = 0; y < Dst.height; ++y)
    {
        srcy = (y * Src.height / Dst.height);
        TARGB32* pSrcLine = ((TARGB32*)((TUInt8*)Src.pdata + Src.byte_width * srcy));
        
        for (x = 0; x < dst_width; ++x)
            pDstLine[x] = pSrcLine[SrcX_Table[x]];
        
        ((TUInt8*&)pDstLine) += Dst.byte_width;
    }

    free(SrcX_Table);//delete [] SrcX_Table;
}

int main(int argc, char **argv)
{
	return 0;
}


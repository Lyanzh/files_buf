看看以下代码的打印值：

	#include<stdio.h>
	typedef struct _SoftArray{
	    int len;
	    int array[];
	}SoftArray;
	
	int main()
	{
	    int len = 10;
	
	    printf("The struct's size is %d\n",sizeof(SoftArray));
	}

![](https://i.imgur.com/6PVebep.png)

我们可以看出，_SoftArray结构体的大小是4，显然，在32位操作系统下一个int型变量大小刚好为4，也就说结构体中的数组没有占用内存。  
这就是我们常说的动态数组，也就是柔性数组。  
让我们再看一段代码：

	#include<stdio.h>
	#include<malloc.h>
	
	typedef struct _SoftArray{
	    int len;
	    int array[];
	}SoftArray;
	
	int main()
	{
	    int len = 10;
	
	    SoftArray *p=(SoftArray*)malloc(sizeof(SoftArray) + sizeof(int)*len);
	    printf("After the malloc function the struct's size is %d\n”,sizeof(SoftArray));
	
	    return 0;
	}

![](https://i.imgur.com/GKofn5U.png)

申请了内存后结构体大小还是4，原因是动态申请的内存只是申请给数组拓展所用，从上个程序我们可以看出结构体的大小在创建时已经确定了，array明确来说不算是结构体成员，只是挂羊头卖狗肉而已。

关于柔性数组

1、什么是柔性数组？

柔性数组既数组大小待定的数组，C语言中结构体的最后一个元素可以是大小未知的数组，也就是所谓的0长度，所以我们可以用结构体来创建柔性数组。

2、柔性数组有什么用途？

它的主要用途是为了满足需要变长度的结构体，为了解决使用数组时内存的冗余和数组的越界问题。

3、用法：在一个结构体的最后，申明一个长度为空的数组，就可以使得这个结构体是可变长的。对于编译器来说，此时长度为0的数组并不占用空间，因为数组名本身不占空间，它只是一个偏移量，数组名这个符号本身代表了一个不可修改的地址常量，但对于这个数组的大小，我们可以进行动态分配,对于编译器而言，数组名仅仅是一个符号，它不会占用任何空间，它在结构体中，只是代表了一个偏移量，代表一个不可修改的地址常量！

对于柔性数组的这个特点，很容易构造出变成结构体，如缓冲区，数据包等等：

	typedef struct _SoftArray
	{
	    Int len;
	    int array[];
	}SoftArray;

这样的变长数组常用于网络通信中构造不定长数据包，不会浪费空间浪费网络流量，比如我要发送1024字节的数据，如果用定长包，假设定长包的长度为2048，就会浪费1024个字节的空间，也会造成不必要的流量浪费。

4、举个简单是实例

	#include<stdio.h>
	#include<malloc.h>
	typedef struct _SoftArray{
		int len;
		int array[];
	}SoftArray;
	
	int main()
	{
	    int len=10,i=0;
	    
	    SoftArray *p=(SoftArray*)malloc(sizeof(SoftArray)+sizeof(int)*len);
	    p->len=len;
	    
	    for(i=0;i<p->len;i++)
	    {
	        p->array[i]=i+1;
	    }
	    for(i=0;i<p->len;i++)
	    {
	        printf("%d\n",p->array[i]);
	    }
	
	    free(p);
	
	    return 0;
	}

这代码的作用是用柔性数组动态创建数组并输出数组内容。
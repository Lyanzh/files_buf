SPI总线协议

一、技术性能

SPI接口是Motorola 首先提出的全双工三线同步串行外围接口，采用主从模式（Master Slave）架构；支持多slave模式应用，一般仅支持单Master。时钟由Master控制，在时钟移位脉冲下，**数据按位传输，高位在前，低位在后（MSB first）**；SPI接口有2根单向数据线，为全双工通信，目前应用中的数据速率可达几Mbps的水平。

总线结构如下图所示：

![](https://i.imgur.com/gZ4j5Mz.png)

二、接口定义

SPI接口共有4根信号线，分别是：设备选择线、时钟线、串行输出数据线、串行输入数据线。

（1）MOSI：主器件数据输出，从器件数据输入  
（2）MISO：主器件数据输入，从器件数据输出  
（3）SCLK ：时钟信号，由主器件产生  
（4）/SS：从器件使能信号，由主器件控制  

![](https://i.imgur.com/XAG1Dmi.png)

三、时钟极性和时钟相位

在SPI操作中**，最重要的两项设置就是时钟极性（CPOL或UCCKPL）和时钟相位（CPHA或UCCKPH）**。**时钟极性设置时钟空闲时的电平，时钟相位设置读取数据和发送数据的时钟沿**。

主机和从机的发送数据是同时完成的，两者的接收数据也是同时完成的。所以为了保证主从机正确通信，**应使得它们的SPI具有相同的时钟极性和时钟相位**。

SPI接口时钟配置心得：在主设备这边配置SPI接口时钟的时候一定要弄清楚从设备的时钟要求，因为主设备这边的时钟极性和相位都是以从设备为基准的。因此**在时钟极性的配置上一定要搞清楚从设备是在时钟的上升沿还是下降沿接收数据，是在时钟的下降沿还是上升沿输出数据**。

四、传输时序

SPI接口在内部硬件实际上是两个简单的移位寄存器,传输的数据为8位，在主器件产生的从器件使能信号和移位脉冲下，按位传输，高位在前，低位在后。如下图所示，在SCLK的下降沿上数据改变，上升沿一位数据被存入移位寄存器。

![](https://i.imgur.com/fjNdbOF.png)

五、数据传输

在一个SPI时钟周期内，会完成如下操作：  
1) 主机通过MOSI线发送1位数据，从机通过该线读取这1位数据；  
2) 从机通过MISO线发送1位数据，主机通过该线读取这1位数据。  

这是通过移位寄存器来实现的。如下图所示，主机和从机各有一个移位寄存器，且二者连接成环。随着时钟脉冲，数据按照从高位到低位的方式依次移出主机寄存器和从机寄存器，并且依次移入从机寄存器和主机寄存器。当寄存器中的内容全部移出时，相当于完成了两个寄存器内容的交换。

![](https://i.imgur.com/0VbXMqe.png)

六、优缺点

SPI接口具有如下优点：  
1) 支持全双工操作；  
2) 操作简单；  
3) 数据传输速率较高。

同时，它也具有如下缺点：  
1) 需要占用主机较多的口线（每个从机都需要一根片选线）；  
2) 只支持单个主机；  
3) 没有指定的流控制，没有应答机制确认是否接收到数据。  

七、代码示例

（软件模拟SPI）

	#include "spi.h"
	
	/************************************************
	函数名称 ： SPI_Delay
	功    能 ： SPI延时
	参    数 ： 无
	返 回 值 ： 无
	*************************************************/
	void SPI_Delay(void)
	{
		u16 cnt = 5;
	
		while(cnt--);
	}
	
	/************************************************
	函数名称 ： SPI_GPIO_Configuration
	功    能 ： SPI引脚配置
	参    数 ： 无
	返 回 值 ： 无
	*************************************************/
	void SPI_GPIO_Configuration(void)
	{
		GPIO_InitTypeDef GPIO_InitStructure;
	
		RCC_APB2PeriphClockCmd(SPI_GPIO_CLK, ENABLE);
	
		/* CS */
		GPIO_InitStructure.GPIO_Pin = PIN_SPI_CS;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(PORT_SPI_CS, &GPIO_InitStructure);
	
		/* SCK */
		GPIO_InitStructure.GPIO_Pin = PIN_SPI_SCK;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(PORT_SPI_SCK, &GPIO_InitStructure);
	
		/* MISO */
		GPIO_InitStructure.GPIO_Pin = PIN_SPI_MISO;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(PORT_SPI_MISO, &GPIO_InitStructure);
	
		/* MOSI */
		GPIO_InitStructure.GPIO_Pin = PIN_SPI_MOSI;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
		GPIO_Init(PORT_SPI_MOSI, &GPIO_InitStructure);
	}
	
	/************************************************
	函数名称 ： SPI_Initializes
	功    能 ： SPI初始化
	参    数 ： 无
	返 回 值 ： 无
	*************************************************/
	void SPI1_Init(void)
	{
		SPI_GPIO_Configuration();
	
		//SPI_CS_DISABLE;
		SPI_SCK_LOW;
		SPI_MOSI_HIGH;
	}
	
	/************************************************
	函数名称 ： SPI1_ReadWriteByte
	功    能 ： SPI读写一字节数据
	参    数 ： TxData --- 发送的字节数据
	返 回 值 ： 读回来的字节数据
	*************************************************/
	/*
	 * 注意:
	 * ENC28J60在时钟上升沿收数据(即是主机发数据)，
	 * 在时钟下降沿发数据(即是主机收数据)
	 */
	u8 SPI1_ReadWriteByte(u8 TxData)
	{
		u8 cnt, RxData = 0;
		for(cnt = 0; cnt < 8; cnt++)
		{
			SPI_SCK_LOW;			//拉低时钟
			if(TxData & 0x80)
				SPI_MOSI_HIGH;		//若最到位为高，则输出高
			else
				SPI_MOSI_LOW;		//若最到位为低，则输出低
			TxData <<= 1; 			// 低一位移位到最高位
			SPI_SCK_HIGH;			//拉高时钟，主机在时钟上升沿发数据
			
			RxData <<= 1; 			//数据左移
			if(SPI_MISO_READ)
				RxData |= 0x01;		//从从机接收到高电平
			SPI_SCK_LOW;			//拉低时钟，主机在时钟下降沿收数据
		}
		return (RxData);			//返回数据
	}

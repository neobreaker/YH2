#include "stm32f10x.h"
#include "stm32f10x_spi.h"		
#include "w5500_port.h"	

void SPI_Configuration(void)
{
	GPIO_InitTypeDef 	GPIO_InitStructure;
	SPI_InitTypeDef   	SPI_InitStructure;	   

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);	

	/* ��ʼ��SCK��MISO��MOSI���� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

	/* ��ʼ��CS���� */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);

	/* ��ʼ������STM32 SPI2 */
	SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;	//SPI����Ϊ˫��˫��ȫ˫��
	SPI_InitStructure.SPI_Mode=SPI_Mode_Master;							//����Ϊ��SPI
	SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;						//SPI���ͽ���8λ֡�ṹ
	SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;							//ʱ�����յ�
	SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;							//���ݲ����ڵ�1��ʱ����
	SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;								//NSS���ⲿ�ܽŹ���
	SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;	//������Ԥ��ƵֵΪ2
	SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;					//���ݴ����MSBλ��ʼ
	SPI_InitStructure.SPI_CRCPolynomial=7;								//CRC����ʽΪ7
	SPI_Init(SPI2,&SPI_InitStructure);									//����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI1�Ĵ���

	SPI_Cmd(SPI2,ENABLE);	//STM32ʹ��SPI1
}

void W5500_GPIO_Configuration(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG , ENABLE);	
	/* W5500_RST���ų�ʼ������*/
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOG, GPIO_Pin_6);

	SPI_Configuration();
}

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI2_ReadWriteByte(u8 TxData)
{
    u8 retry=0;
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
    {
        retry++;
        if(retry>200)return 0;
    }
    SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ������
    retry=0;

    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
    {
        retry++;
        if(retry>200)return 0;
    }
    return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����
}

void SPI2_WriteShort(unsigned short dat)
{
	u8 *p = (u8*)&dat;
	SPI2_ReadWriteByte(*(p+1));		//д���ݸ�λ
	SPI2_ReadWriteByte(*p);			//д���ݵ�λ
}

void w5500_rst(u8 stat)
{
    PGout(6) = stat;
}

void w5500_scs(u8 stat)
{
    PBout(12) = stat;
}

void w5500_cfg_setup(w5500_cfg_t* cfg)
{
	cfg->W5500_Init 				= W5500_GPIO_Configuration;
	cfg->W5500_SPI_ReadWriteByte 	= SPI2_ReadWriteByte;
	cfg->W5500_SPI_WriteShort		= SPI2_WriteShort;
	cfg->W5500_RST					= w5500_rst;
	cfg->W5500_SCS					= w5500_scs;

	if(cfg->W5500_Init)
		cfg->W5500_Init();
}


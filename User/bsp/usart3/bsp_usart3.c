/**
  ******************************************************************************
  * @file    bsp_usart.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   �ض���c��printf������usart�˿�
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ��STM32 F103-�Ե� ������  
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */ 
	
#include "bsp_usart3.h"
#include "rtthread.h"
#include "string.h"
extern struct rt_event DHT11_data_event;
#define EVENT_FLAG8 (1 << 8)

/* �ⲿ�����ź������ƿ� */
//extern rt_sem_t test_sem3;
rt_sem_t test_sem3;
 /**
  * @brief  ����Ƕ�������жϿ�����NVIC
  * @param  ��
  * @retval ��
  */
 u8 Res;
 u16 USART_RX_STA=0;       //�жϡ���ǡ�
 u8 USART_RX_BUF[200];     //���ջ���,���USART_REC_LEN���ֽ�.
static void NVIC_Configuration2(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  
  /* Ƕ�������жϿ�������ѡ�� */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
  
  /* ����USARTΪ�ж�Դ */
  NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART3_IRQ;
  /* �������ȼ�*/
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
  /* �����ȼ� */
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;
  /* ʹ���ж� */
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  /* ��ʼ������NVIC */
  NVIC_Init(&NVIC_InitStructure);
}

 /**
  * @brief  USART GPIO ����,������������
  * @param  ��
  * @retval ��
  */
void USART3_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	
	

	// �򿪴���GPIO��ʱ��
	DEBUG_USART3_GPIO_APBxClkCmd(DEBUG_USART3_GPIO_CLK, ENABLE);
	
	// �򿪴��������ʱ��
	DEBUG_USART3_APBxClkCmd(DEBUG_USART3_CLK, ENABLE);

	// ��USART Tx��GPIO����Ϊ���츴��ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART3_TX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(DEBUG_USART3_TX_GPIO_PORT, &GPIO_InitStructure);

  // ��USART Rx��GPIO����Ϊ��������ģʽ
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART3_RX_GPIO_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(DEBUG_USART3_RX_GPIO_PORT, &GPIO_InitStructure);
	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;				 //
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);					 //
	GPIO_SetBits(GPIOB,GPIO_Pin_1);                  //485EN
	
	
	
	// ���ô��ڵĹ�������
	// ���ò�����
	USART_InitStructure.USART_BaudRate = DEBUG_USART3_BAUDRATE;
	// ���� �������ֳ�
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	// ����ֹͣλ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	// ����У��λ
	USART_InitStructure.USART_Parity = USART_Parity_No ;
	// ����Ӳ��������
	USART_InitStructure.USART_HardwareFlowControl = 
	USART_HardwareFlowControl_None;
	// ���ù���ģʽ���շ�һ��
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	// ��ɴ��ڵĳ�ʼ������
	USART_Init(DEBUG_USART3, &USART_InitStructure);
	// �����ж����ȼ�����
	NVIC_Configuration2();
	// ���� ���ڿ���IDEL �ж�
	USART_ITConfig(DEBUG_USART3, USART_IT_IDLE, ENABLE);  
  // ��������DMA����
	//USART_DMACmd(DEBUG_USART3, USART_DMAReq_Rx, ENABLE); 
	// ʹ�ܴ���
	USART_Cmd(DEBUG_USART3, ENABLE);	    
}

char Usart3_Rx_Buf[USART3_RBUFF_SIZE];

//���ڸ㶨����2��DMA����  ����Ϊʲô����1�Ͳ���Ҫ�ж�������  �ܷѽ⣡������
void USART3_IRQHandler(void)
{
	
    if(USART_GetITStatus(USART3, USART_IT_IDLE) != RESET)  // �����ж�
	Res =USART_ReceiveData(USART3);	//��ȡ���յ�������]
	//�����¼�
	 rt_event_send(& DHT11_data_event, EVENT_FLAG8);
	

	//rt_kprintf("res :%d\n",Res);
	//LCD_ShowNum(10, 230,Res, 5, 16);
    /*if((USART_RX_STA&0x8000)==0)//����δ���
			{
			if(USART_RX_STA&0x4000)//���յ���0x0d
				{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
				}
			else //��û�յ�0X0D
				{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
					{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(200-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
					}		 
				}
			} */

    /*{
        Uart3_DMA_Rx_Data();

        USART_ReceiveData( USART3 ); // Clear IDLE interrupt flag bit
    }*/
}

/*
void Uart3_DMA_Rx_Data(void)
{
   // �ر�DMA ����ֹ����
   DMA_Cmd(USART3_RX_DMA_CHANNEL, DISABLE);      
   // ��DMA��־λ
	 DMA_ClearFlag( DMA1_FLAG_GL3 ); 
   ///DMA_ClearFlag( DMA1_FLAG_GL5 );          
   //  ���¸�ֵ����ֵ��������ڵ��������ܽ��յ�������֡��Ŀ
   USART3_RX_DMA_CHANNEL->CNDTR = USART3_RBUFF_SIZE;    
   DMA_Cmd(USART3_RX_DMA_CHANNEL, ENABLE);       
   //������ֵ�ź��� �����ͽ��յ������ݱ�־����ǰ̨�����ѯ
   rt_sem_release(test_sem3);  	
  /* 
    DMA �������ȴ����ݡ�ע�⣬����жϷ�������֡�����ʺܿ죬MCU����������˴ν��յ������ݣ�
    �ж��ַ������ݵĻ������ﲻ�ܿ������������ݻᱻ���ǡ���2�ַ�ʽ�����

    1. �����¿�������DMAͨ��֮ǰ����LumMod_Rx_Buf��������������ݸ��Ƶ�����һ�������У�
    Ȼ���ٿ���DMA��Ȼ�����ϴ����Ƴ��������ݡ�

    2. ����˫���壬��LumMod_Uart_DMA_Rx_Data�����У���������DMA_MemoryBaseAddr �Ļ�������ַ��
    ��ô�´ν��յ������ݾͻᱣ�浽�µĻ������У������ڱ����ǡ�
  
}*/


/*****************  ����һ���ֽ� **********************/
void Usart3_SendByte( USART_TypeDef * pUSARTx, uint8_t ch)
{
	/* ����һ���ֽ����ݵ�USART */
	USART_SendData(pUSARTx,ch);
		
	/* �ȴ��������ݼĴ���Ϊ�� */
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}

/*******************************************************************************  
* �����ֽ� 
*******************************************************************************/    
void uart3_send_char(u8 temp)      
{        
    USART_SendData(USART3,(u8)temp);        
    while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET); //USART_FLAG_TXE �������ݼĴ����ձ�־λ        
}    

/*******************************************************************************  
* �����ַ���  
*******************************************************************************/    
void uart3_send_buff(u8 *buf,u8 len)     
{    
    u8 t;    
    for(t=0;t<len;t++){    
    USART_SendData(USART3,buf[t]);
	while(USART_GetFlagStatus(USART3,USART_FLAG_TXE)==RESET); //USART_FLAG_TXE �������ݼĴ����ձ�־λ
		}			
}
/****************** ����8λ������ ************************/
void Usart3_SendArray( USART_TypeDef * pUSARTx, uint8_t *array, uint16_t num)
{
  uint8_t i;
	
	for(i=0; i<num; i++)
  {
	    /* ����һ���ֽ����ݵ�USART */
	    Usart3_SendByte(pUSARTx,array[i]);	
  
  }
	/* �ȴ�������� */
	while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET);
}

/*****************  �����ַ��� **********************/
void Usart3_SendString( USART_TypeDef * pUSARTx, char *str)
{
	unsigned int k=0;
  do 
  {
      Usart3_SendByte( pUSARTx, *(str + k) );
      k++;
  } while(*(str + k)!='\0');
  
  /* �ȴ�������� */
  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
  {}
}

/*****************  ����һ��16λ�� **********************/
void Usart3_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;
	
	/* ȡ���߰�λ */
	temp_h = (ch&0XFF00)>>8;
	/* ȡ���Ͱ�λ */
	temp_l = ch&0XFF;
	
	/* ���͸߰�λ */
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);
	
	/* ���͵Ͱ�λ */
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET);	
}


/***********************************************************
�������ƣ�int RS485_SendCmd(u8 *cmd,u8 len,int x,int wait)
�������ܣ�RS485��ѯ�뷵��ָ��
��ڲ�����cmd����ѯָ��
         len�����ݳ���
           x��������ʼλ
        wait����ѯ��ʱ
���ڲ��������ݵ�
�� ע��
rt_kprintf("[NBiot_SendCmd] %s\r\n", cmd);
  rt_thread_delay(100);   ��ʱ100��tick 	
***********************************************************/
 /*
int RS485_SendCmd(u8 *cmd,u8 len,int max_len,int x,int wait)
{   
  int Val = 0; 
	unsigned short CRC_Tmp;
	unsigned short crc;
	memset(Usart3_Rx_Buf,0,USART3_RBUFF_SIZE);//����ջ�����
  //rt_kprintf("[RS485_SendCmd] %d\r\n",len);
	GPIO_SetBits(GPIOB,GPIO_Pin_1);
	rt_thread_delay(2);
  uart3_send_buff(cmd, len);
	rt_thread_delay(2);//������ʱһ�£�ԭ��ȥ�鿴sp3485���ֲ��
	GPIO_ResetBits(GPIOB, GPIO_Pin_1);//
  rt_thread_delay(wait);
 if (strcmp(Usart3_Rx_Buf, "")!= 0) //����ֵ��Ϊ��if (strcmp(Usart2_Rx_Buf, "") == 0) //����ֵΪ��
  {			
//      for(i=0;i<len+1;i++)  //��ӡ�������յİ���9������
//      {
//      rt_kprintf("%X@",Usart3_Rx_Buf[i]);	
//      }		
//			rt_kprintf("��������2; %d\r\n",strlen(Usart3_Rx_Buf));
		  GPIO_SetBits(GPIOB,GPIO_Pin_1);
		  rt_thread_delay(2);//������ʱһ�£�ԭ��ȥ�鿴sp3485���ֲ��
			crc = ((unsigned short)Usart3_Rx_Buf[max_len-2]<<8) +(unsigned short)Usart3_Rx_Buf[max_len-1]; //�յ����ݵ�crcУ��ֵ
			CRC_Tmp = CRC_16_HEX((unsigned char *)Usart3_Rx_Buf,(unsigned char)(max_len-2)); //�����ȥ�����λ������CRCУ�飬���crcУ��ֵ
//			rt_kprintf("%X\r\n",crc);
//			rt_kprintf("%X\r\n",CRC_Tmp);
			if (CRC_Tmp == crc){   //�Ƚ�CRCУ��ֵ�Ƿ���ȣ�����������һ������
      Val = (Usart3_Rx_Buf[x]*256) + (Usart3_Rx_Buf[x+1]*1);
			return Val;
			}
			return Val;
    }  
}




*/
////�ض���c�⺯��printf�����ڣ��ض�����ʹ��printf����
//int fputc(int ch, FILE *f)
//{
//		/* ����һ���ֽ����ݵ����� */
//		USART_SendData(DEBUG_USARTx, (uint8_t) ch);
//		
//		/* �ȴ�������� */
//		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_TXE) == RESET);		
//	
//		return (ch);
//}

/////�ض���c�⺯��scanf�����ڣ���д����ʹ��scanf��getchar�Ⱥ���
//int fgetc(FILE *f)
//{
//		/* �ȴ������������� */
//		while (USART_GetFlagStatus(DEBUG_USARTx, USART_FLAG_RXNE) == RESET);

//		return (int)USART_ReceiveData(DEBUG_USARTx);
//}


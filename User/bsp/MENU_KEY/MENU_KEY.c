#include "MENU_KEY.h"
#include "delay.h"
#include <rtthread.h>


 u8 choice=1;
 u8 c_state=NO;
 int adjust=0;
 int result3;
#define EVENT_FLAG6 (1 << 6)  //IO���жϴ����¼�
#define EVENT_FLAG7 (1 << 7)  //�޸�sampling �¼�
extern struct rt_event memlcd_data_event;
extern u8 samp_set,humset,temset;
extern  uint8_t tem,hum,tem_flag,hum_flag;
extern struct rt_event changesamp_event;
extern  uint16_t samp_freq;
//�ⲿ�ж�0�������
void K_MENU_Init(void)
{
 //�ṹ��init 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
//GPIO init
    GPIO_MENU_Init();	 //	�����˿ڳ�ʼ��
//ʱ�Ӵ�
  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��
//mode choice
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
//GPIOC.2 �ж����Լ��жϳ�ʼ������   �½��ش���
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource2);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line2;	//KEY2
  	EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

//GPIOC.3	  �ж����Լ��жϳ�ʼ������ �½��ش��� //KEY1
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource3);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line3;
  	EXTI_Init(&EXTI_InitStructure);	  	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
//GPIOE.4	  �ж����Լ��жϳ�ʼ������  �½��ش���	//KEY0
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource4);
  	EXTI_InitStructure.EXTI_Line=EXTI_Line4;
  	EXTI_Init(&EXTI_InitStructure);	  	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

//GPIOA.0	  �ж����Լ��жϳ�ʼ������ �����ش��� PA0  WK_UP
 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource0); 
  	EXTI_InitStructure.EXTI_Line=EXTI_Line0;
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  	EXTI_Init(&EXTI_InitStructure);		//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

//���ȼ�����
  	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;			//ʹ�ܰ���WK_UP���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure); 
		
		NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;			//ʹ�ܰ���KEY2���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�2
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure);


  	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			//ʹ�ܰ���KEY1���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;					//�����ȼ�1 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure);  	  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���

	  NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;			//ʹ�ܰ���KEY0���ڵ��ⲿ�ж�ͨ��
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//�����ȼ�0 
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  	NVIC_Init(&NVIC_InitStructure);  	  //����NVIC_InitStruct��ָ���Ĳ�����ʼ������NVIC�Ĵ���
		
		
}

void GPIO_MENU_Init(void) //IO��ʼ��
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOE| RCC_APB2Periph_GPIOC, ENABLE);//ʹ��PORTA,PORTEʱ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;//KEY0-KEY2
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
    GPIO_Init(GPIOE, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;//PC2 PC3 
	  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //���ó���������
	  GPIO_Init(GPIOC, &GPIO_InitStructure);
    //��ʼ�� WK_UP-->GPIOA.0	  ��������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0���ó����룬Ĭ������	  
    GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOA.0

}

//�ⲿ�ж�0������� 
void EXTI0_IRQHandler(void)
{
	rt_kprintf(" in EXIT0\n");
	delay_ms(20);//����
	if(WK_UP==1)	 	 //WK_UP����
	{				 
        if (choice < 6)
		{
            choice++;
			u8 j;
	        for(j=1;j<6;j++)
	        { if(j== choice)
               my_LCD_Color_Fill(0, choice * 30-20, 240, choice * 31, BLUE);
	        	else
	          my_LCD_Color_Fill(0, j * 30-20, 240, j * 31,GRAY) ;	
	        
	        }
	    }
        else
            choice =1;
				while(WK_UP==1)
			        ;
	//����IO���������¼���
	result3=rt_event_send(&memlcd_data_event, EVENT_FLAG6);
	rt_kprintf(" in EXIT0\n");
		if (result3 != RT_EOK)
            {
                rt_kprintf("rt_event_send ERR\n");
            }
	
	}
	EXTI_ClearITPendingBit(EXTI_Line0); //���LINE0�ϵ��жϱ�־λ 
	
}
 
void EXTI4_IRQHandler(void)
{
	rt_kprintf(" in EXIT4\n");
	delay_ms(20);//����
	
	if(KEY0==0)	 //����KEY0
	{
		 
		c_state= !c_state; 
		//��������
		if(c_state==OK)
		{
		LCD_Clear(GRAY);
	    LCD_ShowNum(10,300 ,c_state,5,16);

		}
		else		
		{
			
			LCD_Clear(GRAY);
			display_Mainlist();
		  LCD_ShowNum(10,300 ,c_state,5,16);	
		}
		while(KEY0==0)
			;
	//����IO���������¼���
	result3=rt_event_send(&memlcd_data_event, EVENT_FLAG6);
	if (result3 != RT_EOK)
     {
        rt_kprintf("rt_event_send ERR\n");
     } 
		
	}		 
	EXTI_ClearITPendingBit(EXTI_Line4);  //���LINE4�ϵ��жϱ�־λ  
}
 //�ⲿ�ж�2�������
void EXTI2_IRQHandler(void)
{
	rt_kprintf(" in EXIT2\n");
	delay_ms(10);//����
	
	if(KEY1==0)	  //����KEY2
	{
		 adjust+=1;
		 LCD_ShowNum(10,300 ,adjust,5,16);
		 result3=rt_event_send(&memlcd_data_event, EVENT_FLAG6);
	    if (result3 != RT_EOK)
        {
            rt_kprintf("rt_event_send ERR\n");
        } 
		
	}		 
	EXTI_ClearITPendingBit(EXTI_Line2);  //���LINE2�ϵ��жϱ�־λ  
}
//�ⲿ�ж�3�������
void EXTI3_IRQHandler(void)
{	rt_kprintf(" in EXIT3\n");
	delay_ms(10);//����
	
	if(KEY2==0)	 //����KEY1
	{				 
		 adjust-=1;
		 LCD_ShowNum(10,300 ,adjust,5,16);
		result3=rt_event_send(&memlcd_data_event, EVENT_FLAG6);
	    if (result3 != RT_EOK)
        {
            rt_kprintf("rt_event_send ERR\n");
        } 
	}		 
	EXTI_ClearITPendingBit(EXTI_Line3);  //���LINE3�ϵ��жϱ�־λ  
}

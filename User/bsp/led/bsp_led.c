/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   ledӦ�ú����ӿ�
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:���� F103-�Ե� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "bsp_led.h"   

 /**
  * @brief  ��ʼ������LED��IO
  * @param  ��
  * @retval ��
  */
void LED_Init(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOE, ENABLE);	 //????PB,PE?????��??
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;				 //LED0-->PB.5 ????????
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //???��????
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO????????50MHz
 //GPIO_Init(GPIOB, &GPIO_InitStructure);					 //?��???��?��??????????GPIOB.5
 //GPIO_SetBits(GPIOB,GPIO_Pin_5);						 //PB.5 ??????

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;	    		 //LED1-->PE.5 ????????, ???��????
 GPIO_Init(GPIOE, &GPIO_InitStructure);	  				 //???��???? ??IO????????50MHz
 GPIO_ResetBits(GPIOE,GPIO_Pin_5); 						 //PE.5 ?????? 
}

/*********************************************END OF FILE**********************/

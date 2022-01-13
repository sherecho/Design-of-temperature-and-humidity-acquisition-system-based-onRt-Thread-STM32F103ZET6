#ifndef __MENU_KEY_H
#define __MENU_KEY_H	 
#include "sys.h"
#include "bsp_lcd.h"



#define KEY0  GPIO_ReadInputDataBit(GPIOE,GPIO_Pin_4)//��ȡ����0 
#define KEY1  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_2)//��ȡ����1
#define KEY2  GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_3)//��ȡ����2 
#define WK_UP   GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)//��ȡ����3(WK_UP) 
#define KEY0_PRES 	1	//KEY0����
#define WKUP_PRES   4	//KEY_UP����(��WK_UP/KEY_UP)


#define OK 0
#define NO 1


void GPIO_MENU_Init(void);//IO��ʼ��				
void K_MENU_Init(void);//�ⲿ�жϳ�ʼ��	


#endif


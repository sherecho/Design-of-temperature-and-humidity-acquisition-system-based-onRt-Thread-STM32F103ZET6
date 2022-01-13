/* ������Ӳ�����ͷ�ļ� */
#include "board.h" 
#include "bsp_lcd.h"
/* RT-Thread���ͷ�ļ� */
#include <rthw.h>
#include <rtthread.h>
#include "bsp_dht11.h"
#include "delay.h"
#include "MENU_KEY.h"
#include "w25qxx.h"
#include "malloc.h"	 
#include "rtc.h" 
//#include "bluetooth.h"
#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
#define RT_HEAP_SIZE 1024
/* ���ڲ�SRAM�������һ���־�̬�ڴ�����Ϊrtt�Ķѿռ䣬��������Ϊ4KB */
static uint32_t rt_heap[RT_HEAP_SIZE];



RT_WEAK void *rt_heap_begin_get(void)
{
    return rt_heap;
}

RT_WEAK void *rt_heap_end_get(void)
{
    return rt_heap + RT_HEAP_SIZE;
}
#endif

/**
  * @brief  ������Ӳ����ʼ������
  * @param  ��
  * @retval ��
  *
  * @attention
  * RTT�ѿ�������صĳ�ʼ������ͳһ�ŵ�board.c�ļ���ʵ�֣�
  * ��Ȼ���������Щ����ͳһ�ŵ�main.c�ļ�Ҳ�ǿ��Եġ�
  */
void rt_hw_board_init()
{
 
	/* ��ʼ��SysTick */
  SysTick_Config( SystemCoreClock / RT_TICK_PER_SECOND );	
    
	/* Ӳ��BSP��ʼ��ͳͳ�����������LED�����ڣ�LCD�� */
  /* ��ʼ��������Ĵ��� */
	USART_Config();	
  USART2_Config();	
  USART3_Config();
  //BluetoothInit();
	
  /*LCD ��ʼ��*/
   LCD_Init();
	/*DHT11��ʼ��*/
  DHT11_Init();
	/*LED��ʼ��*/
  //LED_GPIO_Config();
	LED_Init();
    /* ������ʼ�� */
//	Key_GPIO_Config();
    GPIO_MENU_Init();//IO��ʼ��				
    K_MENU_Init();//�ⲿ�жϳ�ʼ��
  //NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	  
	 W25QXX_Init();				//��ʼ��W25Q128
   my_mem_init(SRAMIN);
	//����RTC
	RTC_Init();	  			//RTC��ʼ��
/* ���������ʼ������ (use INIT_BOARD_EXPORT()) */
#ifdef RT_USING_COMPONENTS_INIT
    rt_components_board_init();
#endif
    
#if defined(RT_USING_CONSOLE) && defined(RT_USING_DEVICE)
	rt_console_set_device(RT_CONSOLE_DEVICE_NAME);
#endif
    
#if defined(RT_USING_USER_MAIN) && defined(RT_USING_HEAP)
    rt_system_heap_init(rt_heap_begin_get(), rt_heap_end_get());
#endif

  delay_init();
	extern uint8_t OSRunning;
	OSRunning=1;
}

/**
  * @brief  SysTick�жϷ�����
  * @param  ��
  * @retval ��
  *
  * @attention
  * SysTick�жϷ������ڹ̼����ļ�stm32f10x_it.c��Ҳ�����ˣ�������
  * ��board.c���ֶ���һ�Σ���ô�����ʱ�������ظ�����Ĵ��󣬽��
  * �����ǿ��԰�stm32f10x_it.c�е�ע�ͻ���ɾ�����ɡ�
  */
void SysTick_Handler(void)
{
    /* �����ж� */
    rt_interrupt_enter();

    /* ����ʱ�� */
    rt_tick_increase();

    /* �뿪�ж� */
    rt_interrupt_leave();
}



/**
  * @brief  ��ӳ�䴮��DEBUG_USARTx��rt_kprintf()����
  *   Note��DEBUG_USARTx����bsp_usart.h�ж���ĺ꣬Ĭ��ʹ�ô���1
  * @param  str��Ҫ��������ڵ��ַ���
  * @retval ��
  *
  * @attention
  * 
  */
void rt_hw_console_output(const char *str)
{	
	/* �����ٽ�� */
    rt_enter_critical();

	/* ֱ���ַ������� */
    while (*str!='\0')
	{
		/* ���� */
        if (*str=='\n')
		{
			USART_SendData(DEBUG_USART3, '\r'); 
			while (USART_GetFlagStatus(DEBUG_USART3, USART_FLAG_TXE) == RESET);
		}

		USART_SendData(DEBUG_USART3, *str++); 				
		while (USART_GetFlagStatus(DEBUG_USART3, USART_FLAG_TXE) == RESET);	
	}	

	/* �˳��ٽ�� */
    rt_exit_critical();
}

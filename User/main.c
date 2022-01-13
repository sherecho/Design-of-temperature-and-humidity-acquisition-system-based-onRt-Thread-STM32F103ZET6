
/*
*************************************************************************
*                             ������ͷ�ļ�
*************************************************************************
*/ 

//#include "all_header.h"
#include <rtthread.h>
#include "board.h"
#include "stdio.h"
#include <string.h>
#include <ctype.h>
//bsp

#include "bsp_dht11.h"
#include "bsp_lcd.h"
#include "bsp_led.h"
#include "MENU.h"
#include "MENU_KEY.h"
#include "sys.h"
#include "delay.h"
//SD��  
#include "malloc.h"
#include "sdio_sdcard.h"  
#include "w25qxx.h"    
#include "ff.h"  
//ʱ���
#include "rtc.h" 
/*
**********************************************
               ����

**********************************************
*/
uint16_t samp_freq=200;
uint8_t tem,hum;
uint8_t tem_flag=30,hum_flag=0; //��ʪ�ȱ�����ֵ����ʼ��Ϊ30��50���û��������趨)
u8 index;//ʹ���ź�����ֹ��ͻ
int i=0;
u8 t=0;	
extern  u16 samp_set;
extern u8 change_freq;		
extern  int adjust;			

/*
*************************************************************************
*                           SD ��FATFS����
*************************************************************************
*/
FATFS fatsd;
FATFS fatflash;
FIL fileobj;
FRESULT fr;//���巵��ֵ
UINT brs;
/*
*************************************************************************
*                          RTC����
*************************************************************************
*/


/*
**********************************************
                ��ʱ������

**********************************************
*/

rt_timer_t DH11_timer ;//��ʱ�����ƿ�
//��ʪ�ȶ�ȡ����
static  uint8_t tem,hum;


/*
**********************************************
                ��Ϣ���б���

**********************************************
*/
	
	/* ������Ϣ�����������¶ȴ�������Ϣ */
static struct rt_messagequeue mq_DHT11_tem;
static struct rt_messagequeue mq_DHT11_hum;
/* �¶ȴ�������Ϣ�ڴ�� */
static rt_uint8_t msg_DHT11_pool_tem[1024];
static rt_uint8_t msg_DHT11_pool_hum[1024];
	/* ������Ϣ��������LCD�Ѿ�������ڴ�飬SD������д�벢���ٸ��ڴ��*/
struct rt_messagequeue mq_tem_memlcd;	
static rt_uint8_t mq_tem_pool_memlcd[256];


/*
**********************************************
                �¼�����

**********************************************
*/
#define EVENT_FLAG1 (1 << 1)  //DHT11�¶�
#define EVENT_FLAG2 (1 << 2)  //DHT11LCD�ɶ�ȡ
#define EVENT_FLAG3 (1 << 3)  //�����¼�
//#define EVENT_FLAG4 (1 << 4)  //��ʪ���¼�
#define EVENT_FLAG5 (1 << 5)  //DHT11LCD�ɶ�ȡ
#define EVENT_FLAG6 (1 << 6)  //IO���жϴ����¼�
#define EVENT_FLAG7 (1 << 7)  //�޸Ĳ���Ƶ��
#define EVENT_FLAG8 (1 << 8)  //Ҫ�󴮿ڷ����¼�����Ϣ



/* �����¼����ƿ� */

struct rt_event DHT11_data_event;
struct rt_event memlcd_data_event;
struct rt_event memlcd2_data_event;
struct rt_event changesamp_event;
struct rt_event senddata_event;

//ALIGN(RT_ALIGN_SIZE)
/*
**********************************************
               �����߳�

**********************************************
*/

/* �����¼�����*/
static rt_thread_t uart_deal_event = RT_NULL;  //�¼������߳�


//��ں������ڷ����¶�
	static void uart_deal_thread_entry(void* parameter)
{	
    //rt_kprintf("thread in uartsend\n");
    while (1)
    {
		  //rt_uint32_t e;  
			//rt_kprintf("thread in\n");
			
     if (rt_event_recv(&DHT11_data_event, (EVENT_FLAG1|EVENT_FLAG8),
                      RT_EVENT_FLAG_AND|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,RT_NULL ) == RT_EOK)
    {
			
        rt_kprintf("tem: %d\n",tem);
        rt_kprintf("hum: %d\n",hum);			
    }			
			
     
			   
    }
}

//��ʪ����Ϣ���н��ղ��洢�����߳�
//�������ڱ����¶Ⱥ�ʪ����ʷ��Ϣ���ڴ��
rt_uint8_t lcd_draw_tem[48]=0;
rt_uint8_t lcd_drawn_hum[48]=0;
rt_uint8_t *ptr [48]; //��ౣ��48*mem_slice ����
rt_uint8_t *ptr2 [48]; //��ౣ��48*mem_slice ����
static rt_uint8_t temmempool[1024]; //4K
static rt_uint8_t hummempool[1024];
static struct rt_mempool tem_mp;
static struct rt_mempool hum_mp;
//��ȡ����ȡ�¶����У��������ڴ��֮�У�����LCD��ʾ���߳�ͬ��

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5
static rt_thread_t recevDHT11 = RT_NULL;
static rt_thread_t thread_mp_release = RT_NULL;
u8 mem_slice=25;
//��ں���
static void recevDHT11_entry(void *parameter)
{
   
    int cnt=0;
    int cnt2=0;
    int i=0;
    int j=0;
    uint8_t tem_get,hum_get,result;
    while (1)
    {
			  i=i%48;
        j=j%48;
       // rt_kprintf("thread in recevDHT11\n");
        if (rt_mq_recv(&mq_DHT11_hum, &hum_get, sizeof(hum_get), RT_WAITING_NO) == RT_EOK)
        {
           // rt_kprintf("rt_mq_recv mq_DHT11_hum \n");	
					  //�����ڴ��
            if(cnt2%mem_slice==0)
						{
					   
					  ptr2[j] = rt_mp_alloc(&hum_mp, RT_WAITING_NO);
           
            if (ptr2[j] != RT_NULL)
						{ //rt_kprintf("allocate tempool success\n");	
						   
						
							//��LCD��ʾ�����̷߳��ڴ�����ݿɶ���Ϣ������ɶ���idex���Լ��ɴ����¼���־
							if(j!=0)
							{
								
                index=j;
						    //�ڴ洦����LCD����event(���͵Ŀɶ�ȡ����Ϊ0~(i-1)
								result=rt_event_send(&memlcd2_data_event, EVENT_FLAG5);
		            if (result != RT_EOK)
                {
                    rt_kprintf("rt_event_send ERR\n");
                }
		           
						
						  }	
              j=j+1;
							
						}
					  else
							   rt_kprintf("error no pool left \n");				
						
						}
						//������Ϣ�����浽�ڴ����
					 if (ptr2[j-1] != RT_NULL)
					 {
						//������Ϣ�����浽�ڴ����
					//	rt_kprintf(" recv from msg queue, the content:%d\n", hum);
						ptr2[j-1][cnt2%mem_slice]=hum_get;
            cnt2+=1;
					 }
           lcd_drawn_hum[cnt2%48]=hum_get;
       
        }
      
        if (rt_mq_recv(&mq_DHT11_tem, &tem_get, sizeof(tem_get), RT_WAITING_NO) == RT_EOK)
        {
					  //�����ڴ��
					//  rt_kprintf("receive mq_DHT11_tem \n");
            if(cnt%mem_slice==0)
						{
					  
					  ptr[i] = rt_mp_alloc(&tem_mp, RT_WAITING_NO);
           
            if (ptr[i] != RT_NULL)
						{// rt_kprintf("allocate tempool success\n");	
						   
							
							//��LCD��ʾ�����̷߳��ڴ�����ݿɶ���Ϣ������ɶ���idex���Լ��ɴ����¼���־
							if(i!=0)
							{
								
                index=i;
						    //�ڴ洦����LCD����event(���͵Ŀɶ�ȡ����Ϊ0~(i-1)
								result=rt_event_send(&memlcd_data_event, EVENT_FLAG2);
               // rt_kprintf("send memlcd_data_event  event success  \n");
                //rt_kprintf("index %d  \n",i);
		            if (result != RT_EOK)
                {
                    rt_kprintf("rt_event_send ERR\n");
                }
		            else
                    rt_kprintf("send memlcd_data_event  event success  \n");
                  
						
						  }
						  i=i+1;
						}
					  else
							   rt_kprintf("error no pool left \n");				
						
						}
						//������Ϣ�����浽�ڴ����
					 if (ptr[i-1] != RT_NULL)
					 {
						//������Ϣ�����浽�ڴ����
					//	rt_kprintf(" recv from msg queue, the content: %d \n", tem);
						ptr[i-1][cnt%mem_slice]=tem_get;
            cnt+=1;
					 }
           lcd_draw_tem[cnt%48]=tem_get;
        
        }
   

		}
   
		
		
}
//�Զ���ʮλ��ת�ַ���
u16 DectoStr (u8 *pSrc, u16 SrcLen, u8 *pObj)
{
    u16 i=0;

    for(i=0;    i<SrcLen;   i++)
    {
        sprintf((char *)(pObj + i * 2), "%02d", *(pSrc + i));
    }
    *(pObj + i * 2) = '\0';
    return  (i * 2);
}
//�����ڴ�أ�д��SD���߳̽�LCD�Ѿ���̬��ʾ�����ڴ��ͷţ�������д��SD���б��棨����LCD��ʾ���ߴ����̷߳��͵���Ϣ����)

static void thread_mp_release_entry(void *parameter)
{
    int buf;
    char name[11];
    u8 cnt=0;
	  u8 num;
    while(1)
    {
	 //������Ϣ
     
       
	  if (rt_mq_recv(&mq_tem_memlcd, &buf, sizeof(buf), RT_WAITING_FOREVER) == RT_EOK)
     {
          if(buf>=1)
        {
          buf=buf-1;
         // rt_kprintf("writing to SD Card\n");
          //rt_kprintf("index %d\n",buf);
          //�����ļ�����
		       char str[40];  
           char week[12];   
           u8 hour;
           u8 min;
           u8 sec;   
           u8 month;
           u8 day;  	
	         strcpy (str,"0:data");
	         DectoStr (&cnt, 1, &num);
           strcat (str,&num);
           strcat (str,".csv");           
           fr=f_open(&fileobj,(const TCHAR*)str, 	FA_CREATE_ALWAYS | FA_WRITE | FA_READ);//open the file	   
          // rt_kprintf("file name: %s \n", &str);    
           //rt_kprintf("num: %d \n", cnt);   
          
		     	//���յ�ǰ�ڴ��д��SD���ڴ��λ��    
           f_printf(&fileobj,"tem,hum,time");
          // rt_kprintf("fr %d\n",fr);   
	         f_putc( '\n',&fileobj);
          //��ȡʱ��
          switch(calendar.week)
		          	{
		          		case 0:
		          			strcpy (week,"Sunday");
		          			break;
		          		case 1:
		          			strcpy (week,"Monday");
		          			break;
		          		case 2:
		          			strcpy (week,"Tuesday");
		          			break;
		          		case 3:
		          	  	strcpy (week,"Wednesday");
		          			break;
		          		case 4:
		          			strcpy (week,"Thursday");
		          			break;
		          		case 5:
		          		  strcpy (week,"Friday");
		          			break;
		          		case 6:
		          			strcpy (week,"Saturday");
		          			break;  
		          	}
           		
           DectoStr ((u8*)&calendar.w_month,1,&month);
           DectoStr ((u8*)&calendar.w_date,1,&day);	
			     DectoStr ((u8*)&calendar.hour,1,&hour);		
           DectoStr ((u8*)&calendar.min,1,&min);
           DectoStr ((u8*)&calendar.sec,1,&sec);	          								  
           strcat (str,&month); 
           strcat (str,"-");
           strcat (str,&day); 
           strcat (str,"-");
           strcat (str,week);   
           strcat (str,":"); 
           strcat (str,&hour); 
			     strcat (str,":"); 
           strcat (str,&min); 
           strcat (str,":"); 
           strcat (str,&sec);
         	for (i=0;i<mem_slice;i++)
	          {
		     		
             rt_kprintf(" ptr: %d, %d ,%s\n",ptr [buf][i],ptr2 [buf][i],&str);  			 
	           f_printf(&fileobj, "%d,%d,%s",ptr [buf][i], ptr2 [buf][i],&str);
		     		 f_putc( '\n',&fileobj);
		     		
		     		 
		     	 }
            rt_kprintf(" try to release block\n");
            rt_kprintf(" time %s\n",&str);
            rt_mp_free(ptr[buf]);
            ptr[buf] = RT_NULL;
            ptr2[buf] = RT_NULL;
            f_close(&fileobj);
            cnt=cnt+1;
      }
          
     }
    } 

   
}


/*
**********************************************
               ��ʱ����ں���

**********************************************
*/
//��ʱ����ʱ����
static void timeout1(void *parameter)
{ 
   u8 freq;
	  rt_err_t result,result2,result3;  
   // rt_kprintf("geting tem and hum from sensor \n ");

    /* ��ȡ��ʪ�ȴ�������Ϣ*/
      DHT11_Read_Data(&tem,&hum);	
	
  	  //rt_kprintf("get tem from timer \n");
	   //	rt_kprintf("get hum from timer \n");
	  /*������Ϣ����*/
	   
//	      			
	 //�¶ȷ��Ͷ���
        result = rt_mq_send(&mq_DHT11_tem, &tem, sizeof(tem));        
            if (result != RT_EOK)
            {
                rt_kprintf("rt_mq_send_tem ERR\n");
            }

            //rt_kprintf(" send messageque tem - %d\n", tem);
   //ʪ�ȷ��Ͷ���
						result2 = rt_mq_send(&mq_DHT11_hum, &hum, sizeof(hum));        
            if (result2 != RT_EOK)
            {
                rt_kprintf("rt_mq_send_hum ERR\n");
            }

          //  rt_kprintf(" send messageque hum - %d\n", hum);
      
    
	
	  /*�����¼���*/				 
    result3=rt_event_send(&DHT11_data_event, EVENT_FLAG1);
		if (result3 != RT_EOK)
            {
                rt_kprintf("rt_event_send ERR\n");
            }
		/*else
     rt_kprintf("sendDHT11 data  event success  \n");*/
   // rt_kprintf("sampling once  \n");
   //rt_timer_control(DH11_timer, RT_TIMER_CTRL_GET_TIME , (void *)&freq);
   
						
		/*if(change_freq)
				{
			  samp_freq=samp_set;
				samp_set=0;
				adjust=0;			
				//�����޸Ĳ���Ƶ�ʵ��¼�
				//rt_event_send(&changesamp_event, EVENT_FLAG7);	
				
				rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_TIME, (void *)&samp_freq);
				//rt_timer_stop(DH11_timer);
				//rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_PERIODIC, (void *)&samp_freq);
				//rt_timer_start(DH11_timer);
				//rt_timer_control(DH11_timer, RT_TIMER_CTRL_SET_PERIODIC , (void *)&samp_freq);
				//rt_kprintf("timer set %d \n",timerset);								
				change_freq=0;
				}*/
				rt_kprintf("sampfreq:%d \n",samp_freq);
				
   
}


/*
*************************************************************************
*                             LCD��ʾ�߳�
*************************************************************************
*/


static rt_thread_t lcd_show = RT_NULL;
static void lcd_show_entry(void* parameter)
{	
    int buf =0;
    int result;

    while (1)
    {		
      
      rt_kprintf("thread in LCDSHOW\n");
     // key_choice_handle()	;
      
      if (rt_event_recv(& memlcd_data_event, (EVENT_FLAG2|EVENT_FLAG6),
                      RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER ,&buf ) == RT_EOK)
         
         {      
           //�����Ǳ��¼�6����
            if(buf==EVENT_FLAG6)
            {
                  key_choice_handle()	;
                   rt_kprintf(" wakeup by event6 \n");
            }
            else
            {
                key_choice_handle()	;
                //������ʾ�����Ϣ����
		            result=rt_mq_send(& mq_tem_memlcd, &index, sizeof(index));
                rt_kprintf(" wakeup by event2 %d \n",buf);
	              rt_kprintf(" sending  mq_tem_memlcd \n");
                rt_kprintf("index %d  \n",index);
	               if (result!= RT_EOK)
                {
                    rt_kprintf(" mq_tem_memlcd ERR\n");
                  
                    
                }
            }
         

			   
         }	        
              
    }


}





/*�¶ȱ��������̣߳����ø����ȼ�*/
/* �����¼�����*/
//�߳̿��ƿ�
static rt_thread_t over_temhum = RT_NULL;  //���³�ʪ���¼��߳�

//��ں���
static void overtem_entry(void* parameter)
{
	  //LED1_ON	
	   //�ȴ��¼�����
    
	while(1)
	{
      
     // rt_kprintf("thread in TEMOVER\n");
     	u2_printf("tem: %d\n",tem);
      u2_printf("hum: %d\n",hum);	
      if (rt_event_recv(&DHT11_data_event , (EVENT_FLAG1),
                      RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,RT_NULL ) == RT_EOK)
			{
				if(tem>tem_flag)		
				{					
				  LED=0;
           rt_kprintf("over tem\n");
					
				}
				else
				{
					LED=1;		//���¶Ȼָ�������ʱ��Ϩ��
				}
				
				if(hum>hum_flag)		
				{					
				   LED=0;
           rt_kprintf("over hum\n");
				}
				else
				{
					 LED=1;		//���¶Ȼָ�������ʱ��Ϩ��
           
				}
				
       /* //�̹߳���
        if(tem<tem_flag&&hum<hum_flag)	
           rt_thread_suspend(over_temhum);/* �����߳� */
			}					
	}
	
}

/*
*************************************************************************
*                             wifi�߳�
*************************************************************************
*/

//�߳̿��ƿ�
static rt_thread_t wifi = RT_NULL;
static void wifi_entry(void* parameter)
{
     
     //�����¼�
     while (1)
    {
		 
     rt_kprintf("in wifi \n");
			
     if (rt_event_recv(&DHT11_data_event, (EVENT_FLAG1),
                      RT_EVENT_FLAG_OR|RT_EVENT_FLAG_CLEAR,
                      RT_WAITING_FOREVER,RT_NULL ) == RT_EOK)
    {
			
        		u2_printf("tem: %d\n",tem);
            u2_printf("hum: %d\n",hum);	
    }			
			
     
			   
    }
    


}






/*
*************************************************************************
*                             main ����
*************************************************************************
*/
/**
  * @brief  ������
  * @param  ��
  * @retval ��
  */



int main(void)
{	
    /* 
	 * ������Ӳ����ʼ����RTTϵͳ��ʼ���Ѿ���main����֮ǰ��ɣ�
	 * ����component.c�ļ��е�rtthread_startup()����������ˡ�
	 * ������main�����У�ֻ��Ҫ�����̺߳������̼߳��ɡ�
	 */
  /*������ƽ̨*/

	u2_printf("AT+ATKCLDSTA=\"39581907267257811324\",\"12345678\"\r\n");
  rt_kprintf("** �¶ȼ��ϵͳ**\r\n");
	rt_kprintf("RT0S��RT-thread  \r\n");
	rt_kprintf("MCU��STM32F103ZET6 \r\n");
	
  uint8_t result;
  rt_err_t flag;
	int j=0;	
  /***********************����SD�������ļ�*******************************/
	while(SD_Init())//��ⲻ��SD��
	{
		LCD_ShowString(30,150,200,16,16,"SD Card Error!");				
		LCD_ShowString(50,150,200,16,16,"Please Check! ");
		
	}
   //���ļ�
  f_mount(&fatsd,"0:",1); 					//����SD�� 1:��������


  //*****************��Ϣ���д���	*****************************************/
  //�����ڴ��
	//��ʼ�����ƿ�
    rt_mp_init(&tem_mp, "temmp", &temmempool[0], sizeof(temmempool), mem_slice);
	  rt_mp_init(&hum_mp, "hummp", &hummempool[0], sizeof(hummempool), mem_slice);
	    rt_err_t result2;
    /* ��ʼ����Ϣ���� */
    result= rt_mq_init(& mq_DHT11_tem,
                        " mq_DHT11_tem",
                        &msg_DHT11_pool_tem[0],               /* msg_pool */
                        sizeof(tem),                          /* ÿ����Ϣ�Ĵ�С*/
                        sizeof(msg_DHT11_pool_tem),                     /* msg_pool��С*/
                        RT_IPC_FLAG_FIFO); 
    result2 = rt_mq_init(& mq_DHT11_hum,
                        " mq_DHT11_hum",
                        &msg_DHT11_pool_hum[0],               /* msg_pool */
                        sizeof(hum),                          /* ÿ����Ϣ�Ĵ�С*/
                        sizeof(msg_DHT11_pool_hum),                     /* msg_pool��С*/
                        RT_IPC_FLAG_FIFO); 
    												

    if (result != RT_EOK)
    {
        rt_kprintf("init message_tem queue failed.\n");
        //return -1;
    }
		if (result2 != RT_EOK)
    {
        rt_kprintf("init message_hum queue failed.\n");
        //return -1;
    }
		
		result2 = rt_mq_init(& mq_tem_memlcd,
                        " mq_tem_memlcd",
                        &mq_tem_pool_memlcd[0],               /* msg_pool */
                        sizeof(int),                          /* ÿ����Ϣ�Ĵ�С*/
                        sizeof(mq_tem_pool_memlcd),                     /* msg_pool��С*/
                        RT_IPC_FLAG_FIFO); 
     if (result2 != RT_EOK)
    {
        rt_kprintf("initmq_tem_memlcd failed.\n");
       
    }
		
	//�¼�����
	  //  creat_myevents();
    /* ��ʼ���¼����󣨾�̬�� */
    result = rt_event_init(&DHT11_data_event, "DHT11_data_event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event dht11 failed.\n");
        return -1;
    }

	  result = rt_event_init(&memlcd_data_event, "memlcd_data_event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event memlcd_data_event  failed.\n");
        return -1;
    }	
		result = rt_event_init(&memlcd2_data_event, "memlcd_data_event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event memlcd_data_event  failed.\n");
        return -1;
    }	

    
    result = rt_event_init(&senddata_event, "senddata_event", RT_IPC_FLAG_FIFO);
    if (result != RT_EOK)
    {
        rt_kprintf("init event senddata_event  failed.\n");
        return -1;
    }
    
		
		//��ʱ��
    /* DHT11 TIMER */		
    DH11_timer = rt_timer_create("DHT11_get", timeout1,
                             RT_NULL, samp_freq,
                             RT_TIMER_FLAG_PERIODIC);
    /* ����*/
    if (DH11_timer  != RT_NULL) 
			  flag=rt_timer_start(DH11_timer );	
	  if(flag==RT_EOK)
			 rt_kprintf("DH11_timer start successfull\n");	

	//***********************�¼������߳�*******************************************/
		uart_deal_event =                          /* �߳̿��ƿ�ָ�� */
    rt_thread_create( "usart_deal_event",              /* �߳����� */
                      uart_deal_thread_entry,   /* �߳���ں��� */
                      RT_NULL,             /* �߳���ں������� */
                      256,                 /* �߳�ջ��С */
                      2,                   /* �̵߳����ȼ� */
                      20);                 /* �߳�ʱ��Ƭ */
                   
    /* �����̣߳��������� */
    if (uart_deal_event != RT_NULL)
        rt_thread_startup(uart_deal_event);
    else
         rt_kprintf("led_thread_creat_error");
		
////*******************�����ڴ������߳�***********************************************************//
   for (j = 0; j < 48; j++) ptr[j] = RT_NULL;

    //�����߳�
    recevDHT11 = rt_thread_create("recevDHT11", recevDHT11_entry, RT_NULL,
                            512,
                            4 ,20);
		
    if (recevDHT11 != RT_NULL)
         rt_thread_startup(recevDHT11);

//*******************�����ͷ��ڴ��д��SD���߳�***********************************************************//	
    thread_mp_release = rt_thread_create("thread_mp_release", thread_mp_release_entry, RT_NULL,
                            1024,
                            4, 20);
    if (thread_mp_release != RT_NULL)
        rt_thread_startup(thread_mp_release);
		
///*******************��LCD ��ʾ��ͼ�߳�***********************************************************//	
		lcd_show= rt_thread_create("lcd_showwave", lcd_show_entry, RT_NULL,
                            512,
                            3, 30);
    if (lcd_show != RT_NULL)
         rt_thread_startup(lcd_show );
//*******************���´����̵߳Ĵ���***********************************************************//			
		
		over_temhum = rt_thread_create("over_temhum ", overtem_entry, RT_NULL,
                            256,
                            1, 20);
    if (lcd_show != RT_NULL)
         rt_thread_startup(over_temhum  );		
//*******************wifi�̴߳���***********************************************************//			
		
		wifi= rt_thread_create("wifi ", wifi_entry, RT_NULL,
                            128,
                            2, 20);
    if (wifi != RT_NULL)
         rt_thread_startup(wifi);	
  
     
//*************����wifiģ��************************************************************************//



  
  
 
		
	}



/********************************END OF FILE****************************/

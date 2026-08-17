#include "stm32f10x.h"

#define TIM_PSC (7200-1)  //分频系数
#define TIM_ARR (200-1)   //自动重装值

void Timer2_Init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);//时钟使能
	
	/*时基单元初始化*/
	TIM_TimeBaseInitTypeDef TIM_TimBaseInitStructure;
	TIM_TimBaseInitStructure.TIM_ClockDivision=TIM_CKD_DIV1;
	TIM_TimBaseInitStructure.TIM_CounterMode=TIM_CounterMode_Up;
	TIM_TimBaseInitStructure.TIM_Period=TIM_ARR;
	TIM_TimBaseInitStructure.TIM_Prescaler=TIM_PSC;
	TIM_TimeBaseInit(TIM2,&TIM_TimBaseInitStructure);
	
	/*中断使能*/
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	/*中断控制器NVIC初始化*/
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=1;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_Cmd(TIM2,ENABLE);
}

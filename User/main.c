#include <stdlib.h>
#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "Items.h"
#include "OLED.h"
#include "MyTimer2.h"

int main(void)
{
	/*模块初始化*/
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//中断分组配置
	Key_Init();			//案件初始化
	OLED_Init();		//屏幕初始化

	
	while(1){
		StartFlag=0;
		/*显示封面*/
		Items_Init();		//游戏初始化
		OLED_Refresh((uint8_t *)FrameBuffer);
		
		/*开始*/
		while(StartFlag==0){};
		Game_Start();
		Timer2_Init();
		srand(TIM2->CNT);
		OLED_Refresh((uint8_t *)FrameBuffer);
		
		while (1)
		{
			/*刷新帧*/
			if(BufferFlag==1){
				Game_Refresh();
				OLED_Refresh((uint8_t *)FrameBuffer);
				BufferFlag=0;
			}
			/*失败*/
			if(crash_flag==1){
				StartFlag=0;
				Game_Failure();
				OLED_Refresh((uint8_t *)FrameBuffer);
				break;
			}
		}
		while(StartFlag==0){};
	};
}

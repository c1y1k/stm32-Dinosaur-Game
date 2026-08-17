#include <stdlib.h>
#include "stm32f10x.h"
#include "OLED.h"
#include "OLED_Font.h"

#define JUMP_TOTAL 60
#define PEAK 40

#define CACTUS_DISTANCE 100          
#define CACTUS_NUM_MAX 5

/*用于存储屏幕上的仙人掌位置和类型*/
typedef struct{
	int16_t position;
	uint8_t first,num;
	uint8_t cactus_array[5];
	uint8_t distance;
}cactus_structure;

uint8_t FrameBuffer[8][128]={0}; //帧缓冲
volatile uint8_t BufferFlag=0; //刷新标志
volatile uint8_t JumpFlag  = 0; //跳跃标志
volatile uint8_t JumpNum  = 0; //跳跃计时,跳跃后设为9，每帧减1，减到0则将JumpFlag置为0
volatile uint8_t StartFlag  = 0; //开始标志
volatile cactus_structure cactus;
volatile uint8_t crash_flag = 0;
volatile uint32_t score = 0;

/**
  * @brief  初始化
  * @param  无
  * @retval 无
  */
void Items_Init(void)
{
	for(uint8_t i=0;i<8;i++){
		for(uint8_t j=0;j<128;j++) FrameBuffer[i][j]=COVER[i*128+j]; //！这里应该刷新为游戏初始画面
	}
	cactus.position = 128;
	cactus.first = 0;
	cactus.num = 0;
	cactus.distance = 0;
	crash_flag=0;
	score = 0;
	JumpFlag=0;
	JumpNum=0;
}

/**
  * @brief  OLED次方函数
  * @retval 返回值等于X的Y次方
  */
uint32_t Game_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y--)
	{
		Result *= X;
	}
	return Result;
}

/**
	* @brief 游戏第一帧
	* @param 无
	* @retval 无
	*/
void Game_Start(void)
{
	for(uint8_t i=0;i<8;i++){
		for(uint8_t j=0;j<128;j++){
			FrameBuffer[i][j]=0; //清空缓冲区
		}
	}
	
	/*向缓存中刷入地平线的第一帧*/
	for(uint8_t i=0;i<128;i++){
		FrameBuffer[7][i]=GROUND[i];
	}
	/*向缓存中刷入龙*/
	for(uint8_t i=0;i<16;i++){
		FrameBuffer[6][i+8]=DINO[0][i];
	}
	for(uint8_t i=0;i<16;i++){
		FrameBuffer[7][i+8]=DINO[0][i+16];
	}
	
	/*向缓存中刷入第一个仙人掌*/
	cactus.first = 0;
	cactus.cactus_array[cactus.first] = rand() % 4;
	cactus.num++;
	cactus.position = 127;
	cactus.distance = 1;
	uint8_t cactus_width;
	if(cactus.cactus_array[cactus.first] == 0){
		cactus_width = 8;
		for(uint8_t i = 0;i < cactus_width;i++){
			if(i+cactus.position > 127) break;
			FrameBuffer[6][i+cactus.position] = CACTUS_1[i];
			FrameBuffer[7][i+cactus.position] = CACTUS_1[i+cactus_width];
		}
	}else if(cactus.cactus_array[cactus.first] == 1){
		cactus_width = 16;
		for(uint8_t i = 0;i < cactus_width;i++){
			if(i+cactus.position > 127) break;
			FrameBuffer[6][i+cactus.position] = CACTUS_2[i];
			FrameBuffer[7][i+cactus.position] = CACTUS_2[i+cactus_width];
		}
	}else if(cactus.cactus_array[cactus.first] == 2){
		cactus_width = 24;
		for(uint8_t i = 0;i < cactus_width;i++){
			if(i+cactus.position > 127) break;
			FrameBuffer[6][i+cactus.position] = CACTUS_3[i];
			FrameBuffer[7][i+cactus.position] = CACTUS_3[i+cactus_width];
		}
	}else if(cactus.cactus_array[cactus.first] == 3){
		cactus_width = 24;
		for(uint8_t i = 0;i < cactus_width;i++){
			if(i+cactus.position > 127) break;
			FrameBuffer[6][i+cactus.position] = CACTUS_4[i];
			FrameBuffer[7][i+cactus.position] = CACTUS_4[i+cactus_width];
		}
	}
}

/**
	* @brief 游戏帧刷新
	* @param 无
	* @retval 无
	*/
void Game_Refresh(void)
{
	
	OLED_SetCursor(0,0);
	
	static uint16_t round=1;
	/*向缓存中刷入地平线*/
	for(uint8_t i=0;i<128;i++){
		 FrameBuffer[7][i]=GROUND[(i+round)%597];
	}
	round=(round+1)%597;
	
	/*从缓存中清除0-6页*/
	for(uint8_t i=0;i<7;i++){
		for(uint8_t j=0;j<128;j++){
			FrameBuffer[i][j]=(uint8_t)0;
		}
	}
	
	/*向缓存中刷入精灵*/
	if(JumpFlag==0){
		/*向缓存中刷入地面的龙*/
		for(uint8_t i=0;i<16;i++){
			FrameBuffer[6][i+8]=DINO[score%2][i];
		}
		for(uint8_t i=0;i<16;i++){
			FrameBuffer[7][i+8]=DINO[score%2][i+16];
		}
	}else{
		/*向缓存中刷入空中的龙*/
		uint32_t phase = JUMP_TOTAL - JumpNum;																						//当前帧的相位
		uint32_t height = phase * (JUMP_TOTAL - phase) * 4 * PEAK / (JUMP_TOTAL * JUMP_TOTAL);//龙的脚底的高度
		if(phase==0||phase==JUMP_TOTAL){																												//0号帧或者18号帧
			for(uint8_t i=0;i<16;i++){
				FrameBuffer[5][i+8]=DINO_JUMP[0][i];
			}
			for(uint8_t i=0;i<16;i++){
				FrameBuffer[6][i+8]=DINO_JUMP[0][i+16];
			}
			for(uint8_t i=0;i<16;i++){
				FrameBuffer[7][i+8]=DINO_JUMP[0][i+32];
			}
			if(phase==JUMP_TOTAL) JumpFlag=0;
			else JumpNum--;
		}else{																																					//1-17号帧
			uint8_t row = 7 - height / 8;																									//底部所在行
			uint8_t div = 8 - height % 8;																									//低位需要分割出来的元素
			for(uint8_t i=0;i<16;i++){
				FrameBuffer[row][i+8]=DINO_JUMP[0][i+32] >> (8 - div);
			}
			for(uint8_t i=0;i<16;i++){
				FrameBuffer[row-1][i+8]=(DINO_JUMP[0][i+32] << div) | (DINO_JUMP[0][i+16] >> (8 - div));
			}
			for(uint8_t i=0;i<16;i++){
				FrameBuffer[row-2][i+8]=(DINO_JUMP[0][i+16] << div) | (DINO_JUMP[0][i] >> (8 - div));
			}
			if(row-3>=0){
				for(uint8_t i=0;i<16;i++){
					FrameBuffer[row-3][i+8]=DINO_JUMP[0][i] << div;
				}
			}
			JumpNum--;
		}
	}
	
	/*向缓存中刷入所有仙人掌*/
	cactus.position--;
	cactus.distance++;
	
	uint8_t first_cactus_width;
	if(cactus.cactus_array[cactus.first] == 0){
		first_cactus_width = 8;
	}else if(cactus.cactus_array[cactus.first] == 1){
		first_cactus_width = 16;
	}else if(cactus.cactus_array[cactus.first] == 2){
		first_cactus_width = 24;
	}else if(cactus.cactus_array[cactus.first] == 3){
		first_cactus_width = 24;
	}
	if(cactus.position + first_cactus_width < 0){
		cactus.first=(cactus.first+1)%5;
		cactus.num--;
		cactus.position+=CACTUS_DISTANCE;
	}
	
	if(cactus.distance == CACTUS_DISTANCE+1){
		cactus.distance = 1;
		cactus.cactus_array[(cactus.first+cactus.num)%CACTUS_NUM_MAX] = rand()%4;
		cactus.num++;
	}
	
	for(int k = 0;k < cactus.num;k++){
		uint8_t cactus_width;
		if(cactus.cactus_array[(cactus.first+k)%CACTUS_NUM_MAX] == 0){
			cactus_width = 8;
			for(uint8_t i = 0;i < cactus_width;i++){
				if(i+cactus.position+k*CACTUS_DISTANCE > 127) break;
				if(i+cactus.position+k*CACTUS_DISTANCE < 0) continue;
				if(FrameBuffer[6][i+cactus.position+k*CACTUS_DISTANCE]!=0x00){ crash_flag = 1;}
				FrameBuffer[6][i+cactus.position+k*CACTUS_DISTANCE] = CACTUS_1[i];
				FrameBuffer[7][i+cactus.position+k*CACTUS_DISTANCE] = CACTUS_1[i+cactus_width];
			}
		}else if(cactus.cactus_array[(cactus.first+k)%CACTUS_NUM_MAX] == 1){
			cactus_width = 16;
			for(uint8_t i = 0;i < cactus_width;i++){
				if(i+cactus.position+k*CACTUS_DISTANCE > 127) break;
				if(i+cactus.position+k*CACTUS_DISTANCE < 0) continue;
				if(FrameBuffer[6][i+cactus.position+k*CACTUS_DISTANCE]!=0x00){ crash_flag = 1;}
				FrameBuffer[6][i+cactus.position+k*CACTUS_DISTANCE] = CACTUS_2[i];
				FrameBuffer[7][i+cactus.position+k*CACTUS_DISTANCE] = CACTUS_2[i+cactus_width];
			}
		}else if(cactus.cactus_array[(cactus.first+k)%5] == 2){
			cactus_width = 24;
			for(uint8_t i = 0;i < cactus_width;i++){
				if(i+cactus.position+k*CACTUS_DISTANCE > 127) break;
				if(i+cactus.position+k*CACTUS_DISTANCE < 0) continue;
				if(FrameBuffer[6][i+cactus.position+k*CACTUS_DISTANCE]!=0x00){ crash_flag = 1;}
				FrameBuffer[6][i+cactus.position+k*CACTUS_DISTANCE] = CACTUS_3[i];
				FrameBuffer[7][i+cactus.position+k*CACTUS_DISTANCE] = CACTUS_3[i+cactus_width];
			}
		}else if(cactus.cactus_array[(cactus.first+k)%5] == 3){
			cactus_width = 24;
			for(uint8_t i = 0;i < cactus_width;i++){
				if(i+cactus.position+k*CACTUS_DISTANCE > 127) break;
				if(i+cactus.position+k*CACTUS_DISTANCE < 0) continue;
				if(FrameBuffer[6][i+cactus.position+k*CACTUS_DISTANCE]!=0x00){ crash_flag = 1;}
				FrameBuffer[6][i+cactus.position+k*CACTUS_DISTANCE] = CACTUS_4[i];
				FrameBuffer[7][i+cactus.position+k*CACTUS_DISTANCE] = CACTUS_4[i+cactus_width];
			}
		}
	}
	
	if(crash_flag == 0) score++;
	
	for(uint8_t i=0;i < 10;i++){
		for(uint8_t j=0;j<8;j++){
			FrameBuffer[0][i*8+j]=OLED_F8x16[score/Game_Pow(10,10-i-1)%10 + '0' - ' '][j];
			FrameBuffer[1][i*8+j]=OLED_F8x16[score/Game_Pow(10,10-i-1)%10 + '0' - ' '][j+8];
		}
	}
}

void Game_Failure(void)
{
	for(int i=0;i<8;i++){
		FrameBuffer[0][i+88]=OLED_F8x16['F'-' '][i];
		FrameBuffer[1][i+88]=OLED_F8x16['F'-' '][i+8];
	}
	for(int i=0;i<8;i++){
		FrameBuffer[0][i+8+88]=OLED_F8x16['a'-' '][i];
		FrameBuffer[1][i+8+88]=OLED_F8x16['a'-' '][i+8];
	}
	for(int i=0;i<8;i++){
		FrameBuffer[0][i+16+88]=OLED_F8x16['i'-' '][i];
		FrameBuffer[1][i+16+88]=OLED_F8x16['i'-' '][i+8];
	}
	for(int i=0;i<8;i++){
		FrameBuffer[0][i+24+88]=OLED_F8x16['l'-' '][i];
		FrameBuffer[1][i+24+88]=OLED_F8x16['l'-' '][i+8];
	}
	for(int i=0;i<8;i++){
		FrameBuffer[0][i+32+88]=OLED_F8x16['!'-' '][i];
		FrameBuffer[1][i+32+88]=OLED_F8x16['!'-' '][i+8];
	}
}

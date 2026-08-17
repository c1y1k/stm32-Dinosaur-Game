#ifndef __Items_H
#define __Items_H

extern uint8_t FrameBuffer[8][128];   // 声明：变量定义在 Items.c
extern uint8_t BufferFlag;
extern volatile uint8_t JumpFlag;
extern volatile uint8_t StartFlag;
extern volatile uint8_t JumpNum; //跳跃计时
extern volatile uint8_t crash_flag;

void Items_Init(void);
void Game_Start(void);
void Game_Refresh(void);
void Game_Failure(void);

#endif
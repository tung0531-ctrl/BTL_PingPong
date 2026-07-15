#ifndef BUZZER_MUSIC_H
#define BUZZER_MUSIC_H

#ifdef __cplusplus
extern "C" {
#endif

void BuzzerMusic_Init(void);
void BuzzerMusic_StartGameLoop(void);
void BuzzerMusic_Stop(void);
void BuzzerMusic_Update(void);
void BuzzerMusic_Accent(void);
void BuzzerMusic_PaddleBounce(void);
void BuzzerMusic_WallBounce(void);

#ifdef __cplusplus
}
#endif

#endif
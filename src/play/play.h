#ifndef __PLAY_H
#define __PLAY_H

int MpegAudioDecoder(FIL *InputFp);
int AudioDecode(char *Name);

extern void Convert_Mono(short *buffer);
extern void Convert_Stereo(short *buffer);

#endif /* __PLAY_H	*/

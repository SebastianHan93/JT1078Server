//
// Created by hc on 2020/5/14.
//

#ifndef JT1078SERVER_ADPCM_H
#define JT1078SERVER_ADPCM_H
/*
** adpcm.h - include file for adpcm coder.
**
** Version 1.0, 7-Jul-92.
*/

typedef struct adpcm_state_t {
    short	valprev;	/* Previous output value */
    char	index;		/* Index into stepsize table */
}adpcm_state;


void adpcm_coder(short *indata, char *outdata, int len, adpcm_state *state);

void adpcm_decoder(char *indata, short *outdata, int len, adpcm_state *state);

#endif //JT1078SERVER_ADPCM_H

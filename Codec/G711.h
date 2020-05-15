//
// Created by hc on 2020/5/14.
//

#ifndef JT1078SERVER_G711_H
#define JT1078SERVER_G711_H
/*
* u-law, A-law and linear PCM conversions.
*/
#define	SIGN_BIT	(0x80)		/* Sign bit for a A-law byte. */
#define	QUANT_MASK	(0xf)		/* Quantization field mask. */
#define	NSEGS		(8)		/* Number of A-law segments. */
#define	SEG_SHIFT	(4)		/* Left shift for segment number. */
#define	SEG_MASK	(0x70)		/* Segment field mask. */
#define	BIAS		(0x84)		/* Bias for linear code. */


int g711a_decode(short amp[], const unsigned char g711a_data[], int g711a_bytes);

int g711u_decode(short amp[], const unsigned char g711u_data[], int g711u_bytes);

int g711a_encode(unsigned char g711_data[], const short amp[], int len);

int g711u_encode(unsigned char g711_data[], const short amp[], int len);

#endif //JT1078SERVER_G711_H

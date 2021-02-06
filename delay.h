/* 
 * File:   delay.h
 * Author: Patrick
 *
 * Created on 24. Januar 2021, 21:46
 */

#ifndef DELAY_H
#define	DELAY_H

#ifdef	__cplusplus
extern "C" {
#endif

#if F_CPU == 8000000UL
__attribute__ ( ( section ( ".ramfun" ) ) ) void delay_usec ( uint32_t n )
{
	__asm (
	  "mydelay: \n"
	  " sub  r0, r0, #1 \n"  // 1 cycle
	  " nop             \n"  // 1 cycle
	  " nop             \n"  // 1 cycle
	  " nop             \n"  // 1 cycle
	  " nop             \n"  // 1 cycle
	  " bne  mydelay    \n"  // 2 if taken, 1 otherwise
	);
}
#elif F_CPU == 48000000UL
__attribute__ ( ( section ( ".ramfun" ) ) ) void delay_usec ( uint32_t n )
{
	__asm (
	  "mydelay: \n"
	  " mov  r1, #15    \n"  // 1 cycle
	  "mydelay1: \n"
	  " sub  r1, r1, #1 \n"  // 1 cycle
	  " bne  mydelay1    \n" // 2 if taken, 1 otherwise
	  " sub  r0, r0, #1 \n"  // 1 cycle
	  " bne  mydelay    \n"  // 2 if taken, 1 otherwise
	);
}
#else
#error F_CPU is not defined
#endif

#ifdef	__cplusplus
}
#endif

#endif	/* DELAY_H */


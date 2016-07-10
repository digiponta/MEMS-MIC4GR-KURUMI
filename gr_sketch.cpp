/* GR-KURUMI Sketch Template Version: V1.12*/
/* Modificated by DIGI-P (C) 2016 */
/* This AS-IS file is offered with a BSD lite license */
#include <Arduino.h>

#include <stdlib.h>
#include "RLduino78_mcu_depend.h"
#include "RLduino78_timer.h"
#include "pintable.h"
#include "fastio.h"
#ifdef USE_RTOS
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#endif

#ifdef USE_RTOS
#define FUNC_MUTEX_LOCK		xSemaphoreTake(xFuncMutex, portMAX_DELAY)	//!< 関数用MUTEX LOCKマクロ
#define FUNC_MUTEX_UNLOCK	xSemaphoreGive(xFuncMutex)					//!< 関数用MUTEX UNLOCKマクロ
#else
#define FUNC_MUTEX_LOCK													//!< 関数用MUTEX LOCKマクロ
#define FUNC_MUTEX_UNLOCK												//!< 関数用MUTEX UNLOCKマクロ
#endif

// ライブラリ側で公開するように修正が必要
void _startTAU0(uint16_t u16TimerClock);
void _stopTAU0();
void _startTimerChannel(uint8_t u8TimerChannel, uint16_t u16TimerMode, uint16_t u16Interval, bool bPWM, bool bInterrupt);
void _stopTimerChannel(uint8_t u8TimerChannel);
void outputClock( uint8_t, uint32_t );

// Pin 22,23,24 are assigned to RGB LEDs.
int led_red   = 22; // LOW active
int led_green = 23; // LOW active
int led_blue  = 24; // LOW active

void inputClock(bool start);
int readTimer();
unsigned short min = 0xffff;
unsigned short max = 0x0000;
int current = 0;
#define TIMER_INITIAL_VALUE (0xfffe)

void inputClock(bool start )	// Timer 0 Channel 1; を前提
{
	FUNC_MUTEX_LOCK;

	min = 0xffff;
	max = 0x0000;

	pinMode( 3, INPUT ); // Channel 1
	pinMode( 5, INPUT ); // Channel 2

	if (start) {
		// タイマーアレイユニットの開始
		_startTAU0(TIMER_CLOCK);
		
		// タイマーの開始		
		_startTimerChannel( 1, 0x1106, TIMER_INITIAL_VALUE, false, false); // Channel 1
		_startTimerChannel( 2, 0x1106, TIMER_INITIAL_VALUE, false, false); // Channel 2

	} else {
		// タイマーの停止
		_stopTimerChannel( 1 ); // Channel 1
		_stopTimerChannel( 2 ); // Channel 2

		// タイマーアレイユニットの停止
		_stopTAU0();
	}

	FUNC_MUTEX_UNLOCK;
}

int readTimer()  
{
    // TCR01から値を読み取る
    unsigned short ret1 = TIMER_INITIAL_VALUE - TCR01.tcr01; // Channel 1
    unsigned short ret2 = TIMER_INITIAL_VALUE - TCR02.tcr02; // Channel 1
    // unsigned short ret = TIMER_INITIAL_VALUE - TCR02.tcr02; // Channel 2
	if (ret1 > max) max = ret1;
	if (ret1 < min) min = ret1;
    // rest the timer
    //TS0.ts0 = 0x0002;		// Channel 1
    TS0.ts0 = 0x0006; 	// Channel 1 & 2
    return ( ret1 - (ret2 / 2) );
}

void MyCycle(unsigned long ms)
{
  current = readTimer();
}

// the setup routine runs once when you press reset:
void setup() {

  // initialize the digital pin as an output.
  Serial.begin(115200);
  Serial.println("Hello");

  pinMode(led_red, OUTPUT);
  pinMode(led_green, OUTPUT);
  pinMode(led_blue, OUTPUT);
  pinMode( 4, OUTPUT);
  pinMode( 6, OUTPUT);
  pinMode( 7, OUTPUT);
  pinMode( 8, OUTPUT);
  pinMode( 9, OUTPUT);

  // turn the LEDs on, glow white.
  digitalWrite(led_red, HIGH);
  digitalWrite(led_green, HIGH);
  digitalWrite(led_blue, HIGH);
  digitalWrite( 4, LOW ); 
  digitalWrite( 6, LOW ); 
  digitalWrite( 7, LOW ); 
  digitalWrite( 8, LOW ); 
  digitalWrite( 9, LOW ); 

  outputClock( 10, 1000000 ); // 1MHz clock for the mic

  attachIntervalTimerHandler( MyCycle );
  inputClock( true );	// Timer 0 Channel 1; を前提

}

int cnt = 0;
unsigned avg = 0;

// the loop routine runs over and over again forever:
void loop() {
	
  
  if (((cnt++) % 5 ) == 0) {
    unsigned short diff = max - min;
    unsigned short val;
    avg = diff + (avg / 0.0001);
    val = avg;
  
    if (diff >   5 )  digitalWrite( 4, HIGH );
    else   digitalWrite( 4, LOW );
    if (diff >  10)   digitalWrite( 6, HIGH );
    else   digitalWrite( 6, LOW );
    if (diff >  20)   digitalWrite( 7, HIGH );
    else   digitalWrite( 7, LOW );
    if (diff >  50)   digitalWrite( 8, HIGH );
    else   digitalWrite( 8, LOW );
    if (diff > 100)   digitalWrite( 9, HIGH );
    else   digitalWrite( 9, LOW );

    Serial.print( current );
    Serial.print( "," );
    Serial.println( diff );
  	min = max = (max + min) / 2;
  }
  delay(10);

}

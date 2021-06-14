#pragma once
//
//	Configuration.h
// 
//	(c) Mark Naylor June 2021
//
#define NUM_MOTORS						1			// number of motors used to deliver oil

#define OILED_DEVICE_ACTIVE_PIN1		2			// pin which will go high when machine being oiled is active
#define OILED_DEVICE_ACTIVE_PIN2		3			// pin which will go high when machine being oiled is active
#define ELAPSED_TIME_MODE				1			// Oil after elapsed time
#define ACTION_MODE						2			// OIl after action occurs
#define OILER_MODE						ELAPSED_TIME_MODE
#define ELAPSED_TIME_SECS				30			// number of seconds after which pump(s) is(are) started to deliver oil
#define USING_STEPPER_MOTORS						// comment out if using relays
#define ALERT_PIN						19			// Pin to signal if not completed oiling in multiple of oiler start target (eg elapsed time / revs / powered on time)
#define ALERT_THRESHOLD					2			// Example only - set to twice normal target

#ifdef USING_STEPPER_MOTORS
// Following is used to define a set of four pin relay stepper motors and and an associated sensor that signals when they have output ( e.g. drop of oil)
struct
{
	uint8_t		Pin1;
	uint8_t		Pin2;
	uint8_t		Pin3;
	uint8_t		Pin4;
	uint32_t	Speed;
	uint8_t		MotorOutputPin;
	uint8_t		Drips;
} FourPinMotor [ NUM_MOTORS ] =
// One motor config
{
	{ 4 ,5, 6, 7, 800, OILED_DEVICE_ACTIVE_PIN1, 3}
};
/* Two motor config example, NB change NUM_MOTORS above to 2									
{
	{ 4 ,5,  6,  7, 800, OILED_DEVICE_ACTIVE_PIN1, 3 }, 
	{ 8, 9, 10, 11, 800, OILED_DEVICE_ACTIVE_PIN2, 4 }			// more oil drips on second motor for example
};
*/
#else
// Following is used to define a set of relays used to drive a dc motor and an associated sensor that signals when they have output ( e.g. drop of oil)
struct
{
	uint8_t		Pin1;
	uint8_t		MotorOutputPin;
	uint8_t		Drips;
} RelayMotor [ NUM_MOTORS ] =
// One relay motor example
{
	{ 4, OILED_DEVICE_ACTIVE_PIN1, 3 }
};
// Two relay motor config example, NB change NUM_MOTORS above to 2
/*
{
	{ 4, OILED_DEVICE_ACTIVE_PIN1, 3 },
	{ 5, OILED_DEVICE_ACTIVE_PIN2, 4 }							// more oil drips on second motor for example
};
*/
#endif
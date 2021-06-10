//
//  Oiler.h
// 
// (c) Mark Naylor June 2021
//
//	This class is encapsulates the main functionality of thia project
//	The oiler consists of one or more motors and for each motor it has an associated input signal that tells when the motor has completed a unit of work. 
//  In this case a unit of work is a drop of oil.
// 
//  At its simplest the oiler will output oil on a timed basis, if the oiler is provided with a TargetMachine object that represents the machine being oiled, it will 
//  use this object to detect elapsed time the machine is active (ie ignore idle time) and / or when the TargetMachine has done an amount of work. In our lathe example this is 
//  tied to the number of revolutions it does.
//

#ifndef _OILER_h
#define _OILER_h

#if defined(ARDUINO) && ARDUINO >= 100
	#include <Arduino.h>
#else
	#include "WProgram.h"
#endif
#include "RelayMotor.h"
#include "FourPinStepperMotor.h"
#include "TargetMachine.h"

#define		MAX_MOTORS					2					// MAX the oiler can support
#define		MOTOR_WORK_SIGNAL_MODE		HIGH				// LEVEL of signal when motor output (eg oil seen) is signalled
#define		MOTOR_WORK_SIGNAL_PINMODE	INPUT_PULLUP
#define		TIME_BETWEEN_OILING			30					// In seconds
#define		NUM_ACTION_EVENTS			3					// number of times device being oiled signals action has been taken after which motor(s) is(are) started to deliver oil

class OilerClass
{
 public:
	enum eMode { ON_TIME, ON_POWERED_TIME, ON_TARGET_ACTIVITY, NONE };

			OilerClass ( TargetMachineClass* pMachine = NULL );
	bool	On ();
	void	Off ();
	void	SetMotorsForward ( void );
	void	SetMotorsBackward ( void );
	bool	AddMotor ( uint8_t uiPin1, uint8_t uiPin2, uint8_t uiPin3, uint8_t uiPin4, uint32_t ulSpeed, uint8_t uiWorkPin );		// FourPin Stepper version
	bool	AddMotor ( uint8_t uiPin, uint8_t uiWorkPin );																			// one pin relay version
	void	MotorWork ( uint8_t uiMotorIndex );
	bool	SetMode ( eMode Mode, uint32_t uiModeTarget );
	eMode	GetMode ( void );
	void	CheckElapsedTime ( void );					// Checks time running since olier last finished - this is the basic version not using TargetMachine
	void	CheckTargetReady ( void );					// Checks if target is ready for oil
	void	AddMachine ( TargetMachineClass* pMachine );
	uint8_t GetMotorWorkCount ( uint8_t uiMotorNum );
	MotorClass::eState GetMotorState ( uint8_t uiMotorNum );
	uint32_t GetTimeOilerIdle ( void );					// returns time in seconds the Oiler has been idle (all motors off)
	bool	AllMotorsStopped ( void );

 protected:
	 eMode					m_OilerMode;
	 uint32_t				m_ModeTarget;
	 TargetMachineClass*	m_pMachine;
	 uint32_t				m_timeOilerStopped;
	 union
	 {
		 uint32_t m_ulOilTime;
		 uint32_t m_ulWorkTarget;
	 };
	 struct
	 {
		 uint8_t					uiNumMotors;
		 uint8_t					uiWorkPin [ MAX_MOTORS ];
		 MotorClass*				Motor [ MAX_MOTORS ];
		 uint8_t					uiWorkCount [ MAX_MOTORS ];
	 }						m_Motors;
};

extern OilerClass TheOiler;

#endif


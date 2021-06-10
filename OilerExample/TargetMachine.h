//
// TargetMachine.h
//
// (c) Mark Naylor 2021
//
// This class represents the machine that is being oiled
//
// It has two attributes: 
//		A signal that indicates the machine is active
//		A signal that indicates the machine has completed a unit of work
//
//	In our example machine - a metal working lathe, the active signal indicaates the machine is moving and the unit of work is a completed full rototation of the lather spindle
//
// The class keeps track of active time and number of units of work completed. These are optional inputs for the Oiler class to refine when it delivers oil.
//
#ifndef _TARGETMACHINE_h
#define _TARGETMACHINE_h

#if defined(ARDUINO) && ARDUINO >= 100
#include <Arduino.h>
#else
#include "WProgram.h"
#endif

#define		MACHINE_ACTIVE_PIN			12					// Pin used to indicate machine is doing work, set to NOT_A_PIN if this feature is not implemented
#define		MACHINE_ACTIVE_PIN_MODE		INPUT_PULLUP		// Change to INPUT if internal Arduino pullups not needed
#define		MACHINE_ACTIVE_STATE		HIGH				// signal goes LOW when active, chaneg to HIGH if that is how target machine works
#define		MACHINE_ACTIVE_TIME_TARGET	30					// Number of seconds of active time after which machine is ready

#define		MACHINE_WORK_PIN			13					// Pin on which pulse is sent when a unit of work is completed, needs to be a pin that can be monitored by Pin Change Interrupts, set to NOT_A_PIN if not implemented
#define		MACHINE_WORK_PIN_MODE		INPUT_PULLUP		// Change to INPUT if internal Arduino pullups not needed
#define		MACHINE_WORK_PIN_STATE		LOW					// signal goes LOW when active, change to HIGH if that is how target machine works
#define		WORK_UNITS_TARGET			3					// Number of signals that indicates machine is ready (eg how many revolutions of spindle)

typedef void ( *InterruptCallback )( void );

class TargetMachineClass
{
public:
	enum eMachineState	{ READY, NOT_READY, NO_FEATURES };		// Ready to be oiled or can't tell
	enum eActiveState	{ IDLE, ACTIVE };
					TargetMachineClass ( void );
	void			RestartMonitoring ( void );					// restart monitoring Machine activity and work units completed
	eMachineState	IsReady ( void );
	uint32_t		GetActiveTime ( void );						// Active time in secs since oiler stopped
	uint32_t		GetWorkUnits ( void );						// number of work units since oiler stopped
	void			IncActiveTime ( uint32_t tActive );
	void			GoneActive ( uint32_t tNow );
	void			IncWorkUnit ( uint32_t ulIncAmoount );
	bool			SetActiveTimeTarget ( uint32_t ulTargetSecs );
	bool			SetWorkTarget ( uint32_t ulTargetUnits );
protected:
	eMachineState	m_State;
	eActiveState	m_Active;
	uint32_t		m_timeActive;							// time machine has been active since monitor reset
	uint32_t		m_timeActiveStarted;					// time machine lase went active
	uint32_t		m_ulWorkUnitCount;
	uint32_t		m_ulTargetSecs;
	uint32_t		m_ulTargetUnits;
	void EnablePCI ( uint8_t uiPin, InterruptCallback Fn );	// sets up Pin ChangeInterrupt for given Pin Num
};

extern TargetMachineClass TheMachine;

#endif


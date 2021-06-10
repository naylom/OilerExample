//
// TargetMachine.cpp
//
// (c) Mark Naylor 2021
//
// This class encapsulates the machine that is being oiled
//
// It has two optional attributes: 
//		A signal that indicates the machine is active
//		A signal that indicates the machine has completed a unit of work
//
//	In our example machine - a metal working lathe, the active signal indicates the machine is moving and the unit of work is a completed full rototation of the lather spindle
//
// The class keeps track of active time and number of units of work completed. These are optional inputs for the Oiler class to refine when it delivers oil.
//

#include "TargetMachine.h"
#define IsInThisPCIR( digitalPin, Port ) ( digitalPinToPort ( digitalPin ) -  2 == Port ? true: false)

// Routine to be called if MACHINE_ACTIVE_PIN is signalled - called by interrupt
void MachineActiveSignal ( void )
{
	//static uint32_t	timeStarted = millis ();
	uint32_t tNow = millis ();

	// see how machine has changed state
	if ( digitalRead ( MACHINE_ACTIVE_PIN ) == MACHINE_ACTIVE_STATE )
	{
		// machine gone active so remember when this started
		TheMachine.GoneActive ( tNow );
	}
	else
	{
		// machine gone idle so calc time was active and save it
		TheMachine.IncActiveTime ( tNow );
	}
}

// Routine to be called if MACHINE_WORK_PIN is signalled - called by interrupt
void MachineWorkUnitSignal ( void )
{
	if ( digitalRead ( MACHINE_WORK_PIN ) == MACHINE_WORK_PIN_STATE )
	{
		TheMachine.IncWorkUnit ( 1 );
	}
}

// Class routines
TargetMachineClass::TargetMachineClass ( void )
{
	m_ulTargetSecs = MACHINE_ACTIVE_TIME_TARGET;		// set default
	m_ulTargetUnits = WORK_UNITS_TARGET;
	RestartMonitoring ();
	
	if ( MACHINE_ACTIVE_PIN != NOT_A_PIN )
	{
		pinMode ( MACHINE_ACTIVE_PIN, MACHINE_ACTIVE_PIN_MODE );
		EnablePCI ( MACHINE_ACTIVE_PIN, MachineActiveSignal );
	}
	if ( MACHINE_WORK_PIN != NOT_A_PIN )
	{
		pinMode ( MACHINE_WORK_PIN, MACHINE_WORK_PIN_MODE );
		EnablePCI ( MACHINE_WORK_PIN, MachineWorkUnitSignal );
	}
	if ( MACHINE_ACTIVE_PIN != NOT_A_PIN && MACHINE_WORK_PIN != NOT_A_PIN )
	{
		m_State = NO_FEATURES;
	}
}

void TargetMachineClass::RestartMonitoring ( void )
{
	m_timeActive = 0;
	m_ulWorkUnitCount = 0;
	if ( m_State != NO_FEATURES )
	{
		m_State = NOT_READY;
		m_Active = MACHINE_ACTIVE_PIN == NOT_A_PIN ? IDLE : digitalRead ( MACHINE_ACTIVE_PIN ) == MACHINE_ACTIVE_STATE ? ACTIVE : IDLE;
		if ( m_Active == ACTIVE )
		{
			m_timeActiveStarted = millis ();
		}
	}
}

TargetMachineClass::eMachineState TargetMachineClass::IsReady ( void )
{
	// Check time is up to date
	if ( m_Active == ACTIVE )
	{
		// add time to now and check if passed threshold
		uint32_t tNow = millis ();
		IncActiveTime ( tNow );
		m_timeActiveStarted = tNow;
	}
	return m_State;
}

uint32_t TargetMachineClass::GetActiveTime ( void )
{
	return m_timeActive / 1000;
}

uint32_t TargetMachineClass::GetWorkUnits ( void )
{
	return m_ulWorkUnitCount;
}

// add active time in milliseconds to total since machine became active
void TargetMachineClass::IncActiveTime ( uint32_t tNow )
{
	m_timeActive += (tNow - m_timeActiveStarted );
	if ( m_timeActive / 1000 >= m_ulTargetSecs )
	{
		m_State = READY;
	}
	m_Active = digitalRead ( MACHINE_ACTIVE_PIN ) == MACHINE_ACTIVE_STATE ? ACTIVE : IDLE;
}

void TargetMachineClass::GoneActive ( uint32_t tNow )
{
	m_Active = ACTIVE;
	m_timeActiveStarted = tNow;
}

void TargetMachineClass::IncWorkUnit ( uint32_t ulIncAmoount )
{
	m_ulWorkUnitCount += ulIncAmoount;
	if ( m_ulWorkUnitCount >= m_ulTargetUnits )
	{
		m_State = READY;
	}
}

bool TargetMachineClass::SetActiveTimeTarget ( uint32_t ulTargetSecs )
{
	bool bResult = false;
	if ( MACHINE_ACTIVE_PIN != NOT_A_PIN )
	{
		m_ulTargetSecs = ulTargetSecs;
		bResult = true;
	}
	return bResult;
}

bool TargetMachineClass::SetWorkTarget ( uint32_t ulTargetUnits )
{
	bool bResult = false;
	if ( MACHINE_WORK_PIN != NOT_A_PIN )
	{
		m_ulTargetUnits = ulTargetUnits;
		bResult = true;
	}
	return bResult;
}

void TargetMachineClass::EnablePCI ( uint8_t uiPin, InterruptCallback Fn )
{
	// enable PCI interrupts and set pin
	*digitalPinToPCMSK ( uiPin ) |= ( 1 << digitalPinToPCMSKbit ( uiPin ) );
	 *digitalPinToPCICR ( uiPin ) |= ( 1 << digitalPinToPCICRbit ( uiPin ) );
}


volatile static uint8_t PCintLastValues [ 3 ];					// holds the prior PCINT pin values, used to determine when one changes.

// Called when Pin Change interrupt seen on uiPort (2,3 or 4)
void PCICheckPins ( uint8_t uiPortWithInterrupt )
{

	uint8_t uiCurrentPCIReg = *portInputRegister ( uiPortWithInterrupt );	// Get prior state of pins on this port
	// check if MACHINE_ACTIVE_PIN is on this port
	uint8_t uiPinPort = digitalPinToPort ( ( MACHINE_ACTIVE_PIN ) );  // digitalPinToPort returns 2,3 or 4
	uint8_t uiChangedPins;

	// Check if MACHINE_ACTIVE_PIN is handled by this ISR
	if ( uiPinPort == uiPortWithInterrupt )
	{
		// compare to last value
		uiChangedPins = uiCurrentPCIReg ^ PCintLastValues [ uiPinPort - 2 ];
		if ( uiChangedPins & digitalPinToBitMask ( MACHINE_ACTIVE_PIN ) )
		{
			MachineActiveSignal ();
		}
	}
	// Check if MACHINE_WORK_PIN is handled by this ISR
	uiPinPort = digitalPinToPort ( ( MACHINE_WORK_PIN ) );  // digitalPinToPort returns 2,3 or 4
	if ( uiPinPort == uiPortWithInterrupt )
	{
		// compare to last value
		uiChangedPins = uiCurrentPCIReg ^ PCintLastValues [ uiPinPort - 2 ];
		if ( uiChangedPins & digitalPinToBitMask ( MACHINE_WORK_PIN ) )
		{
			MachineWorkUnitSignal ();
		}
	}
	PCintLastValues [ uiPortWithInterrupt - 2 ] = uiCurrentPCIReg;
}

//Pin Change Interrupt routines, each handles a different set of pins
ISR ( PCINT0_vect )
{
	PCICheckPins ( 2 );
}
ISR ( PCINT1_vect )
{
	PCICheckPins ( 3 );
}
ISR ( PCINT2_vect )
{
	PCICheckPins ( 4 );
}

TargetMachineClass TheMachine;				// Create instance


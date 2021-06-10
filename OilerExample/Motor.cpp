// 
// 
// 

#include "Motor.h"

MotorClass::MotorClass ( uint32_t ulSpeed )
{
	SetSpeed ( ulSpeed );
	m_eState			= STOPPED;
	m_ulTimeStartedms	= 0;
	m_ulTimeStoppedms	= 0;
	m_eDir				= FORWARD;
}

bool MotorClass::On ( void )
{
	m_ulTimeStartedms = millis ();
	m_eState = RUNNING;
	return true;
}

bool MotorClass::Off ( void )
{
	m_ulTimeStoppedms = millis ();
	m_eState = STOPPED;
	return true;
}

uint32_t MotorClass::GetTimeMotorStarted ( void )
{
	return 	m_ulTimeStartedms = 0;
}

uint32_t MotorClass::GetTimeMotorStopped ( void )
{
	return m_ulTimeStoppedms;
}

MotorClass::eState MotorClass::GetMotorState ( void )
{
	return m_eState;
}

uint32_t MotorClass::GetSpeed ( void )
{
	return m_ulSpeed;
}

bool MotorClass::SetSpeed ( uint32_t ulSpeed )
{
	m_ulSpeed = ulSpeed;
	return false;
}

void MotorClass::SetDirection ( eDirection eDir )
{
	// Save requested direction
	m_eDir = eDir;
}


// MotorClass Motor;

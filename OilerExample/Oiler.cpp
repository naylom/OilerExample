// 
//  Oiler.cpp
// 
// (c) Mark Naylor June 2021

#include "Oiler.h"
#include "Timer.h"

void Motor1WorkSignal ( void )
{
	static uint32_t ulLastSignal = 0;
	if ( ( millis () - ulLastSignal ) > DEBOUNCE_THRESHOLD )
	{
		TheOiler.MotorWork ( 0 );
	}
}

void Motor2WorkSignal ( void )
{
	static uint32_t ulLastSignal = 0;
	if ( millis () - ulLastSignal > DEBOUNCE_THRESHOLD )
	{
		TheOiler.MotorWork ( 1 );
	}
}

// list of ISRs for each possible motor
typedef void ( *Callback )( void );
struct
{
	Callback MotorWorkCallback [ MAX_MOTORS ];
} MotorISRs =
{
	Motor1WorkSignal,
	Motor2WorkSignal
};

void OilerTmerCallback ( void )
{
	if ( TheOiler.GetStatus () != OilerClass::OFF )
	{
		// Invoked once per second to check if oiler needs starting
		switch ( TheOiler.GetStartMode () )
		{
			case OilerClass::ON_TIME:
				TheOiler.CheckElapsedTime ();
				break;

			case OilerClass::ON_POWERED_TIME:
			case OilerClass::ON_TARGET_ACTIVITY:
				TheOiler.CheckTargetReady ();
				break;

			default:
				break;
		}
	}
}

OilerClass::OilerClass ( TargetMachineClass* pMachine )
{
	m_pMachine = pMachine;
	m_OilerMode = ON_TIME;
	m_OilerStatus = OFF;
	m_ulOilTime = TIME_BETWEEN_OILING;
	m_Motors.uiNumMotors = 0;
}

bool OilerClass::On ()
{
	// can only start if > 0 motors!
	bool bResult = false;
	if ( m_Motors.uiNumMotors > 0 )
	{
		for ( uint8_t i = 0; i < m_Motors.uiNumMotors; i++ )
		{
			m_Motors.Motor [ i ]->On ();
			m_Motors.uiWorkCount [ i ] = 0;
		}
		// if we have a machine, start monitoring
		if ( m_OilerMode != ON_TIME && m_pMachine != NULL )
		{
			m_pMachine->RestartMonitoring ();
		}

		TheTimer.AddCallBack ( OilerTmerCallback, RESOLUTION );		// callback once per sec
		m_OilerStatus = OILING;
		bResult = true;
	}
	return bResult;
}

void OilerClass::Off ()
{
	for ( uint8_t i = 0; i < m_Motors.uiNumMotors; i++ )
	{
		m_Motors.Motor [ i ]->Off ();
	}
	m_OilerStatus = OFF;
	m_timeOilerStopped = millis ();
}

bool OilerClass::AddMotor ( uint8_t uiPin1, uint8_t uiPin2, uint8_t uiPin3, uint8_t uiPin4, uint32_t ulSpeed, uint8_t uiWorkPin )
{
	bool bResult = false;
	if ( m_Motors.uiNumMotors < MAX_MOTORS && digitalPinToInterrupt  ( uiWorkPin ) != NOT_AN_INTERRUPT )
	{
		// space to add another motor
		m_Motors.Motor [ m_Motors.uiNumMotors ] = (MotorClass*)new FourPinStepperMotorClass ( uiPin1, uiPin2, uiPin3, uiPin4, ulSpeed );
		m_Motors.uiWorkPin [ m_Motors.uiNumMotors ] = uiWorkPin;
		pinMode ( uiWorkPin, MOTOR_WORK_SIGNAL_PINMODE );
		attachInterrupt ( digitalPinToInterrupt ( uiWorkPin ), MotorISRs.MotorWorkCallback [ m_Motors.uiNumMotors ], MOTOR_WORK_SIGNAL_MODE );
		//attachInterrupt ( digitalPinToInterrupt ( uiWorkPin ), Motor1WorkSignal, MOTOR_WORK_SIGNAL_MODE );
		m_Motors.uiNumMotors++;
		bResult = true;
	}
	return bResult;
}

bool OilerClass::AddMotor ( uint8_t uiPin, uint8_t uiWorkPin )
{
	bool bResult = false;
	if ( m_Motors.uiNumMotors < MAX_MOTORS && digitalPinToInterrupt ( uiWorkPin ) != NOT_AN_INTERRUPT )
	{
		// space to add another motor
		m_Motors.Motor [ m_Motors.uiNumMotors ] = (MotorClass *)new RelayMotorClass  ( uiPin );
		pinMode ( uiWorkPin, MOTOR_WORK_SIGNAL_PINMODE );
		attachInterrupt ( digitalPinToInterrupt ( uiWorkPin ), MotorISRs.MotorWorkCallback [ m_Motors.uiNumMotors ], MOTOR_WORK_SIGNAL_MODE );
		//attachInterrupt ( digitalPinToInterrupt ( uiWorkPin ), Motor1WorkSignal, MOTOR_WORK_SIGNAL_MODE );
		m_Motors.uiNumMotors++;
		bResult = true;
	}
	return bResult;
}

void	OilerClass::AddMachine ( TargetMachineClass* pMachine )
{
	m_pMachine = pMachine;
}

void OilerClass::MotorWork ( uint8_t uiMotorIndex )
{
	// One of Oiler motors has completed work
	m_Motors.uiWorkCount [ uiMotorIndex ]++;
	// check if it has hit target
	if ( m_Motors.uiWorkCount [ uiMotorIndex ] >= NUM_ACTION_EVENTS )
	{
		// hit target, stop motor
		m_Motors.Motor [ uiMotorIndex ]->Off ();

		// have we stopped all motors
		if ( AllMotorsStopped() )
		{	
			// restart monitoring, if we have a machine
			if ( m_pMachine != NULL && m_OilerMode != ON_TIME )
			{
				m_pMachine->RestartMonitoring ();
				m_timeOilerStopped = millis ();
			}
			// reset start time count
			if ( m_OilerMode == ON_TIME )
			{
				m_timeOilerStopped = millis ();
			}
			m_OilerStatus = IDLE;
		}
	}
}

OilerClass::eStartMode OilerClass::GetStartMode ( void )
{
	return m_OilerMode;
}

OilerClass::eStatus OilerClass::GetStatus ( void )
{
	return m_OilerStatus;
}

void OilerClass::CheckTargetReady ( void )
{
	switch  ( m_OilerMode )
	{
		case ON_TARGET_ACTIVITY:
			if ( m_pMachine->MachineUnitsDone() )
			{
				On ();
			}
			break;

		case ON_POWERED_TIME:
			if ( m_pMachine->MachinePoweredTimeExpired () )
			{
				On ();
			}
			break;

		default:		// do nothing, shouldn't get here
			break;
	}
}

// returns time since Oiler went idle in seconds
uint32_t OilerClass::GetTimeOilerIdle ( void )
{
	uint32_t	ulResult = 0UL;
	
	// if no oiler motors pumping
	if ( AllMotorsStopped () || GetStartMode() != NONE )
	{
		ulResult = ( millis () - m_timeOilerStopped ) / 1000;
	}
	return ulResult;
}

// called ny timer callback to restart oiler if necessary
void OilerClass::CheckElapsedTime ()
{
	if ( GetTimeOilerIdle () > m_ulOilTime )
	{
		On ();
	}
}

uint8_t OilerClass::GetMotorWorkCount ( uint8_t uiMotorNum )
{
	uint8_t uiResult = 0;
	if ( uiMotorNum < m_Motors.uiNumMotors )
	{
		uiResult =  m_Motors.uiWorkCount [ uiMotorNum ];
	}
	return uiResult;
}

MotorClass::eState OilerClass::GetMotorState ( uint8_t uiMotorNum )
{
	MotorClass::eState eResult = MotorClass::STOPPED;
	if ( uiMotorNum < m_Motors.uiNumMotors )
	{
		eResult = m_Motors.Motor [ uiMotorNum ]->GetMotorState ();
	}
	return eResult;
}

bool OilerClass::AllMotorsStopped ( void )
{
	bool bResult = true;
	for ( uint8_t i = 0; i < m_Motors.uiNumMotors; i++ )
	{
		if ( m_Motors.Motor [ i ]->GetMotorState() == MotorClass::RUNNING )
		{
			bResult = false;
			break;
		}
	}
	return bResult;
}

bool OilerClass::SetStartMode ( eStartMode Mode, uint32_t ulModeTarget )
{
	bool bResult = false;
	switch ( Mode )
	{
		case ON_TIME:
			m_ulOilTime = ulModeTarget;
			m_OilerMode = Mode;
			bResult = true;
			break;

		case ON_POWERED_TIME:
			if ( m_pMachine != NULL )
			{
				if ( m_pMachine->SetActiveTimeTarget ( ulModeTarget ) )
				{
					m_ulOilTime = ulModeTarget;
					m_OilerMode = Mode;
					bResult = true;
				}
			}
			break;

		case ON_TARGET_ACTIVITY:
			if ( m_pMachine != NULL )
			{
				if ( m_pMachine->SetWorkTarget ( ulModeTarget ) )
				{
					m_ulWorkTarget = ulModeTarget;
					m_OilerMode = Mode;
					bResult = true;
				}
			}
			break;

		default:
			break;
	}
	return bResult;
}

void OilerClass::SetMotorsForward ( void )
{
	for ( uint8_t i = 0; i < m_Motors.uiNumMotors; i++ )
	{
		m_Motors.Motor [ i ]->SetDirection ( MotorClass::FORWARD );
	}
}

void OilerClass::SetMotorsBackward ( void )
{
	for ( uint8_t i = 0; i < m_Motors.uiNumMotors; i++ )
	{
		m_Motors.Motor [ i ]->SetDirection ( MotorClass::BACKWARD );
	}
}


OilerClass TheOiler;


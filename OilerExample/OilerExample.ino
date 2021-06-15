/*
 Name:		OilerExample.ino
 Created:	June 4 2021 1:24:23 PM
 Author:	Mark Naylor
*/

/*
This project is designed to implement an oiler for a metal lathe.

The Oiler consists of multiple motors to pump oil and for each motor there is a sensor that signals as drips of oil are delivered.

Additionally there can be inputs from the lathe that signal when the lathe is powered on/off and additionally for each revolution of its spindle. If these inputs are present the
system has additional options to drive when the oiling starts.

This is sketch consists of an example 'OilerExample.ino' sketch that demonstrates how to use the oiler codebase.

The code is implemted in an object orientated approach. The code base defines and instatiates an object to represent the oiler system, called TheOiler.  It also instantiates an object to represent the device being oiled, called TheMachine which
tracks machine units (number of spindle revolutions) and number of seconds of powered on time that has elapsed and finally there is an object called TheTimer that schedules work on requested basis.

When run, the first step is to add motor(s) to TheOiler and provide the pin(s) that signal when the motor generates output (referred to as a unit of work). Once done, TheOiler can be turned on. In this mode it waits for a configurable time
(in seconds) and then starts the motors. As the motors result in drips of oil, this is detected by the sensor and a count kept. The timer object regularly checks for the number of output 'units' and stops the related motor when a configurable
target is reached. When all motors are idle the Timer starts to monitors elasped time and restarts the motors when a configurable length of time has passed.

The functionality above can be enhanced by adding TheMachine object to the TheOiler. Once TheOiler 'knows' about TheMachine it can query TheMachine object about how many revolutions the spindle has done (described in the code as machine
work units) and also how long the lathe has been powered on. This information allows the Oiler to restart the motors on machine units (spindle revolutions) completed or on elapsed powered up time.

Currently the system supports two types of motors, a relay motor which uses the output of a single pin to drive a relay which in turn drives a dc motor. The second type is a four pin stepper driver & motor. This latter type has been tested
using a ULN2003 stepper driver and 28BYJ-48 stepper motor. This latter motor has additional features that the dc motor does not implement e.g. ability to programatically change the motor direction and the ability to alter the speed. For the
relay motor changing direction would be a wiring change at project setup.

Since the code uses a timer to monitor progress there is no need for a user to call any special function in the arduino loop function or worry that the loop is taking a long time and impacting the oiler processing. The loop is free for other
uses such as a user interface to monitor and control TheOiler.
The arduino setup function should be used to add motors to TheOiler, attach TheMachine to TheOiler, if TheMachine inputs are implemented in the project install, and finally turn TheOiler on.

In this example there is code to demonstrate adding a 4 pin stepper motor, starting the oiler, changing direction, using inputs from 'TheMachine' to start the oiler after spindle revolutions or elapsed powered on time. A rudimentary system monitor
and user interface is implemented in the loop function. This uses ANSI terminal control sequences (see https://en.wikipedia.org/wiki/ANSI_escape_code#Fe_Escape_sequences). To benefit from this connect to the Arduino serial port using a better terminal
monitor than the default one that comes with the arduino IDE. This was tested with the free version of PuTTY. Note to use this download the ketch to the arduino and take note of the port the arduino is on. Start your emaulator and connect to that port at 
the baud rate  used in the sketch (currently 19200) and off you go. Note that if you want to download the sketch again you will have to stop the terminal emulator so the arduino IDE can gain access.

*/

#include "RelayMotor.h"
#include "Configuration.h"
#include "Oiler.h"

void setup ()
{
	Serial.begin ( 19200 );
	while ( !Serial );
	ClearScreen ();

	// Add motors to Oiler - see Configuration.h
#ifdef USING_STEPPER_MOTORS
	/* For the purpose of showing how this is done, the next two commented out lines work but the following loop is more elegant 
	TheOiler.AddMotor ( 4, 5, 6, 7, 800, 2, 3 );
	TheOiler.AddMotor ( 8, 9, 10, 11, 800, 3, 4 );
	*/
	for ( uint8_t i = 0; i < NUM_MOTORS; i++ )
	{
		if ( TheOiler.AddMotor ( FourPinMotor [ i ].Pin1, 
								 FourPinMotor [ i ].Pin2, 
								 FourPinMotor [ i ].Pin3, 
								 FourPinMotor [ i ].Pin4, 
								 FourPinMotor [ i ].Speed , 
								 FourPinMotor [ i ].MotorOutputPin,
								 FourPinMotor [ i ].Drips
							   ) == false 
		   )
		{
			Error ( F ( "Unable to add motor to oiler, stopped" ));
			while ( 1 );
		}
	}
	/*
	*		Example using relays to drive a dc motor
	*/
#else
	for ( uint8_t i = 0; i < NUM_MOTORS; i++ )
	{
		if ( TheOiler.AddMotor ( RelayMotor [ i ].Pin1, RelayMotor [ i ].MotorOutputPin, RelayMotor [ i ].Drips ) == false )
		{
			Error ( F ( "Unable to add motor to oiler, stopped" ));
			while ( 1 );
		}
	}
#endif

	// This next step is optional, the Oiler will work without it. It simply gives a better experience.
	// TheMachine that uses two pins to signal when the machine to be oiled is powered on and also when it has completed a 
	// unit of work (e.g. a revolution of a spindle). See TargetMachine.h for configuration of these pins
	// as well as the number of units of work and elapsed time of machine being powered on after which it is ready to be oiled.
	// Note that its not necessary to have both configured if the feature is not available
	
	// Since we have one, we optionally tell TheOiler it exists as follows:
	TheOiler.AddMachine ( &TheMachine );

	// Demonstrate how to turn on functionalty that will generate a signal if oiling takes too long
	if ( TheOiler.SetAlert ( ALERT_PIN, ALERT_THRESHOLD ) )
	{
		Error ( F ( "Unable to add Alert feature to oiler, stopped" ) );
		while ( 1 );
	}

	// By default the oiler will work on an elapsed time basis, see Oiler.h, can also change here the signal level expected when oil is output
	// Other options to below are :
	// TheOiler.SetStartMode ( OilerClass::ON_TARGET_ACTIVITY, NUM_ACTION_EVENTS )		// using TheMachine
	// TheOiler.SetStartMode ( OilerClass::ON_TIME, ELAPSED_TIME_SECS )					// Not using TheMachine, just oiling every n seconds
	//if ( TheOiler.SetStartMode ( OilerClass::ON_POWERED_TIME, ELAPSED_TIME_SECS ) == false )
	//{
	//	Error ( "Unable to set oiler operating mode, stopped" );
	//	while ( 1 );
	//}
	DisplayMenu ();
}
void loop ()
{

	// All work should happen in the background
	// loop can be used to control oiler or do other functions as below

	if ( Serial.available() > 0 ) 
	{
		switch ( Serial.read() )
		{
			case '1':	// On
				if ( TheOiler.On () == false )
				{
					Error ( F ( "Unable to start oiler, stopped"  ));
					while ( 1 );
				}
				else
				{
					DisplayOilerStatus ( F ( "Oiler Started" ) );
				}
				break;

			case '2':	// Off
				TheOiler.Off ();
				DisplayOilerStatus ( F ( "Oiler Stopped" ));
				break;

			case '3':	// Motors Forward
				TheOiler.SetMotorsForward ();
				DisplayOilerStatus ( F ( "Oiler moving forward" ) );
				break;

			case '4':	// Motors Backward
				TheOiler.SetMotorsBackward ();
				DisplayOilerStatus ( F ( "Oiler moving backward" ) );
				break;

			case '5': // TIME_ONLY - basic mode -  oil every 30 secs regardless
				if ( TheOiler.SetStartMode ( OilerClass::ON_TIME, ELAPSED_TIME_SECS ) )
				{
					DisplayOilerStatus ( F ( "Oiler in basic ON_TIME mode" ) );
				}
				else
				{
					Error ( F ( "Unable to set ON_TIME mode" ) );
				}
				break;

			case '6': // POWERED_ON - adv mode - oil every 30 secs target machine is powered on
				if ( TheOiler.SetStartMode ( OilerClass::ON_POWERED_TIME, ELAPSED_TIME_SECS ) )
				{
					DisplayOilerStatus ( F ( "Oiler in advanced ON_POWERED_TIME mode" ) );
				}
				else
				{
					Error ( F ( "Unable to set ON_POWERED_TIME mode" ) );
				}
				break;

			case '7': // WORK_UNITS - adv mode - oil every 3 units done by target machine, eg every n spindle revs
				if ( TheOiler.SetStartMode ( OilerClass::ON_TARGET_ACTIVITY, 3 ) )
				{
					DisplayOilerStatus ( F ( "Oiler in advanced ON_TARGET_ACTIVITY mode" ) );
				}
				else
				{
					Error ( F ( "Unable to set ON_TARGET_ACTIVITY mode" ) );
				}
				break;

			default:
				break;
		}
	}
	// display work units per motor
	DisplayStats ();
}

// code to draw screen
#define ERROR_ROW			25
#define ERROR_COL			1
#define STATUS_LINE			24
#define STATUS_START_COL	40
#define STATS_ROW			8
#define STATS_RESULT_COL	70
#define MODE_ROW			20
#define MODE_RESULT_COL		45
#define MAX_COLS			80
#define MAX_ROWS			25

// defines for ansi terminal sequences
#define CSI				F("\x1b[")
#define SAVE_CURSOR		F("\x1b[s")
#define RESTORE_CURSOR	F("\x1b[u")
#define CLEAR_LINE		F("\x1b [2K")
#define RESET_COLOURS   F("\x1b[0m")

// colors
#define FG_BLACK		30
#define FG_RED			31
#define FG_GREEN		32
#define FG_YELLOW		33
#define FG_BLUE			34
#define FG_MAGENTA		35
#define FG_CYAN			36
#define FG_WHITE		37

#define BG_BLACK		40
#define BG_RED			41
#define BG_GREEN		42
#define BG_YELLOW		43
#define BG_BLUE			44
#define BG_MAGENTA		45
#define BG_CYAN			46
#define BG_WHITE		47

// following are routines to output ANSI style terminal emulation
void DisplayMenu ()
{
	String Heading = F ( "Oiler Example Sketch, Version " );
	Heading += String ( OILER_VERSION ); 
	COLOUR_AT ( FG_GREEN, BG_BLACK, 1, 30, Heading );
	AT ( 5, 10, F( "1 - Turn Oiler On" ) );
	AT ( 6, 10, F ( "2 - Turn Oiler 0ff" ) );
	AT ( 7, 10, F ( "3 - Motors Forward" ) );
	AT ( 8, 10, F ( "4 - Motors Backward" ) );
	AT ( 9, 10, F ( "5 - TIME_ONLY Mode" ) );
	AT ( 10, 10, F ( "6 - POWERED_ON Time" ) );
	AT ( 11, 10, F ( "7 - Machine WORK UNITS" ) );
	AT ( STATS_ROW - 1 , STATS_RESULT_COL - 14, F ( "STATS" ) );
	AT ( STATS_ROW + 0, STATS_RESULT_COL - 14, F ( "Oiler Idle    N/A" ) );
	AT ( STATS_ROW + 1, STATS_RESULT_COL - 14, F ( "Motor1 Units  N/A") );
	AT ( STATS_ROW + 2, STATS_RESULT_COL - 14, F ( "Motor1 State  N/A" ) );
	AT ( STATS_ROW + 3, STATS_RESULT_COL - 14, F ( "Motor1 Act(s) N/A" ) );
	AT ( STATS_ROW + 4, STATS_RESULT_COL - 14, F ( "Motor2 Units  N/A" ) );
	AT ( STATS_ROW + 5, STATS_RESULT_COL - 14, F ( "Motor2 State  N/A" ) );
	AT ( STATS_ROW + 6, STATS_RESULT_COL - 14, F ( "Motor2 Act(s) N/A" ) );
	AT ( STATS_ROW + 7, STATS_RESULT_COL - 14, F ( "Machine Units N/A" ) );	
	AT ( STATS_ROW + 8, STATS_RESULT_COL - 14, F ( "Machine Time  N/A" ) );
	AT ( MODE_ROW + 0, MODE_RESULT_COL - 14, F ( "Oiler Mode    None" ) );
	AT ( MODE_ROW + 1, MODE_RESULT_COL - 14, F ( "Oiler Status  OFF" ) );
}
const char* Modes [] =
{
	"ON TIME",
	"ON POWERED TIME",
	"ON TARGET ACTIVITY",
	"NONE"
};
const char* Statuses [] =
{
	"Oiling",
	"Off",
	"Idle"
};
void DisplayStats ( void )
{
	static uint8_t						uiLastCount [ NUM_MOTORS ];
	static MotorClass::eState			uiLastState [ NUM_MOTORS ];
	static uint32_t						ulLastMotorRunTime [ NUM_MOTORS ];
	static uint32_t						ulLastIdleSecs			= 0UL;
	static uint32_t						ulLastMachineUnits		= 0UL;
	static uint32_t						ulLastMachineIdleSecs	= 0UL;
	static OilerClass::eStartMode		OilerMode				= OilerClass::NONE;
	static OilerClass::eStatus			OilerStatus				= OilerClass::OFF;

	uint32_t ulIdleSecs = TheOiler.GetTimeOilerIdle ();
	if ( ulIdleSecs != ulLastIdleSecs )
	{
		ulLastIdleSecs = ulIdleSecs;
		ClearPartofLine ( STATS_ROW, STATS_RESULT_COL, MAX_COLS - STATS_RESULT_COL );
		AT ( STATS_ROW, STATS_RESULT_COL, String ( ulIdleSecs ) );
	}
	for ( int i = 0; i < NUM_MOTORS; i++ )
	{
		uint8_t uiWorkDone = TheOiler.GetMotorWorkCount ( i );
		if ( uiWorkDone != uiLastCount [ i ] )
		{
			uiLastCount [ i ] = uiWorkDone;
			ClearPartofLine ( STATS_ROW + i * 2 + 1, STATS_RESULT_COL, MAX_COLS - STATS_RESULT_COL );
			AT ( STATS_ROW + i * 2 + 1, STATS_RESULT_COL, String ( uiWorkDone ) );
		}
		MotorClass::eState eResult = TheOiler.GetMotorState ( i );
		if ( eResult != uiLastState [ i ] )
		{
			uiLastState [ i ] = eResult;
			ClearPartofLine ( STATS_ROW + i * 2 + 2, STATS_RESULT_COL, MAX_COLS - STATS_RESULT_COL );
			AT ( STATS_ROW + i * 2 + 2, STATS_RESULT_COL, eResult == MotorClass::STOPPED ? F ( "Stopped" ) : F ( "Running" ) );
		}
		uint32_t ulResult = TheOiler.GetTimeSinceMotorStarted( i );
		if ( ulResult != ulLastMotorRunTime [ i ] )
		{
			ulLastMotorRunTime [ i ] = ulResult;
			ClearPartofLine ( STATS_ROW + i * 2 + 3, STATS_RESULT_COL, MAX_COLS - STATS_RESULT_COL );
			AT ( STATS_ROW + i * 2 + 3, STATS_RESULT_COL, String ( ulResult ) );
		}
	}
	// Update machine info if necessary
	uint32_t ulMachineUnits = TheMachine.GetWorkUnits ();
	if ( ulMachineUnits != ulLastMachineUnits )
	{
		ulLastMachineUnits = ulMachineUnits;
		ClearPartofLine ( STATS_ROW + 7, STATS_RESULT_COL, MAX_COLS - STATS_RESULT_COL );
		AT ( STATS_ROW + 7, STATS_RESULT_COL, String ( ulMachineUnits ) );
	}
	uint32_t ulMachineIdleSecs = TheMachine.GetActiveTime();
	if ( ulMachineIdleSecs != ulLastMachineIdleSecs )
	{
		ulLastMachineIdleSecs = ulMachineIdleSecs;
		ClearPartofLine ( STATS_ROW + 8, STATS_RESULT_COL, MAX_COLS - STATS_RESULT_COL );
		AT ( STATS_ROW + 8, STATS_RESULT_COL, String ( ulMachineIdleSecs ) );
	}
	// Update mode and status if necessary
	OilerClass::eStartMode Mode = TheOiler.GetStartMode ();
	if ( OilerMode != Mode  )
	{
		OilerMode = Mode;
		ClearPartofLine ( MODE_ROW + 0, MODE_RESULT_COL, MAX_COLS - MODE_RESULT_COL );
		AT ( MODE_ROW + 0, MODE_RESULT_COL, Modes [ Mode ] );
	}
	OilerClass::eStatus Status = TheOiler.GetStatus ();
	if ( OilerStatus != Status )
	{
		OilerStatus = Status;
		ClearPartofLine ( MODE_ROW + 1, MODE_RESULT_COL, MAX_COLS - MODE_RESULT_COL );
		AT ( MODE_ROW + 1, MODE_RESULT_COL, Statuses [ Status ] );
	}
}

void ClearScreen ()
{
	Serial.print ( "\x1b[2J" );
}

void AT ( uint8_t row, uint8_t col, String s )
{
	row = row == 0 ? 1 : row;
	col = col == 0 ? 1 : col;
	String m = String ( CSI ) + row + String ( ";") + col + String ("H") + s;
	Serial.print ( m );
}

void COLOUR_AT ( uint8_t FGColour, uint8_t BGColour, uint8_t row, uint8_t col, String s ) 
{
	// set colours
	Serial.print ( String ( CSI  ) + FGColour + ";" + BGColour + "m" );
	AT ( row, col, s);
	// reset colours
	Serial.print ( RESET_COLOURS );
}

void ClearPartofLine ( uint8_t row, uint8_t start_col, uint8_t toclear )
{
	static char buf [ MAX_COLS  + 1];
	memset ( buf, ' ', sizeof (buf) - 1 );
	buf [ MAX_COLS ] = 0;

	// build string of toclear spaces
	toclear %= MAX_COLS + 1;						// ensure toclear is <= MAX_COLS
	if ( start_col + toclear > MAX_COLS + 1 )
	{
		toclear = MAX_COLS - start_col + 1;
	}
	toclear = ( MAX_COLS - start_col + 1 ) %  ( MAX_COLS + 1 );	// ensure toclear doesn't go past end of line
	SaveCursor ();
	buf [ toclear  ] = 0;
	AT ( row, start_col, buf );
	RestoreCursor ();
}

void ClearLine ( uint8_t row )
{
	SaveCursor ();
	AT ( row, 1, String ( CLEAR_LINE )  );
	RestoreCursor ();
}
void SaveCursor ( void )
{
	Serial.print ( SAVE_CURSOR );
}

void RestoreCursor ( void )
{
	Serial.print ( RESTORE_CURSOR );
}

void Error ( String s )
{
	// Clear error line
	ClearLine ( ERROR_ROW );
	// Output new error
	COLOUR_AT ( FG_RED, BG_BLACK, ERROR_ROW, ERROR_COL, s );
}

void DisplayOilerStatus ( String s )
{
	ClearPartofLine ( STATUS_LINE, STATUS_START_COL, MAX_COLS - STATUS_START_COL + 1 );
	AT ( STATUS_LINE, STATUS_START_COL, s );
}


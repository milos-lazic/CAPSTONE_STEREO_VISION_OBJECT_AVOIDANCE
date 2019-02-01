
#include "FSM.h"


FSM::FSM() : bEventGenerated(false), pEventData(nullptr) {}



void FSM::fsmExternEvent( unsigned int   newState,
	                      EventData     *pData)
{
	// if we are supposed to ignore the event
	if ( newState == EVENT_IGNORED)
	{
		// delete the data object (if any)
		if ( pData)
			delete pData;
	}
	else
	{
		fsmInternEvent( newState, pData);
		fsmRun();
	}
}


void FSM::fsmInternEvent( unsigned int   newState, 
	                      EventData     *pData)
{
	pEventData = pData;
	bEventGenerated = true;
	currentState = newState;
}


void FSM::fsmRun( void)
{
	const fsmState_t *map = nullptr;

	while( bEventGenerated)
	{
		// reset event flag
		bEventGenerated = false;

		map = fsmGetStateMap();
		(this->*map[currentState].pStateFunc)(pEventData);
		// call state function here


		// if event data was used, free it
		if ( pEventData)
		{
			delete pEventData;
			pEventData = nullptr;
		}
	}
}
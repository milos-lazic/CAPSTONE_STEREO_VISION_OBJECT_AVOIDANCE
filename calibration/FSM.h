#ifndef _FSM_H_
#define _FSM_H_

/* fsm state type structure declaration */
struct fsmState_t;

// event data structure
struct EventData
{

};


// base class for state machines
class FSM
{
public:
	FSM();
	virtual ~FSM() {}

protected:
	enum { EVENT_IGNORED = 0xFE, EVENT_INVALID };
	unsigned int currentState;

	void fsmExternEvent( unsigned int, EventData * = nullptr);
	void fsmInternEvent( unsigned int, EventData * = nullptr);
	virtual const fsmState_t *fsmGetStateMap() =0;

private:
	bool          bEventGenerated;
	EventData    *pEventData;

	void fsmRun( void);
};


typedef void (FSM::*fsmStateFunc)(EventData *);
struct fsmState_t
{
	fsmStateFunc pStateFunc;
};


#endif
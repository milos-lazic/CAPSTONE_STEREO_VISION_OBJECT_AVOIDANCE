#include "FLC.h"


#define DEPTH_LOW_THRESH     75  // TBD: 75 cm for now
#define DEPTH_MED_THRESH     150 // TBD: 125 cm for now
#define STDDEV_LOW_THRESH    15  // TBD: 10 for now
#define STDDEV_MED_THRESH    30  // TBD: 15 for now

#define EIGHT_BIT_FLC 0

#if EIGHT_BIT_FLC==1
#define NORM_LOW        0
#define NORM_LOWMID     63
#define NORM_MID        127
#define NORM_MIDHIGH    191
#define NORM_HIGH       255
#else
#define NORM_LOW        0
#define NORM_LOWMID     16384
#define NORM_MID        32768
#define NORM_MIDHIGH    49152
#define NORM_HIGH       65535
#endif


static const flcMfPoint_t LOW_MF_MAP[] =
{
	{ NORM_LOW,    NORM_HIGH },
	{ NORM_LOWMID, NORM_HIGH },
	{ NORM_MID,    NORM_LOW },
};
#define LOW_MF_MAP_LEN \
	(sizeof(LOW_MF_MAP)/sizeof(LOW_MF_MAP[0]))



static const flcMfPoint_t MED_MF_MAP[] =
{
	{ NORM_LOWMID,  NORM_LOW },
	{ NORM_MID,     NORM_HIGH },
	{ NORM_MIDHIGH, NORM_LOW },
};
#define MED_MF_MAP_LEN \
	(sizeof(MED_MF_MAP)/sizeof(MED_MF_MAP[0]))



static const flcMfPoint_t HIGH_MF_MAP[] = 
{
	{ NORM_MID,     NORM_LOW },
	{ NORM_MIDHIGH, NORM_HIGH },
	{ NORM_HIGH,    NORM_HIGH },
};
#define HIGH_MF_MAP_LEN \
	(sizeof(HIGH_MF_MAP)/sizeof(HIGH_MF_MAP[0]))



static const flcMfPoint_t DEPTH_INPUT_NORMALIZE_MAP[] = 
{
	/* { estimated depth, normalized depth } */
	{ 0, NORM_LOW },                           /* LOW */
	{ DEPTH_LOW_THRESH, NORM_MID },          /* MED */
	{ DEPTH_MED_THRESH, NORM_HIGH },          /* HIGH */
};
#define DEPTH_INPUT_NORMALIZE_MAP_LEN \
	(sizeof(DEPTH_INPUT_NORMALIZE_MAP)/sizeof(DEPTH_INPUT_NORMALIZE_MAP[0]))



static const flcMfPoint_t STDDEV_INPUT_NORMALIZE_MAP[] =
{
	/* { standard deviation, normalized standard deviation } */
	{ 0, NORM_LOW },                           /* LOW */
	{ STDDEV_LOW_THRESH, NORM_MID },         /* MED */
	{ STDDEV_MED_THRESH, NORM_HIGH },         /* HIGH */
};
#define STDDEV_INPUT_NORMALIZE_MAP_LEN \
	(sizeof(STDDEV_INPUT_NORMALIZE_MAP)/sizeof(STDDEV_INPUT_NORMALIZE_MAP[0]))


static const flcMfPoint_t DEPTH_OUTPUT_DENORMALIZE_MAP[] =
{
	/* { normalized depth, estimated effective depth } */
	{ NORM_LOW, 0 },
	{ NORM_MID, DEPTH_LOW_THRESH },
	{ NORM_HIGH, DEPTH_MED_THRESH },
};
#define DEPTH_OUTPUT_DENORMALIZE_MAP_LEN \
	(sizeof(DEPTH_OUTPUT_DENORMALIZE_MAP)/sizeof(DEPTH_OUTPUT_DENORMALIZE_MAP[0]))



FLC::FLC( std::string _roiName, double _avgDepth, double _minDepth, double _stdDev)
{
	this->sysin_avgDepth = _avgDepth;
	this->sysin_minDepth = _minDepth;
	this->sysin_stdDev = _stdDev;

	this->effDepth_LOW = 0;
	this->effDepth_MED = 0;
	this->effDepth_HIGH = 0;

	this->ROIname = _roiName;
}


FLC::~FLC()
{

}


double FLC::linearMap( double input, const struct flcMfPoint_t *p_map, const size_t map_len)
{
	int i = 0;

	while( i < map_len)
	{
		if ( input < p_map[i].x)
			break;
		i++;
	}

	if ( i == 0)
		return p_map[i].y;
	else if ( i == map_len)
		return p_map[i-1].y;
	else
		return ( p_map[i-1].y + (((input - p_map[i-1].x)*(p_map[i].y - p_map[i-1].y))/(p_map[i].x - p_map[i-1].x)));
}


double FLC::funcCentroid( const flcMfPoint_t *p_map, size_t map_len)
{
	double num = 0, den = 0;
	int i = 0;

	while( i < map_len)
	{
		num += (p_map[i].x * p_map[i].y);
		den += p_map[i].y;

		i++;
	}

	return (num/den);
}


double FLC::normalizeDepth( double input)
{
	return linearMap( (int) input, 
		DEPTH_INPUT_NORMALIZE_MAP,
		DEPTH_INPUT_NORMALIZE_MAP_LEN);
}


double FLC::normalizeStdDev( double input)
{
	return linearMap( (int) input,
		STDDEV_INPUT_NORMALIZE_MAP,
		STDDEV_INPUT_NORMALIZE_MAP_LEN);
}


double FLC::denormalizeDepth( double input)
{
	return linearMap( input,
		DEPTH_OUTPUT_DENORMALIZE_MAP,
		DEPTH_OUTPUT_DENORMALIZE_MAP_LEN);
}


double FLC::computeLowSetMembership( double value)
{
	return linearMap( value,
		LOW_MF_MAP,
		LOW_MF_MAP_LEN);
}


double FLC::computeMedSetMembership( double value)
{
	return linearMap( value,
		MED_MF_MAP,
		MED_MF_MAP_LEN);	
}


double FLC::computeHighSetMembership( double value)
{
	return linearMap( value,
		HIGH_MF_MAP,
		HIGH_MF_MAP_LEN);	
}





double FLC::OR2( double input0, double input1)
{
	return std::max( input0, input1);
}

double FLC::AND2( double input0, double input1)
{
	return std::min( input0, input1);
}


double FLC::AND3( double input0, double input1, double input2)
{
	double minOf0and1 = std::min( input0, input1);

	return std::min( minOf0and1, input2);
}


flcOuput_t FLC::compute()
{
	/* normalized the raw system inputs */
	this->normalized_avgDepth = normalizeDepth( this->sysin_avgDepth);
	//this->normalized_minDepth = normalizeDepth( this->sysin_minDepth);
	this->normalized_stdDev = normalizeStdDev( this->sysin_stdDev);

	/* compute each normalized input's level of membership in each fuzzy set */
	this->avgDepth_LOW = computeLowSetMembership( this->normalized_avgDepth);
	this->avgDepth_MED = computeMedSetMembership( this->normalized_avgDepth);
	this->avgDepth_HIGH = computeHighSetMembership( this->normalized_avgDepth);

#if 0
	this->minDepth_LOW = computeLowSetMembership( this->normalized_minDepth);
	this->minDepth_MED = computeMedSetMembership( this->normalized_minDepth);
	this->minDepth_HIGH = computeHighSetMembership( this->normalized_minDepth);
#endif

	this->stdDev_LOW = computeLowSetMembership( this->normalized_stdDev);
	this->stdDev_MED = computeMedSetMembership( this->normalized_stdDev);
	this->stdDev_HIGH = computeHighSetMembership( this->normalized_stdDev);

	std::cout<<this->ROIname<<" ";
	std::cout<<"normalized_avgDepth = "<<this->normalized_avgDepth;
	std::cout<<", normalized_stdDev = "<<this->normalized_stdDev<<std::endl;



	/* rule evaluation */
	this->effDepth_LOW = OR2( this->effDepth_LOW, AND2( this->avgDepth_LOW, this->stdDev_LOW));

	this->effDepth_LOW = OR2( this->effDepth_LOW, AND2( this->avgDepth_LOW, this->stdDev_MED));

	this->effDepth_LOW = OR2( this->effDepth_LOW, AND2( this->avgDepth_LOW, this->stdDev_HIGH));

	this->effDepth_MED = OR2( this->effDepth_MED, AND2( this->avgDepth_MED, this->stdDev_LOW));

	this->effDepth_MED = OR2( this->effDepth_MED, AND2( this->avgDepth_MED, this->stdDev_MED));

	this->effDepth_LOW = OR2( this->effDepth_LOW, AND2( this->avgDepth_MED, this->stdDev_HIGH));

	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND2( this->avgDepth_HIGH, this->stdDev_LOW));

	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND2( this->avgDepth_HIGH, this->stdDev_MED));

	this->effDepth_MED = OR2( this->effDepth_MED, AND2( this->avgDepth_HIGH, this->stdDev_HIGH));

#if 0
	/* rule evaluation */
	// IF (AVG_DEPTH == LOW) AND (MIN_DEPTH == LOW) AND (STD_DEV == LOW) THEN (EFF_DEPTH == LOW)
	this->effDepth_LOW = OR2( this->effDepth_LOW, AND3( this->avgDepth_LOW, this->minDepth_LOW, this->stdDev_LOW));
	
	// IF (AVG_DEPTH == LOW) AND (MIN_DEPTH == LOW) AND (STD_DEV == MED) THEN (EFF_DEPTH == LOW)
	this->effDepth_LOW = OR2( this->effDepth_LOW, AND3( this->avgDepth_LOW, this->minDepth_LOW, this->stdDev_MED));
	
	// IF (AVG_DEPTH == LOW) AND (MIN_DEPTH == LOW) AND (STD_DEV == HIGH) THEN (EFF_DEPTH == LOW)
	this->effDepth_LOW = OR2( this->effDepth_LOW, AND3( this->avgDepth_LOW, this->minDepth_LOW, this->stdDev_HIGH));

	// IF (AVG_DEPTH == MED) AND (MIN_DEPTH == LOW) AND (STD_DEV == LOW) THEN (EFF_DEPTH == MED)
	this->effDepth_MED = OR2( this->effDepth_MED, AND3( this->avgDepth_MED, this->minDepth_LOW, this->stdDev_LOW));

	// IF (AVG_DEPTH == MED) AND (MIN_DEPTH == LOW) AND (STD_DEV == MED) THEN (EFF_DEPTH == MED)
	this->effDepth_MED = OR2( this->effDepth_MED, AND3( this->avgDepth_MED, this->minDepth_LOW, this->stdDev_MED));

	// IF (AVG_DEPTH == MED) AND (MIN_DEPTH == LOW) AND (STD_DEV == HIGH) THEN (EFF_DEPTH == LOW)
	this->effDepth_LOW = OR2( this->effDepth_LOW, AND3( this->avgDepth_MED, this->minDepth_LOW, this->stdDev_HIGH));

	// IF (AVG_DEPTH == MED) AND (MIN_DEPTH == MED) AND (STD_DEV == LOW) THEN (EFF_DEPTH == MED)
	this->effDepth_MED = OR2( this->effDepth_MED, AND3( this->avgDepth_MED, this->minDepth_MED, this->stdDev_LOW));

	// IF (AVG_DEPTH == MED) AND (MIN_DEPTH == MED) AND (STD_DEV == MED) THEN (EFF_DEPTH == MED)
	this->effDepth_MED = OR2( this->effDepth_MED, AND3( this->avgDepth_MED, this->minDepth_MED, this->stdDev_MED));

	// IF (AVG_DEPTH == MED) AND (MIN_DEPTH == MED) AND (STD_DEV == HIGH) THEN (EFF_DEPTH == LOW)
	this->effDepth_LOW = OR2( this->effDepth_LOW, AND3( this->avgDepth_MED, this->minDepth_MED, this->stdDev_HIGH));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == LOW) AND (STD_DEV == LOW) THEN (EFF_DEPTH == HIGH)
	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND3( this->avgDepth_HIGH, this->minDepth_LOW, this->stdDev_LOW));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == LOW) AND (STD_DEV == MED) THEN (EFF_DEPTH == HIGH)
	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND3( this->avgDepth_HIGH, this->minDepth_LOW, this->stdDev_MED));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == LOW) AND (STD_DEV == HIGH) THEN (EFF_DEPTH == MED)
	this->effDepth_MED = OR2( this->effDepth_MED, AND3( this->avgDepth_HIGH, this->minDepth_LOW, this->stdDev_HIGH));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == MED) AND (STD_DEV == LOW) THEN (EFF_DEPTH == HIGH)
	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND3( this->avgDepth_HIGH, this->minDepth_MED, this->stdDev_LOW));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == MED) AND (STD_DEV == MED) THEN (EFF_DEPTH == HIGH)
	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND3( this->avgDepth_HIGH, this->minDepth_MED, this->stdDev_MED));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == MED) AND (STD_DEV == HIGH) THEN (EFF_DEPTH == MED)
	this->effDepth_MED = OR2( this->effDepth_MED, AND3( this->avgDepth_HIGH, this->minDepth_MED, this->stdDev_HIGH));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == HIGH) AND (STD_DEV == LOW) THEN (EFF_DEPTH == HIGH)
	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND3( this->avgDepth_HIGH, this->minDepth_HIGH, this->stdDev_LOW));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == HIGH) AND (STD_DEV == MED) THEN (EFF_DEPTH == HIGH)
	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND3( this->avgDepth_HIGH, this->minDepth_HIGH, this->stdDev_MED));

	// IF (AVG_DEPTH == HIGH) AND (MIN_DEPTH == HIGH) AND (STD_DEV == HIGH) THEN (EFF_DEPTH == HIGH)
	this->effDepth_HIGH = OR2( this->effDepth_HIGH, AND3( this->avgDepth_HIGH, this->minDepth_HIGH, this->stdDev_HIGH));
#endif

	/* compute normalized output */
	double num = 0, den = 0;

	num = funcCentroid( LOW_MF_MAP, LOW_MF_MAP_LEN) * this->effDepth_LOW
		+ funcCentroid( MED_MF_MAP, MED_MF_MAP_LEN) * this->effDepth_MED
		+ funcCentroid( HIGH_MF_MAP, HIGH_MF_MAP_LEN) * this->effDepth_HIGH;

	den = this->effDepth_LOW + this->effDepth_MED + this->effDepth_HIGH;

	normalized_effDepth = num / den;


	/* convert normalized effective depth to real effective depth */
	this->sysout_effDepth = denormalizeDepth( normalized_effDepth);

	if ( this->sysout_effDepth < DEPTH_LOW_THRESH)
		return FO_BLOCKED;
	else if ( this->sysout_effDepth < DEPTH_MED_THRESH)
		return FO_CAUTION;
	else
		return FO_CLEAR;
}
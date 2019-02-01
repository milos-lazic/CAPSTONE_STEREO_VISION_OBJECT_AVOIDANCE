#ifndef FLC_H
#define FLC_H

#include <cstdlib>
#include <iostream>
#include <algorithm>

#define MAXNAME 16

typedef struct flcMfPoint_t
{
    double x;
    double y;
} flcMfPoint_t;


typedef enum flcOuput_t
{
	FO_BLOCKED = 0,
	FO_CAUTION,
	FO_CLEAR,
	FO_MAX,
} flcOuput_t;



class FLC
{
public:
	/* class constructor */
	FLC( std::string _roiName, double _avgDepth = 0, double _minDepth = 0, double _stdDev = 0);
	/* class destructor */
	~FLC();

	/* class public methods */
	flcOuput_t compute( void);

private:

	std::string ROIname;

	/* FLC raw inputs */
	double sysin_avgDepth;
	double sysin_minDepth;
	double sysin_stdDev;

	/* FLC raw outputs */
	double sysout_effDepth;

	/* FLC normalized inputs */
	double normalized_avgDepth;
	double normalized_minDepth;
	double normalized_stdDev;

	/* FLC normalized outputs */
	double normalized_effDepth;

	/* membership function evals for avgDepth */
	double avgDepth_LOW;
	double avgDepth_MED;
	double avgDepth_HIGH;

	/* membership function evals for minDepth */
	double minDepth_LOW;
	double minDepth_MED;
	double minDepth_HIGH;

	/* membership function evals for stdDev */
	double stdDev_LOW;
	double stdDev_MED;
	double stdDev_HIGH;

	/* membership function evals for (effective depth) effDepth */
	double effDepth_LOW;
	double effDepth_MED;
	double effDepth_HIGH;


	/* class private methods */
	double linearMap( double input, const struct flcMfPoint_t *p_map, const size_t map_len);
	double funcCentroid( const flcMfPoint_t *p_map, size_t map_len);
	double OR2( double input0, double input1);
	double AND2( double input0, double input1);
	double AND3( double input0, double input1, double input2);

	double normalizeDepth( double input);
	double normalizeStdDev( double input);
	double denormalizeDepth( double input);

	double computeLowSetMembership( double value);
	double computeMedSetMembership( double value);
	double computeHighSetMembership( double value);


};

#endif // FLC_H
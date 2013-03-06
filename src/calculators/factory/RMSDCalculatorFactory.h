/*
 * RMSDCalculatorFactory.h
 *
 *  Created on: 06/03/2013
 *      Author: victor
 */

#ifndef RMSDCALCULATORFACTORY_H_
#define RMSDCALCULATORFACTORY_H_
#include "RMSDCalculatorTypes.h"

class RMSDCalculator;

class RMSDCalculatorFactory {
	public:
		RMSDCalculatorFactory();
		virtual ~RMSDCalculatorFactory();

		static RMSDCalculator* createCalculator(RMSDCalculatorType type, int numberOfConformations,
				int atomsPerConformation, double* allCoordinates);
};

#endif /* RMSDCALCULATORFACTORY_H_ */
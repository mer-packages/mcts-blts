/* mwtsstatistics.cpp --
 *
 * This file is part of MCTS 
 * 
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
 * 
 * Contact: Tommi Toropainen; tommi.toropainen@nokia.com; 
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public License 
 * version 2.1 as published by the Free Software Foundation. 
 * 
 * This library is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
 * Lesser General Public License for more details. 
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 
 * 02110-1301 USA 
 *
 */

#include "stable.h"
#include "mwtsstatistics.h"

/** Constructs statistics object and precalculate results*/

MwtsStatistics::MwtsStatistics(QList<double> values)
{
	max=0;
	min=0;
	mean=0;
	median=0;
	stdev=0;
	int i;
	if(values.size()==1)
	{
		max=values[0];
		min=values[0];
		mean=values[0];
		stdev=0;
		median=values[0];
	}
	if(values.size()>1)
	{
		double sum=0;
		for(i=0; i<values.size(); i++)
		{
			sum+=values[i];
		}
		qSort(values);
		mean=sum/values.size();
		median=values.at(values.size()/2);
		min=values.first();
		max=values.last();

		double stdevsum=0;

		for (i=0; i < values.size(); i++)
		{
			double value=values.at(i);
			double diff=mean-value;
			stdevsum += diff*diff;
		}

		stdev=sqrt(stdevsum/(values.size()-1));
	}
}
/** Returns the mean value from values*/
double MwtsStatistics::Mean()
{
	return mean;
}
/** Returns the median value from values*/
double MwtsStatistics::Median()
{
	return median;
}

/** Returns the minimum value from values*/
double MwtsStatistics::Min()
{
	return min;
}

/** Returns the maximum value from values*/
double MwtsStatistics::Max()
{
	return max;
}

/** Returns the standard deviation from values*/
double MwtsStatistics::Stdev()
{
	return stdev;
}

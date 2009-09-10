/*
 * Copyright (c) 2009 Daniël de Kok
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#include <fstream>
#include <iostream>

#include "FeatureSqueeze/stringutil.hh"
#include "FeatureSqueeze/DataSet.hh"
#include "FeatureSqueeze/FeatureSelection.hh"

#include "ProgramOptions.hh"

using namespace std;

void usage(string const &programName)
{
	cerr << "Usage: " << programName << " [OPTION] dataset" << endl << endl <<
		"  -a val\t Alpha convergence threshold" << endl <<
		"  -g val\t Gain threshold" << endl << endl;	
}

int main(int argc, char *argv[])
{
	fsqueeze::ProgramOptions programOptions(argc, argv, "a:g:");
	
	if (programOptions.arguments().size() != 1)
	{
		usage(programOptions.programName());
		return 1;
	}
	
	double alphaThreshold = 1e-6;
	if (programOptions.option('a'))
		alphaThreshold = fsqueeze::parseString<double>(programOptions.optionValue('a'));

	double gradientThreshold = 1e-6;
	if (programOptions.option('g'))
		gradientThreshold = fsqueeze::parseString<double>(programOptions.optionValue('g'));

	ifstream dataStream(programOptions.arguments()[0]);
	if (!dataStream)
	{
		cerr << "Error opening input file!" << endl;
		return 1;
	}

	auto ds = fsqueeze::DataSet::readTADMDataSet(dataStream);
	auto features = fsqueeze::featureSelection(ds, alphaThreshold, gradientThreshold);
	
	for (auto fIter = features.begin(); fIter != features.end(); ++fIter)
		cout << fIter->first << "\t" << fIter->second << "\t" << fIter->third << endl;

	return 0;
}

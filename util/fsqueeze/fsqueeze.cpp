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
#include <limits>

#include "FeatureSqueeze/stringutil.hh"
#include "FeatureSqueeze/DataSet.hh"
#include "FeatureSqueeze/Logger.hh"
#include "FeatureSqueeze/feature_selection.hh"

#include "ProgramOptions.hh"

using namespace std;

void usage(string const &programName)
{
	cerr << "Usage: " << programName << " [OPTION] dataset" << endl << endl <<
		"  -a val\t Alpha convergence threshold (default: 1e-6)" << endl <<
		"  -f\t\t Fast selection algorithm (do not recalculate all gains)" << endl <<
		"  -g val\t Gain threshold (default: 1e-6)" << endl <<
		"  -n val\t Maximum number of features" << endl <<
		"  -o\t\t Find overlap (incompatible with -f)" << endl << endl;
}

int main(int argc, char *argv[])
{
	fsqueeze::ProgramOptions programOptions(argc, argv, "a:fg:n:o");
	
	if (programOptions.arguments().size() != 1)
	{
		usage(programOptions.programName());
		return 1;
	}

	if (programOptions.option('o') && programOptions.option('f'))
	{
		cerr << "Overlap detection (-o) and fast selection(-f) cannot be used " <<
			"simultaneausly!" << endl << endl;
		return 1;
	}
	
	double alphaThreshold = 1e-6;
	if (programOptions.option('a'))
		alphaThreshold = fsqueeze::parseString<double>(programOptions.optionValue('a'));

	double gradientThreshold = 1e-6;
	if (programOptions.option('g'))
		gradientThreshold = fsqueeze::parseString<double>(programOptions.optionValue('g'));
	
	
	size_t nFeatures = numeric_limits<size_t>::max();
	if (programOptions.option('n'))
		nFeatures = fsqueeze::parseString<size_t>(programOptions.optionValue('n'));

	ifstream dataStream(programOptions.arguments()[0].c_str());
	if (!dataStream)
	{
		cerr << "Error opening input file!" << endl;
		return 1;
	}

	fsqueeze::DataSet ds = fsqueeze::DataSet::readTADMDataSet(dataStream);
	
	fsqueeze::Logger logger(cout, cerr);
	if (programOptions.option('f'))
		fsqueeze::fastFeatureSelection(ds, logger, alphaThreshold, gradientThreshold, nFeatures);
	else
		fsqueeze::featureSelection(ds, logger, alphaThreshold, gradientThreshold, nFeatures,
			programOptions.option('o'));
	
	return 0;
}

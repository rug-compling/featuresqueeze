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

#include "FeatureSqueeze/DataSet.hh"
#include "FeatureSqueeze/FeatureSelection.hh"

using namespace std;

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " dataset" << endl;
		return 1;
	}
	
	ifstream dataStream(argv[1]);
	if (!dataStream)
	{
		cerr << "Error opening input file!" << endl;
		return 1;
	}

	auto ds = fsqueeze::DataSet::readTADMDataSet(dataStream);
	auto features = fsqueeze::featureSelection(ds);
	
	for (auto fIter = features.begin(); fIter != features.end(); ++fIter)
		cout << fIter->first << "\t" << fIter->second << "\t" << fIter->third << endl;
	
	return 0;
}

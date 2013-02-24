//Peter J. Lu
//Copyright 2002-2013 Peter J. Lu.
//http://www.peterlu.org
//plu@fas.harvard.edu
//
//This program is free software; you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation; either version 3 of the License, or (at
//your option) any later version.
//
//This program is distributed in the hope that it will be useful, but
//WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, see <http://www.gnu.org/licenses>.
//
//Additional permission under GNU GPL version 3 section 7
//
//If you modify this Program, or any covered work, by linking or
//combining it with MATLAB (or a modified version of that library),
//containing parts covered by the terms of MATLAB User License, the
//licensors of this Program grant you additional permission to convey
//the resulting work.
//
//If you modify this Program, or any covered work, by linking or
//combining it with FreeImage (or a modified version of that library),
//containing parts covered by the terms of freeimage-license, the
//licensors of this Program grant you additional permission to convey
//the resulting work.  Corresponding Source for a non-source form of
//such a combination shall include the source code for the parts of
//FreeImage used as well as that of the covered work.
//
//If you modify this Program, or any covered work, by linking or
//combining it with IPP (or a modified version of that library),
//containing parts covered by the terms of End User License Agreement
//for the Intel(R) Software Development Products, the licensors of
//this Program grant you additional permission to convey the resulting
//work.

#include "fileops1.h"

Filestreams::Filestreams(const Params &Params)
{
	string h5filename = Params.get_outfilestem() + "_2DCenters.h5";
	herr_t hdf5_status;
	hid_t hdf5_group;
	outdata_hdf5_file = H5Fcreate(h5filename.c_str(),
			H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
	for(int stacknumber = Params.get_start_stack(); stacknumber <= Params.get_end_stack(); stacknumber++) {
		ostringstream hdf5_group_name_stream;
		hdf5_group_name_stream << "stack" << stacknumber;
		string hdf5_group_name = hdf5_group_name_stream.str();
		hdf5_group = H5Gcreate2(outdata_hdf5_file, hdf5_group_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		hdf5_status = H5Gclose(hdf5_group);
	}

	string paramfilename = Params.get_outfilestem() + "_2DCenters_log.txt";
	outparamfile.open(paramfilename.c_str(), ios::out);

	if(Params.get_testmode()==3) {
	string datafilename = Params.get_outfilestem() + "_2DCenters.txt";
	outdatafile.open(datafilename.c_str(), ios::out);
	}
}


void Filestreams::closeall()
{
	herr_t hdf5_status = H5Fclose(outdata_hdf5_file);
	outdatafile.close();
	outparamfile.close();
}

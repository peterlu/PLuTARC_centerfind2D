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

#include "params1.h"

string Date_Last_Updated="24 Feb 2013";
string Version_Number="2.0";

Params::Params(const int numargs)
{
	if(numargs < 13) {
		cout << "plu_centerfind 'infilestem' 'outfilestem' start_frame,end_frame,start_stack,end_stack,gauss_rad,hwhm_rad,dil_rad,mask_rad,pct_thresh,testmode" << endl;
		cout << "(C)opyright 2002-2013 Peter J. Lu. All Rights Reserved. http://www.peterlu.org" << endl;
		cout << endl << "****If you use this code, please cite in your publications: " << endl;
		cout << "P. J. Lu, P. A. Sims, H. Oki, J. B. Macarthur, and D. A. Weitz, " << endl;
		cout << "Target-locking acquisition with real-time confocal (TARC) microscopy," << endl;
		cout << "Optics Express Vol. 15, pp. 8702-8712 (2007)." << endl << endl;
		cout << "Version " << Version_Number << "; last updated " << Date_Last_Updated << endl << endl;
		cout << "Analyze frames numbered 'start_frame' to 'end_frame', from stacks numbered 'start_stack' to 'end_stack'" << endl;
		cout << "gauss_rad: radius of Gaussian convolution kernel; corresponds to particle radius, in pixels." << endl;
		cout << "hwhm_rad: half-width, half-maximum [radius] for Gaussian convolution kernel." << endl;
		cout << "dil_rad: radius of dilation kernel mask." << endl;
		cout << "mask_rad: radius of mask for calculating statistics." << endl;
		cout << "pct_thresh: analyse data above this percentile threshold." << endl;
		cout << "testmode: 0--only HDF5 data; 1--show intermediate images;\n          2--images for stacks 1, 101, 201, etc.; 3--plain text output" << endl;
	}
}

Params::Params(const int numargs, char* arguments[]):
start_frameofstack(0), start_stack(0),
end_frameofstack(0), end_stack(0), feature_radius(0), hwhm_length(0),
dilation_radius(0), pctle_threshold(0), mask_radius(0), testmode(0)
{
	file_extension = ".tif";
	infile_stem = arguments[1];
	outfile_stem = arguments[2];

	start_frameofstack = atoi(arguments[3]);
	end_frameofstack = atoi(arguments[4]);
	start_stack = atoi(arguments[5]);
	end_stack = atoi(arguments[6]);

	feature_radius = atoi(arguments[7]);
	hwhm_length = atof(arguments[8]);
	dilation_radius = atoi(arguments[9]);
	mask_radius = atoi(arguments[10]);
	pctle_threshold = atof(arguments[11]);
	testmode = atoi(arguments[12]);
}

Params::~Params()
{
}

const Params &Params::operator =(const Params &Parameters)
{
	if (&Parameters != this) {
		infile_stem = Parameters.get_infilestem();
		outfile_stem = Parameters.get_outfilestem();
		file_extension = Parameters.get_file_extension();
		start_frameofstack = Parameters.get_start_frameofstack();
		end_frameofstack = Parameters.get_end_frameofstack();
		start_stack = Parameters.get_start_stack();
		end_stack = Parameters.get_end_stack();
		feature_radius = Parameters.get_feature_radius();
		hwhm_length = Parameters.get_hwhm_length();
		dilation_radius = Parameters.get_dilation_radius();
		pctle_threshold = Parameters.get_pctle_threshold();
		mask_radius = Parameters.get_mask_radius();
		testmode = Parameters.get_testmode();
	}
	else {
		cout << "Error: Parameters cannot be assigned to self!" << endl;
	}
	return *this;
}

void Params::PrintOutParameters(ostream &out)
{
	out << "Particle 2D Center Locator, ";
	out << "Version " << Version_Number << "; last updated " << Date_Last_Updated << endl;
	out << "(C)opyright 2002-2013 Peter J. Lu. All Rights Reserved." << endl << endl;
	out << endl << "****If you use this code, please cite in your publications: " << endl;
	out << "P. J. Lu, P. A. Sims, H. Oki, J. B. Macarthur, and D. A. Weitz, " << endl;
	out << "Target-locking acquisition with real-time confocal (TARC) microscopy," << endl;
	out << "Optics Express Vol. 15, pp. 8702-8712 (2007)." << endl << endl;

	out << "Parameters used in this analysis run are listed below:" << endl;
	out << "Analysing frame(s) " << start_frameofstack << " to " << end_frameofstack;
	out << " from stack(s) " << start_stack << " to " << end_stack << endl;
	out << "Starting input file: " << infile_stem + make_stack_number(start_stack)+file_extension << endl;
	out << "Ending input file: " << infile_stem + make_stack_number(end_stack)+file_extension << endl;
	out << "Output file stem: " << outfile_stem << endl;
	out << "Gaussian convolution kernel radius (pixels): " << feature_radius << endl;
	out << "Gaussian half-width half-maximum [radius] (pixels): " << hwhm_length << endl;
	out << "Dilation kernel radius (pixels): " << dilation_radius << endl;
	out << "Radius of masks for calculating mass and Rg (pixels): " << mask_radius << endl;
	out << "Percentile threshold below which to discard data: " << pctle_threshold << endl;
	out << endl;
}

string make_stack_number(const int stacknumber)
{
	string stacknumber_string;
	ostringstream stacknumber_stream;
	stacknumber_stream.setf(ios::right, ios::adjustfield);
	stacknumber_stream.fill('0');
	stacknumber_stream << setw(4) << stacknumber;
	stacknumber_string = "_" + stacknumber_stream.str();
	return stacknumber_string;
}

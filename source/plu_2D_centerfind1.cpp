//Peter J. Lu
//Copyright 2002-2013 Peter J. Lu.
//http://www.peterlu.org
//plu@fas.harvard.edu
//
// Particle centroid locator
// Inspired by IDL code "bpass" and "pretrack" by Crocker and Grier
// Started: June 17, 2002
// Major Update: September 11, 2005
// Version 1.4 (4 June 4 2006):		Added file existence verification code
// Version 1.5 (15 October 2006):	Switched to Intel C++ 9.1, and Intel IPP 5.1.1
// Version 1.6 (21 August 2008):		Switched to Intel C++ Compiler 10, and Intel IPP 6; output is a compressed 3D TIFF-stack
// Version 1.7 (25 August 2008):		Switched to 3D TIFF stacks for input, mirroring PLuTARC update
// Version 1.8 (18 April 2009):		Switched to Intel C++ Compiler 11, now cross-platform for both Windows and Linux, timing corrected 11 Oct 2009
// Version 1.81 (11 Feb 2013):		Switched to Intel C++ Compiler 13, IPP 7.1, and tweaked timing code
// Version 1.9 (18 Feb 2013):		Implemented circular convolution instead of tophat; by Eli Sloutskin
// Version 2.0 (24 Feb 2013):		Switched main output format to HDF5 (compression, directory structure, full floating point)
// Version 2.01 (26 Feb 2013):		Added output of number of particles per slice to log file, as configuration mode 4 (for xyt)
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

//header files for my code
#include <ctime>
#include "plu_2D_centerfind1.h"
#include "params1.h"
#include "kernel1.h"
#include "data_proc1.h"
#include "image1.h"
#include "fileops1.h"

//defined constants
#define epsilon 0.0001

void find_centers_2D(Image2D &image_in, const Params &Parameters, Filestreams &files,
					 const int framenumber, const int stacknumber, FIMULTIBITMAP *filt_imagestack, FIMULTIBITMAP *over_imagestack)
{
	IppStatus status;

	Image2D image_bpass(image_in.get_length(), image_in.get_width());			//result of bandpassing original image data
	Image2D image_bpass_thresh(image_in.get_length(), image_in.get_width());	//result of setting lower threshold on bandpassed image
	Image2D image_localmax(image_in.get_length(), image_in.get_width());		//holds 1 in pixel locations with local max, 0 otherwise

	RecenterImage(image_in);

	//Convolve kernels with image (gauss and top-hat); then subtract the two (i.e. bandpass filter them)
	status = BandPass_2D(image_in, image_bpass, Parameters.get_feature_radius(), Parameters.get_hwhm_length());

	//Find local maxima (brightest pixels)
	status = FindLocalMax_2D(image_bpass, image_bpass_thresh, image_localmax, Parameters.get_pctle_threshold(),
		Parameters.get_dilation_radius());

	RecenterImage(image_bpass_thresh);

	//find 1-D indices of local maxima
	int counter = 0;
	Ipp32f (*particledata)[8] = ParticleStatistics(image_localmax, image_bpass_thresh, Parameters.get_mask_radius(),
		Parameters.get_feature_radius(), counter);

	if(Parameters.get_testmode() == 1) {
		//print out all images
		PrintOutputImages(image_in, image_bpass_thresh, image_localmax, Parameters, framenumber, stacknumber, filt_imagestack, over_imagestack);
	}

	if(Parameters.get_testmode() == 2 && (stacknumber) % 100 == 1) {
		//print out images for stack 1, 101, 201, etc.
		PrintOutputImages(image_in, image_bpass_thresh, image_localmax, Parameters, framenumber, stacknumber, filt_imagestack, over_imagestack);
	}

	if(Parameters.get_testmode() == 3) {
		//print out plain text data in old columnar format
		PrintParticleData(files.outdatafile, particledata, 0, counter, framenumber, stacknumber);
	}

	if(Parameters.get_testmode() == 4) {
		//print particle count in log file for each individual slice
		files.outparamfile << framenumber << "\t" << counter << endl;
		//cout << framenumber << counter << endl;
	}

	//in all cases, print out particle position data in binary hdf5 format
	PrintParticleData_hdf5(files.outdata_hdf5_file, particledata, counter, framenumber, stacknumber);

	delete [] particledata;
	particledata = NULL;

}

int main(int argc, char* argv[])
{
	//keep track of timing: start timer
	clock_t starttime = clock();

	//initialize parameters
	Params Params1(argc);
	if (argc < 13) {
		return -1;
	}
	static Params Parameters(argc, argv);

	static const int start_frameofstack = Parameters.get_start_frameofstack();
	static const int end_frameofstack = Parameters.get_end_frameofstack();
	static const int start_stack = Parameters.get_start_stack();
	static const int end_stack = Parameters.get_end_stack();

	Filestreams datafiles(Parameters);

	if(!datafiles.outdatafile) {
		cerr << "Cannot open output data file for writing: check path and try again!" << endl;
		return -1;
	}

	Parameters.PrintOutParameters(cout);
	Parameters.PrintOutParameters(datafiles.outparamfile);

	PrintParticleDataHeader(datafiles.outparamfile);

	int j = 1, i = 1;

	FIMULTIBITMAP *FI_in_image_stack;
	FIMULTIBITMAP *FI_out_image_stack_filt;
	FIMULTIBITMAP *FI_out_image_stack_over;

	//before beginning analysis, validate that all files exist
	for(j = start_stack; j <= end_stack; j++) {
		//validate whether input image files exist
		string infilename = Parameters.get_infilestem() + make_stack_number(j) + Parameters.get_file_extension();
		ifstream file_test;
		file_test.open(infilename.c_str(),ios::in);
		if(!file_test) {
			cerr << "Cannot open input image file: " << infilename << "(check path and try again)" << endl;
			return -1;
		}
		file_test.close();
	}

	for(j = start_stack; j <= end_stack; j++) {

		//open input file
		string infilename = Parameters.get_infilestem() + make_stack_number(j) + Parameters.get_file_extension();
		FI_in_image_stack = FreeImage_OpenMultiBitmap(FIF_TIFF, infilename.c_str(), FALSE, TRUE, TRUE, TIFF_DEFAULT);

		//check page count to make sure frame analyzed actually exists in the stack, and is selected by command-line parameter
		const int actual_frames_in_input_stack = FreeImage_GetPageCount(FI_in_image_stack);
		datafiles.outparamfile << "Pages in multi-page TIFF " << infilename << ": " << actual_frames_in_input_stack << endl;

		int max_frame_num = actual_frames_in_input_stack;
		if(end_frameofstack > actual_frames_in_input_stack) {
			cout << endl << "Total number of slices (pages) in input 3D image stack (multi-page TIFF): " << actual_frames_in_input_stack << endl;
			cout << "Max slice number set by command-line arguments is: " << end_frameofstack << endl;
			cout << "Only actual slices analyzed; command-line parameter overriden and ignored" << endl << endl;
			datafiles.outparamfile << endl << "Total number of slices (pages) in input 3D image stack (multi-page TIFF): " << actual_frames_in_input_stack << endl;
			datafiles.outparamfile << "Max slice number set by command-line arguments is: " << end_frameofstack << endl;
			datafiles.outparamfile << "Only actual slices analyzed; command-line parameter overriden and ignored" << endl << endl;
		}
		if(end_frameofstack < actual_frames_in_input_stack) {
			max_frame_num = end_frameofstack;
		}

		//create the output files
		string outfilename_filt = Parameters.get_outfilestem() + make_stack_number(j) + "_filt" + Parameters.get_file_extension();
		string outfilename_over = Parameters.get_outfilestem() + make_stack_number(j) + "_over" + Parameters.get_file_extension();
		FI_out_image_stack_filt = FreeImage_OpenMultiBitmap(FIF_TIFF, outfilename_filt.c_str(), TRUE, FALSE, TRUE, TIFF_DEFAULT);
		FI_out_image_stack_over = FreeImage_OpenMultiBitmap(FIF_TIFF, outfilename_over.c_str(), TRUE, FALSE, TRUE, TIFF_DEFAULT);


//#pragma omp parallel for
		for(i = start_frameofstack; i <= max_frame_num; i++) {
			//Create IPP images by loading a specific page from multi-page TIFF for analysis, and converting to IPP format
			Image2D image_in = TIFF_to_IPP(FreeImage_LockPage(FI_in_image_stack,i-1));

			//find centers of particles
			find_centers_2D(image_in, Parameters, datafiles, i, j, FI_out_image_stack_filt, FI_out_image_stack_over);
		}


		FreeImage_CloseMultiBitmap(FI_out_image_stack_filt, TIFF_DEFAULT);
		FreeImage_CloseMultiBitmap(FI_out_image_stack_over, TIFF_JPEG);
		FreeImage_CloseMultiBitmap(FI_in_image_stack, TIFF_DEFAULT);

		float elapsed_time = (float) (clock() - starttime)/CLOCKS_PER_SEC;
		datafiles.outparamfile << "Elapsed time after analyzing stack: " << j << ": " << elapsed_time << " seconds" << endl << endl;
		cout << "Elapsed time after analyzing stack: " << j << ": " << elapsed_time << " seconds" << endl;
	}

	//Show total time to run program
	float total_time = (float) (clock() - starttime)/CLOCKS_PER_SEC;
	cout << endl << "Total time: " << total_time << " seconds" << endl;
	datafiles.outparamfile << endl << "Total time: " << total_time << " seconds" << endl;
	datafiles.closeall();

	return 0;
}

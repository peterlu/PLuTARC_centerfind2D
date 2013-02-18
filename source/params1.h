//Peter J. Lu
//Copyright 2008 Peter J. Lu.
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

#ifndef PARAMS1_H
#define PARAMS1_H

#include <fstream>
#include <string>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;


string make_stack_number(const int stacknumber);

//This file contains the code for managing Params, the class that holds parameters from the command-line
class Params
{
public:
	Params();
	Params(const int numargs, char *arguments[]);	//Regular used constructor that reads command-line parameters
	Params(const int numargs);						//Constructor that simply checks for enough arguments

	const Params &operator=(const Params &);	//overloaded assignment operator

	void PrintOutParameters(ostream &out);

	~Params();

	string get_infilestem() const
	{return infile_stem;}
	string get_outfilestem() const
	{return outfile_stem;}
	string get_file_extension() const
	{return file_extension;}

	int get_start_frameofstack() const
	{return start_frameofstack;}
	int get_end_frameofstack() const
	{return end_frameofstack;}
	int get_start_stack() const
	{return start_stack;}
	int get_end_stack() const
	{return end_stack;}
	int get_feature_radius() const
	{return feature_radius;}
	float get_hwhm_length() const
	{return hwhm_length;}
	int get_dilation_radius() const
	{return dilation_radius;}
	float get_pctle_threshold() const
	{return pctle_threshold;}
	int get_mask_radius() const
	{return mask_radius;}
	int get_testmode() const
	{return testmode;}

private:
	string infile_stem;			//base for input file name, without extension
	string outfile_stem;		//base for output file name, without extension
	string file_extension;
	int start_frameofstack;	//starting frame number in each stack
	int end_frameofstack;	//ending frame number in each stack
	int start_stack;			//starting stack number
	int end_stack;			//total number of stacks to analyze
	int feature_radius;		//characteristic radius of particles, in pixels
	float hwhm_length;		//hwhm (half-width, half-maximum for gaussian kernel), in pixels
	int dilation_radius;	//radius for dilation mask, in pixels
	float pctle_threshold;	//percentile brightness to cutoff images
	int mask_radius;		//radius for kernels for calculating mass, x and y positions, r^2, etc.
	int testmode;			//execution modes: 0--print only text data; 1--output all intermediate images;
							//2--output images for stacks 1, 1001, 2001, etc.; 3--prints out number of particles in each slice
};

#endif

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

#ifndef DATAPROC1_H
#define DATAPROC1_H

#include "ipp.h"
#include "image1.h"
#include "kernel1.h"
#include "image1.h"
#include "FreeImage.h"
#include <cmath>
#include <iostream>
#include <fstream>
using namespace std;

//This file contains all of the routines for data and image analysis

IppStatus BandPass_2D(Image2D &image_in, Image2D &image_bpass, const int feature_radius, const int hwhm_length);

void RecenterImage(Image2D &image);

IppStatus FindLocalMax_2D(Image2D &image_bpass, Image2D &image_bpass_thresh, Image2D &image_subtracted,
						  const int threshold, const int dilation_radius);

IppStatus PrintOutputImages(const Image2D &image_in, const Image2D &image_bpass, const Image2D &image_localmax,
							const Params &Parameters, const int framenumber, const int stacknumber,
							FIMULTIBITMAP *filt_imagestack, FIMULTIBITMAP *over_imagestack);

void PrintParticleDataHeader(ostream &out);

void PrintParticleData(ostream &out, const Ipp32f particledata[][8], const int startrow, const int endrow,
					   const int framenumber, const int stacknumber);

Ipp32f (*ParticleStatistics(Image2D &image_localmax, Image2D &image_in, const int mask_radius,
							 const int feature_radius, int &counter))[8];
#endif

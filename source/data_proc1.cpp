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

#include "data_proc1.h"
#define epsilon 0.0001

IppStatus BandPass_2D(Image2D &image_in, Image2D &image_bandpassed, const int feature_radius, const int hwhm_length)
{
	//set status variable
	IppStatus status;
	Gaussian_Kernel GaussKernel(feature_radius, hwhm_length, image_in.get_width(), image_in.get_length());
	Tophat_Kernel TopHatKernel(feature_radius, image_in.get_width(), image_in.get_length());
	int number_of_pixels = image_in.get_numberofpixels();
	int step_size = image_in.get_stepsize();

	//Create and initialize intermediate images
	Image2D image_gauss_col(image_in.get_length(), image_in.get_width());
	Image2D image_gauss_rowcol(image_in.get_length(), image_in.get_width());
	Image2D image_tophat(image_in.get_length(), image_in.get_width());

	//Gaussian kernel convolution
	status = ippiFilterColumn_32f_C1R(image_in.get_image2D() + GaussKernel.get_offset(), step_size,
		image_gauss_col.get_image2D() + GaussKernel.get_offset(), step_size,
		GaussKernel.get_ROI_size(), GaussKernel.get_gaussian_kernel(),
		GaussKernel.get_kernel_length(), GaussKernel.get_anchor_point());
	status = ippiFilterRow_32f_C1R(image_gauss_col.get_image2D() + GaussKernel.get_offset(), step_size,
		image_gauss_rowcol.get_image2D() + GaussKernel.get_offset(), step_size,
		GaussKernel.get_ROI_size(), GaussKernel.get_gaussian_kernel(),
		GaussKernel.get_kernel_length(), GaussKernel.get_anchor_point());

	//tophat kernel convolution/filterbox operation
	status = ippiFilterBox_32f_C1R(image_in.get_image2D() + TopHatKernel.get_offset(), step_size,
		image_tophat.get_image2D() + TopHatKernel.get_offset(), step_size,
		TopHatKernel.get_ROI_size(), TopHatKernel.get_mask_size(),
		TopHatKernel.get_anchor_point());

	//subtract the two images
	status = ippiSub_32f_C1R(image_tophat.get_image2D() + TopHatKernel.get_offset(), step_size,
		image_gauss_rowcol.get_image2D()+TopHatKernel.get_offset(), step_size,
		image_bandpassed.get_image2D() + TopHatKernel.get_offset(), step_size,
		TopHatKernel.get_ROI_size());

	//cutoff values below zero
	status = ippiThreshold_LTVal_32f_C1IR(image_bandpassed.get_image2D() + TopHatKernel.get_offset(), step_size,
		TopHatKernel.get_ROI_size(),0,0);

	return status;
}

void RecenterImage(Image2D &image)
{
	IppStatus status;
	Ipp32f minval = 0, maxval = 0;
	status = ippiMinMax_32f_C1R(image.get_image2D(), image.get_stepsize(), image.get_ROIfull(),
		&minval, &maxval);
	status = ippiAddC_32f_C1IR(-minval, image.get_image2D(), image.get_stepsize(), image.get_ROIfull());
	status = ippiMulC_32f_C1IR(100 / (maxval-minval), image.get_image2D(), image.get_stepsize(), image.get_ROIfull());
}

IppStatus FindLocalMax_2D(Image2D &image_bpass, Image2D &image_bpass_thresh, Image2D &image_subtracted,
						  const int intensity_threshold, const int dilation_radius)
{
	IppStatus status;
	Image2D image_dilated(image_bpass.get_length(), image_bpass.get_width());
	Dilation_Kernel DilationKernel(dilation_radius, image_bpass.get_width(), image_bpass.get_length());

	//Threshold darker pixels in bandpassed image (in preparation for later subtraction)
	RecenterImage(image_bpass);
	status = ippiThreshold_LTVal_32f_C1R(image_bpass.get_image2D(), image_bpass.get_stepsize(),
		image_bpass_thresh.get_image2D(), image_bpass_thresh.get_stepsize(),
		image_bpass.get_ROIfull(), intensity_threshold, intensity_threshold);

	//Dilate Bandpassed image with a circular kernel
	status = ippiSet_32f_C1R(intensity_threshold, image_dilated.get_image2D(), image_dilated.get_stepsize(),
		image_dilated.get_ROIfull());
	status = ippiDilate_32f_C1R(
		//image_bpass.get_image2D() + DilationKernel.get_offset(), image_bpass.get_stepsize(),
		image_bpass_thresh.get_image2D() + DilationKernel.get_offset(), image_bpass_thresh.get_stepsize(),
		image_dilated.get_image2D()+ DilationKernel.get_offset(), image_dilated.get_stepsize(),
		DilationKernel.get_ROI_size(), DilationKernel.get_dilation_kernel(), DilationKernel.get_mask_size(),
		DilationKernel.get_anchor_point());

	//subtract, such that resulting array is negative to zero (for later exponentation)
	status = ippiSub_32f_C1R(
		image_dilated.get_image2D(), image_dilated.get_stepsize(),
		image_bpass.get_image2D(), image_bpass.get_stepsize(),
		image_subtracted.get_image2D(), image_subtracted.get_stepsize(),
		image_bpass.get_ROIfull());

	//exponentiate subtracted array, then threshold
	status = ippiExp_32f_C1IR(image_subtracted.get_image2D(), image_subtracted.get_stepsize(),
		image_subtracted.get_ROIfull());
	status = ippiThreshold_LTValGTVal_32f_C1IR(image_subtracted.get_image2D(), image_subtracted.get_stepsize(),
		image_subtracted.get_ROIfull(), 1-epsilon, 0, 1-epsilon, 1);
	return status;
}

IppStatus PrintOutputImages(const Image2D &image_in, const Image2D &image_bpass, const Image2D &image_localmax,
							const Params &Parameters, const int framenumber, const int stacknumber,
							FIMULTIBITMAP *filt_imagestack, FIMULTIBITMAP *over_imagestack)
{
	IppStatus status;

	Image2D image_out(image_in.get_length(), image_in.get_width());
	Image2D image_overlay(image_in.get_length(), image_in.get_width());
	Image2D centerpoints(image_in.get_length(), image_in.get_width());

	Ipp32f brightness = 75;

	//scale points to brightness
	status = ippiMulC_32f_C1R(image_localmax.get_image2D(), image_localmax.get_stepsize(), brightness,
		centerpoints.get_image2D(), centerpoints.get_stepsize(), image_localmax.get_ROIfull());

	//convert and save files as TIFFs
	status = ippiAdd_32f_C1R(
		image_bpass.get_image2D(), image_bpass.get_stepsize(),
		centerpoints.get_image2D(), centerpoints.get_stepsize(),
		image_out.get_image2D(), image_out.get_stepsize(),
		image_out.get_ROIfull());

	status = ippiAdd_32f_C1R(
		image_in.get_image2D(), image_in.get_stepsize(),
		centerpoints.get_image2D(), centerpoints.get_stepsize(),
		image_overlay.get_image2D(), image_overlay.get_stepsize(),
		image_in.get_ROIfull());

	status = IPP_to_TIFF(image_overlay, over_imagestack);
	status = IPP_to_TIFF(image_out, filt_imagestack);
	return status;
}

void PrintParticleDataHeader(ostream &out)
{
	out << "Six Particle Data Columns:" << endl;
	out << "0:  Stack number" << endl;
	out << "1:  Frame (in stack) number" << endl;
	out << "2:  Final X-position of particle, in pixels" << endl;
	out << "3:  Final Y-position of particle, in pixels" << endl;
	out << "4:  Mass (sum of brightness) of particle, in arbitrary units" << endl;
	out << "5:  Radius of Gyration of particle, in arb. units" << endl;
	out << endl;
}

void PrintParticleData(ostream &out, const Ipp32f particledata[][8], const int startrow,
					   const int endrow, const int framenumber, const int stacknumber)
{
	//totalrows is (basically) number of particles (one for each row); can be a very large number
	out.setf(ios::fixed);
	out.precision(1);
	for(int j = startrow; j < endrow; j++) {
		out << stacknumber << "\t" << framenumber + 1 << "\t";
		out << particledata[j][1] << "\t";		//x-position
		out << particledata[j][2] << "\t";		//y-position
		out << particledata[j][5] << "\t";		//total mass
		out << sqrt(particledata[j][6]);				//radius of gyration
		out << endl;
	}
	out.precision(5);
}

Ipp32f (*ParticleStatistics(Image2D &image_localmax, Image2D &image_in,
					   const int mask_radius, const int feature_radius, int &counter))[8]
{

	IppStatus status;

	//setup kernels
	Convolution_Kernel ConvolutionKernels(mask_radius, image_localmax.get_width(), image_localmax.get_length());

	//Convert 32-bit FP image data in image_localmax to 8-bit integer data
	Ipp8u *localmaxdata = ippsMalloc_8u((int) image_localmax.get_numberofpixels());
	int stepsize = image_localmax.get_width();
	IppiSize fullROI = image_localmax.get_ROIfull();
	status = ippiConvert_32f8u_C1R(image_localmax.get_image2D(), image_localmax.get_stepsize(),
		localmaxdata, stepsize, fullROI, ippRndNear);
	//Determine the number of particles (raw; not yet excluded from border region)
	Ipp64f numberofmaxima = 0;
	status = ippiSum_8u_C1R(localmaxdata, stepsize, image_localmax.get_ROIfull(), &numberofmaxima);

	//Set up structures for counting particle data
	int numberofpixels = image_localmax.get_numberofpixels();
	Ipp32f (*particledata)[8] = new Ipp32f[(int) numberofmaxima + 1][8];

	//border region (local maxima here are excluded from future operations)
	int border = feature_radius;	//minimum distance from the border, in pixels
	int minx = border, miny = border;
	int maxx = image_localmax.get_width() - minx;
	int maxy = image_localmax.get_height() - miny;
	int xval = 0;
	int yval = 0;

	counter = 0;									//index that keeps track of which particle
	int imagewidth = image_in.get_width();
	//determine integer x and y values (in pixels) for each local maximum outside
	//of border exclusion area
	for(int j = 0; j < numberofpixels; j++) {
		if(localmaxdata[j] == 1) {
			xval = image_in.getx(j);
			yval = image_in.gety(j);

			if (xval > minx && xval < maxx && yval > miny && yval < maxy) {
				particledata[counter][0] = j;
				particledata[counter][1] = xval;
				particledata[counter][2] = yval;
				counter++;
			}
		}
	}

	//extract local region around each maximum
	int extract_radius = mask_radius;
	int extract_diameter = 2 * extract_radius + 1;
	int extract_size = extract_diameter * extract_diameter;
	int extract_step = 4 * extract_diameter;
	IppiSize extract_ROI = {extract_diameter, extract_diameter};
	int extract_offset = extract_radius * (1 + image_in.get_width());	//calculate _relative_ offset (in pixels) of the array index
	Ipp32f *extracted_square = ippsMalloc_32f(extract_size);
	Ipp32f *multiply_result = ippsMalloc_32f(extract_size);
	Ipp32f *inputimage = ippsMalloc_32f(image_in.get_numberofpixels());
	status = ippiCopy_32f_C1R(image_in.get_image2D(), image_in.get_stepsize(),
			inputimage, image_in.get_stepsize(), image_in.get_ROIfull());
	for(int i = 0; i < counter; i++) {

		int extract_index = particledata[i][0];
		int copy_offset = extract_index - extract_offset;							//this is the starting point for ROI (in pixels!!)
		status = ippiCopy_32f_C1R(inputimage + copy_offset, image_in.get_stepsize(),
			extracted_square, extract_step, extract_ROI);
		//Calculate mass
		Ipp64f total_mass = 0;
		status = ippiMul_32f_C1R(extracted_square, extract_step, ConvolutionKernels.get_circle_kernel(), ConvolutionKernels.get_kernel_step(),
			multiply_result, extract_step, extract_ROI);
		status = ippiSum_32f_C1R(multiply_result, extract_step, extract_ROI, &total_mass, ippAlgHintNone);

		//Calculate x-offset (to be added to integral position)
		Ipp64f x_sum = 0;
		status = ippiMul_32f_C1R(extracted_square, extract_step, ConvolutionKernels.get_ramp_x_kernel(), ConvolutionKernels.get_kernel_step(),
			multiply_result, extract_step, extract_ROI);
		status = ippiSum_32f_C1R(multiply_result, extract_step, extract_ROI, &x_sum, ippAlgHintNone);
		Ipp32f x_offset = (x_sum / total_mass) - extract_radius - 1;

		//Calculate y-offset (to be added to integral position)
		Ipp64f y_sum = 0;
		status = ippiMul_32f_C1R(extracted_square, extract_step, ConvolutionKernels.get_ramp_y_kernel(), ConvolutionKernels.get_kernel_step(),
			multiply_result, extract_step, extract_ROI);
		status = ippiSum_32f_C1R(multiply_result, extract_step, extract_ROI, &y_sum, ippAlgHintNone);
		Ipp32f y_offset = (y_sum / total_mass) - extract_radius - 1;

		//Calculate r^2
		Ipp64f r2_sum = 0;
		status = ippiMul_32f_C1R(extracted_square, extract_step, ConvolutionKernels.get_r2_kernel(), ConvolutionKernels.get_kernel_step(),
			multiply_result, extract_step, extract_ROI);
		status = ippiSum_32f_C1R(multiply_result, extract_step, extract_ROI, &r2_sum, ippAlgHintNone);
		Ipp32f r2_val = r2_sum / total_mass;

		//Calculate "multiplicity": how many particles are within the area calculated by the masks
		Ipp64f multiplicity = 0;
		status = ippiSum_32f_C1R(image_localmax.get_image2D()+copy_offset, image_localmax.get_stepsize(),
			extract_ROI, &multiplicity, ippAlgHintNone);
		Ipp32f multiplicity_val = multiplicity;

		//assign values to particle data array
		if (total_mass > 0) {
		particledata[i][1] += x_offset;
		particledata[i][2] += y_offset;
		particledata[i][3] = x_offset;
		particledata[i][4] = y_offset;
		particledata[i][5] = total_mass;
		particledata[i][6] = r2_val;
		particledata[i][7] = multiplicity_val;
		}
	}

	//Cleanup memory (this part is critical---program otherwise crashes!!)
	ippsFree(extracted_square);
	extracted_square = NULL;

	ippsFree(multiply_result);
	multiply_result = NULL;

	ippsFree(inputimage);
	inputimage = NULL;

	ippsFree(localmaxdata);
	localmaxdata = NULL;

	return particledata;
}

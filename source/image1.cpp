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

#include "image1.h"

Image2D::Image2D(const int image_length, const int image_width):
imagedata(NULL), stepsize(0), width(0), length(0)
{
	numberofpixels = image_width * image_length;
	length = image_length;
	width = image_width;
	imagedata = ippsMalloc_32f(numberofpixels);
	stepsize = 4 * image_width;
	ROIfull.width = image_width;
	ROIfull.height = image_length;
	// tac 2009-03-02
	// initialize the arrays to 0 to prevent junk in
	// them from around the edges from screwing with the
	// recentering processes
	for(int j = 0;j<image_width*image_length; j++)
	  *(imagedata + j) = 0;

}

Image2D::Image2D():
imagedata(NULL), stepsize(0), width(0), length(0)
{

}

Image2D::~Image2D()
{
	ippsFree(imagedata);
	imagedata = NULL;
}

int Image2D::getx(const int index1D)
{
	return index1D % width;
}

int Image2D::gety(const int index1D)
{
	return (int) floor((float) index1D / width);
}

int Image2D::get1Dindex(const int x, const int y)
{
	return y * width + x;
}

Image2D TIFF_to_IPP(FIBITMAP *dib)
{
	IppStatus status;
	int width = FreeImage_GetWidth(dib);
	int height = FreeImage_GetHeight(dib);

	//even for grayscale, need to use 24-bit images (R = G = B)
	int pitch8 = 4 * ceil ((float) width / 4);
	int pitch24 = pitch8 *3;
	int size8 =  pitch8 * height;
	int size24 = 3 * size8;

	//Create 24-bit IPP image
	Ipp8u *image8 = ippsMalloc_8u(size8);
	Ipp8u *image24 = ippsMalloc_8u(size24);
	IppiSize roi = {width, height};

	FreeImage_ConvertToRawBits(image24, dib, pitch24, 24, 255, 255, 255, true);

	//create IPP image to receive TIFF data
	Image2D image_in(height, pitch8);
	status = ippiCopy_8u_C3C1R(image24, pitch24, image8, pitch8, roi);

	//scale the 8-bit IPP format data into 32-bit floating-point IPP data
	status = ippiScale_8u32f_C1R(image8,pitch8,image_in.get_image2D(),
		image_in.get_stepsize(),image_in.get_ROIfull(),0,255);

	//Cleanup memory
	ippsFree(image8);
	ippsFree(image24);
	image8 = NULL;
	image24 = NULL;
	return image_in;
}

IppStatus IPP_to_TIFF(Image2D &image_out, FIMULTIBITMAP *out_imagestack)
{
	IppStatus status;
	Ipp8u *ippi_out8 = ippsMalloc_8u(image_out.get_numberofpixels());
	Ipp8u *ippi_out24 = ippsMalloc_8u(3 * image_out.get_numberofpixels());
	int step8 = image_out.get_width();
	int step24 = 3 * step8;		//equivalent to pitch8 above
	int width = image_out.get_width();
	int height = image_out.get_height();
	IppiSize roi = {width, height};
	Ipp32f maxbrightness = 0;

	//scale 32-bit floating point image to 8-bit integer image
	status = ippiMax_32f_C1R(image_out.get_image2D(),image_out.get_stepsize(),
		image_out.get_ROIfull(), &maxbrightness);
	status = ippiScale_32f8u_C1R(image_out.get_image2D(),image_out.get_stepsize(),
		ippi_out8,step8,image_out.get_ROIfull(),0,maxbrightness);

	//copy each 8-bit image to separate channel in 24-bit image, such that R = G = B
	status = ippiCopy_8u_C1C3R(ippi_out8, step8, ippi_out24, step24, roi);
	status = ippiCopy_8u_C1C3R(ippi_out8, step8, ippi_out24+1, step24, roi);
	status = ippiCopy_8u_C1C3R(ippi_out8, step8, ippi_out24+2, step24, roi);

	//Convert to FreeImage format and reduce back to 8-bits
	FIBITMAP *dib24 = NULL, *dib8 = NULL;

	dib24 = FreeImage_ConvertFromRawBits(ippi_out24, width, height, step24, 24, 255, 255, 255, true);
	dib8 = FreeImage_ConvertTo8Bits(dib24);

	//Output by appending a page to a multi-page TIFF image stack
	if(dib8 != NULL) {
		FreeImage_AppendPage(out_imagestack, dib8);
	}
	//Cleanup memory
	FreeImage_Unload(dib8);
	FreeImage_Unload(dib24);
	dib8 = NULL;
	dib24 = NULL;

	ippsFree(ippi_out8);
	ippsFree(ippi_out24);
	ippi_out8 = NULL;
	ippi_out24 = NULL;

	return status;
}

//==========================================================//
// Copyright (C) 1999, California Institute of Technology.	//
// U.S. Government sponsorship acknowledged.				//
//==========================================================//

static const char rcs_id_image_c[] =
	"@(#) $Id$";

#include <stdlib.h>
#include "Image.h"
#include "Array.h"
#include "Misc.h"
#include "Constants.h"

#define OUT_OF_RANGE(A,B) (A < 0 || A >= _xSize || B < 0 || B >= _ySize)

//=======//
// Image //
//=======//

Image::Image()
:   _xSize(0), _ySize(0), _image(NULL), _mask(NULL)
{
	return;
}

Image::~Image()
{
    Free();
	return;
}

//-----------------//
// Image::Allocate //
//-----------------//
 
int
Image::Allocate(
    const int  x_size,
    const int  y_size)
{
    if (_xSize == x_size && _ySize == y_size)
        return(1);    // already allocated, do nothing

    Free();

    _image = (float **)make_array(sizeof(float), 2, x_size, y_size);
    if (_image == NULL)
        return(0);
 
    _xSize = x_size;
    _ySize = y_size;
 
    return(1);
}

int
Image::Allocate(
    Image*  image)
{
    return(Allocate(image->_xSize, image->_ySize));
}

//-------------//
// Image::Free //
//-------------//
 
void
Image::Free()
{
    FreeImage();
    FreeMask();
    return;
}
 
//------------------//
// Image::FreeImage //
//------------------//
 
void
Image::FreeImage()
{
    if (_image == NULL)
        return;     // already freed
 
    free_array((void *)_image, 2, _xSize, _ySize);
    _image = NULL;
 
    return;
}
 
//-----------------//
// Image::FreeMask //
//-----------------//
 
void
Image::FreeMask()
{
    if (_mask == NULL)
        return;     // already freed
 
    free_array((void *)_mask, 2, _xSize, _ySize);
    _mask = NULL;

    return;
}

//------------------//
// Image::SizeMatch //
//------------------//

int
Image::SizeMatch(
    Image*  image)
{
    if (_xSize == image->_xSize && _ySize == image->_ySize)
        return(1);
    return(0);
}

//---------------//
// Image::MinMax //
//---------------//

int
Image::MinMax(
    float*  min_value,
    float*  max_value)
{
    int count = 0;
    for (int i = 0; i < _xSize; i++)
    {
        for (int j = 0; j < _ySize; j++)
        {
            float value;
            if (! Get(i, j, &value))
                continue;

            if (count)
            {
                if (value < *min_value)
                    *min_value = value;
                if (value > *max_value)
                    *max_value = value;
            }
            else
            {
                *min_value = value;
                *max_value = value;
            }
            count++;
        }
    }
    if (! count)
        return(0);

    return(1);
}

//------------//
// Image::Set //
//------------//

int
Image::Set(
    int    x,
    int    y,
    float  image_value)
{
    if (OUT_OF_RANGE(x, y))
        return(0);

    if (_image == NULL)
        return(0);

    *(*(_image + x) + y) = image_value;
    return(1);
}

int
Image::Set(
    int    x,
    int    y,
    float  image_value,
    uchar  mask_value)
{
    if (! Set(x, y, image_value))
        return(0);

    if (_mask == NULL)
        return(0);

    *(*(_mask + x) + y) = mask_value;
    return(1);
}

//------------//
// Image::Get //
//------------//

int
Image::Get(
    int     x,
    int     y,
    float*  image_value)
{
    if (OUT_OF_RANGE(x, y))
        return(0);

    if (_mask != NULL)
    {
        if (! *(*(_mask + x) + y))
            return(0);
    }
    *image_value = *(*(_image + x) + y);
    return(1);
}

//------------------//
// Image::WritePtim //
//------------------//

int
Image::WritePtim(
    const char*  filename,
    Image*       x_image,
    Image*       y_image,
    float        x_min,
    float        x_max,
    float        x_res,
    float        y_min,
    float        y_max,
    float        y_res)
{
    //-------------//
    // check sizes //
    //-------------//

    if ( (! SizeMatch(x_image)) || (! SizeMatch(y_image)) )
        return(0);

    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    //-------------------//
    // write header info //
    //-------------------//

    char* ptim_string = PTIM_HEADER;
    if (fwrite(&ptim_string, 4, 1, ofp) != 1 ||
        fwrite(&x_min, sizeof(float), 1, ofp) != 1 ||
        fwrite(&x_max, sizeof(float), 1, ofp) != 1 ||
        fwrite(&x_res, sizeof(float), 1, ofp) != 1 ||
        fwrite(&y_min, sizeof(float), 1, ofp) != 1 ||
        fwrite(&y_max, sizeof(float), 1, ofp) != 1 ||
        fwrite(&y_res, sizeof(float), 1, ofp) != 1)
    {
        fclose(ofp);
        return(0);
    }

    //------------//
    // write data //
    //------------//

    for (int i = 0; i < _xSize; i++)
    {
        for (int j = 0; j < _ySize; j++)
        {
            float x, y, value;
            if ( (! Get(i, j, &value)) ||
                 (! x_image->Get(i, j, &x)) ||
                 (! y_image->Get(i, j, &y)) )
            {
                continue;
            }
            if (fwrite(&x, sizeof(float), 1, ofp) != 1 ||
                fwrite(&y, sizeof(float), 1, ofp) != 1 ||
                fwrite(&value, sizeof(float), 1, ofp) )
            {
                fclose(ofp);
                return(0);
            }
        }
    }

    //-------------------//
    // close output file //
    //-------------------//

    fclose(ofp);

    return(1);
}

//-------------//
// Image::Step //
//-------------//

int
Image::Step(
    float  angle)
{
    // convert angle to radians
    double target_angle = angle * dtr;

    double x_center = (double)(_xSize - 1) / 2.0;
    double y_center = (double)(_ySize - 1) / 2.0;
    double max_center = MAX(x_center, y_center);
    double delta_angle = atan2(max_center, 0.5);
    double min_angle = pi_over_two - delta_angle;
    double max_angle = pi_over_two + delta_angle;

    for (int i = 0; i < _xSize; i++)
    {
        double dx = (double)i - x_center;
        for (int j = 0; j < _ySize; j++)
        {
            double dy = (double)j - y_center;
            double pt_angle = atan2(dy, dx);
            double ang_dif = ANGDIF(target_angle, pt_angle);
            if (ang_dif < min_angle)
                *(*(_image + i) + j) = 1.0;
            else if (ang_dif > max_angle)
                *(*(_image + i) + j) = -1.0;
            else
                *(*(_image + i) + j) = 0.0;
        }
    }
    return(1);
}

//-----------------//
// Image::Convolve //
//-----------------//
// slow, time domain convolution

int
Image::Convolve(
    Image*  image_1,
    Image*  image_2)
{
    //------------------------------------//
    // make sure result is the right size //
    //------------------------------------//

    if (! Allocate(image_1))
        return(0);

    //--------------------------------------//
    // make sure image 2 dimensions are odd //
    //--------------------------------------//

    if (IS_EVEN(image_2->_xSize) || IS_EVEN(image_2->_ySize))
        return(0);

    //----------//
    // convolve //
    //----------//
    // or could this really be correllation?

    int delta_x = image_2->_xSize / 2;
    int delta_y = image_2->_ySize / 2;

    for (int result_x = 0; result_x < _xSize; result_x++)
    {
        for (int result_y = 0; result_y < _ySize; result_y++)
        {
            int count = 0;
            double sum = 0.0;
            for (int dx = -delta_x; dx <= delta_x; dx++)
            {
                for (int dy = -delta_y; dy <= delta_y; dy++)
                {
                    float value_1, value_2;
                    if ( (! image_1->Get(result_x + dx, result_y + dy,
                             &value_1)) ||
                         (! image_2->Get(delta_x + dx, delta_y + dy,
                             &value_2)) )
                    {
                        continue;
                    }
                    count++;
                    sum += value_1 * value_2;
                }
            }
            if (count == 0)
            {
                // no value, clear the mask
                Set(result_x, result_y, 0.0, 0);
            }
            else
            {
                Set(result_x, result_y, sum, 1);
            }
        }
    }
    return(1);
}

//-----------------//
// Image::Gradient //
//-----------------//

int
Image::Gradient(
    int  operator_size)
{
    //------------------------------------------------//
    // construct the horizontal and verical operators //
    //------------------------------------------------//

    Image hop;
    hop.Allocate(operator_size, operator_size);
    hop.Step(0.0);

    Image vop;
    vop.Allocate(operator_size, operator_size);
    vop.Step(90.0);

    //---------------------//
    // apply the operators //
    //---------------------//

    Image hgrad;
    hgrad.Convolve(this, &hop);

    Image vgrad;
    vgrad.Convolve(this, &vop);

    //--------------------//
    // take the magnitude //
    //--------------------//

    Magnitude(&hgrad, &vgrad);

    return(1);
}

//------------------//
// Image::Magnitude //
//------------------//

int
Image::Magnitude(
    Image*  image_1,
    Image*  image_2)
{
    //-------------//
    // check sizes //
    //-------------//

    if (! image_1->SizeMatch(image_2))
        return(0);

    //------------------------------------//
    // make sure result is the right size //
    //------------------------------------//

    if (! Allocate(image_1))
        return(0);

    //-------------------------//
    // calculate the magnitude //
    //-------------------------//

    for (int i = 0; i < _xSize; i++)
    {
        for (int j = 0; j < _ySize; j++)
        {
            float value_1, value_2;
            if ( (! image_1->Get(i, j, &value_1)) ||
                 (! image_2->Get(i, j, &value_2)) )
            {
                // no value, clear the mask
                Set(i, j, 0.0, 0);
            }
            else
            {
                Set(i, j, sqrt(value_1*value_1 + value_2*value_2), 1);
            }
        }
    }
    return(1);
}

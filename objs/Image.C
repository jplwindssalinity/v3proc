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

    FreeImage();

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

//---------------------//
// Image::AllocateMask //
//---------------------//

int
Image::AllocateMask(
    uchar  fill_value)
{
    FreeMask();

    _mask = (uchar **)make_array(sizeof(uchar), 2, _xSize, _ySize);
    if (_mask == NULL)
        return(0);

    FillMask(fill_value);
    return(1);
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
    for (int x = 0; x < _xSize; x++)
    {
        for (int y = 0; y < _ySize; y++)
        {
            float value;
            if (! Get(x, y, &value))
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
// Image::WritePltr //
//------------------//

int
Image::WritePltr(
    const char*  filename,
    Image*       x_image,
    Image*       y_image)
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

    char* pltr_string = PLTR_HEADER;
    if (fwrite(pltr_string, 4, 1, ofp) != 1)
    {
        fclose(ofp);
        return(0);
    }

    //------------//
    // write data //
    //------------//

    for (int x = 0; x < _xSize; x++)
    {
        for (int y = 0; y < _ySize; y++)
        {
            // for each point...

            // ...get its value
            float value;
            if (! Get(x, y, &value))
                continue;

            // ...clear the edge array
            float edge_sum[2][2];    // [0=x, 1=y][0=lower, 1=upper]
            float edge_count[2][2];
            for (int i = 0; i < 2; i++)
            {
                for (int j = 0; j < 2; j++)
                {
                    edge_sum[i][j] = 0.0;
                    edge_count[i][j] = 0;
                }
            }

            // ...get its corners
            for (int dx = -1; dx < 2; dx++)
            {
                int edge_x = x + dx;
                for (int dy = -1; dy < 2; dy++)
                {
                    int edge_y = y + dy;
                    float x1, y1;
                    if ( (! x_image->Get(edge_x, edge_y, &x1)) ||
                         (! y_image->Get(edge_x, edge_y, &y1)) )
                    {
                        continue;
                    }
                    if (dx == -1)
                    {
                        edge_sum[0][0] += x1;
                        edge_count[0][0]++;
                    }
                    if (dx == 1)
                    {
                        edge_sum[0][1] += x1;
                        edge_count[0][1]++;
                    }
                    if (dy == -1)
                    {
                        edge_sum[1][0] += y1;
                        edge_count[1][0]++;
                    }
                    if (dy == 1)
                    {
                        edge_sum[1][1] += y1;
                        edge_count[1][1]++;
                    }
                }
            }
            if (edge_count[0][0] == 0 || edge_count[0][1] == 0 ||
                edge_count[1][0] == 0 || edge_count[1][1] == 0)
            {
                continue;
            }
            float buffer[12];

            buffer[0] = edge_sum[0][0] / edge_count[0][0]; // low x
            buffer[1] = edge_sum[1][0] / edge_count[1][0]; // low y

            buffer[2] = edge_sum[0][1] / edge_count[0][1]; // high x
            buffer[3] = edge_sum[1][0] / edge_count[1][0]; // low y
            
            buffer[4] = edge_sum[0][1] / edge_count[0][1]; // high x
            buffer[5] = edge_sum[1][1] / edge_count[1][1]; // high y
            
            buffer[6] = edge_sum[0][0] / edge_count[0][0]; // low x
            buffer[7] = edge_sum[1][1] / edge_count[1][1]; // high y

            buffer[8] = value;
            buffer[9] = 0.0;

            buffer[10] = (float)HUGE_VAL;
            buffer[11] = (float)HUGE_VAL;
            
            if (fwrite(&buffer, sizeof(float) * 12, 1, ofp) != 1)
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

//-----------------//
// Image::FillMask //
//-----------------//

int
Image::FillMask(
    uchar  fill_value)
{
    if (_mask == NULL)
        return(0);

    for (int x = 0; x < _xSize; x++)
    {
        for (int y = 0; y < _ySize; y++)
        {
            *(*(_mask + x) + y) = fill_value;
        }
    }
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

    for (int x = 0; x < _xSize; x++)
    {
        double dx = (double)x - x_center;
        for (int y = 0; y < _ySize; y++)
        {
            double dy = (double)y - y_center;
            double pt_angle = atan2(dy, dx);
            double ang_dif = ANGDIF(target_angle, pt_angle);
            if (ang_dif < min_angle)
                *(*(_image + x) + y) = 1.0;
            else if (ang_dif > max_angle)
                *(*(_image + x) + y) = -1.0;
            else
                *(*(_image + x) + y) = 0.0;
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

    if (! AllocateMask(0))
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

    for (int x = 0; x < _xSize; x++)
    {
        for (int y = 0; y < _ySize; y++)
        {
            float value_1, value_2;
            if ( (! image_1->Get(x, y, &value_1)) ||
                 (! image_2->Get(x, y, &value_2)) )
            {
                // no value, clear the mask
                Set(x, y, 0.0, 0);
            }
            else
            {
                Set(x, y, sqrt(value_1*value_1 + value_2*value_2), 1);
            }
        }
    }
    return(1);
}

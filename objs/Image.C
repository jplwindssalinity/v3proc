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

//----------------//
// Image::WriteIm //
//----------------//

#define IM_HEADER  "im  "

int
Image::WriteIm(
    const char*  filename)
{
    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    //-------------------//
    // write header info //
    //-------------------//

    char* im_string = IM_HEADER;
    if (fwrite(im_string, 4, 1, ofp) != 1)
    {
        fclose(ofp);
        return(0);
    }

    if (fwrite(&_xSize, 4, 1, ofp) != 1 ||
        fwrite(&_ySize, 4, 1, ofp) != 1)
    {
        fclose(ofp);
        return(0);
    }

    //------------//
    // write data //
    //------------//

    for (int x = 0; x < _xSize; x++)
    {
        if (fwrite(*(_image + x), sizeof(float), _ySize, ofp) !=
            (unsigned int)_ySize)
        {
            fclose(ofp);
            return(0);
        }
    }

    //-------------------//
    // close output file //
    //-------------------//

    fclose(ofp);

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

            // ...get its value and location
            float value;
            if (! Get(x, y, &value))
                continue;
            float center_x, center_y;
            if (! x_image->Get(x, y, &center_x))
                continue;
            if (! y_image->Get(x, y, &center_y))
                continue;

            if (center_x == 0.0 || center_y == 0.0)
                continue;

            // ...get its delta corners
            float edges[4][2];
            float dx[4] = { -1,  1,  1, -1 };
            float dy[4] = { -1, -1,  1,  1 };
            int edge_count = 0;
            for (int i = 0; i < 4; i++)
            {
                if (! x_image->Get(x + dx[i], y + dy[i], &(edges[i][0])))
                    break;
                if (! y_image->Get(x + dx[i], y + dy[i], &(edges[i][1])))
                    break;
                if (edges[i][0] == 0.0 && edges[i][1] == 0.0)
                    continue;
                edge_count++;
            }
            if (edge_count != 4)
                continue;
       
            float buffer[12];

            buffer[0] = (center_x + edges[0][0]) / 2.0;
            buffer[1] = (center_y + edges[0][1]) / 2.0;
            buffer[2] = (center_x + edges[1][0]) / 2.0;
            buffer[3] = (center_y + edges[1][1]) / 2.0;
            buffer[4] = (center_x + edges[2][0]) / 2.0;
            buffer[5] = (center_y + edges[2][1]) / 2.0;
            buffer[6] = (center_x + edges[3][0]) / 2.0;
            buffer[7] = (center_y + edges[3][1]) / 2.0;

            buffer[8] = value;
            buffer[9] = 0.0;

            buffer[10] = (float)HUGE_VAL;
            buffer[11] = (float)HUGE_VAL;

            for (int i = 0; i < 8; i++)
                buffer[i] *= rtd;
            
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

//------------------//
// Image::WriteVctr //
//------------------//

int
Image::WriteVctr(
    const char*  filename,
    Image*       dy_image,
    Image*       x_image,
    Image*       y_image)
{
    //-------------//
    // check sizes //
    //-------------//

    if ( (! SizeMatch(dy_image)) || (! SizeMatch(x_image)) ||
        (! SizeMatch(y_image)) )
    {
        return(0);
    }

    //------------------//
    // open output file //
    //------------------//

    FILE* ofp = fopen(filename, "w");
    if (ofp == NULL)
        return(0);

    //-------------------//
    // write header info //
    //-------------------//

    char* vctr_string = VCTR_HEADER;
    if (fwrite(vctr_string, 4, 1, ofp) != 1)
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

            float xvalue, yvalue, dy, dx, magnitude, direction;
            if (! Get(x, y, &dx))
                continue;
            if (! dy_image->Get(x, y, &dy))
                continue;
            if (! x_image->Get(x, y, &xvalue))
                continue;
            if (! y_image->Get(x, y, &yvalue))
                continue;

            if (xvalue == 0.0 || yvalue == 0.0)
                continue;

            magnitude = sqrt(dx * dx + dy * dy);
            direction = atan2(dy, dx);

            float buffer[4];
            buffer[0] = xvalue;
            buffer[1] = yvalue;
            buffer[2] = magnitude;
            buffer[3] = direction;

            if (fwrite(&buffer, sizeof(float) * 4, 1, ofp) != 1)
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

//-------------------//
// Image::WriteAscii //
//-------------------//

int
Image::WriteAscii(
    FILE*  ofp)
{
    fprintf(ofp, "%d x %d\n", _xSize, _ySize);
    for (int x = 0; x < _xSize; x++)
    {
        for (int y = 0; y < _ySize; y++)
        {
            fprintf(ofp, " %g", _image[x][y]);
        }
        fprintf(ofp, "\n");
    }
    return(1);
}

//-------------//
// Image::Fill //
//-------------//

int
Image::Fill(
    float  fill_value)
{
    for (int x = 0; x < _xSize; x++)
    {
        for (int y = 0; y < _ySize; y++)
        {
            *(*(_image + x) + y) = fill_value;
        }
    }
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
    double delta_angle = atan2(0.5, max_center);
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
    int x_cen = (int)(x_center + 0.5);
    int y_cen = (int)(y_center + 0.5);
    *(*(_image + x_cen) + y_cen) = 0.0;
    return(1);
}

//------------------//
// Image::LinearMap //
//------------------//

int
Image::LinearMap(
    float  old_1,
    float  new_1,
    float  old_2,
    float  new_2,
    float  new_min,
    float  new_max)
{
    //----------------------------//
    // calculate mapping function //
    //----------------------------//

    float m = (new_2 - new_1) / (old_2 - old_1);
    float b = (old_2 * new_1 - old_1 * new_2) / (old_2 - old_1);

    for (int x = 0; x < _xSize; x++)
    {
        for (int y = 0; y < _ySize; y++)
        {
            float new_value = *(*(_image + x) + y) * m + b;
            if (new_value < new_min)
                new_value = new_min;
            if (new_value > new_max)
                new_value = new_max;
            *(*(_image + x) + y) = new_value;
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
            int missing_value = 0;
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
                        missing_value = 1;
                        break;
                    }
                    sum += value_1 * value_2;
                }
            }
            if (missing_value)
            {
                // missing values, clear the mask
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
    int     operator_size,
    Image*  gx,
    Image*  gy)
{
    //------------------------------------------------//
    // construct the horizontal and verical operators //
    //------------------------------------------------//

    Image gx_op;
    gx_op.Allocate(operator_size, operator_size);
    gx_op.Step(0.0);

    Image gy_op;
    gy_op.Allocate(operator_size, operator_size);
    gy_op.Step(90.0);

    //---------------------//
    // apply the operators //
    //---------------------//

    Image* use_gx;
    Image tmp_gx;
    if (gx != NULL)
        use_gx = gx;
    else
        use_gx = &tmp_gx;
    use_gx->Convolve(this, &gx_op);

    Image* use_gy;
    Image tmp_gy;
    if (gy != NULL)
        use_gy = gy;
    else
        use_gy = &tmp_gy;
    use_gy->Convolve(this, &gy_op);

    //--------------------//
    // take the magnitude //
    //--------------------//

    Magnitude(use_gx, use_gy);

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

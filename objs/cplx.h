typedef struct
{
   float r;   /* real part */
   float i;   /* imag part */
} complex;

typedef struct
{
   double r;   /* real part */
   double i;   /* imag part */
} doublecomplex;

complex C_cplx (float r, float i);
complex C_conj (complex a);
complex C_assign (complex a);
complex C_add (complex a, complex b);
complex C_sub (complex a, complex b);
complex C_mul (complex a, complex b);
complex C_div (complex a, complex b);
complex C_para (complex a, complex b);
complex C_conj (complex a);
complex C_inv (complex a);
float C_mag (complex a);
float C_ang (complex a);
complex C_exp (float f);


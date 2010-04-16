#ifndef _SWAP_ENDIAN
#define _SWAP_ENDIAN

#if defined __cplusplus
        extern "C" {
#endif


// Macs and SGIs are Big-Endian; PCs are little endian
// returns TRUE if current machine is little endian
extern int IsLittleEndian(void);

/******************************************************************************
  FUNCTION: SwapEndian
  PURPOSE: Swap the byte order of a structure
  EXAMPLE: float F=123.456;; SWAP_FLOAT(F);
******************************************************************************/

#define SWAP_SHORT(Var)  Var = *(short*)         SwapEndian((void*)&Var, sizeof(short))
#define SWAP_USHORT(Var) Var = *(unsigned short*)SwapEndian((void*)&Var, sizeof(short))
#define SWAP_LONG(Var)   Var = *(long*)          SwapEndian((void*)&Var, sizeof(long))
#define SWAP_ULONG(Var)  Var = *(unsigned long*) SwapEndian((void*)&Var, sizeof(long))
#define SWAP_RGB(Var)    Var = *(int*)           SwapEndian((void*)&Var, 3)
#define SWAP_FLOAT(Var)  Var = *(float*)         SwapEndian((void*)&Var, sizeof(float))
#define SWAP_DOUBLE(Var) Var = *(double*)        SwapEndian((void*)&Var, sizeof(double))

#define SWAP_VAR(Var, type)       Var = *(type*)        SwapEndian((void*)&Var, sizeof(type))
#define SWAP_VAR_PTR(Ptr, type)  *Ptr = *(type*)        SwapEndian((void*)Ptr, sizeof(type))
#define SWAP_ARRAY(ar_ptr, ar_size, type)  \
    for (type *work_ptr = (type*)ar_ptr; work_ptr < ((type*)ar_ptr + ar_size); work_ptr++) { \
        SWAP_VAR_PTR(work_ptr, type); \
    }

//        printf("swapping value %d of %d; work_ptr = %u; end = %u\n", i++, ar_size, work_ptr, ((type*)ar_ptr + ar_size));

extern void *SwapEndian(void* Addr, const int Nb);

#if defined __cplusplus
        }
#endif

#endif

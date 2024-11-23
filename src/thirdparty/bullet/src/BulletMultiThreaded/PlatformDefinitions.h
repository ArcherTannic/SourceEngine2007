#include <stdint.h>
#include <vcruntime.h>
#ifndef BT_TYPE_DEFINITIONS_H
#define BT_TYPE_DEFINITIONS_H

#include "LinearMath/btDefines.h"
#include "LinearMath/btMinMax.h"

#ifdef PFX_USE_FREE_VECTORMATH
#include "physics_effects/base_level/base/pfx_vectormath_include.win32.h"
typedef Vectormath::Aos::Vector3    vmVector3;
typedef Vectormath::Aos::Quat       vmQuat;
typedef Vectormath::Aos::Matrix3    vmMatrix3;
typedef Vectormath::Aos::Transform3 vmTransform3;
typedef Vectormath::Aos::Point3     vmPoint3;
#else
#include "vectormath/vmInclude.h"
#endif //PFX_USE_FREE_VECTORMATH

#ifdef _WIN32

typedef union
{
    unsigned int u;
    void* p;
} addr64;

#define USE_WIN32_THREADING 1

typedef unsigned char     uint8_t;

#ifndef __PHYSICS_COMMON_H__
#ifndef PFX_USE_FREE_VECTORMATH
typedef unsigned int uint32_t;
#endif // PFX_USE_FREE_VECTORMATH
#endif // __PHYSICS_COMMON_H__

typedef unsigned short    uint16_t;

#include <malloc.h>
#define memalign(alignment, size) malloc(size)

#include <string.h>
#include <stdio.h>
#define spu_printf printf

#else
#include <stdlib.h>
#include <string.h>

#if defined (__CELLOS_LV2__)
#include <spu_printf.h>
#else
#define USE_PTHREADS (1)

#ifdef USE_LIBSPE2
#include <stdio.h>
#define spu_printf printf
#define DWORD unsigned int
typedef union
{
    unsigned long long ull;
    unsigned int ui[2];
    void* p;
} addr64;
#endif // USE_LIBSPE2

#endif // __CELLOS_LV2__

#endif

#ifdef __SPU__
#include <stdio.h>
#define printf spu_printf
#endif

#include "PpuAddressSpace.h"

#endif // BT_TYPE_DEFINITIONS_H

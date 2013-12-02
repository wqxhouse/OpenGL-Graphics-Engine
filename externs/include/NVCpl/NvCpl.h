
/*********************************************************************NVMH4****

File:  NvCpl.h

Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

******************************************************************************/

#ifndef NV_CPL_H
#define NV_CPL_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************

    Desktop configuration

*******************************************************************************/

// Display quality
#define NV_DISPLAY_DIGITAL_VIBRANCE_MIN 0
#define NV_DISPLAY_DIGITAL_VIBRANCE_MAX 63
#define NV_DISPLAY_BRIGHTNESS_MIN -125
#define NV_DISPLAY_BRIGHTNESS_MAX 125
#define NV_DISPLAY_CONTRAST_MIN -82
#define NV_DISPLAY_CONTRAST_MAX 82
#define NV_DISPLAY_GAMMA_MIN 0.5
#define NV_DISPLAY_GAMMA_MAX 6

// Gamma ramp types
typedef struct NVGammaRampStruct {
    WORD Red[256];
    WORD Green[256];
    WORD Blue[256];
} NVGammaRamp;

// Display states
typedef enum {
	NV_DISPLAY_STATE_FAILED       = 0,
	NV_DISPLAY_STATE_NOT_FOUND    = 1,
	NV_DISPLAY_STATE_UNATTACHED   = 2,
	NV_DISPLAY_STATE_ATTACHED     = 3,
	NV_DISPLAY_STATE_STANDARD     = 4,
	NV_DISPLAY_STATE_DUALVIEW     = 5,
	NV_DISPLAY_STATE_CLONE        = 6,
	NV_DISPLAY_STATE_HSPAN        = 7,
	NV_DISPLAY_STATE_VSPAN        = 8,
	NV_DISPLAY_STATE_VIEW_UNKNOWN = -1
} NVDisplayState;

// Function prototypes
typedef DWORD (APIENTRY* dtcfgexType)(LPSTR);
typedef BOOL (APIENTRY* NvCplGetConnectedDevicesStringType)(LPTSTR, DWORD, BOOL);
typedef int (APIENTRY* NvGetWindowsDisplayStateType)(int);
typedef BOOL (*NvColorGetGammaRampType)(LPTSTR, NVGammaRamp*);
typedef BOOL (*NvColorSetGammaRampType)(LPTSTR, DWORD, NVGammaRamp*);

/*******************************************************************************

    GPU configuration

*******************************************************************************/

// Data type
typedef enum {
	// legacy defines
    NV_DATA_TYPE_BUS_MODE          = 1,
    NV_DATA_TYPE_BUS_TRANSFER_RATE = 3,
    NV_DATA_TYPE_AGP_MEMORY_SIZE   = 5,
    NV_DATA_TYPE_VIDEO_MEMORY_SIZE = 2,
    NV_DATA_TYPE_ANTIALIASING_MODE = 4,

	// new defines in accordance w/ user doc
    NVCPL_API_AGP_BUS_MODE          = 1,
    NVCPL_API_VIDEO_RAM_SIZE        = 2,
    NVCPL_API_TX_RATE               = 3,
    NVCPL_API_CURRENT_AA_VALUE      = 4,
    NVCPL_API_AGP_LIMIT             = 5,
    NVCPL_API_FRAME_QUEUE_LIMIT     = 6,
	NVCPL_API_NUMBER_OF_GPUS	    = 7,
	NVCPL_API_NUMBER_OF_SLI_GPUS    = 8,
    NVCPL_API_SLI_MULTI_GPU_RENDERING_MODE = 9,
} NVDataType;

// Bus modes
typedef enum {
    NV_PCI          = 1,
    NV_AGP          = 4,
    NV_PCI_EXPRESS  = 8,
} NVBusMode;

// Antialiasing modes
typedef enum {
    NV_AA_MODE_OFF      = 0,
    NV_AA_MODE_2X       = 1,
    NV_AA_MODE_QUINCUNX = 2,
    NV_AA_MODE_4X       = 3,
    NV_AA_MODE_4X9T     = 4,
    NV_AA_MODE_4XS      = 5,
    NV_AA_MODE_6XS      = 6,
    NV_AA_MODE_8XS      = 7,
    NV_AA_MODE_16X      = 8,
    NV_AA_MODE_UNKNOWN = -1
} NVAAMode;

typedef enum {
    NVCPL_API_SLI_ENABLED                   = 0x10000000,   // SLI enabled, when this bit-mask is set. 
    NVCPL_API_SLI_RENDERING_MODE_AUTOSELECT = 0x00000000,   // SLI multi-GPU redering mode - Autoselect.
    NVCPL_API_SLI_RENDERING_MODE_AFR        = 0x00000001,   // SLI multi-GPU redering mode - Alternate Frames.
    NVCPL_API_SLI_RENDERING_MODE_SFR        = 0x00000002,   // SLI multi-GPU redering mode - Split Frame.
    NVCPL_API_SLI_RENDERING_MODE_SINGLE_GPU = 0x00000004,   // SLI multi-GPU redering mode - Single GPU.
} NVSLIMode;

// Function prototypes
typedef BOOL (*NvCplGetDataIntType)(long, long*);
typedef BOOL (*NvCplSetDataIntType)(long, long);

#ifdef __cplusplus
}
#endif

#endif /* NV_CPL_H */

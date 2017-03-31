// ==============================================================
// File generated by Vivado(TM) HLS - High-Level Synthesis from C, C++ and SystemC
// Version: 2015.3
// Copyright (C) 2015 Xilinx Inc. All rights reserved.
//
// ==============================================================

#ifndef XV_MIX_H
#define XV_MIX_H

#ifdef __cplusplus
extern "C" {
#endif

/***************************** Include Files *********************************/
#ifndef __linux__
#include "xil_types.h"
#include "xil_assert.h"
#include "xstatus.h"
#include "xil_io.h"
#else
#include <stdint.h>
#include <assert.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stddef.h>
#endif
#include "xv_mix_hw.h"

/**************************** Type Definitions ******************************/
#ifdef __linux__
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
#else

#define XV_MIX_MAX_MEMORY_LAYERS    (7)
/**
* This typedef contains Alpha feature enable flag per memory layer
*/
typedef struct {
  u8  Layer1AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer2AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer3AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer4AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer5AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer6AlphaEn;     /**< Layer Alpha support indicator flag  */
  u8  Layer7AlphaEn;     /**< Layer Alpha support indicator flag  */
}XVMix_AlphaFlag;

/**
* This typedef contains Scaling feature enable flag per memory layer
*/
typedef struct {
  u8  Layer1ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer2ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer3ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer4ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer5ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer6ScalingEn;  /**< Layer scaling support indicator flag  */
  u8  Layer7ScalingEn;  /**< Layer scaling support indicator flag  */
}XVMix_ScaleFlag;

/**
* This typedef contains Interface Type per layer
*/
typedef struct {
  u8  Layer1IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer2IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer3IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer4IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer5IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer6IntfType;  /**< Layer Interface type (Memory/Stream)  */
  u8  Layer7IntfType;  /**< Layer Interface type (Memory/Stream)  */
}XVMix_LayerIntfType;

/**
* This typedef contains color format per memory layer
*/
typedef struct {
  u8  Layer1ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer2ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer3ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer4ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer5ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer6ColorFmt;  /**< Layer Video Color Format  */
  u8  Layer7ColorFmt;  /**< Layer Video Color Format  */
}XVMix_LayerColorFormat;

/**
* This typedef contains maximum width per memory layer
*/
typedef struct {
  u16 Layer1MaxWidth;    /**< Layer maximum column width  */
  u16 Layer2MaxWidth;    /**< Layer maximum column width  */
  u16 Layer3MaxWidth;    /**< Layer maximum column width  */
  u16 Layer4MaxWidth;    /**< Layer maximum column width  */
  u16 Layer5MaxWidth;    /**< Layer maximum column width  */
  u16 Layer6MaxWidth;    /**< Layer maximum column width  */
  u16 Layer7MaxWidth;    /**< Layer maximum column width  */
}XVMix_LayerMaxWidth;

/**
* This typedef contains configuration information for the mixer core
* Each core instance should have a configuration structure associated.
*/
typedef struct {
  u16 DeviceId;          /**< Unique ID  of device */
  UINTPTR BaseAddress;   /**< The base address of the core instance. */
  u16 PixPerClk;         /**< Samples Per Clock supported by core instance */
  u16 MaxWidth;          /**< Maximum columns supported by core instance */
  u16 MaxHeight;         /**< Maximum rows supported by core instance */
  u16 MaxDataWidth;      /**< Maximum Data width of each channel */
  u16 ColorFormat;       /**< Master layer color format */
  u8  NumLayers;         /**< Number of layers supported */
  u8  LogoEn;            /**< Logo layer support indicator flag  */
  u16 MaxLogoWidth;      /**< Maximum columns supported by log layer */
  u16 MaxLogoHeight;     /**< Maximum rows supported by log layer */
  u16 LogoColorKeyEn;    /**< Logo layer color key feature indicator flag */
  u16 LogoPixAlphaEn;    /**< Logo layer per pixel alpha feature indicator flag */
  union {                /**< Alpha feature enable flag per memory layer */
	  XVMix_AlphaFlag AlphaFlag;
	  u8 AlphaEn[XV_MIX_MAX_MEMORY_LAYERS];
  };
  union {                /**< Scaling feature enable flag per memory layer */
	  XVMix_ScaleFlag ScaleFlag;
	  u8 ScalingEn[XV_MIX_MAX_MEMORY_LAYERS];
  };
  union {                /**< Maximum width per memory layer */
	  XVMix_LayerMaxWidth LyrMaxWidth;
	  u16 LayerMaxWidth[XV_MIX_MAX_MEMORY_LAYERS];
  };
  union {                /**< Layer Interface Type */
	  XVMix_LayerIntfType LyrIntfType;
	  u8 LayerIntrfType[XV_MIX_MAX_MEMORY_LAYERS];
  };
  union {                /**< Layer Interface Type */
	  XVMix_LayerColorFormat LyrColorFmt;
	  u8 LayerColorFmt[XV_MIX_MAX_MEMORY_LAYERS];
  };
} XV_mix_Config;
#endif

/**
* Driver instance data. An instance must be allocated for each core in use.
*/
typedef struct {
  XV_mix_Config Config;  /**< Hardware Configuration */
  u32 IsReady;           /**< Device is initialized and ready */
} XV_mix;

/***************** Macros (Inline Functions) Definitions *********************/
#ifndef __linux__
#define XV_mix_WriteReg(BaseAddress, RegOffset, Data) \
    Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define XV_mix_ReadReg(BaseAddress, RegOffset) \
    Xil_In32((BaseAddress) + (RegOffset))
#else
#define XV_mix_WriteReg(BaseAddress, RegOffset, Data) \
    *(volatile u32*)((BaseAddress) + (RegOffset)) = (u32)(Data)
#define XV_mix_ReadReg(BaseAddress, RegOffset) \
    *(volatile u32*)((BaseAddress) + (RegOffset))

#define Xil_AssertVoid(expr)    assert(expr)
#define Xil_AssertNonvoid(expr) assert(expr)

#define XST_SUCCESS             0
#define XST_DEVICE_NOT_FOUND    2
#define XST_OPEN_DEVICE_FAILED  3
#define XIL_COMPONENT_IS_READY  1
#endif

/************************** Function Prototypes *****************************/
#ifndef __linux__
int XV_mix_Initialize(XV_mix *InstancePtr, u16 DeviceId);
XV_mix_Config* XV_mix_LookupConfig(u16 DeviceId);
int XV_mix_CfgInitialize(XV_mix *InstancePtr,
		                 XV_mix_Config *ConfigPtr,
						 UINTPTR EffectiveAddr);
#else
int XV_mix_Initialize(XV_mix *InstancePtr, const char* InstanceName);
int XV_mix_Release(XV_mix *InstancePtr);
#endif

void XV_mix_Start(XV_mix *InstancePtr);
u32 XV_mix_IsDone(XV_mix *InstancePtr);
u32 XV_mix_IsIdle(XV_mix *InstancePtr);
u32 XV_mix_IsReady(XV_mix *InstancePtr);
void XV_mix_EnableAutoRestart(XV_mix *InstancePtr);
void XV_mix_DisableAutoRestart(XV_mix *InstancePtr);

void XV_mix_Set_HwReg_width(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_width(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_height(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_height(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_video_format(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_video_format(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_background_Y_R(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_background_Y_R(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_background_U_G(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_background_U_G(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_background_V_B(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_background_V_B(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerEnable(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerEnable(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer1_V(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layer1_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer2_V(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layer2_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer3_V(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layer3_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer4_V(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layer4_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer5_V(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layer5_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer6_V(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layer6_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layer7_V(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layer7_V(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerAlpha_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerAlpha_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartX_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartX_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStartY_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStartY_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerWidth_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerWidth_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerHeight_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerHeight_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerScaleFactor_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerScaleFactor_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerVideoFormat_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerVideoFormat_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoStartX(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoStartX(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoStartY(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoStartY(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoWidth(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoWidth(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoHeight(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoHeight(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoScaleFactor(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoScaleFactor(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoAlpha(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoAlpha(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_0(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_0(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_1(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_1(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_2(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_2(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_3(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_3(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_4(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_4(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_5(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_5(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_6(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_6(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_layerStride_7(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_layerStride_7(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMin_R(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMin_R(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMin_G(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMin_G(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMin_B(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMin_B(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMax_R(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMax_R(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMax_G(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMax_G(XV_mix *InstancePtr);
void XV_mix_Set_HwReg_logoClrKeyMax_B(XV_mix *InstancePtr, u32 Data);
u32 XV_mix_Get_HwReg_logoClrKeyMax_B(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_BaseAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_HighAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_TotalBytes(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_BitWidth(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoR_V_Depth(XV_mix *InstancePtr);
u32 XV_mix_Write_HwReg_logoR_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Read_HwReg_logoR_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Write_HwReg_logoR_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Read_HwReg_logoR_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Get_HwReg_logoG_V_BaseAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoG_V_HighAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoG_V_TotalBytes(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoG_V_BitWidth(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoG_V_Depth(XV_mix *InstancePtr);
u32 XV_mix_Write_HwReg_logoG_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Read_HwReg_logoG_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Write_HwReg_logoG_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Read_HwReg_logoG_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Get_HwReg_logoB_V_BaseAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoB_V_HighAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoB_V_TotalBytes(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoB_V_BitWidth(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoB_V_Depth(XV_mix *InstancePtr);
u32 XV_mix_Write_HwReg_logoB_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Read_HwReg_logoB_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Write_HwReg_logoB_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Read_HwReg_logoB_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Get_HwReg_logoA_V_BaseAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoA_V_HighAddress(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoA_V_TotalBytes(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoA_V_BitWidth(XV_mix *InstancePtr);
u32 XV_mix_Get_HwReg_logoA_V_Depth(XV_mix *InstancePtr);
u32 XV_mix_Write_HwReg_logoA_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Read_HwReg_logoA_V_Words(XV_mix *InstancePtr, int offset, int *data, int length);
u32 XV_mix_Write_HwReg_logoA_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);
u32 XV_mix_Read_HwReg_logoA_V_Bytes(XV_mix *InstancePtr, int offset, char *data, int length);

void XV_mix_InterruptGlobalEnable(XV_mix *InstancePtr);
void XV_mix_InterruptGlobalDisable(XV_mix *InstancePtr);
void XV_mix_InterruptEnable(XV_mix *InstancePtr, u32 Mask);
void XV_mix_InterruptDisable(XV_mix *InstancePtr, u32 Mask);
void XV_mix_InterruptClear(XV_mix *InstancePtr, u32 Mask);
u32 XV_mix_InterruptGetEnabled(XV_mix *InstancePtr);
u32 XV_mix_InterruptGetStatus(XV_mix *InstancePtr);

#ifdef __cplusplus
}
#endif

#endif
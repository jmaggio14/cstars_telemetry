/******************************************************************************
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/
/****************************************************************************/
/**
*
* @file xusbpsu_class_storage.h
*
*
* <pre>
* MODIFICATION HISTORY:
*
* Ver   Who  Date     Changes
* ----- ---- -------- -------------------------------------------------------
* 1.0   sg  06/06/16  First release
*
* </pre>
*
*****************************************************************************/

#include "xparameters.h"
#include "xusbpsu_class_storage.h"
#include "xusbpsu_ch9_storage.h"

/************************** Constant Definitions ****************************/

/************************** Function Prototypes ******************************/

void XUsbPsu_ClassReq(struct XUsbPsu *InstancePtr,
							SetupPacket *SetupData);
void ParseCBW(struct XUsbPsu *InstancePtr);
void SendCSW(struct XUsbPsu *InstancePtr, u32 Length);

/************************** Variable Definitions *****************************/

extern u8 Phase;
extern u8 VirtFlash[];

/*
 * Pre-manufactured response to the SCSI Inquiry command.
 */
const static SCSI_INQUIRY scsiInquiry[] ALIGNMENT_CACHELINE = {
		{
			0x00,
			0x80,
			0x00,
			0x01,
			0x1f,
			0x00,
			0x00,
			0x00,
			{"Xilinx  "},		/* Vendor ID:  must be  8 characters long. */
			{"PS USB VirtDisk"},/* Product ID: must be 16 characters long. */
			{"1.00"}		/* Revision:   must be  4 characters long. */
		},
		{
			0x00,
			0x80,
			0x02,
			0x02,
			0x1F,
			0x00,
			0x00,
			0x00,
			{"Linux  "},		/* Vendor ID:  must be  8 characters long. */
			{"File-Stor Gadget"},	/* Product ID: must be 16 characters long. */
			{"0404"}		/* Revision:   must be  4 characters long. */
		}
};

static u8 MaxLUN ALIGNMENT_CACHELINE = 0;

extern USB_CBW CBW;
extern USB_CSW CSW;

extern int	rxBytesLeft;
extern u8 *VirtFlashWritePointer;

/* Local transmit buffer for simple replies. */
static u8 txBuffer[128] ALIGNMENT_CACHELINE;


/****************************************************************************/
/**
* This function is class handler for Mass storage and is called when
* Setup packet received is for Class request(not Standard Device request)
*
* @param	InstancePtr is pointer to XUsbPsu instance.
* @param	SetupData is pointer to SetupPacket received.
*
* @return	None
*
* @note		None.
*
*****************************************************************************/
void XUsbPsu_ClassReq(struct XUsbPsu *InstancePtr,
								SetupPacket *SetupData)
{
	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(SetupData   != NULL);

	switch(SetupData->bRequest) {
	case XUSBPSU_CLASSREQ_MASS_STORAGE_RESET:
		/* For Control transfers, Status Phase is handled by driver */
		break;

	case XUSBPSU_CLASSREQ_GET_MAX_LUN:
		XUsbPsu_EpBufferSend(InstancePtr, 0, &MaxLUN, 1);
		break;

	default:
		/*
		 * Unsupported command. Stall the end point.
		 */
		XUsbPsu_EpSetStall(InstancePtr, 0, XUSBPSU_EP_DIR_OUT);
		break;
	}
}

/*****************************************************************************/
/**
* This function handles Reduced Block Command (RBC) requests from the host.
*
* @param	InstancePtr is a pointer to XUsbPsu instance of the controller.
*
* @return	None.
*
* @note		None.
*
******************************************************************************/
void ParseCBW(struct XUsbPsu *InstancePtr)
{
	u32	Offset;
	u8 Array[50]= {0x0F, 0x00, 0x00, 0x00, 0x08, 0x0A, 0x04, 0x00, 0xFF, 0xFF,
        0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF};
	u8 Array1[50];
	u8 Index;
	s32 Status;

	switch (CBW.CBWCB[0]) {
	case USB_RBC_INQUIRY:
#ifdef CLASS_STORAGE_DEBUG
		printf("SCSI: INQUIRY\r\n");
#endif
		Phase = USB_EP_STATE_DATA;

		Status = XUsbPsu_IsSuperSpeed(InstancePtr);
		if(Status != XST_SUCCESS) {
			/* USB 2.0 */
			Index = 0;
		} else {
			/* USB 3.0 */
			Index = 1;
		}

		XUsbPsu_EpBufferSend(InstancePtr, 1, (void *) &scsiInquiry[Index],
								sizeof(scsiInquiry[Index]));
		break;

	case USB_UFI_GET_CAP_LIST:
	{
		SCSI_CAP_LIST	*CapList;

		CapList = (SCSI_CAP_LIST *) txBuffer;
#ifdef CLASS_STORAGE_DEBUG
		printf("SCSI: CAPLIST\r\n");
#endif
		CapList->listLength	= 8;
		CapList->descCode	= 3;
		CapList->numBlocks	= htonl(VFLASH_NUM_BLOCKS);
		CapList->blockLength = htons(VFLASH_BLOCK_SIZE);

		Phase = USB_EP_STATE_DATA;
		XUsbPsu_EpBufferSend(InstancePtr, 1, txBuffer, sizeof(SCSI_CAP_LIST));
	}
		break;

	case USB_RBC_READ_CAP:
	{
		SCSI_READ_CAPACITY	*Cap;

		Cap = (SCSI_READ_CAPACITY *) txBuffer;
#ifdef CLASS_STORAGE_DEBUG
		printf("SCSI: READCAP\r\n");
#endif
		Cap->numBlocks = htonl(VFLASH_NUM_BLOCKS - 1);
		Cap->blockSize = htonl(VFLASH_BLOCK_SIZE);
		Phase = USB_EP_STATE_DATA;
		XUsbPsu_EpBufferSend(InstancePtr, 1, txBuffer,
								sizeof(SCSI_READ_CAPACITY));
	}
		break;

	case USB_RBC_READ:
		Offset = htonl(((SCSI_READ_WRITE *) &CBW.CBWCB)-> block) *
					VFLASH_BLOCK_SIZE;
#ifdef CLASS_STORAGE_DEBUG
		printf("SCSI: READ Offset 0x%08x\r\n", (int) Offset);
#endif

		Phase = USB_EP_STATE_DATA;
		int RetVal = XUsbPsu_EpBufferSend(InstancePtr, 1, &VirtFlash[Offset],
						htons(((SCSI_READ_WRITE *) &CBW.CBWCB)->
							 length) * VFLASH_BLOCK_SIZE);
		if (RetVal != XST_SUCCESS) {
			xil_printf("Failed: READ Offset 0x%08x\n", (int) Offset);
			return;
		}
		break;

	case USB_RBC_MODE_SENSE:
#ifdef CLASS_STORAGE_DEBUG
		printf("SCSI: MODE SENSE\r\n");
#endif
		Phase = USB_EP_STATE_DATA;
		XUsbPsu_EpBufferSend(InstancePtr,
				1, (u8 *)Array, 16);
		break;

	case USB_RBC_MODE_SELECT:
	#ifdef CLASS_STORAGE_DEBUG
			printf("SCSI: MODE_SELECT\r\n");
	#endif
		Phase = USB_EP_STATE_DATA;
		XUsbPsu_EpBufferRecv(InstancePtr,
				1, (u8 *)Array1, 24);
		break;

	case USB_RBC_TEST_UNIT_READY:
#ifdef CLASS_STORAGE_DEBUG
	printf("SCSI: TEST UNIT READY\r\n");
#endif
		SendCSW(InstancePtr, 0);
		break;

	case USB_RBC_MEDIUM_REMOVAL:
#ifdef CLASS_STORAGE_DEBUG
	printf("SCSI: MEDIUM REMOVAL\r\n");
#endif
		SendCSW(InstancePtr, 0);
		break;

	case USB_RBC_VERIFY:
#ifdef CLASS_STORAGE_DEBUG
	printf("SCSI: VERIFY\n");
#endif
		SendCSW(InstancePtr, 0);
		break;

	case USB_RBC_WRITE:
		Offset = htonl(((SCSI_READ_WRITE *) &CBW.CBWCB)->
				       block) * VFLASH_BLOCK_SIZE;
#ifdef CLASS_STORAGE_DEBUG
		printf("SCSI: WRITE Offset 0x%08x\r\n", (int) Offset);
#endif
		VirtFlashWritePointer = &VirtFlash[Offset];

		rxBytesLeft = htons(((SCSI_READ_WRITE *) &CBW.CBWCB)->length)
							* VFLASH_BLOCK_SIZE;

		Phase = USB_EP_STATE_DATA;
		XUsbPsu_EpBufferRecv(InstancePtr, 1, &VirtFlash[Offset],
							rxBytesLeft);
		break;

	case USB_RBC_STARTSTOP_UNIT:
	{
		u8 immed;

		immed = ((SCSI_START_STOP *) &CBW.CBWCB)->immed;
#ifdef CLASS_STORAGE_DEBUG
		printf("SCSI: START/STOP unit: immed %02x\r\n", immed);
#endif
		/* If the immediate bit is 0 we are supposed to send
		 * a success status.
		 */
		if (0 == (immed & 0x01)) {
			SendCSW(InstancePtr, 0);
		}
		break;
	}

	case USB_RBC_REQUEST_SENSE:
#ifdef CLASS_STORAGE_DEBUG
		printf("SCSI: REQUEST_SENSE\r\n");
#endif
		break;
	}
}

/****************************************************************************/
/**
* This function is used to send SCSI Command Status Wrapper to Host.
*
* @param	InstancePtr is pointer to XUsbPsu instance.
* @param	Length is the data residue.
*
* @return	None
*
* @note
*
*****************************************************************************/
void SendCSW(struct XUsbPsu *InstancePtr, u32 Length)
{
	CSW.dCSWSignature = 0x53425355;
	CSW.dCSWTag = CBW.dCBWTag;
	CSW.dCSWDataResidue = Length;
	CSW.bCSWStatus = 0;
	Phase = USB_EP_STATE_STATUS;
	XUsbPsu_EpBufferSend(InstancePtr, 1, (void *) &CSW, 13);
}

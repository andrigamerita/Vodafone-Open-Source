#ifndef __FAP_HW_H_INCLUDED__
#define __FAP_HW_H_INCLUDED__

/*
 <:copyright-BRCM:2007:DUAL/GPL:standard
 
    Copyright (c) 2007 Broadcom Corporation
    All Rights Reserved
 
 Unless you and Broadcom execute a separate written software license 
 agreement governing use of this software, this software is licensed 
 to you under the terms of the GNU General Public License version 2 
 (the "GPL"), available at http://www.broadcom.com/licenses/GPLv2.php, 
 with the following added to such license:
 
    As a special exception, the copyright holders of this software give 
    you permission to link this software with independent modules, and 
    to copy and distribute the resulting executable under terms of your 
    choice, provided that you also meet, for each linked independent 
    module, the terms and conditions of the license of that module. 
    An independent module is a module which is not derived from this
    software.  The special exception does not apply to any modifications 
    of the software.  
 
 Not withstanding the above, under no circumstances may you combine 
 this software in any way with any other Broadcom software provided 
 under a license other than the GPL, without Broadcom's express prior 
 written consent. 
 
:>
*/

/*
 *******************************************************************************
 * File Name  : fap_hw.h
 *
 * Description: This file contains ...
 *
 *******************************************************************************
 */

#if defined(CONFIG_BCM96362)
#include "fap_hw_6362.h"
#elif defined(CONFIG_BCM963268)
#include "fap_hw_63268.h"
#elif defined(CONFIG_BCM96828)
#include "fap_hw_6828.h"
#elif defined(CONFIG_BCM96818)
#include "fap_hw_6818.h"
#else
#error "FAP BEING INCLUDED FOR NON SUPPORTED CHIP"
#endif


#endif /* __FAP_HW_H_INCLUDED__ */


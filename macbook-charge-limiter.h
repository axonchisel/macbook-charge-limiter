/*
 * MacBook Charge Limiter Tool
 * Copyright (c) 2021 AxonChisel.net
 * Copyright (C) 2015 theopolis
 * Copyright (C) 2006 devnull
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef __MACBOOK_CHARGE_LIMITER_H__
#define __MACBOOK_CHARGE_LIMITER_H__
#endif

#include <unistd.h>
#include <sys/types.h>

//#include "OSTypes.h"
#include <IOKit/IOKitLib.h>


// ---------------------------------------------------------------------------
// App Constants
// ---------------------------------------------------------------------------

#define VERSION "1.0.4"

#define OP_NONE 0
#define OP_READ 1
#define OP_WRITE 2

#define BCLM_VAL_MIN 1
#define BCLM_VAL_MAX 100
#define BCLM_VAL_DEFAULT 100


// ---------------------------------------------------------------------------
// App Data Model
// ---------------------------------------------------------------------------

typedef struct {
    bool verbose = false;
    int new_limit;
    int op = OP_NONE;
} MCL_AppOptions_t;


// ---------------------------------------------------------------------------
// SMC Constants
// ---------------------------------------------------------------------------

#define KERNEL_INDEX_SMC 2

#define SMC_CMD_READ_BYTES 5
#define SMC_CMD_WRITE_BYTES 6
#define SMC_CMD_READ_KEYINFO 9

#define SMC_KEY_BATTERY_CHARGE_LEVEL_MAX "BCLM"


// ---------------------------------------------------------------------------
// SMC Data Model
// ---------------------------------------------------------------------------

typedef struct {
    char major;
    char minor;
    char build;
    char reserved[1];
    UInt16 release;
} SMCKeyData_vers_t;

typedef struct {
    UInt16 version;
    UInt16 length;
    UInt32 cpuPLimit;
    UInt32 gpuPLimit;
    UInt32 memPLimit;
} SMCKeyData_pLimitData_t;

typedef struct {
    UInt32 dataSize;
    UInt32 dataType;
    char dataAttributes;
} SMCKeyData_keyInfo_t;

typedef char SMCBytes_t[32];

typedef struct {
    UInt32 key;
    SMCKeyData_vers_t vers;
    SMCKeyData_pLimitData_t pLimitData;
    SMCKeyData_keyInfo_t keyInfo;
    char result;
    char status;
    char data8;
    UInt32 data32;
    SMCBytes_t bytes;
} SMCKeyData_t;

typedef char UInt32Char_t[5];

typedef struct {
    UInt32Char_t key;
    UInt32 dataSize;
    UInt32Char_t dataType;
    SMCBytes_t bytes;
} SMCVal_t;

// ---------------------------------------------------------------------------

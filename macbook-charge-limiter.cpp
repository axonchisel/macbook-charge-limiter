/*
 * MacBook Charge Limiter Tool
 * Copyright (c) 2021 AxonChisel.net
 * Copyright (C) 2015 theopolis
 * Copyright (C) 2006 devnull
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301,
 * USA.
 */

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "macbook-charge-limiter.h"


// ---------------------------------------------------------------------------
// Globals
// ---------------------------------------------------------------------------

// We only need 1 open connection, might as well be global.
io_connect_t kIOConnection;


// ---------------------------------------------------------------------------
// Byte/String Conversion Utilities
// ---------------------------------------------------------------------------

UInt32 _strtoul(const char *str, int size, int base) {
    UInt32 total = 0;
    int i;

    for (i = 0; i < size; i++) {
        if (base == 16)
            total += str[i] << (size - 1 - i) * 8;
        else
            total += (unsigned char)(str[i] << (size - 1 - i) * 8);
    }
    return total;
}

void _ultostr(char *str, UInt32 val) {
    str[0] = '\0';
    snprintf(str,
             5,
             "%c%c%c%c",
             (unsigned int)val >> 24,
             (unsigned int)val >> 16,
             (unsigned int)val >> 8,
             (unsigned int)val);
}

void printUInt(SMCVal_t val) {
    printf("%u ", (unsigned int)_strtoul(val.bytes, val.dataSize, 10));
}

void printVal(SMCVal_t val) {
    printf("  %s  [%-4s]  ", val.key, val.dataType);
    if (val.dataSize > 0) {
        if ((strcmp(val.dataType, DATATYPE_UINT8) == 0) ||
            (strcmp(val.dataType, DATATYPE_UINT16) == 0) ||
            (strcmp(val.dataType, DATATYPE_UINT32) == 0))
            printUInt(val);
    } else {
        printf("no data\n");
    }
}


// ---------------------------------------------------------------------------
// SMC Interface
// ---------------------------------------------------------------------------

kern_return_t SMCOpen(io_connect_t *conn) {
    kern_return_t result;
    mach_port_t masterPort;
    io_iterator_t iterator;
    io_object_t device;

    result = IOMasterPort(MACH_PORT_NULL, &masterPort);

    CFMutableDictionaryRef matchingDictionary = IOServiceMatching("AppleSMC");
    result =
            IOServiceGetMatchingServices(masterPort, matchingDictionary, &iterator);
    if (result != kIOReturnSuccess) {
        printf("Error: IOServiceGetMatchingServices() = %08x\n", result);
        return 1;
    }

    device = IOIteratorNext(iterator);
    IOObjectRelease((io_object_t)iterator);
    if (device == 0) {
        printf("Error: no SMC found\n");
        return 1;
    }

    result = IOServiceOpen(device, mach_task_self(), 0, conn);
    IOObjectRelease(device);
    if (result != kIOReturnSuccess) {
        printf("Error: IOServiceOpen() = %08x\n", result);
        return 1;
    }

    return kIOReturnSuccess;
}

kern_return_t SMCClose(io_connect_t conn) { return IOServiceClose(conn); }

kern_return_t SMCCall(  uint32_t selector,
                        SMCKeyData_t *inputStructure,
                        SMCKeyData_t *outputStructure) {
    size_t structureInputSize;
    size_t structureOutputSize;

    structureInputSize = sizeof(SMCKeyData_t);
    structureOutputSize = sizeof(SMCKeyData_t);

    return IOConnectCallStructMethod(kIOConnection,
                                     selector,
                                     inputStructure,
                                     structureInputSize,
                                     outputStructure,
                                     &structureOutputSize);
}

kern_return_t SMCReadKey(const std::string &key, SMCVal_t *val) {
    kern_return_t result;
    SMCKeyData_t inputStructure;
    SMCKeyData_t outputStructure;

    memset(&inputStructure, 0, sizeof(SMCKeyData_t));
    memset(&outputStructure, 0, sizeof(SMCKeyData_t));
    memset(val, 0, sizeof(SMCVal_t));

    inputStructure.key = _strtoul(key.c_str(), 4, 16);
    snprintf(val->key, 5, "%s", key.c_str());
    inputStructure.data8 = SMC_CMD_READ_KEYINFO;

    result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
    if (result != kIOReturnSuccess)
        return result;

    val->dataSize = outputStructure.keyInfo.dataSize;
    _ultostr(val->dataType, outputStructure.keyInfo.dataType);
    inputStructure.keyInfo.dataSize = val->dataSize;
    inputStructure.data8 = SMC_CMD_READ_BYTES;

    result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
    if (result != kIOReturnSuccess)
        return result;

    memcpy(val->bytes, outputStructure.bytes, sizeof(outputStructure.bytes));

    return kIOReturnSuccess;
}

kern_return_t SMCWriteKey(SMCVal_t writeVal) {
    kern_return_t result;
    SMCKeyData_t inputStructure;
    SMCKeyData_t outputStructure;

    SMCVal_t readVal;

    result = SMCReadKey(writeVal.key, &readVal);
    if (result != kIOReturnSuccess) {
        return result;
    }

    if (readVal.dataSize != writeVal.dataSize) {
        writeVal.dataSize = readVal.dataSize;
    }

    memset(&inputStructure, 0, sizeof(SMCKeyData_t));
    memset(&outputStructure, 0, sizeof(SMCKeyData_t));

    inputStructure.key = _strtoul(writeVal.key, 4, 16);
    inputStructure.data8 = SMC_CMD_WRITE_BYTES;
    inputStructure.keyInfo.dataSize = writeVal.dataSize;
    memcpy(inputStructure.bytes, writeVal.bytes, sizeof(writeVal.bytes));

    result = SMCCall(KERNEL_INDEX_SMC, &inputStructure, &outputStructure);
    if (result != kIOReturnSuccess) {
        return result;
    }

    return kIOReturnSuccess;
}


// ---------------------------------------------------------------------------
// Tool Logic
// ---------------------------------------------------------------------------

void usage(char *prog) {
    printf("MacBook Charge Limiter tool %s\n", VERSION);
    printf("Usage:\n");
    printf("%s [options]\n", prog);
    printf("    -h         : help\n");
    printf("    -w <value> : write the specified value to a key\n");
    printf("    -v         : version\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    int c;
    extern char *optarg;
    extern int optind, optopt, opterr;

    kern_return_t result;
    int op = OP_NONE;
    UInt32Char_t key = SMC_KEY_BATTERY_CHARGE_LEVEL_MAX;
    SMCVal_t val;

    bool fixed_val = false;
    while ((c = getopt(argc, argv, "hk:rw:v")) != -1) {
        switch (c) {
        case 'r':
            op = OP_READ;
            break;
        case 'v':
            printf("%s\n", VERSION);
            return 0;
            break;
        case 'w':
            fixed_val = true;
            op = OP_WRITE;

            {
                size_t i;
                int j, k1, k2;
                char c;
                char *p = optarg;
                j = 0;
                i = 0;
                while (i < strlen(optarg)) {
                    c = *p++;
                    k1 = k2 = 0;
                    i++;
                    if ((c >= '0') && (c <= '9')) {
                        k1 = c - '0';
                    } else if ((c >= 'a') && (c <= 'f')) {
                        k1 = c - 'a' + 10;
                    }
                    c = *p++;
                    i++;
                    if ((c >= '0') && (c <= '9')) {
                        k2 = c - '0';
                    } else if ((c >= 'a') && (c <= 'f')) {
                        k2 = c - 'a' + 10;
                    }

                    val.bytes[j++] = (int)(((k1 & 0xf) << 4) + (k2 & 0xf));
                }
                val.dataSize = j;
            }
            break;
        case 'h':
        case '?':
            op = OP_NONE;
            break;
        }
    }

    if (op == OP_NONE) {
        usage(argv[0]);
        return 1;
    }

    SMCOpen(&kIOConnection);

    int retcode = 0;
    switch (op) {
        case OP_READ:
            result = SMCReadKey(key, &val);
            if (result != kIOReturnSuccess) {
                fprintf(stderr, "Error: SMCReadKey() = %08x\n", result);
                retcode = 1;
            }
            else {
                printVal(val);
            }
            break;
        case OP_WRITE:
            snprintf(val.key, 5, "%s", key);
            result = SMCWriteKey(val);
            if (result != kIOReturnSuccess) {
                fprintf(stderr, "Error: SMCWriteKey() = %08x\n", result);
                retcode = 1;
            }
            break;
    }

    SMCClose(kIOConnection);
    return retcode;
}

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

UInt32 _strtoul(const char *str, int size) {
    UInt32 total = 0;
    int i;
    for (i = 0; i < size; i++) {
        total += str[i] << (size - 1 - i) * 8;
    }
    return total;
}

void _ultostr(char *str, UInt32 val) {
    str[0] = '\0';
    snprintf(str, 5, "%c%c%c%c",
             (unsigned int)val >> 24,
             (unsigned int)val >> 16,
             (unsigned int)val >> 8,
             (unsigned int)val);
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

kern_return_t SMCClose(io_connect_t conn) {
    return IOServiceClose(conn);
}

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

    inputStructure.key = _strtoul(key.c_str(), 4);
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

    inputStructure.key = _strtoul(writeVal.key, 4);
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
    printf("MacBook Charge Limiter Tool %s -- Copyright (C) 2021 AxonChisel.net\n", VERSION);
    printf("\n");
    printf("Usage:\n");
    printf("%s [options] [new-limit]\n", prog);
    printf("    -h         : help\n");
    printf("    -v         : verbose mode\n");
    printf("Invoke with no arguments to read current value,\n");
    printf("or specify value %d-%d to set new charge limit.\n",
            BCLM_VAL_MIN, BCLM_VAL_MAX);
    printf("\n");
    printf("This software comes with ABSOLUTELY NO WARRANTY. Use at your own risk.\n");
    printf("More info: https://github.com/axonchisel/macbook-charge-limiter\n");
    printf("\n");
}

int main(int argc, char *argv[]) {
    int c;
    int retcode = 0;
    bool verbose = false;
    extern char *optarg;
    extern int optind, optopt, opterr;

    kern_return_t result;
    int op = OP_NONE;
    UInt32Char_t key = SMC_KEY_BATTERY_CHARGE_LEVEL_MAX;
    SMCVal_t val;
    int new_limit;

    while ((c = getopt(argc, argv, "hv")) != -1) {
        switch (c) {
        case 'v':
            verbose = true;
            break;
        case 'h':
        case '?':
            usage(argv[0]);
            return 0;
        }
    }

    if (retcode != 0) {
        return retcode;
    }

    if (optind == (argc)) { // (if no more args past getopt)
        op = OP_READ;
    }

    if (optind == (argc-1)) { // (if exactly one more arg past getopt)
        op = OP_WRITE;
        if ((sscanf(argv[optind], "%d", &new_limit) != 1) ||
            (new_limit < BCLM_VAL_MIN) || (new_limit > BCLM_VAL_MAX))
        {
            fprintf(stderr, "Error: Invalid value '%s' (must be %d-%d)\n",
                            argv[optind], BCLM_VAL_MIN, BCLM_VAL_MAX);
            return 1;
        }
        val.dataSize = 1;
        val.bytes[0] = (new_limit & 0xff);
    }

    if (op == OP_NONE) {
        usage(argv[0]);
        return 1;
    }

    SMCOpen(&kIOConnection);

    switch (op) {
        case OP_READ:
            result = SMCReadKey(key, &val);
            if (result != kIOReturnSuccess) {
                fprintf(stderr, "Error: SMCReadKey() = %08x\n", result);
                retcode = 1;
            }
            else {
                if (verbose) {
                    printf("Current battery charge limit (%d-%d): %d%%\n",
                        BCLM_VAL_MIN, BCLM_VAL_MAX, val.bytes[0]);
                } else {
                    printf("%d\n", val.bytes[0]);
                }
            }
            break;
        case OP_WRITE:
            snprintf(val.key, 5, "%s", key);
            result = SMCWriteKey(val);
            if (result != kIOReturnSuccess) {
                fprintf(stderr, "Error: SMCWriteKey() = %08x (Did you run with sudo?)\n", result);
                retcode = 1;
            }
            if (verbose) {
                printf("Set battery charge limit (%d-%d) to: %d%%\n",
                    BCLM_VAL_MIN, BCLM_VAL_MAX, val.bytes[0]);
            }
            break;
    }

    SMCClose(kIOConnection);
    return retcode;
}

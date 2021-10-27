#include "config.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <stdio.h>
#include <math.h>


// Number of bytes per thread partition                         2^21 = 2,097,152
uint32_t BytesPerDRAMPartition=1<<TinselLogBytesPerDRAMPartition;

// The number of cores that share a DRAM                            2^(2+3) = 32
uint32_t CoresPerDRAM=1<<(TinselLogCoresPerDCache+TinselLogDCachesPerDRAM);

// The number of threads per DRAM                                    32*16 = 512
uint32_t ThreadsPerDRAM=CoresPerDRAM*TinselThreadsPerCore;

uint32_t InterleavedPartitionOffset=0x80000000;

int main(void)
{
    // The output binary
    std::ofstream dataBin;

    dataBin.open("threadCtxInit_data.v");
    if(dataBin.fail()) // Check that the file opened
    {                 // if it didn't, barf
        std::cout << "***ERROR*** FAILED TO OPEN DATA BINARY\n" << std::endl;
        return -1;
    }

    // Populate the bin
    uint32_t i = 0;
    for(int coreNum = 0; coreNum<CoresPerDRAM; coreNum++)
    {
        for(int threadNum = 1; threadNum<=TinselThreadsPerCore; threadNum++)
        {
            uint32_t threadAddr, threadOffset;
            
            threadOffset = ((coreNum << TinselLogThreadsPerCore) + threadNum);
            threadOffset *= BytesPerDRAMPartition;
            
            threadAddr = TinselBytesPerDRAM - threadOffset;
            
            threadAddr |= InterleavedPartitionOffset;
            
            // Write the address
            dataBin << std::hex << std::uppercase << "@" << threadAddr << "\n";
            
            // Write out the initialiser
            dataBin << "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 \n";  
            dataBin << "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 \n";  
            dataBin << "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 \n"; 
            dataBin << "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 \n"; 
            dataBin << "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 \n"; 
            dataBin << "00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 \n"; 
            dataBin << "00 00 00 00 00 00 00 00 " << std::endl;
            
            i++;
        }
    }

    // And we are done
    dataBin.close();
    
    return 0;
}


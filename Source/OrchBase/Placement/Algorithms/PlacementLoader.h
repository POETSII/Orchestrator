#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_PLACEMENTLOADER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_ALGORITHMS_PLACEMENTLOADER_H

/* Describes an algorithm that loads a placement file, and uses it to place a
 * task. Written in a hurry. */

class HardwareIterator;

#include "HardwareIterator.h"

class PlacementLoader: public Algorithm
{
public:
    std::map<std::string, std::string> dataFromFile;
    std::string filePath;
    PlacementLoader(Placer* placer, std::string path);
    float do_it(P_task* task);
    void load_file();
};

#endif

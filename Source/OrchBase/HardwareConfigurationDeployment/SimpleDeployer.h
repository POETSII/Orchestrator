#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_SIMPLEDEPLOYER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_SIMPLEDEPLOYER_H

/* Defines a configuration sufficient to deploy "an Simple engine". It's all in
 * the constructor. */

#include "Dialect1Deployer.h"

class SimpleDeployer: public Dialect1Deployer
{
public:
    SimpleDeployer();
};

#endif

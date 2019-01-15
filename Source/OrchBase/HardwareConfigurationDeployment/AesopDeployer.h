#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_AESOPDEPLOYER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_AESOPDEPLOYER_H

/* Defines a configuration sufficient to deploy "an Aesop engine". It's all in
 * the constructor. */

#include "Dialect1Deployer.h"

class AesopDeployer: public Dialect1Deployer
{
public:
    AesopDeployer();
};

#endif

#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_MULTIAESOPDEPLOYER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_MULTIAESOPDEPLOYER_H

/* Defines a configuration sufficient to deploy some number of Aesop
 * engines connected together. It's all in the constructor. */

#include "AesopDeployer.h"

class MultiAesopDeployer: public AesopDeployer
{
public:
    MultiAesopDeployer(unsigned multiple);
private:
    unsigned word_length_from_quantity(unsigned quantity);
};

#endif

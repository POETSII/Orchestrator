#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_MULTISIMPLEDEPLOYER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWARECONFIGURATION_MULTISIMPLEDEPLOYER_H

/* Defines a configuration sufficient to deploy some number of Simple
 * engines connected together. It's all in the constructor. */

#include "SimpleDeployer.h"

class MultiSimpleDeployer: public SimpleDeployer
{
public:
    MultiSimpleDeployer(unsigned multiple);
private:
    unsigned word_length_from_quantity(unsigned quantity);
};

#endif

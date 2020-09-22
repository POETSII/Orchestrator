#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_LINK_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_P_LINK_H

/* Describes the object that sits on an edge of the Engine's board graph, or on
 * the edge of a board's mailbox graph.
 *
 * See the hardware model documentation for further information on these
 * links. */

#include "DumpUtils.h"
#include "NameBase.h"
#include "pdigraph.hpp"
class    Meta_t;

class P_link : public NameBase, public DumpChan
{
public:
                    P_link(string);
                    P_link(float weight);
                    P_link(float weight, NameBase* parent);
virtual ~           P_link();
float               weight;
void                Dump(FILE* = stdout);
static void         LnkDat_cb(P_link * const &);
static void         LnkKey_cb(unsigned const &);
unsigned            key;
vector<Meta_t *>    Meta_v;             // MetaData vector
};

#endif

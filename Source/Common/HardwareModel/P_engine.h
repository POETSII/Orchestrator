#ifndef __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSENGINE_H
#define __ORCHESTRATOR_SOURCE_COMMON_HARDWAREMODEL_POETSENGINE_H

/* Describes the concept of a "POETS Engine" in the model of the POETS hardware
 * stack.
 *
 * The POETS Engine is a self-contained model of the hardware that contains
 * boxes and a graph of boards.
 *
 * See the hardware model documentation for further information POETS
 * engines. */

#include <sstream>

#include "HardwareAddress.h"
#include "dumpchan.h"
#include "NameBase.h"
#include "OrchBase.h"
#include "OwnershipException.h"
#include "P_box.h"
#include "P_board.h"
#include "P_link.h"
#include "P_port.h"
#include "pdigraph.hpp"

#define MAXIMUM_BREAKER_LENGTH 80
#define CALLBACK static void

/* Facilitate out-of-order includes. */
struct OrchBase;
struct P_box;
struct P_board;

class P_engine: public NameBase, protected DumpChan
{
public:
    P_engine(std::string name);

    /* Destruction */
    ~P_engine();
    void clear();

    /* Engines contain information on how addresses should be constructed. */
    HardwareAddressFormat addressFormat;

    /* Engines are not contained by anything, but are have OrchBase as a
     * namebase parent. */
    OrchBase* parent;

    /* Engines contain boxes, mapped by their hardware address component. */
    void contain(AddressComponent addressComponent, P_box* box);
    std::map<AddressComponent, P_box*> P_boxm;
    bool is_empty();

    /* Engines also contain a graph of boards, again mapped by their hardware
     * components. */
    pdigraph<AddressComponent, P_board*,
             unsigned, P_link*,
             unsigned, P_port*> G;
    void contain(AddressComponent addressComponent, P_board* board);
    void connect(AddressComponent start, AddressComponent end, float weight,
                 bool oneWay=false);

    /* Engines may be created from a configuration file. If so, some portion of
     * their metadata will be set. */
    std::string author;
    long datetime;
    std::string version;
    std::string fileOrigin;

    float costExternalBox;
    void dump(FILE* = stdout);

private:
    /* Keys for arcs in the graph, incremented when items are connected. */
    unsigned int arcKey;
};

#endif

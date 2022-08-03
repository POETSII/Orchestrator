#ifndef __ORCHESTRATOR_SOURCE_MOTHERSHIP_INSTRUMENTATIONWRITER_H
#define __ORCHESTRATOR_SOURCE_MOTHERSHIP_INSTRUMENTATIONWRITER_H

/* Describes how instrumentation packets are handled by the Mothership. */

#include <cerrno>
#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#include "dfprintf.h"
#include "InstrumentationException.h"
#include "poets_pkt.h"

#define DEFAULT_INSTRUMENTATION_DIRECTORY \
    dformat("%s/.orchestrator/instrumentation", getenv("HOME"))

struct ThreadInstrumentationDatum
{
    double totalTime;
    uint64_t txCount;
    uint64_t rxCount;
};

class InstrumentationWriter
{
public:
    InstrumentationWriter(std::string directory="");
	~InstrumentationWriter();
    std::map<uint32_t, ThreadInstrumentationDatum> cumulativeData;
    std::string outDirectory;

    bool consume_instrumentation_packet(P_Pkt_t*);

private:
    void setup_directory();
    bool fileFailureTriggered;  /* So that we only warn the first time... */
	
	void open_socket();
	int instrSocket;
	struct addrinfo hints;
	struct addrinfo *res;
	struct addrinfo ServAddrinfo;
	bool instrSocketValid;
};

#endif

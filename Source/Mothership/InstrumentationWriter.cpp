/* Writes instrumentation data, obviously :) */

#include "InstrumentationWriter.h"

InstrumentationWriter::InstrumentationWriter(std::string directory)
{
    fileFailureTriggered = false;
    if (directory.empty()) outDirectory = DEFAULT_INSTRUMENTATION_DIRECTORY;
    else outDirectory = directory;
    setup_directory();
    instrSocketValid = false;
    instrSocket = 0;
    
    //open_socket();        // Calling this here breaks stuff. Ask GMB
}

InstrumentationWriter::~InstrumentationWriter()
{
    // Kill the socket
    if(instrSocketValid)
    {
        close(instrSocket);
        instrSocketValid = false;
        instrSocket = 0;
    }
}

int InstrumentationWriter::open_socket()
{
    struct addrinfo hints;
    struct addrinfo *res;

    instrSocketValid = false;
    
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // Allow IPv4 or IPv6
    hints.ai_socktype = SOCK_DGRAM;     // Datagram socket
    hints.ai_flags = 0;
    hints.ai_protocol = 0;              // Any protocol
    
    // Static for now!
    std::string addr = "127.0.0.1";         //**** CHANGE ME FOR ACTUAL USE ****
    std::string port = "9000";              // I'm just not putting actual hostnames in git...
    
    // Resolve the name/address
    int s = getaddrinfo(addr.c_str(), port.c_str(), &hints, &res);
    
    if (s != 0) {
        //printf("ERROR server not found\n");
        //fflush(stdout);             
        return -1;      // Fix the seg fault?
    }
    
    for (ServAddrinfo = res; ServAddrinfo != PNULL; 
                ServAddrinfo = ServAddrinfo->ai_next) {
        instrSocket = socket(ServAddrinfo->ai_family, ServAddrinfo->ai_socktype,
                                ServAddrinfo->ai_protocol);
        if (instrSocket == -1)        // Oops, try the next one
            continue;

        if (connect(instrSocket, ServAddrinfo->ai_addr,
                    ServAddrinfo->ai_addrlen) != -1) {
            //printf("Socket Connected");
            //fflush(stdout);  
            break;                  /* Success */
        }
        close(instrSocket);
    }
    
    freeaddrinfo(res);           // Clean up res
    
    if (ServAddrinfo == PNULL) {           // Failed to connect to any of the options, barf
        //printf("ERROR could not connect\n");
        //fflush(stdout);
        return -2;
    }
    
    instrSocketValid = true;
}

/* Set up directory structure, in a shamelessly UNIXy way. */
void InstrumentationWriter::setup_directory()
{
    std::vector<std::string> commands;

    /* Remove any existing instrumentation files. Terrible removal. */
    commands.push_back(dformat("rm -rf %s", outDirectory.c_str()));

    /* Create directory */
    commands.push_back(dformat("mkdir --parents %s", outDirectory.c_str()));

    /* Run the commands, warning on failure. */
    for (std::vector<std::string>::iterator commandIt=commands.begin();
         commandIt!=commands.end(); commandIt++)
    {
        if (system(commandIt->c_str()) > 0)
        {
            /* Only warn once, lest we spam. */
            if (fileFailureTriggered) return;
            fileFailureTriggered = true;

            if (errno == 0)
            {
                throw InstrumentationException(dformat(
                    "While initialising instrumentation, the system command "
                    "'%s' ran unsuccessfully. Instrumentation data may not be "
                    "recorded correctly.", commandIt->c_str()));
            }
            else
            {
                throw InstrumentationException(dformat(
                    "While initialising instrumentation, the system command "
                    "'%s' ran unsuccessfully with message '%s'. "
                    "Instrumentation data may not be recorded correctly.",
                    commandIt->c_str(),
                    OSFixes::getSysErrorString(errno).c_str()));
            }
        }
    }
}

/* Take a packet, extract instrumentation data from it, and write the data to
 * an appropriate file. Returns true on success, and false on failure. */
bool InstrumentationWriter::consume_instrumentation_packet(P_Pkt_t* packet)
{
    /* Extract source address for instrumentation packet, so that the compute
     * thread can be identified. The softswitch explicitly puts the hardware
     * address of the sender of instrumentation packets in the pinAddr
     * field. */
    uint32_t source = packet->header.pinAddr;

    /* Deduce a full path for the output file, as a function of the source
     * address. */
    std::string fileTarget = dformat("%s/instrumentation_thread_%u.csv",
                                     outDirectory.c_str(), source);

    /* Attempt to open the file, complaining if we can't. */
    FILE* file = fopen(fileTarget.c_str(), "a");
    if (file == PNULL)
    {
        if (fileFailureTriggered) return false;
        throw InstrumentationException(dformat(
            "Unable to open instrumentation file at '%s'. Error: '%s'.",
            fileTarget.c_str(), OSFixes::getSysErrorString(errno).c_str()));
    }

    /* Extract instrumentation data from the packet. */
    P_Instr_Pkt_Pyld_t* packetDatum = reinterpret_cast<P_Instr_Pkt_Pyld_t*>
        (packet->payload);

    /* Get cumulative datum for this thread, if it exists. If not, seed it with
     * zeroes and write a header for the CSV file. */
    ThreadInstrumentationDatum* cumulativeDatum;
    std::map<uint32_t, ThreadInstrumentationDatum>::iterator dataFinder;
    dataFinder = cumulativeData.find(source);

    /* Existence */
    if (dataFinder != cumulativeData.end())
    {
        cumulativeDatum = &(dataFinder->second);
    }

    /* Non-existence */
    else
    {
        cumulativeDatum = &(cumulativeData[source]);
        cumulativeDatum->totalTime = 0;
        cumulativeDatum->txCount = 0;
        cumulativeDatum->rxCount = 0;

        fprintf(file, "ThreadID, cIDX, Time, cycles, deltaT, RX, OnRX, TX, "
                      "SupTX, OnTX, Idle, OnIdle, Blocked, ");
#if TinselEnablePerfCount == true
        fprintf(file, "CacheMiss, CacheHit, CacheWB, CPUIdle, ");
#endif
        fprintf(file, "RX/s, TX/s, Sup/s\n");

        /* Write a zero-row for GMB's sanity. */
#if TinselEnablePerfCount == true
        unsigned columns = 20;
#else
        unsigned columns = 16;
#endif

        fprintf(file, "%u, ", source);  // First column must be the thread ID
        for (unsigned column=2; column<columns; column++)   // The rest get 0'ed
            fprintf(file, "0, ");
        fprintf(file, "0\n");
    }

    /* NB: I (MLV) am copying this comment from the old Mothership. I don't
     * understand it. GMB will find it in the review and tell me what it's
     * about.
     *
     * TODO: parameterise this - this needs to be tied back to the task. */

    /* Convert cycles into seconds. */
    double deltaT;
    deltaT = static_cast<double>(packetDatum->cycles) / P_INSTR_INTERVAL;

    /* Update cumulative totals with information from this packet. */
    cumulativeDatum->totalTime += deltaT;
    cumulativeDatum->txCount += packetDatum->txCnt;
    cumulativeDatum->rxCount += packetDatum->rxCnt;
    
    char buffer [1024];
    int wb = 0;
    
    /* Finally, write the data to the file. */
    wb = sprintf(buffer, "%u, %u, %f, %u, %f, %u, %u, %u, %u, %u, %u, %u, %u, ",
            source,                      /* Hardware address */
            packetDatum->cIDX,           /* Index of the packet */
            cumulativeDatum->totalTime,  /* Total time */
            packetDatum->cycles,         /* Cycle difference */
            deltaT,                      /* Change in time */
            packetDatum->rxCnt,          /* Number of packets received */
            packetDatum->rxHanCnt,       /* Number of times application
                                          * OnRecieve handler called. */
            packetDatum->txCnt,          /* Number of packets sent */
            packetDatum->supCnt,         /* Number of packets sent to the
                                          * supervisor device */
            packetDatum->txHanCnt,       /* Number of times application OnSend
                                          * handler called */
            packetDatum->idleCnt,        /* Number of times SoftswitchOnIdle
                                          * called */
            packetDatum->idleHanCnt,     /* Number of times application
                                          * OnCompute called */
            packetDatum->blockCnt);      /* Number of times send has been
                                          * blocked */
#if TinselEnablePerfCount == true
    wb += sprintf((buffer + wb), "%u, %u, %u, %u, ",
            packetDatum->missCount,       /* Cache miss count since last
                                           * instrumentation */
            packetDatum->hitCount,        /* Cache hit count since last
                                           * instrumentation */
            packetDatum->writebackCount,  /* Cache writeback count since last
                                           * instrumentation */
            packetDatum->CPUIdleCount);   /* CPU idle count since last
                                           * instrumentation */
#endif
    wb += sprintf((buffer + wb), "%f, %f, %f",
            packetDatum->rxCnt/deltaT,    /* RX per second */
            packetDatum->txCnt/deltaT,    /* TX per second */
            packetDatum->supCnt/deltaT);  /* Sup TX per second */

    fprintf(file, "%s\n", buffer); 
    fclose(file);
    
    //Temporary UDP sender
    if(instrSocketValid)
    {
        // Vestigeal code in case we need better error handling.
        //int error = 0;
        //socklen_t len = sizeof (error);
        //int retval = getsockopt (instrSocket, SOL_SOCKET, SO_ERROR, &error, &len);
        //if (retval != 0) {
        //    /* there was a problem getting the error code */
        //    int errsv = errno;
        //    printf("error getting socket error code: %s\n", strerror(errsv));
        //} else if (error != 0) {
        //    /* socket has a non zero error status */
        //    printf("socket error: %s\n", strerror(error));
        //}
        //fflush(stdout);  
        
        // Punt the data over UDP
        send(instrSocket, buffer, strlen(buffer), 0);
    }
    
    
    return true;
}

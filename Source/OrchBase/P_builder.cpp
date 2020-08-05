//------------------------------------------------------------------------------

#include "OSFixes.hpp"
#include "HardwareModel.h"
#include "MultiSimpleDeployer.h"
#include "P_builder.h"
#include "P_task.h"
#include "P_devtyp.h"
#include "P_device.h"
#include "P_pintyp.h"
#include "P_pin.h"
#include "poets_pkt.h"
#include <sstream>
#include <algorithm>
#include <set>
#include <iostream>

#ifdef __BORLANDC__

//==============================================================================

P_builder::P_builder(int argc, char** argv, OrchBase * _p):par(_p) //, def(NULL), app(argc, argv)
{
//def = new I_Graph(par, &app);
}

//------------------------------------------------------------------------------

P_builder::~P_builder()
{
// if (def != 0) delete def;   // get rid of Qt objects
}

//------------------------------------------------------------------------------

void P_builder::Build(P_task * pT)
// Generates the application binaries - virtually mapped to a single board.
{
if (!pT) pT = par->P_taskm.begin()->second;
par->Post(801,pT->Name(),pT->filename);
//Preplace(pT, par->pVB);
//GenFiles(par->pVB);
//MakeFiles(par->pVB);
//CompileBins(par->pVB);
}

//------------------------------------------------------------------------------

void P_builder::Dump(FILE * fp)
{
fprintf(fp,"P_builder+++++++++++++++++++++++++++++++++++\n");
fprintf(fp,"Parent         %#08p\n",par);
if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());

fprintf(fp,"P_builder-----------------------------------\n");
fflush(fp);
}

//------------------------------------------------------------------------------

void P_builder::Load(const string& name, const string& filename)
{
//def->translate(QString::fromStdString(filename), QString::fromStdString(name), par);
}

//------------------------------------------------------------------------------
#else
//==============================================================================

P_builder::P_builder(int argc, char** argv, OrchBase * _p):par(_p),app(argc, argv),defs()
{

}

//------------------------------------------------------------------------------

P_builder::~P_builder()
{
  for (map<string, I_Graph*>::iterator d = defs.begin(); d != defs.end(); d++) delete d->second;   // get rid of Qt objects
}

//------------------------------------------------------------------------------

//==============================================================================
// Build: called when "task /build" is called.
//==============================================================================
void P_builder::Build(P_task * pT)
// Generates the application binaries - virtually mapped to a single board.
{
  if (!pT) pT = par->P_taskm.begin()->second; // default to the first task
  par->Post(801,pT->Name(),pT->filename);
  Preplace(pT);                               // Map to the system if necessary
  if (GenFiles(pT)) return;    // Try to generate source files. Bail if we fail.
  CompileBins(pT);
}
//==============================================================================

//------------------------------------------------------------------------------

void P_builder::Clear(P_task * pT)
// Clears imported task definitions
{
  if (!pT) // default to all tasks
  {
    for (map<string, I_Graph*>::iterator T = defs.begin(); T != defs.end(); T++) delete T->second;
    defs.clear();
  }
  else if (defs.find(pT->filename) != defs.end())
  {
    delete defs[pT->filename];
    defs.erase(pT->filename);
  }
  else par->Post(814,pT->filename);
}

//------------------------------------------------------------------------------

void P_builder::Dump(FILE * fp)
{
  fprintf(fp,"P_builder+++++++++++++++++++++++++++++++++++\n");
  fprintf(fp,"Parent         %#018lx\n", (uint64_t) par);
  if (par!=0) fprintf(fp,"...%s\n",par->FullName().c_str());

  fprintf(fp,"P_builder-----------------------------------\n");
  fflush(fp);
}

//------------------------------------------------------------------------------

void P_builder::Load(const string& filename)
{
  if (defs.find(filename)!=defs.end()) par->Post(809,filename);                 // already imported file?
  defs[filename] = new I_Graph(QString::fromStdString(filename), par, &app);    // no: then do it here.
  if (defs[filename]->translate(QString::fromStdString(filename), par) != I_Graph::SUCCESS) par->Post(808,filename);
}

//------------------------------------------------------------------------------

void P_builder::Preplace(P_task* task)
{
  // this first naive preplace will sort everything out according to device type.
  if (!task->linked)
  {
    if (par->pE == 0) // no topology?
    {
      // we will set up a virtual topology. Compute how many virtual boxes would
      // be needed.
      unsigned numCores = 0;
      for (vector<P_devtyp*>::iterator dev_typ = task->pP_typdcl->P_devtypv.begin(); dev_typ != task->pP_typdcl->P_devtypv.end(); dev_typ++)
      {
        unsigned int deviceMem = (*dev_typ)->MemPerDevice();
        if (deviceMem > BYTES_PER_THREAD)
        {
          par->Post(810, (*dev_typ)->Name(), int2str(deviceMem), int2str(BYTES_PER_THREAD));
          return;
        }
        // chunk through devices in blocks equal to the thread size
        unsigned int devicesPerThread = min(BYTES_PER_THREAD/deviceMem, MAX_DEVICES_PER_THREAD);
        unsigned int numDevices = task->pD->DevicesOfType(*dev_typ).size();
        numCores += numDevices/(devicesPerThread*THREADS_PER_CORE);
        if (numDevices%(devicesPerThread*THREADS_PER_CORE)) ++numCores;
      }
      unsigned numBoxes = numCores/(CORES_PER_BOARD*BOARDS_PER_BOX); // number of boxes
      if (numCores%(CORES_PER_BOARD*BOARDS_PER_BOX)) ++numBoxes;     // one more if needed
      // Initialise the topology as an N-box Simple system.
      par->pE = new P_engine("VirtualSystem");
      par->pE->parent = par;
      MultiSimpleDeployer deployer(numBoxes);
      par->Post(138,par->pE->Name());
      deployer.deploy(par->pE);
      par->PlacementReset();
    }
    if (!par->pPlacer->place(task, "buck")) task->LinkFlag(); // then preplace on the real or virtual board.
  }
  // if we need to we could aggregate some threads here to achieve maximum packing by merging partially-full threads.
}

//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * Method to generate the required softswitch source files.
 *
 * This method takes all of the handler, type and initialiser cfrags from the
 * datastructure and assembles them (with some boilerplate code) into a coherent
 * set of source files that can be used to build binaries for the cores.
 *
 * Takes a pointer to a task.
 * Returns non-0 (and posts to Logserver) if the creation of any directories or
 * files fails. Essentially a Bool for now but could propagate error codes in
 * future.
 *----------------------------------------------------------------------------*/
unsigned P_builder::GenFiles(P_task* task)
{
  std::string task_dir(par->taskpath+task->Name());

  //TODO: a lot of this system() calling needs to be made more cross-platform, and should probably be moved to OSFixes.hpp.
  //TODO: fix the path specifiers for system() calls throughout as they are currently inconsistent on what has a "/" at the end.
  //TODO: make the system() path specifiers cross-platform - these will fall over on Windows.

  //============================================================================
  // Remove the task directory and recreate it to clean any previous build.
  //============================================================================
  //TODO: make this safer. Currently the remove users an "rm -rf" without any safety.
  if(system((REMOVEDIR+" "+task_dir).c_str())) // Check that the directory deleted
  {                                  // if it didn't, tell logserver and exit
    par->Post(817, task_dir, POETS::getSysErrorString(errno));
    return 1;
  }

  if(system((MAKEDIR+" "+ task_dir).c_str()))// Check that the directory created
  {                                 // if it didn't, tell logserver and exit
    par->Post(818, (task_dir), POETS::getSysErrorString(errno));
    return 1;
  }
  //============================================================================


  task_dir += "/";


  //============================================================================
  // Create the directory for the generated code and the src and inc directories
  // below it. If any of these fail, tell the logserver and bail.
  //============================================================================
  std::string mkdirGenPath(task_dir + GENERATED_PATH);
  if(system((MAKEDIR + " " + mkdirGenPath).c_str()))
  {
    par->Post(818, mkdirGenPath, POETS::getSysErrorString(errno));
    return 1;
  }

  std::string mkdirGenHPath(task_dir + GENERATED_H_PATH);
  if(system((MAKEDIR + " " + mkdirGenHPath).c_str()))
  {
    par->Post(818, mkdirGenHPath, POETS::getSysErrorString(errno));
    return 1;
  }

  std::string mkdirGenCPath(task_dir + GENERATED_CPP_PATH);
  if(system((MAKEDIR + " " + mkdirGenCPath).c_str()))
  {
    par->Post(818, mkdirGenCPath, POETS::getSysErrorString(errno));
    return 1;
  }
  //============================================================================

  unsigned int coreNum = 0;
  if (!task->linked)
  {
    par->Post(811, task->Name());
    return 1;
  }

  //============================================================================
  // Generate Supervisor files.
  //============================================================================
  if(GenSupervisor(task))
  {                     // Generation of Supervisor files failed, bail.
    return 1;           // Don't need to post as that has already been done.
  }
  //============================================================================


  //============================================================================
  // vars_<CORENUM>.h
  // vars_<CORENUM>.cpp
  // handlers_<CORENUM>.h
  // handlers_<CORENUM>.cpp
  //
  // Build a core map visible to the make script (as a shell script). Each of
  // the cores is uniquely identified within a board.
  //============================================================================
  P_core* thisCore;  // Core available during iteration.
  P_thread* firstThread;  // The "first" thread in thisCore. "first" is arbitrary, because cores are stored in a map.
  AddressComponent mailboxCoreId;  // Concatenated mailbox-core address (staging area)

  fstream cores_sh((task_dir+GENERATED_PATH+"/cores.sh").c_str(),
                    fstream::in | fstream::out | fstream::trunc);      // Open the cores shell script


  // Walk through all cores in the system that are used by this task.
  WALKSET(P_core*, par->pPlacer->taskToCores[task], coreNode)
  {
      thisCore = *coreNode;                               // Reference to the current core
      firstThread = thisCore->P_threadm.begin()->second;  // Reference to the first thread on the core

      mailboxCoreId = thisCore->get_hardware_address()->get_mailbox()   \
	      << par->pE->addressFormat.coreWordLength;
      mailboxCoreId += thisCore->get_hardware_address()->get_core();

      cores_sh << "cores[" << coreNum << "]=";
      cores_sh << mailboxCoreId << "\n";
      // these consist of the declarations and definitions of variables and the handler functions.

      //====================================================================
      // Create empty files for the per-core variables declarations
      //====================================================================
      std::stringstream vars_hFName;
      vars_hFName << task_dir << GENERATED_H_PATH;
      vars_hFName << "/vars_" << coreNum << ".h";
      std::ofstream vars_h(vars_hFName.str().c_str());  // variables header


      //====================================================================


      //====================================================================
      // Write core vars.
      //====================================================================
      if (WriteCoreVars(task_dir, coreNum, thisCore, firstThread, vars_h))
      {                     // Writing core vars failed - bail
          vars_h.close();
          cores_sh.close();
          return 1;
      }
      //====================================================================


      //====================================================================
      // Generate thread variables
      //====================================================================
      WALKMAP(AddressComponent,P_thread*,thisCore->P_threadm,threadIterator)
      {
          if (par->pPlacer->threadToDevices[threadIterator->second].size())
          {
              if(WriteThreadVars(task_dir, coreNum, threadIterator->first,
                                 threadIterator->second, vars_h))
              {                     // Writing thread vars failed - bail
                  vars_h.close();
                  cores_sh.close();
                  return 1;
              }
          }
      }
      //====================================================================


      vars_h.close();       // close the core's declarations
      ++coreNum;            // move on to the next core.
  }
  cores_sh.close();

  return 0;
}
//------------------------------------------------------------------------------



/*------------------------------------------------------------------------------
 * Method to generate the required supervisor.cpp and supervisor.h source files.
 *
 * For the default supervisor, this method simply copies supervisor.cpp and
 * supervisor.h.
 *
 * For a non-default supervisor, this method scours the datastructure and
 * appends to the default files.
 *
 * When built, the supervisor is compiled into a .so.
 *
 * Takes a pointer to a task.
 * Returns non-0 (and posts to Logserver) if the file copy failed or the file
 * open failed. Essentially a Bool for now but could propagate error codes in
 * future.
 *----------------------------------------------------------------------------*/
unsigned P_builder::GenSupervisor(P_task* task)
{
  std::string task_dir(par->taskpath+task->Name());

  //============================================================================
  // Copy the default supervisor code into the right place.
  // This is used by the default supervisor without modification.
  //============================================================================
  std::stringstream cpCmd;
  cpCmd << SYS_COPY << " ";
  cpCmd << par->taskpath << STATIC_SRC_PATH << "Supervisor.* "; // Source
  cpCmd << task_dir << "/" << GENERATED_PATH;                   // Destination
  if(system(cpCmd.str().c_str()))
  {
    par->Post(807, (task_dir+GENERATED_PATH), POETS::getSysErrorString(errno));
    return 1;
  }
  //============================================================================


  //============================================================================
  // Build the application-specific supervisor if one is defined
  //============================================================================
  if (task->pSup->pP_devtyp->Name() != "_DEFAULT_SUPERVISOR_") // is there a non-default supervisor? If so, build it.
  {
    //==========================================================================
    // Open the supervisor files. These are appended to rather than truncated.
    // Return 1 if it fails.
    //==========================================================================
    std::stringstream supervisor_hFName;
    supervisor_hFName << task_dir << "/" << GENERATED_PATH <<"/Supervisor.h";
    std::ofstream supervisor_h(supervisor_hFName.str().c_str(),
                               fstream::app);    // Supervisor header, open in append

    if(supervisor_h.fail()) // Check that the file opened
    {                       // if it didn't, tell logserver and exit
      par->Post(816, supervisor_hFName.str(), POETS::getSysErrorString(errno));
      return 1;
    }

    std::stringstream supervisor_cFName;
    supervisor_cFName << task_dir << "/" << GENERATED_PATH <<"/Supervisor.cpp";
    std::ofstream supervisor_cpp(supervisor_cFName.str().c_str(),
                                 fstream::app);  // Supervisor code, open in append
    if(supervisor_cpp.fail()) // Check that the file opened
    {                         // if it didn't, tell logserver, close .h and exit
      par->Post(816, supervisor_cFName.str(), POETS::getSysErrorString(errno));
      supervisor_h.close();
      return 1;
    }

    supervisor_h << "\n";
    supervisor_cpp << "\n";
    //==========================================================================





    P_devtyp* supervisor_type = task->pSup->pP_devtyp;
    set<P_message*> sup_pkts; // set used to retain only unique message types used by supervisors

    std::stringstream sup_inPin_typedefs("");
    std::stringstream sup_pin_handlers("");
    std::stringstream sup_pin_vectors("");
    std::stringstream sup_inPin_props("");
    std::stringstream sup_inPin_state("");

    // build supervisor pin handlers.
    sup_pin_handlers << "vector<supInputPin*> Supervisor::inputs;\n";
    sup_pin_handlers << "vector<supOutputPin*> Supervisor::outputs;\n\n";

    //==========================================================================
    // Assemble output pin variables & handlers.
    //==========================================================================
    WALKVECTOR(P_pintyp*, supervisor_type->P_pintypIv, sI_pin)
    {
      sup_pkts.insert((*sI_pin)->pMsg);

      //========================================================================
      // Add the declaration for the pin's receive handler
      //========================================================================
      string sIpin_name = (*sI_pin)->Name();
      sup_pin_handlers << "unsigned super_InPin_" << sIpin_name;
      sup_pin_handlers << "_Recv_handler (const void* pinProps, ";
      sup_pin_handlers << "void* pinState, const P_Pkt_t* inMsg, ";
      sup_pin_handlers << "PMsg_p* outMsg, void* pktBuf)\n";
      //========================================================================

      sup_pin_handlers << "{\n";
      sup_inPin_props.str("");

      //========================================================================
      // If the pin has properties, write them out
      //========================================================================
      if ((*sI_pin)->pPropsD)
      {
        sup_inPin_typedefs << "typedef ";
        sup_inPin_typedefs << string((*sI_pin)->pPropsD->c_src).erase((*sI_pin)->pPropsD->c_src.length()-2).c_str();
        sup_inPin_typedefs << " super_InPin_" << sIpin_name << "_props_t;\n\n";

        sup_pin_handlers << "   const super_InPin_" << sIpin_name;
        sup_pin_handlers << "_props_t* sEdgeProperties OS_ATTRIBUTE_UNUSED= ";
        sup_pin_handlers << "static_cast<const super_InPin_";
        sup_pin_handlers << sIpin_name << "_props_t*>(pinProps);\n";
        sup_pin_handlers << "OS_PRAGMA_UNUSED(sEdgeProperties)\n";

        sup_inPin_props <<  "new const super_InPin_" << sIpin_name;
        sup_inPin_props << "_props_t " << (*sI_pin)->pPropsI->c_src;
      }
      else sup_inPin_props << "0";
      //========================================================================


      //========================================================================
      // If the pin has state, write it out
      //========================================================================
      sup_inPin_state.str("");
      if ((*sI_pin)->pStateD)
      {
        sup_inPin_typedefs << "typedef ";
        sup_inPin_typedefs << string((*sI_pin)->pStateD->c_src).erase((*sI_pin)->pStateD->c_src.length()-2).c_str();
        sup_inPin_typedefs << " super_InPin_" << sIpin_name << "_state_t;\n\n";

        sup_pin_handlers << "   super_InPin_" << sIpin_name;
        sup_pin_handlers << "_state_t* sEdgeState OS_ATTRIBUTE_UNUSED= ";
        sup_pin_handlers << "static_cast<super_InPin_";
        sup_pin_handlers << sIpin_name << "_state_t*>(pinState);\n";
        sup_pin_handlers<< "OS_PRAGMA_UNUSED(sEdgeState)\n";

        sup_inPin_state << "new super_InPin_" << sIpin_name;
        sup_inPin_state << "_state_t " << (*sI_pin)->pStateI->c_src;
      }
      else sup_inPin_state << "0";
      //========================================================================

      if ((*sI_pin)->pMsg->pPropsD)
      {
        sup_pin_handlers << "   const s_pkt_" << (*sI_pin)->pMsg->Name();
        sup_pin_handlers << "_pyld_t* message = ";
        sup_pin_handlers << "static_cast<const s_pkt_";
        sup_pin_handlers << (*sI_pin)->pMsg->Name() << "_pyld_t*>";
        sup_pin_handlers << "(static_cast<const void*>(inMsg->payload));\n";
      }


      sup_pin_handlers << (*sI_pin)->pHandl->c_src.c_str() << "\n";

      // return no error by default if the handler bottoms out without problems
      sup_pin_handlers << "   return 0;\n";
      sup_pin_handlers << "}\n\n";


      //========================================================================
      // Add the pin's teardown code - this deletes the properties and state if
      // they exist. Called by the destructor for supInputPin in Supervisor.cpp
      //========================================================================
      sup_pin_handlers << "unsigned super_InPin_" << sIpin_name;
      sup_pin_handlers << "_PinTeardown (const void* pinProps, ";
      sup_pin_handlers << "void* pinState)\n";
      sup_pin_handlers << "{\n";

      if ((*sI_pin)->pPropsD)
      {
        sup_pin_handlers << "    delete static_cast<const super_InPin_";
        sup_pin_handlers << sIpin_name << "_props_t*>(pinProps);\n";
      }

      if ((*sI_pin)->pStateD)
      {
        sup_pin_handlers << "    delete static_cast<super_InPin_";
        sup_pin_handlers << sIpin_name << "_state_t*>(pinState);\n";
      }

      sup_pin_handlers << "    return 0;\n";
      sup_pin_handlers << "}\n\n";
      //========================================================================


      // this will create a new pin object - how should this be deleted on exit since it's held in a static class member?
      sup_pin_vectors << "Supervisor::inputs.push_back";
      sup_pin_vectors << "(new supInputPin(";
      sup_pin_vectors << "&super_InPin_" << sIpin_name << "_Recv_handler,";
      sup_pin_vectors << "&super_InPin_" << sIpin_name << "_PinTeardown,";
      sup_pin_vectors << sup_inPin_props.str() << ",";
      sup_pin_vectors << sup_inPin_state.str();
      sup_pin_vectors << "));\n";
    }
    sup_pin_handlers << "\n";
    //==========================================================================


    //==========================================================================
    // Assemble output pin variables & handlers.
    //==========================================================================
    WALKVECTOR(P_pintyp*, supervisor_type->P_pintypOv, sO_pin)
    {
        sup_pkts.insert((*sO_pin)->pMsg);
        string sOpin_name = (*sO_pin)->Name();
        sup_pin_handlers << "unsigned super_OutPin_" << sOpin_name;
        sup_pin_handlers << "_Send_handler";
        sup_pin_handlers << "(PMsg_p* outMsg, void* pktBuf, unsigned superMsg)";
        sup_pin_handlers << "\n{\n";
        sup_pin_handlers << "   int s_c = 0;\n";
        sup_pin_handlers << "   /*\n";  //TEMPORARY FUDGE: until we resolve supervisor outputs.
        sup_pin_handlers << "   P_Pkt_t* s_pkt;\n";
        sup_pin_handlers << "   P_Pkt_Hdr_t s_pkt_hdr;\n";
        sup_pin_handlers << "   if (!(s_pkt = outMsg->Get<P_Pkt_t>(0, s_c))) return -1;\n";
        sup_pin_handlers << "   s_pkt_hdr = s_pkt->header;\n";
        if ((*sO_pin)->pMsg->pPropsD) // message has some sort of payload
        {
           /* rather awkward code for payload extraction. a PMsg, unfortunately, has an interface that expects you
            * to test any extracted data item before trying to dereference it. On the other hand, when you create
            * a new data item, it copies it into the message from a presumed existing data item. Well, what we want is
            * to create the data item directly in the PMsg, since we are massaging the data in some way from an existing
            * buffer, and a second buffer-heave-across is undesirable. You also can't create a new heap item directly in
            * the Put PMsg function (which inserts a data item) because if you do, given that PMsg itself copies the data
            * across, the newly-created item can't be deleted. So you have to go through the strange sequence below of
            * creating a new data item, inserting it into the PMsg, deleting the allocation, then referring to the contained
            * object in the PMsg if everything is to be well-behaved.
           */
           sup_pin_handlers << "   s_pkt_" << (*sO_pin)->pMsg->Name();
           sup_pin_handlers << "_pyld_t* outPyld;\n";

           sup_pin_handlers << "   if (superMsg) outPyld = static_cast<s_pkt_";
           sup_pin_handlers << (*sO_pin)->pMsg->Name();
           sup_pin_handlers << "_pyld_t*>(static_cast<void*>(s_pkt->data);\n";

           sup_pin_handlers << "   else\n";
           sup_pin_handlers << "   {\n";
           sup_pin_handlers << "      P_Pkt_t* pkt = new P_Pkt_t();\n";
           sup_pin_handlers << "      outMsg->Put<P_Pkt_t>(0, pkt, 1);\n";
           sup_pin_handlers << "      delete pkt;\n";
           sup_pin_handlers << "      if (!(pkt = outMsg->Get<P_Pkt_t>(0, s_c))) return -1;\n";

           sup_pin_handlers << "      outPyld = static_cast<s_pkt_";
           sup_pin_handlers << (*sO_pin)->pMsg->Name();
           sup_pin_handlers << "_pyld_t*>(static_cast<void*>(pkt->data));\n";
           sup_pin_handlers << "   }\n";
        }
        sup_pin_handlers << "   */\n";  //TEMPORARY FUDGE: until we resolve supervisor outputs.

        sup_pin_handlers << (*sO_pin)->pHandl->c_src.c_str() << "\n";

        sup_pin_handlers << "   /*\n";  //TEMPORARY FUDGE: until we resolve supervisor outputs.

        // last part sets up to send the messages (which is automatically handled upon exit from the SupervisorCall).
        sup_pin_handlers << "   if (!superMsg)\n";
        sup_pin_handlers << "   {\n";
        sup_pin_handlers << "   P_Pkt_t* sendMsg;\n";
        sup_pin_handlers << "   if (!(sendMsg = outMsg->Get<P_Pkt_t>(0, s_c)))";
        sup_pin_handlers << " return -1;\n";

        sup_pin_handlers << "   P_Pkt_Hdr_t* outHdr = &sendMsg->header;\n";
        sup_pin_handlers << "   outHdr->destDeviceAddr =";
        sup_pin_handlers << " s_pkt_hdr.sourceDeviceAddr;\n";
        sup_pin_handlers << "   outHdr->messageLenBytes = p_hdr_size();\n";

        if ((*sO_pin)->pMsg->pPropsD)
        {
          sup_pin_handlers << "   outHdr->messageLenBytes += sizeof(s_pkt_";
          sup_pin_handlers << (*sO_pin)->pMsg->Name() << "_pyld_t);\n";
        }

        sup_pin_handlers << "   }\n";
        // return number of messages to send if no error.

        sup_pin_handlers << "   */\n";  //TEMPORARY FUDGE: until we resolve supervisor outputs.

        sup_pin_handlers << "   return s_c;\n";
        sup_pin_handlers << "}\n\n";

        sup_pin_vectors << "Supervisor::outputs.push_back(new ";
        sup_pin_vectors << "supOutputPin(&super_OutPin_";
        sup_pin_vectors << sOpin_name << "_Send_handler));\n";
    }
    //==========================================================================


    //==========================================================================
    // Write everything to the supervisor source files & close them.
    //==========================================================================
    supervisor_h << "#define _APPLICATION_SUPERVISOR_ 1\n\n";

    // supervisor message types. Note that for all objects with properties and state, both these values are optional so we need to check for their existence.
    WALKSET(P_message*, sup_pkts, s_pkt)
    {
      if ((*s_pkt)->pPropsD)
      {
        supervisor_h << "typedef ";
        supervisor_h << string((*s_pkt)->pPropsD->c_src).erase((*s_pkt)->pPropsD->c_src.length()-2).c_str();
        supervisor_h << " s_pkt_" <<(*s_pkt)->Name() << "_pyld_t;\n\n";
      }
    }

    supervisor_h << sup_inPin_typedefs.str(); // put types into h

    WALKVECTOR(CFrag*,supervisor_type->pHandlv,sCode)
    {
        supervisor_cpp << (*sCode)->c_src << "\n";      // add generic code fragments to cpp (they might have function declarations, type declarations, etc.)
    }
    supervisor_cpp << "\n" << sup_pin_handlers.str() << "\n"; // Put handlers into cpp

    // have to build the static vectors inside a loadable function
    supervisor_cpp << "extern \"C\"" << "{\n";
    supervisor_cpp << "int SupervisorInit()\n" << "{\n";
    supervisor_cpp << sup_pin_vectors.str();
    supervisor_cpp << "return 0;\n";
    supervisor_cpp << "}\n" << "}\n\n";

    //supervisor_cpp << "#ifdef _APPLICATION_SUPERVISOR_\n\n"; //Superfluous as  we WILL have an app supervisor at this point
    //supervisor_cpp << "#endif";           //Superfluous as  we WILL have an app supervisor at this point

    supervisor_cpp.close();
    supervisor_h.close();
  }
  //============================================================================

  return 0;
}
//------------------------------------------------------------------------------


/*------------------------------------------------------------------------------
 * Generate the variables and initialisers for a single core.
 *
 * Conveniently each core currently hosts a single device type, making looking
 * up its vars is easy. This needs to be refactored when this restriction is
 * removed.
 *----------------------------------------------------------------------------*/
unsigned P_builder::WriteCoreVars(std::string& task_dir, unsigned coreNum,
                                  P_core* thisCore, P_thread* firstThread,
                                  ofstream& vars_h)
{
  P_device* firstDevice = par->pPlacer->threadToDevices[firstThread].front();
  P_devtyp* c_devtyp = firstDevice->pP_devtyp;  // Pointer to the core's device type
  std::string devtyp_name = c_devtyp->Name();   // grab a local copy of the devtype name


  //============================================================================
  // Create empty files for the per-core variables and handlers
  //============================================================================
  std::stringstream vars_cppFName;
  vars_cppFName << task_dir << GENERATED_CPP_PATH;
  vars_cppFName << "/vars_" << coreNum << ".cpp";
  std::ofstream vars_cpp(vars_cppFName.str().c_str());  // variables source
  if(vars_cpp.fail()) // Check that the file opened
  {                       // if it didn't, tell logserver and exit
    par->Post(816, vars_cppFName.str(), POETS::getSysErrorString(errno));
    return 1;
  }

  std::stringstream handlers_hFName;
  handlers_hFName << task_dir << GENERATED_H_PATH;
  handlers_hFName << "/handlers_" << coreNum << ".h";
  std::ofstream handlers_h(handlers_hFName.str().c_str());  // handlers header
  if(handlers_h.fail()) // Check that the file opened
  {                       // if it didn't, tell logserver and exit
    par->Post(816, handlers_hFName.str(), POETS::getSysErrorString(errno));
    vars_cpp.close();
    return 1;
  }

  std::stringstream handlers_cppFName;
  handlers_cppFName << task_dir << GENERATED_CPP_PATH;
  handlers_cppFName << "/handlers_" << coreNum << ".cpp";
  std::ofstream handlers_cpp(handlers_cppFName.str().c_str());  // handlers source
  if(handlers_cpp.fail()) // Check that the file opened
  {                       // if it didn't, tell logserver and exit
    par->Post(816, handlers_cppFName.str(), POETS::getSysErrorString(errno));
    vars_cpp.close();
    handlers_h.close();
    return 1;
  }
  //============================================================================


  //============================================================================
  // Write the common includes to each of the relevant places
  //============================================================================
  vars_h << "#include <cstdint>\n";
  vars_h << "#include \"softswitch_common.h\"\n\n";

  handlers_cpp << "#include \"vars_" << coreNum << ".h\"\n";        // This goes in the CPP because the h has no include guards. TODO: fix this.

  handlers_cpp << "#include \"handlers_" << coreNum << ".h\"\n\n";

  vars_cpp << "#include \"vars_" << coreNum << ".h\"\n";
  //============================================================================



  // assemble a typedef for each of the fragments. If the enclosing class doesn't provide a struct {} enclosure for data objects with more
  //  than one member this can be easily inserted here as well. We copy the string from the definition to avoid inavertent in-place modification.



  //============================================================================
  // If we have a global properties definition, write it and the initialiser out.
  //============================================================================
  if (c_devtyp->par->pPropsD)                                   // pPropsD = the Declaration
  {
    vars_h << "typedef ";
    vars_h << std::string(c_devtyp->par->pPropsD->c_src).erase(c_devtyp->par->pPropsD->c_src.length()-2);
    vars_h << " global_props_t;\n\n";
    vars_h << "extern const global_props_t GraphProperties;\n";

    std::string global_init("");
    if (firstDevice->par->pPropsI)        // Graph Instance properties
    {
      global_init = firstDevice->par->pPropsI->c_src;
    }
    else //if (c_devtyp->par->pPropsI)                          // Default graph type properties
    {
      global_init = c_devtyp->par->pPropsI->c_src;
    }

    vars_cpp << "const global_props_t GraphProperties ";
    vars_cpp << "OS_ATTRIBUTE_UNUSED= ";
    vars_cpp << global_init << ";\n";
    vars_cpp << "OS_PRAGMA_UNUSED(GraphProperties)\n";
  }
  //============================================================================


  //============================================================================
  // Write the shared graph-level shared code as it may be used in handlers.
  //============================================================================
  for (vector<CFrag*>::iterator g_code = c_devtyp->par->General.begin();
        g_code != c_devtyp->par->General.end();
        g_code++)
  {
    handlers_cpp << (*g_code)->c_src << "\n"; // newline assumes general code fragments are unrelated and don't expect back-to-back concatenation
  }
  //============================================================================


  //============================================================================
  // Write the message typedefs..
  //============================================================================
  for (vector<P_message*>::iterator pkt = c_devtyp->par->P_messagev.begin();
        pkt != c_devtyp->par->P_messagev.end();
        pkt++)
  {
    if ((*pkt)->pPropsD)
    {
      vars_h << "typedef ";
      vars_h << string((*pkt)->pPropsD->c_src).erase((*pkt)->pPropsD->c_src.length()-2).c_str();
      vars_h << " pkt_" <<(*pkt)->Name() << "_pyld_t;\n";
    }
  }
  vars_h << "\n";
  //============================================================================


  //============================================================================
  // Write general-purpose handlers. Do we/can we actually have any of these?!?!
  //============================================================================
  for (vector<CFrag*>::iterator d_code = c_devtyp->pHandlv.begin();
        d_code != c_devtyp->pHandlv.end();
        d_code++)
  {
    handlers_cpp << (*d_code)->c_src << "\n";
  }
  //============================================================================


  //============================================================================
  // Form the common handler preamble (deviceProperties & deviceState)
  //============================================================================
  stringstream handlerPreamble("");
  stringstream handlerPreambleS("");        // "normal" state
  stringstream handlerPreambleCS("");       // Const-protected state
  handlerPreamble << "{\n";

  if (c_devtyp->par->pPropsD)
  {
    handlerPreamble << "   const global_props_t* graphProperties ";
    handlerPreamble << "OS_ATTRIBUTE_UNUSED= ";
    handlerPreamble << "static_cast<const global_props_t*>";
    handlerPreamble << "(graphProps);\n";
    handlerPreamble << "   OS_PRAGMA_UNUSED(graphProperties)\n";
  }
  handlerPreamble << "   PDeviceInstance* deviceInstance ";
  handlerPreamble << "OS_ATTRIBUTE_UNUSED= ";
  handlerPreamble << "static_cast<PDeviceInstance*>(device);\n";
  handlerPreamble << "   OS_PRAGMA_UNUSED(deviceInstance)\n";

  // deviceProperties (with unused variable handling)
  if (c_devtyp->pPropsD)
  {
    vars_h << "typedef ";
    vars_h << string(c_devtyp->pPropsD->c_src).erase(c_devtyp->pPropsD->c_src.length()-2).c_str();
    vars_h << " devtyp_" << devtyp_name << "_props_t;\n\n";

    handlerPreamble << "   const devtyp_" << devtyp_name;
    handlerPreamble << "_props_t* deviceProperties ";
    handlerPreamble << "OS_ATTRIBUTE_UNUSED= ";
    handlerPreamble << "static_cast<const devtyp_";
    handlerPreamble << devtyp_name;
    handlerPreamble << "_props_t*>(deviceInstance->properties);\n";
    handlerPreamble << "   OS_PRAGMA_UNUSED(deviceProperties)\n";
  }

  // deviceState (with unused variable handling)
  if (c_devtyp->pStateD)
  {
    vars_h << "typedef ";
    vars_h << string(c_devtyp->pStateD->c_src).erase(c_devtyp->pStateD->c_src.length()-2).c_str();
    vars_h << " devtyp_" << devtyp_name << "_state_t;\n\n";

    // Const-protected state
    handlerPreambleCS << "   const devtyp_" << devtyp_name;
    handlerPreambleCS << "_state_t* deviceState ";
    handlerPreambleCS << "OS_ATTRIBUTE_UNUSED= ";
    handlerPreambleCS << "static_cast<devtyp_";
    handlerPreambleCS << devtyp_name;
    handlerPreambleCS << "_state_t*>(deviceInstance->state);\n";
    handlerPreambleCS << "   OS_PRAGMA_UNUSED(deviceState)\n";

    // "normal" state
    handlerPreambleS << "   devtyp_" << devtyp_name;
    handlerPreambleS << "_state_t* deviceState ";
    handlerPreambleS << "OS_ATTRIBUTE_UNUSED= ";
    handlerPreambleS << "static_cast<devtyp_";
    handlerPreambleS << devtyp_name;
    handlerPreambleS << "_state_t*>(deviceInstance->state);\n";
    handlerPreambleS << "   OS_PRAGMA_UNUSED(deviceState)\n";
  }
  handlers_cpp << "\n";
  //============================================================================


  //============================================================================
  // Write the device-level handlers
  //============================================================================
  // ReadyToSend
  handlers_h << "uint32_t devtyp_" << devtyp_name;
  handlers_h << "_RTS_handler (const void* graphProps, ";
  handlers_h << "void* device, uint32_t* readyToSend);\n";

  handlers_cpp << "uint32_t devtyp_" << devtyp_name;
  handlers_cpp << "_RTS_handler (const void* graphProps, ";
  handlers_cpp << "void* device, uint32_t* readyToSend)\n";
  handlers_cpp << handlerPreamble.str();
  handlers_cpp << handlerPreambleCS.str();
  handlers_cpp << c_devtyp->pOnRTS->c_src << "\n";
  handlers_cpp << "   return *readyToSend;\n"; // we assume here the return value is intended to be an RTS bitmap.
  handlers_cpp << "}\n\n";

  // OnIdle
  handlers_h << "uint32_t devtyp_" << devtyp_name;
  handlers_h << "_OnIdle_handler (const void* graphProps, ";
  handlers_h << "void* device);\n";

  handlers_cpp << "uint32_t devtyp_" << devtyp_name;
  handlers_cpp << "_OnIdle_handler (const void* graphProps, ";
  handlers_cpp << "void* device)\n";
  handlers_cpp << handlerPreamble.str();
  handlers_cpp << handlerPreambleS.str() << "\n";

  if (c_devtyp->pOnIdle) // insert the OnIdle handler if there is one
  {
    handlers_cpp << c_devtyp->pOnIdle->c_src << "\n";
    handlers_cpp << "    return 1;\n";  // Default return 1
  }
  else handlers_cpp << "   return 0;\n"; // or a stub if not
  handlers_cpp << "}\n\n";

  // OnCtl - stub until this can be resolved with DBT
  handlers_h << "uint32_t devtyp_" << devtyp_name;
  handlers_h << "_OnCtl_handler (const void* graphProps, ";
  handlers_h << "void* device, const uint8_t opcode, const void* pkt);\n\n";

  handlers_cpp << "uint32_t devtyp_" << devtyp_name;
  handlers_cpp << "_OnCtl_handler (const void* graphProps, ";
  handlers_cpp << "void* device, const uint8_t opcode, const void* pkt)";
  handlers_cpp << " {return 0;}\n\n";
  //============================================================================


  //============================================================================
  // Write input pin variables and handlers for each pin
  //============================================================================
  for (vector<P_pintyp*>::iterator I_pin = c_devtyp->P_pintypIv.begin();
        I_pin != c_devtyp->P_pintypIv.end();
        I_pin++)
  {
    std::string Ipin_name = (*I_pin)->Name();

    handlers_h << "uint32_t devtyp_" << devtyp_name;
    handlers_h << "_InPin_" << Ipin_name;
    handlers_h << "_Recv_handler (const void* graphProps, ";
    handlers_h << "void* device, void* edge, const void* pkt);\n";

    handlers_cpp << "uint32_t devtyp_" << devtyp_name;
    handlers_cpp << "_InPin_" << Ipin_name;
    handlers_cpp << "_Recv_handler (const void* graphProps, ";
    handlers_cpp << "void* device, void* edge, const void* pkt)\n";
    handlers_cpp << handlerPreamble.str();
    handlers_cpp << handlerPreambleS.str();
    handlers_cpp << "   inEdge_t* edgeInstance ";
    handlers_cpp << "OS_ATTRIBUTE_UNUSED= ";
    handlers_cpp << "static_cast<inEdge_t*>(edge);\n";
    handlers_cpp << "OS_PRAGMA_UNUSED(edgeInstance)\n";

    if ((*I_pin)->pPropsD)
    {
      vars_h << "typedef ";
      vars_h << string((*I_pin)->pPropsD->c_src).erase((*I_pin)->pPropsD->c_src.length()-2).c_str();
      vars_h << " devtyp_" << devtyp_name;
      vars_h << "_InPin_" << Ipin_name << "_props_t;\n\n";

      handlers_cpp << "   const devtyp_" << devtyp_name;
      handlers_cpp << "_InPin_" << Ipin_name;
      handlers_cpp << "_props_t* edgeProperties ";
      handlers_cpp << "OS_ATTRIBUTE_UNUSED= ";
      handlers_cpp << "static_cast<const devtyp_" << devtyp_name;
      handlers_cpp << "_InPin_" << Ipin_name;
      handlers_cpp << "_props_t*>(edgeInstance->properties);\n";
      handlers_cpp << "OS_PRAGMA_UNUSED(edgeProperties)\n";
    }

    if ((*I_pin)->pStateD)
    {
      vars_h << "typedef ";
      vars_h << string((*I_pin)->pStateD->c_src).erase((*I_pin)->pStateD->c_src.length()-2).c_str();
      vars_h << " devtyp_" << devtyp_name;
      vars_h << "_InPin_" << Ipin_name << "_state_t;\n\n";

      handlers_cpp << "   devtyp_" << devtyp_name;
      handlers_cpp << "_InPin_" << Ipin_name;
      handlers_cpp << "_state_t* edgeState ";
      handlers_cpp << "OS_ATTRIBUTE_UNUSED= ";
      handlers_cpp << "static_cast<devtyp_" << devtyp_name;
      handlers_cpp << "_InPin_"<< Ipin_name;
      handlers_cpp << "_state_t*>(edgeInstance->state);\n";
      handlers_cpp << "OS_PRAGMA_UNUSED(edgeState)\n";
    }

    if ((*I_pin)->pMsg->pPropsD)
    {
      handlers_cpp << "   const pkt_" << (*I_pin)->pMsg->Name();
      handlers_cpp << "_pyld_t* message = ";
      handlers_cpp << "static_cast<const pkt_" << (*I_pin)->pMsg->Name();
      handlers_cpp << "_pyld_t*>(pkt);\n";
    }

    handlers_cpp << (*I_pin)->pHandl->c_src << "\n";

    // return type is indicated as uint32_t yet in the handlers we see from DBT
    // no return value is set. Is something expected here?
    handlers_cpp << "   return 0;\n";
    handlers_cpp << "}\n\n";
  }
  vars_h << "\n";
  handlers_h << "\n";
  //============================================================================


  //============================================================================
  // Write output pin handlers for each pin
  //============================================================================
  for (vector<P_pintyp*>::iterator O_pin = c_devtyp->P_pintypOv.begin();
        O_pin != c_devtyp->P_pintypOv.end();
        O_pin++)
  {
    std::string Opin_name = (*O_pin)->Name();

    handlers_h << "const uint32_t RTS_INDEX_" << (*O_pin)->Name();
    handlers_h << " = " << (*O_pin)->idx << ";\n";

    handlers_h << "const uint32_t RTS_FLAG_" << (*O_pin)->Name();
    handlers_h << " = 0x1 << " << (*O_pin)->idx << ";\n";

    handlers_h << "uint32_t devtyp_" << devtyp_name;
    handlers_h << "_OutPin_" << Opin_name;
    handlers_h << "_Send_handler (const void* graphProps, ";
    handlers_h << "void* device, void* pkt);\n";

    handlers_cpp << "uint32_t devtyp_" << devtyp_name;
    handlers_cpp << "_OutPin_" << Opin_name;
    handlers_cpp << "_Send_handler (const void* graphProps, ";
    handlers_cpp << "void* device, void* pkt)\n";

    handlers_cpp << handlerPreamble.str();
    handlers_cpp << handlerPreambleS.str();

    if ((*O_pin)->pMsg->pPropsD)
    {
      handlers_cpp << "   pkt_" << (*O_pin)->pMsg->Name();
      handlers_cpp << "_pyld_t* message = static_cast<pkt_";
      handlers_cpp <<  (*O_pin)->pMsg->Name() << "_pyld_t*>(pkt);\n";
    }
    handlers_cpp << (*O_pin)->pHandl->c_src << "\n";

    // same thing: what is this function expected to return?
    handlers_cpp << "   return 0;\n";
    handlers_cpp << "}\n\n";
  }
  //============================================================================


  //============================================================================
  // Close the core's var and handler files
  //============================================================================
  vars_cpp.close();
  handlers_h.close();
  handlers_cpp.close();
  //============================================================================

  return 0;
}
//------------------------------------------------------------------------------



//------------------------------------------------------------------------------
// Generate the thread variables and initialisers for a single thread.
//------------------------------------------------------------------------------
unsigned P_builder::WriteThreadVars(string& task_dir, unsigned coreNum,
                                    unsigned thread_num, P_thread* thread,
                                    ofstream& vars_h)
{
  // a trivial bit more overhead, perhaps, then passing this as an argument.
  // The *general* method would extract the number of device types from the
  // thread's device list.
  P_device* device = *par->pPlacer->threadToDevices.at(thread).begin();
  P_devtyp* devTyp = device->pP_devtyp;

  std::list<P_device*>::size_type numberOfDevices =
      par->pPlacer->threadToDevices.at(thread).size();

  unsigned int inTypCnt = devTyp->P_pintypIv.size();       // Grab the number of input pins for the device type
  unsigned int outTypCnt = devTyp->P_pintypOv.size();      // Grab the number of output pins for the device type

  vector<unsigned int>inPinArcs;    //Vector used for finding how many connections a pin has. TODO: fix the type
  vector<unsigned int>outPinArcs;   //Vector used for finding how many connections a pin has. TODO: fix the type

  // we could choose to create separate .h files for each thread giving the externs but it seems simpler
  // and arguably more flexible to put all the external declarations in a single .h
  // The actual data definitions go one file per thread so we can load the resultant data files individually.


  //============================================================================
  // Create the vars.cpp for the thread
  //============================================================================
  std::stringstream vars_cppFName;
  vars_cppFName << task_dir << GENERATED_CPP_PATH;
  vars_cppFName << "/vars_" << coreNum << "_" <<thread_num << ".cpp";
  std::ofstream vars_cpp(vars_cppFName.str().c_str());  // variables source
  if(vars_cpp.fail()) // Check that the file opened
  {                       // if it didn't, tell logserver and exit
    par->Post(816, vars_cppFName.str(), POETS::getSysErrorString(errno));
    return 1;
  }
  //============================================================================


  //============================================================================
  // Build the input pin map. Need counts to put in the device type table.
  //============================================================================
  set<P_message*> dev_in_pkt_types;
  set<P_message*> dev_out_pkt_types;
  vector<P_pintyp*>::iterator pin;
  for (pin = devTyp->P_pintypIv.begin(); pin != devTyp->P_pintypIv.end(); pin++)
  {
    dev_in_pkt_types.insert((*pin)->pMsg);
  }
  for (pin = devTyp->P_pintypOv.begin(); pin != devTyp->P_pintypOv.end(); pin++)
  {
    dev_out_pkt_types.insert((*pin)->pMsg);
  }
  //============================================================================


  //============================================================================
  // write thread preamble to vars.h and vars.cpp
  //============================================================================
  vars_h << "\n";
  vars_h << "//-------------------- ";
  vars_h << "Core " << coreNum << " Thread " << thread_num << " variables";
  vars_h << " --------------------\n";
  vars_h << "extern ThreadCtxt_t Thread_" << thread_num << "_Context;\n";

  vars_cpp << "#include \"vars_" << coreNum << ".h\"\n";
  vars_cpp << "#include \"handlers_" << coreNum << ".h\"\n\n";
  vars_cpp << "//-------------------- Core " << coreNum;
  vars_cpp << " Thread " << thread_num << " variables --------------------\n";
  //============================================================================


  //============================================================================
  // Generate the ThreadContext initialiser. PThreadContext/ThreadCtxt_t struct
  //============================================================================
  vars_cpp << "ThreadCtxt_t Thread_" << thread_num << "_Context ";              // Set the ThreadContext name
  vars_cpp << "__attribute__ ((section (\".thr" << thread_num << "_base\"))) "; // Set the target memory area
  vars_cpp << "= {";
  vars_cpp << "1,";                                                 // numDevTyps
  vars_cpp << "Thread_" << thread_num << "_DeviceTypes,";           // devTyps
  vars_cpp << numberOfDevices <<  ",";                              // numDevInsts
  vars_cpp << "Thread_" << thread_num << "_Devices,";               // devInsts
  vars_cpp << ((devTyp->par->pPropsD)?"&GraphProperties,":"PNULL,");// properties



  /* Work out the required size for rtsBuffSize: The size of the RTS buffer is
   * dependant on the number of connected output pins hosted on the Softswitch.
   * The size is set to 1 + <number of connected pins>, as long as this is less
   * than MAX_RTSBUFFSIZE, so that each connected pin can have a pending send.
   *
   * The additional slot is required to ensure that the crude, simple wrapping
   * mechanism for the circular buffer does not set rtsEnd to be the same as
   * rtsStart when adding to the buffer. If this occurs, softswitch_IsRTSReady()
   * will always return false (as it simply checks that rtsStart != rtsEnd) and
   * no further application-generated packets will be sent by the softswitch (as
   * softswitch_onRTS will only alter rtsEnd if it adds an entry to the buffer,
   * which it wont do in this case as all pins will already be marked as send
   * pending).
   *
   * If the buffer size is constrained to MAX_RTSBUFFSIZE, a warning is
   * generated - if this occurs frequently, more graceful handling of rtsBuf
   * overflowing may be required.
   */
  uint32_t outputCount = 1;     // Yes, this is intentionally 1 to cope with wrapping.
  for (list<P_device*>::iterator device =
         par->pPlacer->threadToDevices.at(thread).begin();
       device != par->pPlacer->threadToDevices.at(thread).end(); device++)
  { // Iterate through all devices counting pins.
    if (outTypCnt)
    {
      for (vector<P_pintyp*>::iterator pin = devTyp->P_pintypOv.begin();
            pin != devTyp->P_pintypOv.end(); pin++)
      {
        // Check that we have connections.
        if((*device)->par->G.FindArcs((*device)->idx,
            (*pin)->idx,inPinArcs,outPinArcs))
        {
          outputCount++;    // If we do, add an rtsBuffSlot for the pin.
        }
      }
    }
  }
  if (outputCount > MAX_RTSBUFFSIZE)
  { // If we have too many pins for one buffer entry per ping, set to max &warn.
    // This may need a check adding to the Softswitch to stop buffer overflow.
    outputCount = MAX_RTSBUFFSIZE;
    par->Post(819,int2str(thread_num),int2str(coreNum),int2str(MAX_RTSBUFFSIZE),
              int2str(MAX_RTSBUFFSIZE));
  }
  else if (outputCount < MIN_RTSBUFFSIZE)
  {
    outputCount = MIN_RTSBUFFSIZE;
  }
  vars_cpp << outputCount << ",";                                   // rtsBuffSize

  vars_cpp << "PNULL,";                                             // rtsBuf
  vars_cpp << "0,";                                                 // rtsStart
  vars_cpp << "0,";                                                 // rtsEnd
  vars_cpp << "0,";                                                 // idleStart
  vars_cpp << "0,";                                                 // ctlEnd

  // Instrumentation
  vars_cpp << "0,";                                 // lastCycles
  vars_cpp << "0,";                                 // pendCycles
  vars_cpp << "0,";                                 // txCount
  vars_cpp << "0,";                                 // superCount
  vars_cpp << "0,";                                 // rxCount
  vars_cpp << "0,";                                 // txHandlerCount
  vars_cpp << "0,";                                 // rxHandlerCount
  vars_cpp << "0,";                                 // idleCount
  vars_cpp << "0,";                                 // idleHandlerCount
  vars_cpp << "0,";                                 // blockCount
  vars_cpp << "0";                                  // cycleIdx
  vars_cpp << "};\n";
  //============================================================================


  // more partial streams because pin lists may or may not exist for a given device.
  std::stringstream inpinlist("");
  std::stringstream outpinlist("");
  std::stringstream initialiser("");




  //============================================================================
  // Generate the device type declaration and initialiser. PDeviceType/devTyp_t struct
  //============================================================================
  /* Currently, there is one device-type per thread. It is intended that this
   * restriction is removed in future.
   */
  vars_h << "//------------------------------ Device Type Tables ";
  vars_h << "------------------------------\n";
  vars_h << "extern struct PDeviceType Thread_" << thread_num;
  vars_h << "_DeviceTypes[1];\n";

  vars_cpp << "devTyp_t Thread_" << thread_num << "_DeviceTypes[1] ";           //set devTyp_t name
  vars_cpp << "= {";
  vars_cpp << "&devtyp_" << devTyp->Name() << "_RTS_handler,";                  // RTS_Handler
  vars_cpp << "&devtyp_" << devTyp->Name() << "_OnIdle_handler,";               // OnIdle_Handler
  vars_cpp << "&devtyp_" << devTyp->Name() << "_OnCtl_handler,";                // OnCtl_Handler
  if (devTyp->pPropsD)
    vars_cpp << "sizeof(devtyp_" << devTyp->Name() << "_props_t),";             // sz_props
  else vars_cpp << "0,";
  if (devTyp->pStateD)
    vars_cpp << "sizeof(devtyp_" << devTyp->Name() << "_state_t),";             // sz_state
  else vars_cpp << "0,";

  vars_cpp << inTypCnt << ",";                                                  // numInputTypes
  if (inTypCnt) vars_cpp << "Thread_" << thread_num << "_DevTyp_0_InputPins,";  // inputTypes
  else vars_cpp << "PNULL,";

  vars_cpp << outTypCnt << ",";                                                 // numOutputTypes
  if (outTypCnt) vars_cpp << "Thread_" << thread_num << "_DevTyp_0_OutputPins"; // outputTypes
  else vars_cpp << "PNULL,";

  vars_cpp << "};\n";
  //============================================================================


  //============================================================================
  // Form the initialiser(s) for the input pins array if we have input pins.
  // PInputType/in_pintyp_t struct
  //============================================================================
  vars_h << "//------------------------------ Pin Type Tables ";
  vars_h << "-------------------------------\n";

  if (inTypCnt)
  {
    vars_h << "extern in_pintyp_t Thread_" << thread_num;       // Add declaration for the input pins array to relevant vars header
    vars_h << "_DevTyp_0_InputPins[" << inTypCnt << "];\n";

    std::string dTypInPin = std::string("devtyp_");                             // Build the dev type input pin name string
    dTypInPin += devTyp->Name() + std::string("_InPin_");

    initialiser.str("");                                                       // Clear the initialiser
    initialiser << "{";
    for (vector<P_pintyp*>::iterator ipin = devTyp->P_pintypIv.begin();         // Iterate over all the pins
          ipin != devTyp->P_pintypIv.end();                                     //  and build the initialiser
          ipin++)                                                               //  for each one.
    {
      initialiser << "{";
      initialiser << "&" << dTypInPin << (*ipin)->Name() << "_Recv_handler,";   // Recv_handler

      if ((*ipin)->pMsg->pPropsD)
        initialiser << "sizeof(pkt_" << (*ipin)->pMsg->Name() << "_pyld_t),";   // sz_pkt
      else initialiser << "0,";

      initialiser << (*ipin)->pMsg->MsgType << ",";                             // pktType

      if ((*ipin)->pPropsD)                                                     // sz_props
       initialiser << "sizeof(" << dTypInPin << (*ipin)->Name() << "_props_t),";
      else initialiser << "0,";

      if ((*ipin)->pStateD)                                                     // sz_state
       initialiser << "sizeof(" << dTypInPin << (*ipin)->Name() << "_state_t)";
      else initialiser << "0";
      initialiser << "},";
    }
    initialiser.seekp(-1,ios_base::cur);    // Rewind one place to remove the stray ","
    initialiser << "}";                     // properly terminate the initialiser

    // Add the initialiser to vars cpp.
    vars_cpp << "in_pintyp_t Thread_" << thread_num << "_DevTyp_0_InputPins";
    vars_cpp << "[" << inTypCnt << "] = " << initialiser.str() << ";\n";
  }
  //============================================================================


  //============================================================================
  // Form the initialiser(s) for the output pins array if we have output pins.
  // POutputType/in_pouttyp_t struct
  //============================================================================
  if (outTypCnt)
  {
    vars_h << "extern out_pintyp_t Thread_" << thread_num;      // Add declaration for the output pins array to relevant vars header
    vars_h << "_DevTyp_0_OutputPins[" << outTypCnt << "];\n";

    std::string dTypInPin = std::string("devtyp_");                             // Build the dev type input pin name string
    dTypInPin += devTyp->Name() + std::string("_OutPin_");

    initialiser.str("");                                                       // Clear the initialiser
    initialiser << "{";
    for (vector<P_pintyp*>::iterator opin = devTyp->P_pintypOv.begin();         // Iterate over all the pins
          opin != devTyp->P_pintypOv.end();                                     //  and build the initialiser
          opin++)                                                               //  for each one.
    {
      initialiser << "{";
      initialiser << "&" << dTypInPin << (*opin)->Name() << "_Send_handler,";   // Send_Handler
      if ((*opin)->pMsg->pPropsD)
        initialiser << "sizeof(pkt_" << (*opin)->pMsg->Name() << "_pyld_t),";   // sz_pkt
      else initialiser << "0,";
      initialiser << (*opin)->pMsg->MsgType << "},";                            // pktType
    }
    initialiser.seekp(-1,ios_base::cur);   // Rewind one place to remove the stray ","
    initialiser << "}";                    // properly terminate the initialiser

    // Add the initialiser to vars cpp.
    vars_cpp << "out_pintyp_t Thread_" << thread_num << "_DevTyp_0_OutputPins";
    vars_cpp << "[" << outTypCnt << "] = " << initialiser.str() << ";\n";
  }
  //============================================================================

  vars_cpp << "\n"; // Add a little bit of padding


  //============================================================================
  // Add the device instances declaration to vars h
  //============================================================================
  vars_h << "//--------------------------- Device Instance Tables ";
  vars_h << "---------------------------\n";
  vars_h << "extern devInst_t Thread_" << thread_num;
  vars_h << "_Devices[" << numberOfDevices << "];\n\n";
  //============================================================================



  unsigned index = 0;

  std::stringstream devInstInitialiser("");   // Device Instance Initialisers
  std::stringstream pinInitialiser("");       // Initialiser for input & output pins
  std::stringstream edgeInitialiser("");      // Initialiser for input & output edges

  std::stringstream devPropsInitialiser("");  // device type properties (devtyp_XXX_props_t) initialiser
  std::stringstream devStateInitialiser("");  // device type state (devtyp_XXX_state_t) initialiser

  std::stringstream inPinPropsInitialiser("");// device type input pin properties (devtyp_XXX_InPin_YYY_props_t) initialiser
  std::stringstream inPinStateInitialiser("");// device type input pin properties (devtyp_XXX_InPin_YYY_state_t) initialiser


  devPropsInitialiser << "{";   // Add the first { to the device properties array initialiser
  devStateInitialiser << "{";   // Add the first { to the device state array initialiser
  devInstInitialiser << "{";    // Add the first { to the device instance array initialiser


  // Iterate through all of the devices
  for (list<P_device*>::iterator device =
         par->pPlacer->threadToDevices.at(thread).begin();
       device != par->pPlacer->threadToDevices.at(thread).end(); device++)
  {
    //==========================================================================
    // Form the first part of the PDeviceInstance/devInst_t initialiser
    //==========================================================================
    // device index should be replaced by a GetHardwareAddress(...) call.
    devInstInitialiser << "{";
    devInstInitialiser << "&Thread_" << thread_num << "_Context,";          // thread
    devInstInitialiser << "&Thread_" << thread_num << "_DeviceTypes[0],";   // devType
    devInstInitialiser << (*device)->addr.A_device << ",";                  // deviceID
    //==========================================================================

    // need to descend into the pins and set them up before setting up the device
    if (inTypCnt)
    {
      std::string thrDevNameIn = "Thread_";
      thrDevNameIn += TO_STRING(thread_num) + std::string("_Device_");
      thrDevNameIn += (*device)->Name() + std::string("_InputPins");

      vars_h << "//----------------------- Input Pin (Associative) Tables ";
      vars_h << "-----------------------\n";
      vars_h << "extern inPin_t " << thrDevNameIn << "[" << inTypCnt << "];\n";


      //========================================================================
      // Form some more of the PDeviceInstance/devInst_t initialiser
      //========================================================================
      devInstInitialiser << inTypCnt << ",";                                // numInputs
      devInstInitialiser << thrDevNameIn << ",";                            // inputPins
      //========================================================================


      pinInitialiser.str("");   // Reset for the initialiser for PInputPin/inPin_t.
      pinInitialiser << "{";

      for (unsigned int pin_num = 0; pin_num < inTypCnt; pin_num++)   // Iterate through all of the input pins
      {
        // within the pin, build the internal edge information (the pin's source list)

        edgeInitialiser.str("");        // Reset the edgeInitialiser for this pin - array of PInputEdge/inEdge_t
        edgeInitialiser << "{";

        inPinPropsInitialiser.str("");  // Reset the input pin properties initialiser for this pin - array of devtyp_XXX_InPin_YYY_props_t
        inPinPropsInitialiser << "{";

        inPinStateInitialiser.str("");  // Reset the input pin state initialiser for this pin - array of devtyp_XXX_InPin_YYY_state_t
        inPinStateInitialiser << "{";

        pdigraph<unsigned int, P_device*, unsigned int, P_message*, unsigned int, P_pin*>::TPp_it next_pin = (*device)->par->G.index_n[(*device)->idx].fani.upper_bound(pin_num << PIN_POS | 0xFFFFFFFF >> (32-PIN_POS));
        // ADR 10 December 2019: A hack here to get the names of edge properties and state arrays the same in initialiser: the
        // variable definition in a section below creates a name dependent upon a condition in next pin which we won't have
        // here unless we decrement the iterator, but we want to retain an identical iterator for the generation of
        // the definition itself. This awkward approach simply creates an identical iterator and uses it as is
        pdigraph<unsigned int, P_device*, unsigned int, P_message*, unsigned int, P_pin*>::TPp_it next_init_pin = next_pin;
        bool pinHasConnections = (next_pin != (*device)->par->G.index_n[(*device)->idx].fani.lower_bound(pin_num << PIN_POS)); // pin has connections?
        if (pinHasConnections) --next_pin;
        for (pdigraph<unsigned int, P_device*, unsigned int, P_message*, unsigned int, P_pin*>::TPp_it p_edge = (*device)->par->G.index_n[(*device)->idx].fani.lower_bound(pin_num << PIN_POS); p_edge != next_init_pin; p_edge++)
        {
          //====================================================================
          // Form the first bit of an initialiser for PInputEdge/inEdge_t array member
          //====================================================================
          edgeInitialiser << "{";

          // first field is intentionally null as it is populated at runtime.
          edgeInitialiser << "PNULL,";                                          // pin
          edgeInitialiser << (*device)->idx << ",";                             // tgt
          edgeInitialiser << p_edge->second.iArc->second.fr_n->first << ",";    // src
          //====================================================================

          // if the pin has properties,
          if (pinHasConnections && p_edge->second.data->pP_pintyp->pPropsD)
          {     // set them up in the edge list
            //==================================================================
            // Form some more of the PInputEdge/inEdge_t initialiser
            //==================================================================

            edgeInitialiser << "&Thread_" << thread_num;
            edgeInitialiser << "_Device_" << (*device)->Name();
            edgeInitialiser << "_Pin_" << next_pin->second.data->pP_pintyp->Name();
            edgeInitialiser << "_InEdgeProps[";
            edgeInitialiser << (p_edge->first & (0xFFFFFFFF >> (32-PIN_POS)));
            edgeInitialiser << "],";                                            // properties
            //==================================================================


            //==================================================================
            // Write the initialiser for the input pin properties. Either the
            // overloaded properties, or the defaults from typedef. devtyp_XXX_InPin_YYY_props_t
            //==================================================================
            if (p_edge->second.data->pPropsI)   // using the set properties if available
            {
              inPinPropsInitialiser << p_edge->second.data->pPropsI->c_src;
              inPinPropsInitialiser << ",";
            }
            else    // or defaults if not.
            {
              inPinPropsInitialiser << p_edge->second.data->pP_pintyp->pPropsI->c_src;
              inPinPropsInitialiser << ",";
            }
            //==================================================================
          }
          else
          {
            //==================================================================
            // Form some more of the PInputEdge/inEdge_t initialiser with defaults
            //==================================================================
            edgeInitialiser << "PNULL,";                                        // properties
            //==================================================================
          }

          // if the pin has state,
          if (pinHasConnections && p_edge->second.data->pP_pintyp->pStateD)
          {
            //==================================================================
            // Form the last bit of this PInputEdge/inEdge_t initialiser
            //==================================================================
            edgeInitialiser << "&Thread_" << thread_num;
            edgeInitialiser << "_Device_" << (*device)->Name();
            edgeInitialiser << "_Pin_" << next_pin->second.data->pP_pintyp->Name();
            edgeInitialiser << "_InEdgeStates[";
            edgeInitialiser << (p_edge->first & (0xFFFFFFFF >> (32-PIN_POS)));
            edgeInitialiser << "]";                                             // state
            edgeInitialiser << "},";
            //==================================================================


            //==================================================================
            // Write the initialiser for the input pin state. Either the
            // overloaded state, or the default of 0. devtyp_XXX_InPin_YYY_props_t
            //==================================================================
            if (p_edge->second.data->pStateI)
            {
              inPinStateInitialiser << p_edge->second.data->pStateI->c_src << ",";
            }
            else
            {
              inPinStateInitialiser << p_edge->second.data->pP_pintyp->pStateI->c_src;
              inPinStateInitialiser << ",";
            }
            //==================================================================

          }
          else
          {
            //==================================================================
            // Form the last bit of this PInputEdge/inEdge_t initialiser with defaults
            //==================================================================
            edgeInitialiser << "PNULL";                                         // state
            edgeInitialiser << "},";
            //==================================================================
          }
        }


        if (pinHasConnections)
        {
          // create input edge data structures


          //====================================================================
          // Add the input edges array declaration to vars h
          //====================================================================
          vars_h << "extern inEdge_t ";
          vars_h << "Thread_" << thread_num << "_Device_";
          vars_h << (*device)->Name() << "_Pin_";
          vars_h << next_pin->second.data->pP_pintyp->Name();
          vars_h << "_InEdges[";
          vars_h << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1;
          vars_h << "];\n";
          //====================================================================


          edgeInitialiser.seekp(-1, ios_base::cur);   // Rewind one place to remove the stray ","
          edgeInitialiser << "};\n";                  // properly terminate the initialiser

          //====================================================================
          // Write out the Input Edges Array Initialiser (edgeInitialiser)
          //====================================================================
          vars_cpp << "inEdge_t Thread_" << thread_num;
          vars_cpp << "_Device_" << (*device)->Name();
          vars_cpp << "_Pin_" << next_pin->second.data->pP_pintyp->Name();
          vars_cpp << "_InEdges[";
          vars_cpp << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1;
          vars_cpp << "] = ";
          vars_cpp << edgeInitialiser.str();
          //====================================================================



          // with associated properties if they exist
          if (next_pin->second.data->pP_pintyp->pPropsD)
          {
            //==================================================================
            // Add the input pin properties array declaration to vars h
            //==================================================================
            vars_h << "extern devtyp_" << devTyp->Name();
            vars_h << "_InPin_" << next_pin->second.data->pP_pintyp->Name();
            vars_h << "_props_t Thread_" << thread_num;
            vars_h << "_Device_" << (*device)->Name();
            vars_h << "_Pin_" << next_pin->second.data->pP_pintyp->Name();
            vars_h << "_InEdgeProps[";
            vars_h << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1;
            vars_h << "];\n";
            //==================================================================


            inPinPropsInitialiser.seekp(-1, ios_base::cur);     // Rewind one place to remove the stray ","
            inPinPropsInitialiser << "};\n";                    // properly terminate the initialiser

            //==================================================================
            // Write the input pin properties array initialiser
            //==================================================================
            vars_cpp << "devtyp_" << devTyp->Name();
            vars_cpp << "_InPin_" << next_pin->second.data->pP_pintyp->Name();
            vars_cpp << "_props_t Thread_" << thread_num;
            vars_cpp << "_Device_" << (*device)->Name();
            vars_cpp << "_Pin_" << next_pin->second.data->pP_pintyp->Name();
            vars_cpp << "_InEdgeProps[";
            vars_cpp << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1;
            vars_cpp << "] = ";
            vars_cpp << inPinPropsInitialiser.str();
            //==================================================================
          }


          // and associated state (again, if it exists)
          if (next_pin->second.data->pP_pintyp->pStateD)
          {
            vars_h << "extern devtyp_" << devTyp->Name();
            vars_h << "_InPin_" << next_pin->second.data->pP_pintyp->Name();
            vars_h << "_state_t  Thread_" << thread_num;
            vars_h << "_Device_" << (*device)->Name();
            vars_h << "_Pin_" << next_pin->second.data->pP_pintyp->Name();
            vars_h << "_InEdgeStates[";
            vars_h << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1;
            vars_h << "];\n";


            inPinStateInitialiser.seekp(-1, ios_base::cur);
            inPinStateInitialiser << "};\n";


            vars_cpp << "devtyp_" << devTyp->Name();
            vars_cpp << "_InPin_" << next_pin->second.data->pP_pintyp->Name();
            vars_cpp << "_state_t Thread_" << thread_num;
            vars_cpp << "_Device_" << (*device)->Name();
            vars_cpp << "_Pin_" << next_pin->second.data->pP_pintyp->Name();
            vars_cpp << "_InEdgeStates[";
            vars_cpp << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1;
            vars_cpp << "] = ";
            vars_cpp << inPinStateInitialiser.str();
          }

          //====================================================================
          // Build the Initialiser for the PInputPin/inPin_t array member.
          //====================================================================
          pinInitialiser << "{";

          pinInitialiser << "PNULL,";                                           // device

          pinInitialiser << "&Thread_" << thread_num;
          pinInitialiser << "_DevTyp_0_InputPins[" << pin_num << "],";          // pinType
          pinInitialiser << (next_pin->first&(0xFFFFFFFF>>(32-PIN_POS)))+1<<",";// numSrcs

          pinInitialiser << "Thread_" << thread_num;
          pinInitialiser << "_Device_" << (*device)->Name();
          pinInitialiser << "_Pin_" << next_pin->second.data->pP_pintyp->Name();
          pinInitialiser << "_InEdges";                                         // sources

          pinInitialiser << "},";
          //====================================================================
        }
        else
        {
          //====================================================================
          // Build the (default) Initialiser for the PInputPin/inPin_t array member.
          //====================================================================
          pinInitialiser << "{";
          pinInitialiser << "PNULL,";                                           // device

          pinInitialiser << "&Thread_" << thread_num << "_DevTyp_0_InputPins";
          pinInitialiser << "[" << pin_num << "],";                             // pinType

          pinInitialiser << "0,";                                               // numSrcs
          pinInitialiser << "PNULL";                                            // sources
          pinInitialiser << "},";
          //====================================================================
        }
      }


      pinInitialiser.seekp(-1, ios_base::cur);  // Rewind one place to remove the stray ","
      pinInitialiser << "};\n";                 // properly terminate the initialiser

      //========================================================================
      // Write the initialiser for the input pin array.
      //========================================================================
      vars_cpp << "inPin_t Thread_" << thread_num;
      vars_cpp << "_Device_" << (*device)->Name();
      vars_cpp << "_InputPins[";
      vars_cpp << inTypCnt;
      vars_cpp << "] = ";
      vars_cpp << pinInitialiser.str();
      //========================================================================
    }
    else
    {
      //========================================================================
      // Form some more of the PDeviceInstance/devInst_t initialiser with defaults
      //========================================================================
      devInstInitialiser << "0,";                                           // numInputs
      devInstInitialiser << "PNULL,";                                       // inputPins
      //========================================================================
    }

    if (outTypCnt)
    {
      //========================================================================
      // Form some more of the PDeviceInstance/devInst_t initialiser
      //========================================================================
      devInstInitialiser << outTypCnt << ",";                               // numOutputs
      devInstInitialiser << "Thread_" << thread_num << "_Device_";
      devInstInitialiser << (*device)->Name() << "_OutputPins,";            // outputPins
      //========================================================================

      vars_h << "//------------------------------ Output Pin Tables ";
      vars_h << "-----------------------------\n";
      vars_h << "extern outPin_t Thread_" << thread_num;
      vars_h << "_Device_" << (*device)->Name();
      vars_h << "_OutputPins[" << outTypCnt << "];\n";


      pinInitialiser.str("");   // Reset for the initialiser for POutputPin/outPin_t.
      pinInitialiser << "{";

      for (vector<P_pintyp*>::iterator pin = devTyp->P_pintypOv.begin();
            pin != devTyp->P_pintypOv.end();
            pin++)
      {
        //======================================================================
        // Build part of the Initialiser for the POutputPin/outPin_t array member.
        //======================================================================
        pinInitialiser << "{";
        pinInitialiser << "PNULL,";                                         // device

        pinInitialiser << "&Thread_" << thread_num;
        pinInitialiser << "_DevTyp_0_OutputPins[" << (*pin)->idx << "],";   // pinType
        //======================================================================


        // Find out how many connections this pin has
        (*device)->par->G.FindArcs((*device)->idx,(*pin)->idx,inPinArcs,outPinArcs);

        if (outPinArcs.size() == 0)
        {   // If the pin has no connections
          //====================================================================
          // Finish the Initialiser for the POutputPin/outPin_t array member with defaults.
          //====================================================================
          pinInitialiser << "0,";                                           // numTgts
          pinInitialiser << "PNULL,";                                       // targets
          pinInitialiser << "0,";                                           // idxTgts
          pinInitialiser << "0";                                            // sendPending
          pinInitialiser << "},";
          //====================================================================
        }
        else
        {   // Otherwise add the targets.
          //====================================================================
          // Finish the Initialiser for the POutputPin/outPin_t array member.
          //====================================================================
          pinInitialiser << outPinArcs.size() << ",";                       // numTgts

          pinInitialiser << "Thread_" << thread_num;
          pinInitialiser << "_Device_" << (*device)->Name();
          pinInitialiser << "_OutPin_" << (*pin)->Name() << "_Tgts,";       // targets

          pinInitialiser << "0,";                                           // idxTgts
          pinInitialiser << "0";                                            // sendPending
          pinInitialiser << "},";
          //====================================================================


          //====================================================================
          // Add the declaration for the outedge array to the vars header
          //====================================================================
          vars_h << "extern outEdge_t Thread_" << thread_num;
          vars_h << "_Device_" << (*device)->Name();
          vars_h << "_OutPin_" << (*pin)->Name();
          vars_h << "_Tgts[" << outPinArcs.size() << "];\n";
          //====================================================================


          //====================================================================
          // Generate the targets (HW addresses) for each edge.
          // TODO: re-work this with the correct HW address class.
          //====================================================================
          edgeInitialiser.str("");      // Reset the edge initialiser for POutputEdge/outEdge_t
          edgeInitialiser << "{";

          for (vector<unsigned int>::iterator tgt = outPinArcs.begin();
                tgt != outPinArcs.end();
                tgt++)
          {
            // as for the case of inputs, the target device should be replaced by GetHardwareAddress(...)
            unsigned int tgt_idx = ((*device)->par->G.index_a.find(*tgt)->second.to_p)->first;
            P_addr tgt_addr = ((*device)->par->G.index_a.find(*tgt)->second.to_n)->second.data->addr;


            //------------------------------------------------------------------
            // Assemble the Hardware address. TODO: use the HW Model
            //------------------------------------------------------------------
            uint32_t tgt_hwaddr = tgt_addr.A_box << (TinselLogCoresPerMailbox
                                                    + TinselLogMailboxesPerBoard
                                                    + TinselMeshXBits
                                                    + TinselMeshYBits);
            tgt_hwaddr |= tgt_addr.A_board << (TinselLogThreadsPerCore
                                              + TinselLogCoresPerMailbox
                                              + TinselLogMailboxesPerBoard);
            tgt_hwaddr |= tgt_addr.A_mailbox << (TinselLogThreadsPerCore
                                                + TinselLogCoresPerMailbox);
            tgt_hwaddr |= tgt_addr.A_core << (TinselLogThreadsPerCore);
            tgt_hwaddr |= tgt_addr.A_thread;
            //------------------------------------------------------------------


            //------------------------------------------------------------------
            // Assemble the Software address. TODO: use the SW Address class
            //------------------------------------------------------------------
            uint32_t tgt_swaddr;

            if(((tgt_hwaddr << LOG_DEVICES_PER_THREAD) | tgt_addr.A_device)
                  == DEST_BROADCAST) // Quick and dirty hack to detect Super pkt
            {
                tgt_swaddr = 0;
                tgt_swaddr |= P_SW_MOTHERSHIP_MASK;
                tgt_swaddr |= P_SW_CNC_MASK;
            }
            else
            {
                tgt_swaddr = ((tgt_addr.A_device << P_SW_DEVICE_SHIFT)
                            & P_SW_DEVICE_MASK);
            }
            //------------------------------------------------------------------


            //------------------------------------------------------------------
            // Assemble the Pin Address. TODO: rationalise this.
            //------------------------------------------------------------------
            uint32_t tgt_pinaddr;

            tgt_pinaddr = (tgt_idx >> PIN_POS) & P_HD_TGTPIN_MASK; // Get the Pin Index
            tgt_pinaddr |= (((tgt_idx & (0xFFFFFFFF >> (32-PIN_POS)))
                                << P_HD_DESTEDGEINDEX_SHIFT)
                                & P_HD_DESTEDGEINDEX_MASK);

            //------------------------------------------------------------------

            //==================================================================
            // Form an initialiser for the POutputEdge/outEdge_t array member.
            //==================================================================
            edgeInitialiser << "{";

            // first field is intentionally null as it is populated at runtime.
            edgeInitialiser << "PNULL,";                                  // pin

            edgeInitialiser << tgt_hwaddr << ",";   // hwAddr
            edgeInitialiser << tgt_swaddr << ",";   // swAddr
            edgeInitialiser << tgt_pinaddr;         // pinAddr
            edgeInitialiser << "},";
            //==================================================================
          }

          edgeInitialiser.seekp(-1,ios_base::cur);    // Rewind one place to remove the stray ","
          edgeInitialiser << "};\n";                  // properly terminate the initialiser


          //====================================================================
          // Write the initialiser for the output edge array.
          //====================================================================
          vars_cpp << "outEdge_t Thread_" << thread_num;
          vars_cpp << "_Device_" << (*device)->Name();
          vars_cpp << "_OutPin_" << (*pin)->Name();
          vars_cpp << "_Tgts[" << outPinArcs.size() << "] = ";
          vars_cpp << edgeInitialiser.str();
          //====================================================================

          //====================================================================
        }
      }

      pinInitialiser.seekp(-1,ios_base::cur);   // Rewind one place to remove the stray ","
      pinInitialiser << "};\n";                 // properly terminate the initialiser


      //========================================================================
      // Write the initialiser for the output pin array.
      //========================================================================
      vars_cpp << "outPin_t Thread_" << thread_num;
      vars_cpp << "_Device_" << (*device)->Name();
      vars_cpp << "_OutputPins[" << outTypCnt << "] = ";
      vars_cpp << pinInitialiser.str();
      //========================================================================
    }
    else
    {
      //========================================================================
      // Form some more of the PDeviceInstance/devInst_t initialiser with defaults
      //========================================================================
      devInstInitialiser << "0,";                                           // numOutputs
      devInstInitialiser << "PNULL,";                                       // outputPins
      //========================================================================
    }



    if (devTyp->pPropsD)
    {
      //========================================================================
      // Form some more of the PDeviceInstance/devInst_t initialiser
      //========================================================================
      devInstInitialiser << "&Thread_" << thread_num;
      devInstInitialiser << "_DeviceProperties[" << index << "],";          // properties
      //========================================================================


      //========================================================================
      // Write the initialiser for the device properties. This is either the
      // device's overloaded properties, or the defaults from typedef. devtyp_XXX_props_t
      //========================================================================
      if ((*device)->pPropsI)
      {
        devPropsInitialiser << (*device)->pPropsI->c_src << ",";
      }
      else
      {
        devPropsInitialiser << devTyp->pPropsI->c_src << ",";
      }
      //========================================================================
    }
    else
    {
      //========================================================================
      // Form some more of the PDeviceInstance/devInst_t initialiser with defaults
      //========================================================================
      devInstInitialiser << "PNULL,";                                       // properties
      //========================================================================
    }

    if (devTyp->pStateD)
    {
      //========================================================================
      // Form some more of the PDeviceInstance/devInst_t initialiser
      //========================================================================
      devInstInitialiser << "&Thread_" << thread_num;
      devInstInitialiser << "_DeviceState[" << index << "],";               // state
      //========================================================================


      //========================================================================
      // Write the initialiser for the device state. This is either the device's
      // overloaded state, or the defaults from typedef. devtyp_XXX_state_t
      //========================================================================
      if ((*device)->pStateI)
      {
        devStateInitialiser << (*device)->pStateI->c_src << ",";
      }
      else
      {
        devStateInitialiser << devTyp->pStateI->c_src <<",";
      }
      //========================================================================
    }
    else
    {
      //========================================================================
      // Form some more of the PDeviceInstance/devInst_t initialiser with defaults
      //========================================================================
      devInstInitialiser << "PNULL,";                                       // state
      //========================================================================
    }


    //==========================================================================
    // Form the last bit of this PDeviceInstance/devInst_t initialiser with defaults
    //==========================================================================
    devInstInitialiser << "},";
    //==========================================================================

    ++index;
  }

  devInstInitialiser.seekp(-1,ios_base::cur);   // Rewind one place to remove the stray ","
  devInstInitialiser << "};\n";                 // properly terminate the initialiser

  //============================================================================
  // Write the device instance array initialisers
  //============================================================================
  vars_cpp << "devInst_t Thread_" << thread_num;
  vars_cpp << "_Devices[" << numberOfDevices << "] = ";
  vars_cpp << devInstInitialiser.str();
  //============================================================================


  if (devTyp->pPropsD)      // If the device type has properties
  {
    //==========================================================================
    // Write the device type properties declaration
    //==========================================================================
    vars_h << "extern devtyp_" << devTyp->Name();
    vars_h << "_props_t Thread_" << thread_num;
    vars_h << "_DeviceProperties[" << numberOfDevices << "];\n";
    //==========================================================================


    devPropsInitialiser.seekp(-1,ios_base::cur);    // Rewind one place to remove the stray ","
    devPropsInitialiser << "};\n";                  // properly terminate the initialiser

    //==========================================================================
    // Write the device type properties initialiser
    //==========================================================================
    vars_cpp << "devtyp_" << devTyp->Name();
    vars_cpp << "_props_t Thread_" << thread_num;
    vars_cpp << "_DeviceProperties[" << numberOfDevices << "] = ";
    vars_cpp << devPropsInitialiser.str();
    //==========================================================================
  }

  if (devTyp->pStateD)      // If the device type has state
  {
    //==========================================================================
    // Write the device type state declaration
    //==========================================================================
    vars_h << "extern devtyp_" << devTyp->Name();
    vars_h << "_state_t Thread_" << thread_num;
    vars_h << "_DeviceState[" << numberOfDevices << "];\n";
    //==========================================================================


    devStateInitialiser.seekp(-1,ios_base::cur);    // Rewind one place to remove the stray ","
    devStateInitialiser << "};\n";                  // properly terminate the initialiser

    //==========================================================================
    // Write the device type state initialiser
    //==========================================================================
    vars_cpp << "devtyp_" << devTyp->Name();
    vars_cpp << "_state_t Thread_" << thread_num;
    vars_cpp << "_DeviceState[" << numberOfDevices << "] = ";
    vars_cpp << devStateInitialiser.str();
    //==========================================================================
  }

  vars_cpp.close();
  return 0;
}
//------------------------------------------------------------------------------



/*------------------------------------------------------------------------------
 * Compile the binaries for loading onto POETS hardware
 *----------------------------------------------------------------------------*/
unsigned P_builder::CompileBins(P_task * task)
{
  string task_dir(par->taskpath+task->Name()+"/");
  if (!task->linked)
  {
     par->Post(811, task->Name());
     return 1;
  }

  //============================================================================
  // Create all of the necessary build directories - bail if any fail.
  //============================================================================
  std::string mkdirCommonPath(task_dir + COMMON_PATH);
  if(system((MAKEDIR + " " + mkdirCommonPath).c_str()))
  {
    par->Post(818, mkdirCommonPath, POETS::getSysErrorString(errno));
    return 1;
  }

  std::string mkdirTinselPath(task_dir + TINSEL_PATH);      // Tinsel path
  if(system((MAKEDIR + " " + mkdirTinselPath).c_str()))
  {
    par->Post(818, mkdirTinselPath, POETS::getSysErrorString(errno));
    return 1;
  }

  std::string mkdirOrchPath(task_dir + ORCH_PATH);          // Orchestrator path
  if(system((MAKEDIR + " " + mkdirOrchPath).c_str()))
  {
    par->Post(818, mkdirOrchPath, POETS::getSysErrorString(errno));
    return 1;
  }

  std::string mkdirBinPath(task_dir + BIN_PATH);           // Binary path
  if(system((MAKEDIR + " " + mkdirBinPath).c_str()))
  {
    par->Post(818, mkdirBinPath, POETS::getSysErrorString(errno));
    return 1;
  }

  std::string mkdirBuildPath(task_dir + BUILD_PATH);       // Build path
  if(system((MAKEDIR + " " + mkdirBuildPath).c_str()))
  {
    par->Post(818, mkdirBuildPath, POETS::getSysErrorString(errno));
    return 1;
  }
  //============================================================================


  //============================================================================
  // Copy static files to the correct places - bail if any fail
  //============================================================================
  std::string cpCmd(SYS_COPY+" "+RECURSIVE_CPY+" "+PERMISSION_CPY); // Common copy command
  std::string cpSrc;        // Source
  std::string cpDest;       // Destination

  cpSrc = par->taskpath+COMMON_SRC_PATH+"/*";       //  softswitch sources?
  cpDest = task_dir + COMMON_PATH;
  if(system((cpCmd + " " + cpSrc + " " + cpDest).c_str()))
  {
    par->Post(807, cpDest, POETS::getSysErrorString(errno));
    return 1;
  }

  cpSrc = par->taskpath+TINSEL_SRC_PATH+"/*";       // Tinsel code
  cpDest = task_dir + TINSEL_PATH;
  if(system((cpCmd + " " + cpSrc + " " + cpDest).c_str()))
  {
    par->Post(807, cpDest, POETS::getSysErrorString(errno));
    return 1;
  }

  cpSrc = par->taskpath+ORCH_SRC_PATH+"/*";         // Orchestrator code
  cpDest = task_dir + ORCH_PATH;
  if(system((cpCmd + " " + cpSrc + " " + cpDest).c_str()))
  {
    par->Post(807, cpDest, POETS::getSysErrorString(errno));
    return 1;
  }

  cpSrc = par->taskpath+STATIC_SRC_PATH+"Makefile"; // Makefile
  cpDest = task_dir + BUILD_PATH;                 // last one as we reuse cpDest
  if(system((SYS_COPY + " " + cpSrc + " " + cpDest).c_str()))
  {
    par->Post(807, cpDest, POETS::getSysErrorString(errno));
    return 1;
  }
  //============================================================================


  //============================================================================
  // CD to build dir and build the actual binaries - this calls make.
  //============================================================================
  if(system(("(cd "+cpDest+";"+COREMAKE_BASE+")").c_str()))
  {
    par->Post(805);
    return 1;
  }
  //============================================================================


  //============================================================================
  // Check that the backend-compute binaries were made, and link them to each core.
  //============================================================================
  unsigned int coreNum = 0;  // Just a counter.

  // Walk through all cores in the system that are used by this task.
  P_thread* firstThread;  // The "first" thread in thisCore. "first" is
                          // arbitrary.
  WALKSET(P_core*, par->pPlacer->taskToCores[task], coreNode)
  {
      // a failure to read the generated binary may not be absolutely fatal;
      // this could be retrieved later if there was a transient read error
      // (e.g. reading over a network connection).

      std::string binName;
      binName = task_dir+BIN_PATH+"/"+COREBIN_BASE+TO_STRING(coreNum)+".elf";

      FILE* binary = fopen(binName.c_str(),"r");

      if(binary == PNULL)
      {
        firstThread = (*coreNode)->P_threadm.begin()->second;
        if (!(par->pPlacer->threadToDevices.at(firstThread).empty())
            && (par->pPlacer->threadToDevices.at(firstThread).front()->par->par
                == task)) // only for cores which have something placed on them
                          // and which belong to the task
        {
          // a failure to read the generated binaries may not be absolutely
          // fatal; this could be retrieved later if there was a transient read
          // error (e.g. reading over a network connection).
          std::vector<std::string> binaries;
          FILE* binary;

          // Instruction binary
          binaries.push_back(task_dir+BIN_PATH+"/"+COREBIN_CODE_BASE+TO_STRING(coreNum)+".v");
          (*coreNode)->instructionBinary = binaries.back();

          // Data binary
          binaries.push_back(task_dir+BIN_PATH+"/"+COREBIN_DATA_BASE+TO_STRING(coreNum)+".v");
          (*coreNode)->dataBinary = binaries.back();

          for (std::vector<std::string>::iterator binaryIt = binaries.begin();
               binaryIt != binaries.end(); binaryIt++)
          {
            binary = fopen(binaryIt->c_str(),"r");
            // Check that the file opened successfully.
            if(binary == PNULL)
                par->Post(806, binaryIt->c_str(),
                          POETS::getSysErrorString(errno));
            else fclose(binary);
          }

          ++coreNum;                // Move onto the next core.
        }
      }
      else
      {
          // Add the file pointer to the core.
          (*coreNode)->instructionBinary = binName;
          fclose(binary);
      }

      ++coreNum;  // Move onto the next core.
  }

  //============================================================================
  // Check that the supervisor binary was made.
  //============================================================================
  FILE* binary;
  std::string binaryPath;
  binaryPath = task_dir+BIN_PATH+"/libSupervisor.so";
  task->pSup->binPath = binaryPath;
  binary = fopen(binaryPath.c_str(),"r");
  // Check that the file opened successfully.
  if(binary == PNULL)
      par->Post(806, binaryPath, POETS::getSysErrorString(errno));
  else fclose(binary);
  //============================================================================
  return 0;
}

//==============================================================================
#endif

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
    }
    if (!par->pPlace->Place(task)) task->LinkFlag(); // then preplace on the real or virtual board.
  }
  // if we need to we could aggregate some threads here to achieve maximum packing by merging partially-full threads.
}

//------------------------------------------------------------------------------



// creates the files that have to be generated from data rather than be pre-existing
unsigned P_builder::GenFiles(P_task* task)
{
  std::string task_dir(par->taskpath+task->Name());
  
  
  //============================================================================
  // Remove the task directory and recreate it to clean any previous build.
  //============================================================================
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
  // build a core map visible to the make script (as a shell script)
  //============================================================================
  P_core* thisCore;  // Core available during iteration.
  P_thread* firstThread;  // The "first" thread in thisCore. "first" is arbitrary, because cores are stored in a map.
  
  fstream cores_sh((task_dir+GENERATED_PATH+"/cores.sh").c_str(),
                    fstream::in | fstream::out | fstream::trunc);      // Open the cores shell script       
                    
  // Walk through all of the boards in the graph
  WALKPDIGRAPHNODES(AddressComponent,P_board*,unsigned,P_link*,
                      unsigned,P_port*,par->pE->G,boardNode)
  {
    
    // Walk through all of the mailboxes on the board.
    WALKPDIGRAPHNODES(AddressComponent,P_mailbox*,unsigned,P_link*,unsigned,
                        P_port*,par->pE->G.NodeData(boardNode)->G,mailboxNode)
    {
      
      // Walk through all of the cores on the mailbox
      WALKMAP(AddressComponent,P_core*,
              par->pE->G.NodeData(boardNode)->G.NodeData(mailboxNode)->P_corem,
              coreNode)
      {  
        thisCore = coreNode->second;                        // Reference to the current core
        firstThread = thisCore->P_threadm.begin()->second;  // Reference to the first thread on the core
        
        if (firstThread->P_devicel.size()                           // only for cores with something placed 
            && (firstThread->P_devicel.front()->par->par == task))  // and that belong to the task
        {
          cores_sh << "cores[" << coreNum << "]=";
          cores_sh << thisCore->get_hardware_address()->get_core() << "\n";
          // these consist of the declarations and definitions of variables and the handler functions.
          
          //====================================================================
          // Create empty files for the per-core variables declarations
          //====================================================================
          std::stringstream vars_hFName;
          vars_hFName << task_dir << GENERATED_H_PATH;
          vars_hFName << "/vars_" << coreNum << ".h";
          std::ofstream vars_h(vars_hFName.str());              // variables header
          
          
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
            if (threadIterator->second->P_devicel.size())
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
      }
    }
  }
  cores_sh.close();
  
  return 0;
}

//------------------------------------------------------------------------------



/*------------------------------------------------------------------------------
 * Method to generate the required supervisor.cpp and supervisor.h source files.
 *
 * For the default supervisor, this method simply copys supervisor.cpp and 
 * supervisor.h.
 * 
 * For a non-default supervisor, this method scours the datastructure and
 * appends to the default files.
 *
 * When built, the supervisor is compiled into a .so.
 *
 * Takes a pointer to a task.
 * Returns non-0 if the file copy failed or the file open failed.
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
    std::ofstream supervisor_h(supervisor_hFName.str(), fstream::app);    // Supervisor header, open in append
    
    if(supervisor_h.fail()) // Check that the file opened
    {                       // if it didn't, tell logserver and exit
      par->Post(816, supervisor_hFName.str(), POETS::getSysErrorString(errno));
      return 1;
    }
    
    std::stringstream supervisor_cFName;
    supervisor_cFName << task_dir << "/" << GENERATED_PATH <<"/Supervisor.cpp";
    std::ofstream supervisor_cpp(supervisor_cFName.str(), fstream::app);  // Supervisor code, open in append
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
    set<P_message*> sup_msgs; // set used to retain only unique message types used by supervisors
    
    std::stringstream sup_inPin_typedefs("");
    std::stringstream sup_pin_handlers("");
    std::stringstream sup_pin_vectors("");
    std::stringstream sup_inPin_props("");
    std::stringstream sup_inPin_state("");
    
    // build supervisor pin handlers.
    sup_pin_handlers << "vector<supInputPin*> Supervisor::inputs;\n";
    sup_pin_handlers << "vector<supOutputPin*> Supervisor::outputs;\n\n";
    
    sup_pin_vectors << "std::cout << \"Starting Application Supervisor for "; // Why is this an std::cout and not a logserver?
    sup_pin_vectors << "application " << task->Name() << "\" << std::endl;\n";
    
    
    //==========================================================================
    // Assemble output pin variables & handlers.
    //==========================================================================
    WALKVECTOR(P_pintyp*, supervisor_type->P_pintypIv, sI_pin)
    {
      sup_msgs.insert((*sI_pin)->pMsg);
      
      //========================================================================
      // Add the declaration for the pin's receive handler
      //========================================================================
      string sIpin_name = (*sI_pin)->Name();
      sup_pin_handlers << "unsigned super_InPin_" << sIpin_name;
      sup_pin_handlers << "_Recv_handler (const void* pinProps, ";
      sup_pin_handlers << "void* pinState, const P_Sup_Msg_t* inMsg, ";
      sup_pin_handlers << "PMsg_p* outMsg, void* msgBuf)\n";
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
        sup_pin_handlers << "_props_t* sEdgeProperties = ";
        sup_pin_handlers << "static_cast<const super_InPin_";
        sup_pin_handlers << sIpin_name << "_props_t*>(pinProps);\n";
        
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
        sup_pin_handlers << "_state_t* sEdgeState = ";
        sup_pin_handlers << "static_cast<super_InPin_";
        sup_pin_handlers << sIpin_name << "_state_t*>(pinState);\n";
        
        sup_inPin_state << "new super_InPin_" << sIpin_name;
        sup_inPin_state << "_state_t " << (*sI_pin)->pStateI->c_src;
      }
      else sup_inPin_state << "0";
      //========================================================================
      
      if ((*sI_pin)->pMsg->pPropsD)
      {
        sup_pin_handlers << "   const s_msg_" << (*sI_pin)->pMsg->Name();
        sup_pin_handlers << "_pyld_t* message = ";
        sup_pin_handlers << "static_cast<const s_msg_";
        sup_pin_handlers << (*sI_pin)->pMsg->Name() << "_pyld_t*>";
        sup_pin_handlers << "(static_cast<const void*>(inMsg->data));\n";
      }
      
      
      sup_pin_handlers << (*sI_pin)->pHandl->c_src.c_str() << "\n";
      
      // return no error by default if the handler bottoms out without problems
      sup_pin_handlers << "   return 0;\n";
      sup_pin_handlers << "}\n\n";
      
      // this will create a new pin object - how should this be deleted on exit since it's held in a static class member?
      sup_pin_vectors << "Supervisor::inputs.push_back";
      sup_pin_vectors << "(new supInputPin(";
      sup_pin_vectors << "&super_InPin_" << sIpin_name << "_Recv_handler,";
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
        sup_msgs.insert((*sO_pin)->pMsg);
        string sOpin_name = (*sO_pin)->Name();
        sup_pin_handlers << "unsigned super_OutPin_" << sOpin_name;
        sup_pin_handlers << "_Send_handler";
        sup_pin_handlers << "(PMsg_p* outMsg, void* msgBuf, unsigned superMsg)";
        sup_pin_handlers << "\n{\n";
        sup_pin_handlers << "   int s_c;\n";
        sup_pin_handlers << "   P_Sup_Msg_t* s_msg;\n";
        sup_pin_handlers << "   P_Sup_Hdr_t s_msg_hdr;\n";
        sup_pin_handlers << "   if (!(s_msg = outMsg->Get<P_Sup_Msg_t>(0, s_c))) return -1;\n";
        sup_pin_handlers << "   s_msg_hdr = s_msg->header;\n";
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
           sup_pin_handlers << "   s_msg_" << (*sO_pin)->pMsg->Name();
           sup_pin_handlers << "_pyld_t* outPyld;\n";
           
           sup_pin_handlers << "   if (superMsg) outPyld = static_cast<s_msg_";
           sup_pin_handlers << (*sO_pin)->pMsg->Name();
           sup_pin_handlers << "_pyld_t*>(static_cast<void*>(s_msg->data);\n";
           
           sup_pin_handlers << "   else\n";
           sup_pin_handlers << "   {\n";
           sup_pin_handlers << "      P_Msg_t* msg = new P_Msg_t();\n";
           sup_pin_handlers << "      outMsg->Put<P_Sup_Msg_t>(0, msg, 1);\n";
           sup_pin_handlers << "      delete msg;\n";
           sup_pin_handlers << "      if (!(msg = outMsg->Get<P_Msg_t>(0, s_c))) return -1;\n";
           
           sup_pin_handlers << "      outPyld = static_cast<s_msg_";
           sup_pin_handlers << (*sO_pin)->pMsg->Name();
           sup_pin_handlers << "_pyld_t*>(static_cast<void*>(msg->data));\n";
           sup_pin_handlers << "   }\n";
        }
        
        sup_pin_handlers << (*sO_pin)->pHandl->c_src.c_str() << "\n";
        // last part sets up to send the messages (which is automatically handled upon exit from the SupervisorCall).
        sup_pin_handlers << "   if (!superMsg)\n";
        sup_pin_handlers << "   {\n";
        sup_pin_handlers << "   P_Msg_t* sendMsg;\n";
        sup_pin_handlers << "   if (!(sendMsg = outMsg->Get<P_Msg_t>(0, s_c)))";
        sup_pin_handlers << " return -1;\n";
        
        sup_pin_handlers << "   P_Msg_Hdr_t* outHdr = &sendMsg->header;\n";
        sup_pin_handlers << "   outHdr->destDeviceAddr =";
        sup_pin_handlers << " s_msg_hdr.sourceDeviceAddr;\n";
        sup_pin_handlers << "   outHdr->messageLenBytes = p_hdr_size();\n";
        
        if ((*sO_pin)->pMsg->pPropsD)
        {
          sup_pin_handlers << "   outHdr->messageLenBytes += sizeof(s_msg_";
          sup_pin_handlers << (*sO_pin)->pMsg->Name() << "_pyld_t);\n";
        }
        
        sup_pin_handlers << "   }\n";
        // return number of messages to send if no error.
        sup_pin_handlers << "   return s_c;\n";
        sup_pin_handlers << "}\n\n";
        
        sup_pin_vectors << "Supervisor::outputs.push_back(new ";
        sup_pin_vectors << "supOutputPin(&super_OutPin_";
        sup_pin_vectors << sOpin_name << "_Send_handler));\n";
    }
    //============================================================================
    
    
    //==========================================================================
    // Write everything to the supervisor source files & close them.
    //==========================================================================
    supervisor_h << "#define _APPLICATION_SUPERVISOR_ 1\n\n";
    
    // supervisor message types. Note that for all objects with properties and state, both these values are optional so we need to check for their existence.
    WALKSET(P_message*, sup_msgs, s_msg)
    {
      if ((*s_msg)->pPropsD) 
      {
        supervisor_h << "typedef ";
        supervisor_h << string((*s_msg)->pPropsD->c_src).erase((*s_msg)->pPropsD->c_src.length()-2).c_str();
        supervisor_h << " s_msg_" <<(*s_msg)->Name() << "_pyld_t;\n\n";
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
  P_devtyp* c_devtyp = (*firstThread->P_devicel.begin())->pP_devtyp;    // Pointer to the core's device type
  std::string devtyp_name = c_devtyp->Name();                           // grab a local copy of the devtype name
  
  
  //============================================================================
  // Create empty files for the per-core variables and handlers
  //============================================================================    
  std::stringstream vars_cppFName;
  vars_cppFName << task_dir << GENERATED_CPP_PATH;
  vars_cppFName << "/vars_" << coreNum << ".cpp";
  std::ofstream vars_cpp(vars_cppFName.str());            // variables source
  if(vars_cpp.fail()) // Check that the file opened
  {                       // if it didn't, tell logserver and exit
    par->Post(816, vars_cppFName.str(), POETS::getSysErrorString(errno));
    return 1;
  }
  
  std::stringstream handlers_hFName;
  handlers_hFName << task_dir << GENERATED_H_PATH;
  handlers_hFName << "/handlers_" << coreNum << ".h";
  std::ofstream handlers_h(handlers_hFName.str());      // handlers header
  if(handlers_h.fail()) // Check that the file opened
  {                       // if it didn't, tell logserver and exit
    par->Post(816, handlers_hFName.str(), POETS::getSysErrorString(errno));
    vars_cpp.close();
    return 1;
  }
  
  std::stringstream handlers_cppFName;
  handlers_cppFName << task_dir << GENERATED_CPP_PATH;
  handlers_cppFName << "/handlers_" << coreNum << ".cpp";
  std::ofstream handlers_cpp(handlers_cppFName.str());  // handlers source
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
    if ((*firstThread->P_devicel.begin())->par->pPropsI)        // Graph Instance properties
    {
      global_init = (*firstThread->P_devicel.begin())->par->pPropsI->c_src;
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
  for (vector<P_message*>::iterator msg = c_devtyp->par->P_messagev.begin();
        msg != c_devtyp->par->P_messagev.end();
        msg++)
  {    
    if ((*msg)->pPropsD) 
    {
      vars_h << "typedef ";
      vars_h << string((*msg)->pPropsD->c_src).erase((*msg)->pPropsD->c_src.length()-2).c_str();
      vars_h << " msg_" <<(*msg)->Name() << "_pyld_t;\n";
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
  handlerPreamble << "{\n";
  
  if (c_devtyp->par->pPropsD)
  {
    handlerPreamble << "   const global_props_t* graphProperties ";
    handlerPreamble << "OS_ATTRIBUTE_UNUSED= ";
    handlerPreamble << "static_cast<const global_props_t*>";
    handlerPreamble << "(graphProps);\n";
    handlerPreamble << "OS_PRAGMA_UNUSED(graphProperties)\n";
  }
  handlerPreamble << "   PDeviceInstance* deviceInstance ";
  handlerPreamble << "OS_ATTRIBUTE_UNUSED= ";
  handlerPreamble << "static_cast<PDeviceInstance*>(device);\n";
  handlerPreamble << "OS_PRAGMA_UNUSED(deviceInstance)\n";
  
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
    handlerPreamble << "OS_PRAGMA_UNUSED(deviceProperties)\n";
  }
  
  // deviceState (with unused variable handling)
  if (c_devtyp->pStateD)
  {
    vars_h << "typedef ";
    vars_h << string(c_devtyp->pStateD->c_src).erase(c_devtyp->pStateD->c_src.length()-2).c_str();
    vars_h << " devtyp_" << devtyp_name << "_state_t;\n\n";
    
    handlerPreamble << "   devtyp_" << devtyp_name;
    handlerPreamble << "_state_t* deviceState ";
    handlerPreamble << "OS_ATTRIBUTE_UNUSED= ";
    handlerPreamble << "static_cast<devtyp_";
    handlerPreamble << devtyp_name;
    handlerPreamble << "_state_t*>(deviceInstance->state);\n";
    handlerPreamble << "OS_PRAGMA_UNUSED(deviceState)\n";
  }
  handlers_cpp << "\n";
  //============================================================================
  
  
  //============================================================================
  // Write the device-level handlers
  //============================================================================
  // ReadyToSend 
  handlers_h << "uint32_t devtyp_" << devtyp_name;
  handlers_h << "_RTS_handler (const void* graphProps, ";
  handlers_h << "void* device, uint32_t* readyToSend, ";
  handlers_h << "void** msg_buf);\n";
  
  handlers_cpp << "uint32_t devtyp_" << devtyp_name;
  handlers_cpp << "_RTS_handler (const void* graphProps, ";
  handlers_cpp << "void* device, uint32_t* readyToSend, ";
  handlers_cpp << "void** msg_buf)\n";
  handlers_cpp << handlerPreamble.str();
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
  handlers_cpp << handlerPreamble.str() << "\n";
  
  if (c_devtyp->pOnIdle) // insert the OnIdle handler if there is one
  {
    handlers_cpp << c_devtyp->pOnIdle->c_src << "\n"; 
  }
  else handlers_cpp << "   return 0;\n"; // or a stub if not
  handlers_cpp << "}\n\n";
  
  // OnCtl - stub until this can be resolved with DBT
  handlers_h << "uint32_t devtyp_" << devtyp_name;
  handlers_h << "_OnCtl_handler (const void* graphProps, ";
  handlers_h << "void* device, const void* msg);\n\n";
  
  handlers_cpp << "uint32_t devtyp_" << devtyp_name;
  handlers_cpp << "_OnCtl_handler (const void* graphProps, ";
  handlers_cpp << "void* device, const void* msg) {return 0;}\n\n";
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
    handlers_h << "void* device, void* edge, const void* msg);\n";
    
    handlers_cpp << "uint32_t devtyp_" << devtyp_name;
    handlers_cpp << "_InPin_" << Ipin_name;
    handlers_cpp << "_Recv_handler (const void* graphProps, ";
    handlers_cpp << "void* device, void* edge, const void* msg)\n";
    handlers_cpp << handlerPreamble.str().c_str();
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
      handlers_cpp << "   const msg_" << (*I_pin)->pMsg->Name();
      handlers_cpp << "_pyld_t* message = ";
      handlers_cpp << "static_cast<const msg_" << (*I_pin)->pMsg->Name();
      handlers_cpp << "_pyld_t*>(msg);\n";
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
    handlers_h << "void* device, void* msg, uint32_t buffered);\n";
    
    handlers_cpp << "uint32_t devtyp_" << devtyp_name;
    handlers_cpp << "_OutPin_" << Opin_name;
    handlers_cpp << "_Send_handler (const void* graphProps, ";
    handlers_cpp << "void* device, void* msg, uint32_t buffered)\n";
    
    handlers_cpp << handlerPreamble.str();
    
    if ((*O_pin)->pMsg->pPropsD)
    {
      handlers_cpp << "   msg_" << (*O_pin)->pMsg->Name();
      handlers_cpp << "_pyld_t* message = static_cast<msg_";
      handlers_cpp <<  (*O_pin)->pMsg->Name() << "_pyld_t*>(msg);\n";
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
  P_devtyp* devTyp = (*thread->P_devicel.begin())->pP_devtyp;
  
  // we could choose to create separate .h files for each thread giving the externs but it seems simpler
  // and arguably more flexible to put all the external declarations in a single .h
  // The actual data definitions go one file per thread so we can load the resultant data files individually.
  
  
  //============================================================================
  // Create the vars.cpp for the thread
  //============================================================================
  std::stringstream vars_cppFName;
  vars_cppFName << task_dir << GENERATED_CPP_PATH;
  vars_cppFName << "/vars_" << coreNum << "_" <<thread_num << ".cpp";
  std::ofstream vars_cpp(vars_cppFName.str());            // variables source
  if(vars_cpp.fail()) // Check that the file opened
  {                       // if it didn't, tell logserver and exit
    par->Post(816, vars_cppFName.str(), POETS::getSysErrorString(errno));
    return 1;
  }
  //============================================================================
  
  
  //============================================================================
  // Build the input pin map. Need counts to put in the device type table.
  //============================================================================
  set<P_message*> dev_in_msg_types;
  set<P_message*> dev_out_msg_types;
  vector<P_pintyp*>::iterator pin;
  for (pin = devTyp->P_pintypIv.begin(); pin != devTyp->P_pintypIv.end(); pin++) 
  {
    dev_in_msg_types.insert((*pin)->pMsg);
  }
  for (pin = devTyp->P_pintypOv.begin(); pin != devTyp->P_pintypOv.end(); pin++)
  {
    dev_out_msg_types.insert((*pin)->pMsg);
  }
  //============================================================================
  
  
  //============================================================================
  // write thread preable to vars.h and vars.cpp
  //============================================================================
  vars_h << "\n";
  vars_h << "//-------------------- Core " << coreNum;
  vars_h << " Thread " << thread_num << " variables --------------------\n";
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
  vars_cpp << "&Thread_" << thread_num << "_Context,";              // VirtualAddr
  vars_cpp << "1,";                                                 // numDevTyps        
  vars_cpp << "Thread_" << thread_num << "_DeviceTypes,";           // devTyps
  vars_cpp << thread->P_devicel.size() <<  ",";                     // numDevInsts
  vars_cpp << "Thread_" << thread_num << "_Devices,";               // devInsts
  vars_cpp << ((devTyp->par->pPropsD)?"&GraphProperties,":"PNULL,");// properties

  vars_cpp << "PNULL,";                                             // RTSHead              // TODO: Remove
  vars_cpp << "PNULL,";                                             // RTSTail              // TODO: Remove
  vars_cpp << "0,";                                                 // nextOnIdle           // TODO: Remove
  vars_cpp << "1,";                                                 // receiveHasPriority   // TODO: Remove
  vars_cpp << "0";                                                  // ctlEnd               // TODO: Remove
  vars_cpp << "};\n";
  
  /* Replacement for the above for the "new" softswitch
  vars_cpp << ",";                              // rtsBuffSize              //TODO:
  vars_cpp << "PNULL,";                                             // rtsBuf
  vars_cpp << "0,";                                                 // rtsStart
  vars_cpp << "0,";                                                 // rtsEnd
  vars_cpp << "0,";                                                 // idleStart
  vars_cpp << "0";                                                  // ctlEnd
  vars_cpp << "};\n";
  */
  //============================================================================
  
  
  // more partial streams because pin lists may or may not exist for a given device. 
  std::stringstream inpinlist("");
  std::stringstream outpinlist("");
  std::stringstream initialiser("");
  
  unsigned int inTypCnt = devTyp->P_pintypIv.size();       // Grab the number of input pin types to save derefs
  unsigned int outTypCnt = devTyp->P_pintypOv.size();      // Grab the number of output pin types to save derefs
  
  
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
  // Form the initialiser(s) for the input pins array. PInputType/in_pintyp_t struct
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
        initialiser << "sizeof(msg_" << (*ipin)->pMsg->Name() << "_pyld_t),";   // sz_msg
      else initialiser << "0,";
      
      initialiser << (*ipin)->pMsg->MsgType << ",";                             // msgType
      
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
  // Form the initialiser(s) for the output pins array. POutputType/in_pouttyp_t struct
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
        initialiser << "sizeof(msg_" << (*opin)->pMsg->Name() << "_pyld_t),";   // sz_msg
      else initialiser << "0,";
      initialiser << (*opin)->pMsg->MsgType << "},";                            // msgType
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
  vars_h << "_Devices[" << thread->P_devicel.size() << "];\n\n";
  //============================================================================
  
  
  
  unsigned index = 0;
  
  std::stringstream devInstInitialiser("");   // Device Instance Initialisers
  std::stringstream pinInitialiser("");       // Initialiser for input & output pins
  std::stringstream edgeInitialiser("");      // Initialiser for input & output edges
  
  std::stringstream devPropsInitialiser("");  // device type properties (devtyp_XXX_props_t) initialiser
  std::stringstream devStateInitialiser("");  // device type state (devtyp_XXX_state_t) initialiser
  
  std::stringstream inPinPropsInitialiser("");// device type input pin properties (devtyp_XXX_InPin_YYY_props_t) initialiser
  std::stringstream inPinStateInitialiser("");// device type input pin properties (devtyp_XXX_InPin_YYY_state_t) initialiser
  
  vector<unsigned int>in_pin_idxs;
  vector<unsigned int>out_pin_idxs;
  
  
  devPropsInitialiser << "{";   // Add the first { to the device properties array initialiser
  devStateInitialiser << "{";   // Add the first { to the device state array initialiser
  devInstInitialiser << "{";    // Add the first { to the device instance array initialiser
  
  for (list<P_device*>::iterator device = thread->P_devicel.begin(); 
        device != thread->P_devicel.end(); 
        device++)
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
      thrDevNameIn += std::to_string(thread_num) + std::string("_Device_");
      thrDevNameIn += (*device)->Name() + std::string("_InputPins");
      
      vars_h << "////----------------------- Input Pin (Associative) Tables ";
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
        for (pdigraph<unsigned int, P_device*, unsigned int, P_message*, unsigned int, P_pin*>::TPp_it p_edge = (*device)->par->G.index_n[(*device)->idx].fani.lower_bound(pin_num << PIN_POS); p_edge != next_pin; p_edge++)
        {
          //====================================================================
          // Form the first bit of an initialiser for PInputEdge/inEdge_t array member
          //====================================================================
          edgeInitialiser << "{";
          edgeInitialiser << "0,";                                              // pin      // TODO: populate this?
          edgeInitialiser << (*device)->idx << ",";                             // tgt
          edgeInitialiser << p_edge->second.iArc->second.fr_n->first << ",";    // src
          //====================================================================
          
          // if the pin has properties,
          if (p_edge->second.data->pP_pintyp->PinPropsSize)
          {     // set them up in the edge list
            //==================================================================
            // Form some more of the PInputEdge/inEdge_t initialiser
            //==================================================================
            edgeInitialiser << "&Device_" << (*device)->Name();
            edgeInitialiser << "_InEdge_" << (p_edge->first >> PIN_POS);
            edgeInitialiser << "_Properties[";
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
          if (p_edge->second.data->pP_pintyp->PinStateSize)
          {
            //==================================================================
            // Form the last bit of this PInputEdge/inEdge_t initialiser
            //==================================================================
            edgeInitialiser << "&Device_" << (*device)->Name(); 
            edgeInitialiser << "_InEdge_" << (p_edge->first >> PIN_POS); 
            edgeInitialiser << "_State["; 
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
              inPinStateInitialiser << "0"; // TODO: should this actually be p_edge->second.data->pP_pintyp->pStateI->c_src???
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
        
        
        if (next_pin != (*device)->par->G.index_n[(*device)->idx].fani.lower_bound(pin_num << PIN_POS)) // pin has connections?
        {
          // create input edge data structures
          --next_pin;
          
          
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
          if (next_pin->second.data->pP_pintyp->PinPropsSize)
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
          if (next_pin->second.data->pP_pintyp->PinStateSize)
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
          
          pinInitialiser << "0,";                                               // device
          
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
          pinInitialiser << "0,";                                               // device
          
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
        pinInitialiser << "0,";                                             // device
        
        pinInitialiser << "&Thread_" << thread_num;
        pinInitialiser << "_DevTyp_0_OutputPins[" << (*pin)->idx << "],";   // pinType
        
        pinInitialiser << "{},";                                            // msg_q_buf[P_MSG_Q_MAXCOUNT]  // TODO: Remove
        pinInitialiser << "PNULL,";                                         // msg_q_head                   // TODO: Remove
        pinInitialiser << "PNULL,";                                         // msg_q_tail.                  // TODO: Remove
        //======================================================================
        
        
        if ((!(*device)->par->G.FindArcs((*device)->idx,(*pin)->idx,in_pin_idxs,out_pin_idxs)) || !out_pin_idxs.size())
        {
          //====================================================================
          // Finish the Initialiser for the POutputPin/outPin_t array member with defaults.
          //====================================================================
          pinInitialiser << "0,";                                           // numTgts
          pinInitialiser << "PNULL,";                                       // targets
          pinInitialiser << "PNULL,";                                       // RTSPinPrev                   // TODO: Remove
          pinInitialiser << "PNULL";                                        // RTSPinNext                   // TODO: Remove
          pinInitialiser << "},";  
          //====================================================================
        }
        else
        {
          //====================================================================
          // Finish the Initialiser for the POutputPin/outPin_t array member.
          //====================================================================
          pinInitialiser << out_pin_idxs.size() << ",";                     // numTgts
          
          pinInitialiser << "Thread_" << thread_num;
          pinInitialiser << "_Device_" << (*device)->Name();
          pinInitialiser << "_OutPin_" << (*pin)->Name() << "_Tgts,";       // targets
          
          pinInitialiser << "PNULL,";                                       // RTSPinPrev                   // TODO: Remove
          pinInitialiser << "PNULL";                                        // RTSPinNext                   // TODO: Remove
          pinInitialiser << "},";
          //====================================================================
          
          
          //====================================================================
          // Add the declaration for the outedge array to the vars header
          //====================================================================
          vars_h << "extern outEdge_t Thread_" << thread_num;
          vars_h << "_Device_" << (*device)->Name();
          vars_h << "_OutPin_" << (*pin)->Name();
          vars_h << "_Tgts[" << out_pin_idxs.size() << "];\n";
          //====================================================================
          
          
          //====================================================================       
          // Generate the targets (HW addresses) for each edge. 
          // TODO: re-work this with the correct HW address class.
          //==================================================================== 
          edgeInitialiser.str("");      // Reset the edge initialiser for POutputEdge/outEdge_t
          edgeInitialiser << "{";
          
          for (vector<unsigned int>::iterator tgt = out_pin_idxs.begin();
                tgt != out_pin_idxs.end();
                tgt++)
          {
            // as for the case of inputs, the target device should be replaced by GetHardwareAddress(...)
            unsigned int tgt_idx = ((*device)->par->G.index_a.find(*tgt)->second.to_p)->first;
            P_addr tgt_addr = ((*device)->par->G.index_a.find(*tgt)->second.to_n)->second.data->addr;
            unsigned tgt_hwaddr = tgt_addr.A_box << (LOG_DEVICES_PER_THREAD + TinselLogThreadsPerCore + TinselLogCoresPerMailbox + TinselLogMailboxesPerBoard + TinselMeshXBits + TinselMeshYBits);
            tgt_hwaddr |= tgt_addr.A_board << (LOG_DEVICES_PER_THREAD + TinselLogThreadsPerCore + TinselLogCoresPerMailbox + TinselLogMailboxesPerBoard);
            tgt_hwaddr |= tgt_addr.A_mailbox << (LOG_DEVICES_PER_THREAD + TinselLogThreadsPerCore + TinselLogCoresPerMailbox);
            tgt_hwaddr |= tgt_addr.A_core << (LOG_DEVICES_PER_THREAD + TinselLogThreadsPerCore);
            tgt_hwaddr |= tgt_addr.A_thread << (LOG_DEVICES_PER_THREAD);
            
            //==================================================================
            // Form an initialiser for the POutputEdge/outEdge_t array member.
            //==================================================================
            edgeInitialiser << "{";
            edgeInitialiser << "0,";                                      // pin      // TODO: populate this?
            edgeInitialiser << (tgt_addr.A_device | tgt_hwaddr) << ",";   // tgt
            edgeInitialiser << (tgt_idx >> PIN_POS) << ",";               // tgtPin
            edgeInitialiser << (tgt_idx & (0xFFFFFFFF >> (32-PIN_POS)));  // tgtEdge
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
          vars_cpp << "_Tgts[" << out_pin_idxs.size() << "] = ";
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
    devInstInitialiser << "PNULL,";                                         // RTSPrev                   // TODO: Remove
    devInstInitialiser << "PNULL,";                                         // RTSNext                   // TODO: Remove
    devInstInitialiser << "PNULL,";                                         // RTSPinHead                // TODO: Remove
    devInstInitialiser << "PNULL,";                                         // RTSPinTail                // TODO: Remove
    devInstInitialiser << "0";                                              // currTgt                   // TODO: Remove
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
  vars_cpp << "_Devices[" << thread->P_devicel.size() << "] = ";
  vars_cpp << devInstInitialiser.str();
  //============================================================================
  
  
  if (devTyp->pPropsD)      // If the device type has properties
  {
    //==========================================================================
    // Write the device type properties declaration
    //==========================================================================
    vars_h << "extern devtyp_" << devTyp->Name();
    vars_h << "_props_t Thread_" << thread_num;
    vars_h << "_DeviceProperties[" << thread->P_devicel.size() << "];\n";
    //==========================================================================
    
    
    devPropsInitialiser.seekp(-1,ios_base::cur);    // Rewind one place to remove the stray ","
    devPropsInitialiser << "};\n";                  // properly terminate the initialiser
    
    //==========================================================================
    // Write the device type properties initialiser
    //==========================================================================
    vars_cpp << "devtyp_" << devTyp->Name();
    vars_cpp << "_props_t Thread_" << thread_num;
    vars_cpp << "_DeviceProperties[" << thread->P_devicel.size() << "] = ";
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
    vars_h << "_DeviceState[" << thread->P_devicel.size() << "];\n";
    //==========================================================================
    
    
    devStateInitialiser.seekp(-1,ios_base::cur);    // Rewind one place to remove the stray ","
    devStateInitialiser << "};\n";                  // properly terminate the initialiser
    
    //==========================================================================
    // Write the device type state initialiser
    //==========================================================================
    vars_cpp << "devtyp_" << devTyp->Name();
    vars_cpp << "_state_t Thread_" << thread_num;
    vars_cpp << "_DeviceState[" << thread->P_devicel.size() << "] = ";
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


  unsigned int coreNum = 0;
  P_core* thisCore;  // Core available during iteration.
  P_thread* firstThread;  // The "first" thread in thisCore. "first" is arbitrary, because cores are stored in a map.
  WALKPDIGRAPHNODES(AddressComponent,P_board*,unsigned,P_link*,unsigned,P_port*,par->pE->G,boardNode)
  {
  WALKPDIGRAPHNODES(AddressComponent,P_mailbox*,unsigned,P_link*,unsigned,P_port*,par->pE->G.NodeData(boardNode)->G,mailboxNode)
  {
  WALKMAP(AddressComponent,P_core*,par->pE->G.NodeData(boardNode)->G.NodeData(mailboxNode)->P_corem,coreNode)
  {
      thisCore = coreNode->second;
      firstThread = thisCore->P_threadm.begin()->second;
      if (firstThread->P_devicel.size() && (firstThread->P_devicel.front()->par->par == task)) // only for cores which have something placed on them and which belong to the task
      {
         // a failure to read the generated binary may not be absolutely fatal; this could be retrieved later if
         // there was a transient read error (e.g. reading over a network connection)
         string binary_name(static_cast<stringstream*>(&(stringstream(task_dir+BIN_PATH, ios_base::out | ios_base::ate)<<"/"<<COREBIN_BASE<<coreNum<<".elf"))->str());
         if (!(thisCore->instructionBinary->Binary = fopen(binary_name.c_str(),"r")))
            par->Post(806, binary_name);
         ++coreNum;
      }
  }
  }
  }
  return 0;
}

//==============================================================================
#endif

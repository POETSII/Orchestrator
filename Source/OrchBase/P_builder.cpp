//------------------------------------------------------------------------------

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

void P_builder::Build(P_task * pT)
// Generates the application binaries - virtually mapped to a single board.
{
if (!pT) pT = par->P_taskm.begin()->second; // default to the first task
par->Post(801,pT->Name(),pT->filename);
Preplace(pT);                               // Map to the system if necessary
GenFiles(pT);
CompileBins(pT);
}

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
void P_builder::GenFiles(P_task* task)
{
  string task_dir(par->taskpath+task->Name());
  system((string("rm -r -f ")+task_dir).c_str());
  system((MAKEDIR+" "+ task_dir).c_str());
  task_dir += "/";
  system(static_cast<stringstream*>(&(stringstream(MAKEDIR, ios_base::out | ios_base::ate)<<" "<<task_dir+GENERATED_PATH))->str().c_str());
  system(static_cast<stringstream*>(&(stringstream(MAKEDIR, ios_base::out | ios_base::ate)<<" "<<task_dir+GENERATED_H_PATH))->str().c_str());
  system(static_cast<stringstream*>(&(stringstream(MAKEDIR, ios_base::out | ios_base::ate)<<" "<<task_dir+GENERATED_CPP_PATH))->str().c_str());
  unsigned int coreNum = 0;
  if (!task->linked)
  {
    par->Post(811, task->Name());
    return;
  }

  // build the supervisor (which will be compiled into a .so)
  if (system(static_cast<stringstream*>(&(stringstream(SYS_COPY, ios_base::out | ios_base::ate)<<" "<<par->taskpath+STATIC_SRC_PATH<<"Supervisor.* "<<task_dir+GENERATED_PATH))->str().c_str()))
  {
      par->Post(807, (task_dir+GENERATED_PATH).c_str());
      return;
  }
  // open the default supervisor
  fstream supervisor_h((task_dir+GENERATED_PATH+"/Supervisor.h").c_str(), fstream::in | fstream::out | fstream::ate);
  fstream supervisor_cpp((task_dir+GENERATED_PATH+"/Supervisor.cpp").c_str(), fstream::in | fstream::out | fstream::ate);
  supervisor_h << "\n";
  supervisor_cpp << "\n";
  if (task->pSup->pP_devtyp->Name() != "_DEFAULT_SUPERVISOR_") // is there a non-default supervisor? If so, build it.
  {
     supervisor_h << "#define _APPLICATION_SUPERVISOR_ 1\n\n";
     P_devtyp* supervisor_type = task->pSup->pP_devtyp;
     set<P_message*> sup_msgs; // set used to retain only unique message types used by supervisors
     stringstream sup_inPin_typedefs("",ios_base::out | ios_base::ate);
     stringstream sup_pin_handlers("",ios_base::out | ios_base::ate);
     stringstream sup_pin_vectors("",ios_base::out | ios_base::ate);
     stringstream sup_inPin_props("",ios_base::out | ios_base::ate);
     stringstream sup_inPin_state("",ios_base::out | ios_base::ate);
     // build supervisor pin handlers.
     const char* s_handl_pre = "{\n";
     const char* s_handl_post = "}\n\n";
     sup_pin_handlers << "vector<supInputPin*> Supervisor::inputs;\n";
     sup_pin_handlers << "vector<supOutputPin*> Supervisor::outputs;\n\n";
     sup_pin_vectors << "cout << \"Starting Application Supervisor for application " << task->Name() << "\\n\";\n";
     sup_pin_vectors << "cout.flush();\n";
     // input pin variables and handlers
     WALKVECTOR(P_pintyp*, supervisor_type->P_pintypIv, sI_pin)
     {
         sup_msgs.insert((*sI_pin)->pMsg);
         string sIpin_name = (*sI_pin)->Name();
         sup_pin_handlers << "unsigned super_InPin_" << sIpin_name.c_str() << "_Recv_handler (const void* pinProps, void* pinState, const P_Sup_Msg_t* inMsg, PMsg_p* outMsg, void* msgBuf)\n";
         sup_pin_handlers << s_handl_pre;
         sup_inPin_props.str("");
         if ((*sI_pin)->pPropsD)
         {
             sup_inPin_typedefs << "typedef " << string((*sI_pin)->pPropsD->c_src).erase((*sI_pin)->pPropsD->c_src.length()-2).c_str() << " super_InPin_" << sIpin_name.c_str() << "_props_t;\n\n";
             sup_pin_handlers << "   const super_InPin_" << sIpin_name.c_str() << "_props_t* sEdgeProperties = static_cast<const super_InPin_" << sIpin_name.c_str() << "_props_t*>(pinProps);\n";
             sup_inPin_props <<  "new const super_InPin_" << sIpin_name.c_str() << "_props_t " << (*sI_pin)->pPropsI->c_src.c_str();
         }
         else sup_inPin_props << "0";
         sup_inPin_state.str("");
         if ((*sI_pin)->pStateD)
         {
             sup_inPin_typedefs << "typedef " << string((*sI_pin)->pStateD->c_src).erase((*sI_pin)->pStateD->c_src.length()-2).c_str() << " super_InPin_" << sIpin_name.c_str() << "_state_t;\n\n";
             sup_pin_handlers << "   super_InPin_" << sIpin_name.c_str() << "_state_t* sEdgeState = static_cast<super_InPin_" << sIpin_name.c_str() << "_state_t*>(pinState);\n";
             sup_inPin_state  << "new super_InPin_" << sIpin_name.c_str() << "_state_t " << (*sI_pin)->pStateI->c_src.c_str();
         }
         else sup_inPin_state << "0";
         if ((*sI_pin)->pMsg->pPropsD) sup_pin_handlers << "   const s_msg_" << (*sI_pin)->pMsg->Name().c_str() << "_pyld_t* message = static_cast<const s_msg_" <<  (*sI_pin)->pMsg->Name().c_str() << "_pyld_t*>(static_cast<const void*>(inMsg->data));\n";
         sup_pin_handlers << (*sI_pin)->pHandl->c_src.c_str() << "\n";
         // return no error by default if the handler bottoms out without problems
         sup_pin_handlers << "   return 0;\n";
         sup_pin_handlers << s_handl_post;
         // this will create a new pin object - how should this be deleted on exit since it's held in a static class member?
         sup_pin_vectors << "Supervisor::inputs.push_back(new supInputPin(&super_InPin_" << sIpin_name.c_str() << "_Recv_handler," << sup_inPin_props.str().c_str() << "," << sup_inPin_state.str().c_str() << "));\n";
     }
     sup_pin_handlers << "\n";
     // output pin handlers
     WALKVECTOR(P_pintyp*, supervisor_type->P_pintypOv, sO_pin)
     {
         sup_msgs.insert((*sO_pin)->pMsg);
         string sOpin_name = (*sO_pin)->Name();
         sup_pin_handlers << "unsigned super_OutPin_" << sOpin_name.c_str() << "_Send_handler (PMsg_p* outMsg, void* msgBuf, unsigned superMsg)\n";
         sup_pin_handlers << s_handl_pre;
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
            sup_pin_handlers << "   s_msg_" << (*sO_pin)->pMsg->Name().c_str() << "_pyld_t* outPyld;\n";
            sup_pin_handlers << "   if (superMsg) outPyld = static_cast<s_msg_" << (*sO_pin)->pMsg->Name().c_str() << "_pyld_t*>(static_cast<void*>(s_msg->data);\n";
            sup_pin_handlers << "   else\n";
            sup_pin_handlers << "   {\n";
            sup_pin_handlers << "      P_Msg_t* msg = new P_Msg_t();\n";
            sup_pin_handlers << "      outMsg->Put<P_Sup_Msg_t>(0, msg, 1);\n";
            sup_pin_handlers << "      delete msg;\n";
            sup_pin_handlers << "      if (!(msg = outMsg->Get<P_Msg_t>(0, s_c))) return -1;\n";
            sup_pin_handlers << "      outPyld = static_cast<s_msg_" << (*sO_pin)->pMsg->Name().c_str() << "_pyld_t*>(static_cast<void*>(msg->data));\n";
            sup_pin_handlers << "   }\n";
         }
         sup_pin_handlers << (*sO_pin)->pHandl->c_src.c_str() << "\n";
         // last part sets up to send the messages (which is automatically handled upon exit from the SupervisorCall).
         sup_pin_handlers << "   if (!superMsg)\n";
         sup_pin_handlers << "   {\n";
         sup_pin_handlers << "   P_Msg_t* sendMsg;\n";
         sup_pin_handlers << "   if (!(sendMsg = outMsg->Get<P_Msg_t>(0, s_c))) return -1;\n";
         sup_pin_handlers << "   P_Msg_Hdr_t* outHdr = &sendMsg->header;\n";
         sup_pin_handlers << "   outHdr->destDeviceAddr = s_msg_hdr.sourceDeviceAddr;\n";
         sup_pin_handlers << "   outHdr->messageLenBytes = p_hdr_size();\n";
         if ((*sO_pin)->pMsg->pPropsD) sup_pin_handlers << "   outHdr->messageLenBytes += sizeof(s_msg_" << (*sO_pin)->pMsg->Name().c_str() << "_pyld_t);\n";
         sup_pin_handlers << "   }\n";
         // return number of messages to send if no error.
         sup_pin_handlers << "   return s_c;\n";
         sup_pin_handlers << s_handl_post;
         sup_pin_vectors << "Supervisor::outputs.push_back(new supOutputPin(&super_OutPin_" << sOpin_name.c_str() << "_Send_handler));\n";
     }
     // supervisor message types. Note that for all objects with properties and state, both these values are optional so we need to check for their existence.
     WALKSET(P_message*, sup_msgs, s_msg)
         if ((*s_msg)->pPropsD) supervisor_h << "typedef " << string((*s_msg)->pPropsD->c_src).erase((*s_msg)->pPropsD->c_src.length()-2).c_str() << " s_msg_" <<(*s_msg)->Name().c_str() << "_pyld_t;\n\n";
     // bung all the user-defined stuff into the supervisor: types first, then handlers, then static pin vector initialisers.
     supervisor_h << sup_inPin_typedefs.str().c_str();
     // lay down all the generic code fragments before handlers (they might have function declarations, type declarations, etc.)
     supervisor_cpp << "#ifdef _APPLICATION_SUPERVISOR_\n\n";
     WALKVECTOR(CFrag*,supervisor_type->pHandlv,sCode)
     {
         supervisor_cpp << (*sCode)->c_src.c_str() << "\n";
     }
     supervisor_cpp << "\n" << sup_pin_handlers.str().c_str();
     // have to build the static vectors inside a loadable function
     supervisor_cpp << "\n";
     supervisor_cpp << "extern \"C\"" << s_handl_pre;
     supervisor_cpp << "int SupervisorInit()\n" << s_handl_pre << sup_pin_vectors.str().c_str();
     supervisor_cpp << "return 0;\n" << "}\n" << s_handl_post;
     supervisor_cpp << "#endif";
  }
  supervisor_cpp.close();

  // build a core map visible to the make script (as a shell script)
  P_core* thisCore;  // Core available during iteration.
  P_thread* firstThread;  // The "first" thread in thisCore. "first" is arbitrary, because cores are stored in a map.
  fstream cores_sh((task_dir+GENERATED_PATH+"/cores.sh").c_str(), fstream::in | fstream::out | fstream::trunc);
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
      cores_sh << "cores[" << coreNum << "]=" << thisCore->get_hardware_address()->get_core() << "\n";
      // these consist of the declarations and definitions of variables and the handler functions.
      fstream vars_h(static_cast<stringstream*>(&(stringstream(task_dir+GENERATED_H_PATH, ios_base::out | ios_base::ate)<<"/vars_"<<coreNum<<".h"))->str().c_str(), fstream::in | fstream::out | fstream::trunc);
      fstream vars_cpp(static_cast<stringstream*>(&(stringstream(task_dir+GENERATED_CPP_PATH, ios_base::out | ios_base::ate)<<"/vars_"<<coreNum<<".cpp"))->str().c_str(), fstream::in | fstream::out | fstream::trunc);
      fstream handlers_h(static_cast<stringstream*>(&(stringstream(task_dir+GENERATED_H_PATH, ios_base::out | ios_base::ate)<<"/handlers_"<<coreNum<<".h"))->str().c_str(), fstream::in | fstream::out | fstream::trunc);
      fstream handlers_cpp(static_cast<stringstream*>(&(stringstream(task_dir+GENERATED_CPP_PATH, ios_base::out | ios_base::ate)<<"/handlers_"<<coreNum<<".cpp"))->str().c_str(), fstream::in | fstream::out | fstream::trunc);

      // conveniently since so far we have arranged it so that each core has a monolithic device type looking up its vars is
      // easy. Later this will need smoother access. It would be nice if we could define this as a const P_devtyp* but
      // unfortunately none of its member functions are const-qualified which hampers usability.
      P_devtyp* c_devtyp = (*firstThread->P_devicel.begin())->pP_devtyp;
      /* The seemingly-innocuous commented line below doesn't work as expected. Apparently devtyp_name becomes volatile even though
         we aren't accessing the Name() field later in any way that might invalidate the pointer. This is because functions (such as
         Name() return a temporary object (on the stack) and the c_str() function directly references its object to get the object's
         internal string representation. Thus as soon as control moves beyond this line the string temporary returned by Name() goes
         out of scope, and the pointer obtained through c_str() is invalidated.
         See https://stackoverflow.com/questions/42542055/trouble-with-stdstring-c-str?noredirect=1&lq=1
      */
      // const char* devtyp_name = c_devtyp->Name().c_str();
      vars_h << "#include <cstdint>\n";
      vars_h << "#include \"softswitch_common.h\"\n\n";
      handlers_cpp << "#include \"vars_" << coreNum << ".h\"\n";
      handlers_cpp << "#include \"handlers_" << coreNum << ".h\"\n\n";
      vars_cpp << "#include \"vars_" << coreNum << ".h\"\n";
      // assemble a typedef for each of the fragments. If the enclosing class doesn't provide a struct {} enclosure for data objects with more
      //  than one member this can be easily inserted here as well. We copy the string from the definition to avoid inavertent in-place modification.
      //
      // globals first
      if (c_devtyp->par->pPropsD)
      {
         vars_h << "typedef " << string(c_devtyp->par->pPropsD->c_src).erase(c_devtyp->par->pPropsD->c_src.length()-2).c_str() << " global_props_t;\n\n";
         vars_h << "extern const global_props_t GraphProperties;\n";
         string global_init("");
         if ((*firstThread->P_devicel.begin())->par->pPropsI) global_init = (*firstThread->P_devicel.begin())->par->pPropsI->c_src;
         else if (c_devtyp->par->pPropsI) global_init = c_devtyp->par->pPropsI->c_src;
         vars_cpp << "const global_props_t GraphProperties OS_ATTRIBUTE_UNUSED= " << global_init.c_str() << ";\n";
         vars_cpp << "OS_PRAGMA_UNUSED(GraphProperties)\n";
      }
      for (vector<CFrag*>::iterator g_code = c_devtyp->par->General.begin(); g_code != c_devtyp->par->General.end(); g_code++)
          handlers_cpp << (*g_code)->c_src.c_str() << "\n"; // newline assumes general code fragments are unrelated and don't expect back-to-back concatenation
      // messages. Note that for all objects with properties and state, both these values are optional so we need to check for their existence.
      for (vector<P_message*>::iterator msg = c_devtyp->par->P_messagev.begin(); msg != c_devtyp->par->P_messagev.end(); msg++)
          if ((*msg)->pPropsD) vars_h << "typedef " << string((*msg)->pPropsD->c_src).erase((*msg)->pPropsD->c_src.length()-2).c_str() << " msg_" <<(*msg)->Name().c_str() << "_pyld_t;\n";
      vars_h << "\n";
      // device handlers
      handlers_h << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_RTS_handler (const void* graphProps, void* device, uint32_t* readyToSend, void** msg_buf);\n";
      handlers_h << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_OnIdle_handler (const void* graphProps, void* device);\n";
      handlers_h << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_OnCtl_handler (const void* graphProps, void* device, const void* msg);\n\n";
      for (vector<CFrag*>::iterator d_code = c_devtyp->pHandlv.begin(); d_code != c_devtyp->pHandlv.end(); d_code++)
          handlers_cpp << (*d_code)->c_src.c_str() << "\n";
      // this preamble is common to all the handlers.
      stringstream handl_pre_strm("{\n", ios_base::out | ios_base::ate);
      if (c_devtyp->par->pPropsD)
      {
        handl_pre_strm << "   const global_props_t* graphProperties OS_ATTRIBUTE_UNUSED";
        handl_pre_strm << "= static_cast<const global_props_t*>(graphProps);\n";
        handl_pre_strm << "OS_PRAGMA_UNUSED(graphProperties)\n";
      }
      
      handl_pre_strm << "   PDeviceInstance* deviceInstance OS_ATTRIBUTE_UNUSED= static_cast<PDeviceInstance*>(device);\n";
      handl_pre_strm << "OS_PRAGMA_UNUSED(deviceInstance)\n";
      // device variables
      if (c_devtyp->pPropsD)
      {
          vars_h << "typedef " << string(c_devtyp->pPropsD->c_src).erase(c_devtyp->pPropsD->c_src.length()-2).c_str() << " devtyp_" << c_devtyp->Name().c_str() << "_props_t;\n\n";
          handl_pre_strm << "   const devtyp_" << c_devtyp->Name().c_str(); 
          handl_pre_strm << "_props_t* deviceProperties OS_ATTRIBUTE_UNUSED = static_cast<const devtyp_"; 
          handl_pre_strm << c_devtyp->Name().c_str() << "_props_t*>(deviceInstance->properties);\n";
          handl_pre_strm << "OS_PRAGMA_UNUSED(deviceProperties)\n";
      }
      if (c_devtyp->pStateD)
      {
          vars_h << "typedef " << string(c_devtyp->pStateD->c_src).erase(c_devtyp->pStateD->c_src.length()-2).c_str() << " devtyp_" << c_devtyp->Name().c_str() << "_state_t;\n\n";
          handl_pre_strm << "   devtyp_" << c_devtyp->Name().c_str(); 
          handl_pre_strm << "_state_t* deviceState OS_ATTRIBUTE_UNUSED= static_cast<devtyp_"; 
          handl_pre_strm << c_devtyp->Name().c_str() << "_state_t*>(deviceInstance->state);\n";
          handl_pre_strm << "OS_PRAGMA_UNUSED(deviceState)\n";
      }
      // end preamble
      const char* handl_post = "}\n\n";
      handlers_cpp << "\n";
      // stub OnCtl handler until this can be resolved with DBT
      handlers_cpp << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_OnCtl_handler (const void* graphProps, void* device, const void* msg) {return 0;}\n\n";
      handlers_cpp << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_RTS_handler (const void* graphProps, void* device, uint32_t* readyToSend, void** msg_buf)\n";
      handlers_cpp << handl_pre_strm.str().c_str();
      handlers_cpp << c_devtyp->pOnRTS->c_src.c_str() << "\n";
      handlers_cpp << "   return *readyToSend;\n"; // we assume here the return value is intended to be an RTS bitmap.
      handlers_cpp << handl_post;
      handlers_cpp << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_OnIdle_handler (const void* graphProps, void* device)\n";
      handlers_cpp << handl_pre_strm.str().c_str() << "\n";
      if (c_devtyp->pOnIdle) handlers_cpp << c_devtyp->pOnIdle->c_src.c_str() << "\n"; // insert the OnIdle handler if there is one
      else handlers_cpp << "   return 0;\n"; // or a stub if not
      handlers_cpp << handl_post;
      // input pin variables and handlers
      for (vector<P_pintyp*>::iterator I_pin = c_devtyp->P_pintypIv.begin(); I_pin != c_devtyp->P_pintypIv.end(); I_pin++)
      {
          string Ipin_name = (*I_pin)->Name();
          handlers_h << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_InPin_" << Ipin_name.c_str() << "_Recv_handler (const void* graphProps, void* device, void* edge, const void* msg);\n";
          handlers_cpp << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_InPin_" << Ipin_name.c_str() << "_Recv_handler (const void* graphProps, void* device, void* edge, const void* msg)\n";
          handlers_cpp << handl_pre_strm.str().c_str();
          handlers_cpp << "   inEdge_t* edgeInstance OS_ATTRIBUTE_UNUSED= static_cast<inEdge_t*>(edge);\n";
          handlers_cpp << "OS_PRAGMA_UNUSED(edgeInstance)\n";
          if ((*I_pin)->pPropsD)
          {
              vars_h << "typedef " << string((*I_pin)->pPropsD->c_src).erase((*I_pin)->pPropsD->c_src.length()-2).c_str() << " devtyp_" << c_devtyp->Name().c_str() << "_InPin_" << Ipin_name.c_str() << "_props_t;\n\n";
              handlers_cpp << "   const devtyp_" << c_devtyp->Name().c_str() << "_InPin_" << Ipin_name.c_str() << "_props_t* edgeProperties = static_cast<const devtyp_" << c_devtyp->Name().c_str() << "_InPin_" << Ipin_name.c_str() << "_props_t*>(edgeInstance->properties);\n";
          }
          if ((*I_pin)->pStateD)
          {
              vars_h << "typedef " << string((*I_pin)->pStateD->c_src).erase((*I_pin)->pStateD->c_src.length()-2).c_str() << " devtyp_" << c_devtyp->Name().c_str() << "_InPin_" << Ipin_name.c_str() << "_state_t;\n\n";
              handlers_cpp << "   devtyp_" << c_devtyp->Name().c_str() << "_InPin_" << Ipin_name.c_str() << "_state_t* edgeState = static_cast<devtyp_" << c_devtyp->Name().c_str() << "_InPin_" << Ipin_name.c_str() << "_state_t*>(edgeInstance->state);\n";
          }
          if ((*I_pin)->pMsg->pPropsD) handlers_cpp << "   const msg_" << (*I_pin)->pMsg->Name().c_str() << "_pyld_t* message = static_cast<const msg_" <<  (*I_pin)->pMsg->Name().c_str() << "_pyld_t*>(msg);\n";
          handlers_cpp << (*I_pin)->pHandl->c_src.c_str() << "\n";
          // return type is indicated as uint32_t yet in the handlers we see from DBT no return value is set. Is something expected here?
          handlers_cpp << "   return 0;\n";
          handlers_cpp << handl_post;
      }
      vars_h << "\n";
      handlers_h << "\n";
      // output pin handlers
      for (vector<P_pintyp*>::iterator O_pin = c_devtyp->P_pintypOv.begin(); O_pin != c_devtyp->P_pintypOv.end(); O_pin++)
      {
          string Opin_name = (*O_pin)->Name();
          handlers_h << "const uint32_t RTS_INDEX_" << (*O_pin)->Name().c_str() << " = " << (*O_pin)->idx << ";\n";
          handlers_h << "const uint32_t RTS_FLAG_" << (*O_pin)->Name().c_str() << " = 0x1 << " << (*O_pin)->idx << ";\n";
          handlers_h << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_OutPin_" << Opin_name.c_str() << "_Send_handler (const void* graphProps, void* device, void* msg, uint32_t buffered);\n";
          handlers_cpp << "uint32_t devtyp_" << c_devtyp->Name().c_str() << "_OutPin_" << Opin_name.c_str() << "_Send_handler (const void* graphProps, void* device, void* msg, uint32_t buffered)\n";
          handlers_cpp << handl_pre_strm.str().c_str();
          if ((*O_pin)->pMsg->pPropsD) handlers_cpp << "   msg_" << (*O_pin)->pMsg->Name().c_str() << "_pyld_t* message = static_cast<msg_" <<  (*O_pin)->pMsg->Name().c_str() << "_pyld_t*>(msg);\n";
          handlers_cpp << (*O_pin)->pHandl->c_src.c_str() << "\n";
          // same thing: what is this function expected to return?
          handlers_cpp << "   return 0;\n";
          handlers_cpp << handl_post;
      }
      WALKMAP(AddressComponent,P_thread*,thisCore->P_threadm,threadIterator)
      {
          if (threadIterator->second->P_devicel.size())
          {
              WriteThreadVars(task_dir, coreNum, threadIterator->first, threadIterator->second, vars_h);
          }
      }
      vars_h.close();
      vars_cpp.close();
      handlers_h.close();
      handlers_cpp.close();
      ++coreNum;
  }
  }
  }
  }
  cores_sh.close();
}

//------------------------------------------------------------------------------

void P_builder::WriteThreadVars(string& task_dir, unsigned int core_num, unsigned int thread_num, P_thread* thread, fstream& vars_h)
{
     // a trivial bit more overhead, perhaps, then passing this as an argument. The *general* method would extract the number of device types from the thread's device list.
     P_devtyp* t_devtyp = (*thread->P_devicel.begin())->pP_devtyp;
     // we could choose to create separate .h files for each thread giving the externs but it seems simpler
     // and arguably more flexible to put all the external declarations in a single .h
     // The actual data definitions go one file per thread so we can load the resultant data files individually.
     fstream vars_cpp(static_cast<stringstream*>(&(stringstream(task_dir+GENERATED_CPP_PATH, ios_base::out | ios_base::ate)<<"/vars_"<<core_num<<"_"<<thread_num<<".cpp"))->str().c_str(), fstream::in | fstream::out | fstream::trunc);
     vars_h << "\n";
     vars_h << "//-------------------- Core " << core_num << " Thread " << thread_num << " variables --------------------\n";
     // At the bottom of the memory structure is the ThreadContext
     vars_h << "extern ThreadCtxt_t Thread_" << thread_num << "_Context;\n";
     vars_cpp << "#include \"vars_" << core_num << ".h\"\n";
     vars_cpp << "#include \"handlers_" << core_num << ".h\"\n\n";
     vars_cpp << "//-------------------- Core " << core_num << " Thread " << thread_num << " variables --------------------\n";
     vars_cpp << "ThreadCtxt_t Thread_" << thread_num <<"_Context __attribute__ ((section (\".thr" << thread_num << "_base\"))) = {&Thread_" << thread_num << "_Context,1,Thread_" << thread_num << "_DeviceTypes," << thread->P_devicel.size() <<  ",Thread_" << thread_num << "_Devices,";
     if (t_devtyp->par->pPropsD) vars_cpp << "&GraphProperties,0,0,0,1,0};\n";
     else vars_cpp << "0,0,0,0,1,0};\n";
     vars_h << "extern struct PDeviceType Thread_" << thread_num << "_DeviceTypes[1];\n";
     // how should these and subsequent arrays be initialised?
     vars_h << "//------------------------------ Device Type Tables ------------------------------\n";
     // to build the input pin map a complex series of operations has to be carried out. The first step is to get a list of unique message types.
     // this needs to be done far in advance because the counts will be placed in the device type table.
     set<P_message*> dev_in_msg_types;
     set<P_message*> dev_out_msg_types;
     for (vector<P_pintyp*>::iterator ipin = t_devtyp->P_pintypIv.begin(); ipin != t_devtyp->P_pintypIv.end(); ipin++) dev_in_msg_types.insert((*ipin)->pMsg);
     for (vector<P_pintyp*>::iterator opin = t_devtyp->P_pintypOv.begin(); opin != t_devtyp->P_pintypOv.end(); opin++) dev_out_msg_types.insert((*opin)->pMsg);
     vars_cpp << "devTyp_t Thread_" << thread_num << "_DeviceTypes[1] = {&devtyp_" << t_devtyp->Name().c_str() << "_RTS_handler,";
     vars_cpp << "&devtyp_" << t_devtyp->Name().c_str() << "_OnIdle_handler,";
     vars_cpp << "&devtyp_" << t_devtyp->Name().c_str() << "_OnCtl_handler,";
     if (t_devtyp->pPropsD) vars_cpp << "sizeof(devtyp_" << t_devtyp->Name().c_str() << "_props_t),";
     else vars_cpp << "0,";
     if (t_devtyp->pStateD) vars_cpp << "sizeof(devtyp_" << t_devtyp->Name().c_str() << "_state_t),";
     else vars_cpp << "0,";
     // more partial streams because pin lists may or may not exist for a given device. By not using ios_base::ate we ensure that actual values for these initialisers override the 0 default.
     stringstream inpinlist("");
     stringstream outpinlist("");
     stringstream initialiser("", ios_base::out | ios_base::ate);
     if (t_devtyp->P_pintypIv.size())
     {
        vars_cpp << t_devtyp->P_pintypIv.size() << ",Thread_" << thread_num << "_DevTyp_0_InputPins,";
        vars_h << "extern in_pintyp_t Thread_" << thread_num << "_DevTyp_0_InputPins[" << t_devtyp->P_pintypIv.size() << "];\n";
        initialiser.str("{");
        for (vector<P_pintyp*>::iterator ipin = t_devtyp->P_pintypIv.begin(); ipin != t_devtyp->P_pintypIv.end(); ipin++)
        {
            initialiser << "{&devtyp_" << t_devtyp->Name().c_str() << "_InPin_" << (*ipin)->Name().c_str() << "_Recv_handler,";
            if ((*ipin)->pMsg->pPropsD) initialiser << "sizeof(msg_" << (*ipin)->pMsg->Name().c_str() << "_pyld_t),";
            else initialiser << "0,";
            initialiser << (*ipin)->pMsg->MsgType;
            if ((*ipin)->pPropsD) initialiser << ",sizeof(devtyp_" << t_devtyp->Name().c_str() << "_InPin_" << (*ipin)->Name().c_str() << "_props_t),";
            else initialiser << ",0,";
            if ((*ipin)->pStateD) initialiser << "sizeof(devtyp_" << t_devtyp->Name().c_str() << "_InPin_" << (*ipin)->Name().c_str() << "_state_t)},";
            else initialiser << "0},";
        }
        initialiser.seekp(-1,ios_base::cur);
        initialiser << "};\n";
        inpinlist << "in_pintyp_t Thread_" << thread_num << "_DevTyp_0_InputPins[" << t_devtyp->P_pintypIv.size() << "] = " << initialiser.str().c_str();
     }
     else vars_cpp << "0,0,";  // no input pins
     if (t_devtyp->P_pintypOv.size())
     {
        vars_cpp << t_devtyp->P_pintypOv.size() << ",Thread_" << thread_num << "_DevTyp_0_OutputPins};\n";
        vars_h << "//------------------------------ Pin Type Tables ------------------------------\n";
        vars_h << "extern out_pintyp_t Thread_" << thread_num << "_DevTyp_0_OutputPins[" << t_devtyp->P_pintypOv.size() << "];\n";
        initialiser.str("{");
        for (vector<P_pintyp*>::iterator opin = t_devtyp->P_pintypOv.begin(); opin != t_devtyp->P_pintypOv.end(); opin++)
        {
            initialiser << "{&devtyp_" << t_devtyp->Name().c_str() << "_OutPin_" << (*opin)->Name().c_str() << "_Send_handler,";
            if ((*opin)->pMsg->pPropsD) initialiser << "sizeof(msg_" << (*opin)->pMsg->Name().c_str() << "_pyld_t),";
            else initialiser << "0,";
            initialiser << (*opin)->pMsg->MsgType << "},";
        }
        initialiser.seekp(-1,ios_base::cur);
        initialiser << "};\n";
        outpinlist << "out_pintyp_t Thread_" << thread_num << "_DevTyp_0_OutputPins[" << t_devtyp->P_pintypOv.size() << "] = " << initialiser.str().c_str();
     }
     else vars_cpp << "0,0};\n"; // no output pins
     vars_cpp << inpinlist.str().c_str();
     vars_cpp <<outpinlist.str().c_str();
     vars_cpp << "\n";
     vars_h << "//------------------------------ Device Instance Tables ------------------------------\n";
     vars_h << "extern devInst_t Thread_" << thread_num << "_Devices[" << thread->P_devicel.size() << "];\n\n";
     initialiser.str("{");
     unsigned index = 0;
     stringstream initialiser_2("", ios_base::out | ios_base::ate);
     stringstream initialiser_3("", ios_base::out | ios_base::ate);
     stringstream initialiser_4("{", ios_base::out | ios_base::ate);
     stringstream initialiser_5("{", ios_base::out | ios_base::ate);
     stringstream initialiser_6("", ios_base::out | ios_base::ate);
     stringstream initialiser_7("", ios_base::out | ios_base::ate);
     vector<unsigned int>in_pin_idxs;
     vector<unsigned int>out_pin_idxs;
     for (list<P_device*>::iterator device = thread->P_devicel.begin(); device != thread->P_devicel.end(); device++)
     {
         // device index should be replaced by a GetHardwareAddress(...) call.
         initialiser << "{&Thread_" << thread_num << "_Context,&Thread_" << thread_num << "_DeviceTypes[0]," << (*device)->addr.A_device << ",";
         // need to descend into the pins and set them up before setting up the device
         if (t_devtyp->P_pintypIv.size())
         {
             initialiser << t_devtyp->P_pintypIv.size() << ",Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_InputPins,";
             vars_h << "//------------------------------ Input Pin (Associative) Tables ------------------------------\n";
             vars_h << "extern inPin_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_InputPins[" << t_devtyp->P_pintypIv.size() << "];\n";
             initialiser_2.str("{");
             for (unsigned int pin_num = 0; pin_num < t_devtyp->P_pintypIv.size(); pin_num++)
             {
                 // within the pin, build the internal edge information (the pin's source list)
                 initialiser_3.str("{");
                 initialiser_6.str("{");
                 initialiser_7.str("{");
                 //multimap<unsigned int, pdigraph<unsigned int, P_device*, unsigned int, P_message*, unsigned int, P_pin*>::pin>::iterator next_pin = (*device)->par->G.index_n[(*device)->idx].fani.upper_bound(pin_num << PIN_POS | 0xFFFFFFFF >> (32-PIN_POS));
                 //for (multimap<unsigned int, pdigraph<unsigned int, P_device*, unsigned int, P_message*, unsigned int, P_pin*>::pin>::iterator p_edge = (*device)->par->G.index_n[(*device)->idx].fani.lower_bound(pin_num << PIN_POS); p_edge != next_pin; p_edge++)
                 pdigraph<unsigned int, P_device*, unsigned int, P_message*, unsigned int, P_pin*>::TPp_it next_pin = (*device)->par->G.index_n[(*device)->idx].fani.upper_bound(pin_num << PIN_POS | 0xFFFFFFFF >> (32-PIN_POS));
                 for (pdigraph<unsigned int, P_device*, unsigned int, P_message*, unsigned int, P_pin*>::TPp_it p_edge = (*device)->par->G.index_n[(*device)->idx].fani.lower_bound(pin_num << PIN_POS); p_edge != next_pin; p_edge++)
                 {
                     initialiser_3 << "{0," << (*device)->idx << "," << p_edge->second.iArc->second.fr_n->first << ",";
                     // if the pin has properties,
                     if (p_edge->second.data->pP_pintyp->PinPropsSize)
                     {
                        // set them up in the edge list
                        initialiser_3 << "&Device_" << (*device)->Name().c_str() << "_InEdge_" << (p_edge->first >> PIN_POS) << "_Properties[" << (p_edge->first & (0xFFFFFFFF >> (32-PIN_POS))) << "],";
                        // using the set properties if available
                        if (p_edge->second.data->pPropsI) initialiser_6 << p_edge->second.data->pPropsI->c_src.c_str() << ",";
                        // or defaults if not. (should this be pPropsD rather than pPropsI?)
                        else initialiser_6 << p_edge->second.data->pP_pintyp->pPropsI->c_src.c_str() << ",";
                     }
                     else initialiser_3 << "0,";
                     if (p_edge->second.data->pP_pintyp->PinStateSize)
                     {
                        initialiser_3 << "&Device_" << (*device)->Name().c_str() << "_InEdge_" << (p_edge->first >> PIN_POS) << "_State[" << (p_edge->first & (0xFFFFFFFF >> (32-PIN_POS))) << "]},";
                        if (p_edge->second.data->pStateI) initialiser_7 << p_edge->second.data->pStateI->c_src.c_str() << ",";
                        else initialiser_7 << "0,";
                     }
                     else initialiser_3 << "0},";
                 }
                 initialiser_3.seekp(-1, ios_base::cur);
                 initialiser_6.seekp(-1, ios_base::cur);
                 initialiser_7.seekp(-1, ios_base::cur);
                 initialiser_3 << "};\n";
                 initialiser_6 << "};\n";
                 initialiser_7 << "};\n";
                 if (next_pin != (*device)->par->G.index_n[(*device)->idx].fani.lower_bound(pin_num << PIN_POS)) // pin has connections?
                 {
                    // create input edge data structures
                    --next_pin;
                    vars_h << "extern inEdge_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_Pin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_InEdges[" << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1 << "];\n";
                    vars_cpp << "inEdge_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_Pin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_InEdges[" << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1 << "] = " << initialiser_3.str().c_str();
                    // with associated properties if they exist
                    if (next_pin->second.data->pP_pintyp->PinPropsSize)
                    {
                       vars_h << "extern devtyp_" << t_devtyp->Name().c_str() << "_InPin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_props_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_Pin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_InEdgeProps[" << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1 << "];\n";
                       vars_cpp << "devtyp_" << t_devtyp->Name().c_str() << "_InPin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_props_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_Pin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_InEdgeProps[" << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1 << "] = " << initialiser_6.str().c_str();
                    }
                    // and associated state (again, if it exists)
                    if (next_pin->second.data->pP_pintyp->PinStateSize)
                    {
                       vars_h << "extern devtyp_" << t_devtyp->Name().c_str() << "_InPin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_state_t  Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_Pin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_InEdgeStates[" << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1 << "];\n";
                       vars_cpp << "devtyp_" << t_devtyp->Name().c_str() << "_InPin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_state_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_Pin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_InEdgeStates[" << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1 << "] = " << initialiser_7.str().c_str();
                    }
                    // once the edges have been built, build up the input pin list itself.
                    initialiser_2 << "{0,&Thread_" << thread_num << "_DevTyp_0_InputPins[" << pin_num << "]," << (next_pin->first & (0xFFFFFFFF >> (32-PIN_POS)))+1 << ",Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_Pin_" << next_pin->second.data->pP_pintyp->Name().c_str() << "_InEdges},";
                 }
                 else initialiser_2 << "{0,&Thread_" << thread_num << "_DevTyp_0_InputPins[" << pin_num << "],0,0},";
             }
             initialiser_2.seekp(-1, ios_base::cur);
             initialiser_2 << "};\n";
             vars_cpp << "inPin_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_InputPins[" << t_devtyp->P_pintypIv.size() << "] = " << initialiser_2.str().c_str();
         }
         else initialiser << "0,0,";
         if (t_devtyp->P_pintypOv.size())
         {
             initialiser << t_devtyp->P_pintypOv.size() << ",Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_OutputPins,";
             vars_h << "//------------------------------ Output Pin Tables ------------------------------\n";
             vars_h << "extern outPin_t Thread_" << thread_num << "_Device_" << (*device)->Name() << "_OutputPins[" << t_devtyp->P_pintypOv.size() << "];\n";
             initialiser_2.str("{");
             for (vector<P_pintyp*>::iterator pin = t_devtyp->P_pintypOv.begin(); pin != t_devtyp->P_pintypOv.end(); pin++)
             {
                 initialiser_2 << "{0,&Thread_" << thread_num << "_DevTyp_0_OutputPins[" << (*pin)->idx << "],{},0,0,";
                 if ((!(*device)->par->G.FindArcs((*device)->idx,(*pin)->idx,in_pin_idxs,out_pin_idxs)) || !out_pin_idxs.size()) initialiser_2 << "0,0,0,0},";
                 else
                 {
                    vars_h << "extern outEdge_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_OutPin_" << (*pin)->Name().c_str() << "_Tgts[" << out_pin_idxs.size() << "];\n";
                    initialiser_2 << out_pin_idxs.size() << ",Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_OutPin_" << (*pin)->Name().c_str() << "_Tgts,0,0},";
                    // insert target list generation here
                    initialiser_3.str("{");
                    for (vector<unsigned int>::iterator tgt = out_pin_idxs.begin(); tgt != out_pin_idxs.end(); tgt++)
                    {
                        // as for the case of inputs, the target device should be replaced by GetHardwareAddress(...)
                        unsigned int tgt_idx = ((*device)->par->G.index_a.find(*tgt)->second.to_p)->first;
                        P_addr tgt_addr = ((*device)->par->G.index_a.find(*tgt)->second.to_n)->second.data->addr;
                        unsigned tgt_hwaddr = tgt_addr.A_box << (LOG_DEVICES_PER_THREAD + TinselLogThreadsPerCore + TinselLogCoresPerMailbox + TinselLogMailboxesPerBoard + TinselMeshXBits + TinselMeshYBits);
                        tgt_hwaddr |= tgt_addr.A_board << (LOG_DEVICES_PER_THREAD + TinselLogThreadsPerCore + TinselLogCoresPerMailbox + TinselLogMailboxesPerBoard);
                        tgt_hwaddr |= tgt_addr.A_mailbox << (LOG_DEVICES_PER_THREAD + TinselLogThreadsPerCore + TinselLogCoresPerMailbox);
                        tgt_hwaddr |= tgt_addr.A_core << (LOG_DEVICES_PER_THREAD + TinselLogThreadsPerCore);
                        tgt_hwaddr |= tgt_addr.A_thread << (LOG_DEVICES_PER_THREAD);
                        initialiser_3 << "{0," << (tgt_addr.A_device | tgt_hwaddr) << "," << (tgt_idx >> PIN_POS) << "," << (tgt_idx & (0xFFFFFFFF >> (32-PIN_POS))) << "},";
                    }
                    initialiser_3.seekp(-1,ios_base::cur);
                    initialiser_3 << "};\n";
                    vars_cpp << "outEdge_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_OutPin_" << (*pin)->Name().c_str() << "_Tgts[" << out_pin_idxs.size() << "] = " << initialiser_3.str().c_str();
                 }
             }
             initialiser_2.seekp(-1,ios_base::cur);
             initialiser_2 << "};\n";
             vars_cpp << "outPin_t Thread_" << thread_num << "_Device_" << (*device)->Name().c_str() << "_OutputPins[" << t_devtyp->P_pintypOv.size() << "] = " << initialiser_2.str().c_str();
         }
         else initialiser << "0,0,";
         if (t_devtyp->pPropsD)
         {
            initialiser << "&Thread_" << thread_num << "_DeviceProperties[" << index << "],";
            if ((*device)->pPropsI) initialiser_4 << ((*device)->pPropsI->c_src.c_str()) << ",";
            else initialiser_4 << (t_devtyp->pPropsI->c_src.c_str()) << ",";
         }
         else initialiser << "0,";
         if (t_devtyp->pStateD)
         {
            initialiser << "&Thread_" << thread_num << "_DeviceState[" << index << "],";
            if ((*device)->pStateI) initialiser_5 << ((*device)->pStateI->c_src.c_str()) << ",";
            else initialiser_5 << (t_devtyp->pStateI->c_src.c_str()) <<",";
         }
         else initialiser << "0,";
         initialiser << "0,0,0,0,0},";
         ++index;
     }
     initialiser.seekp(-1,ios_base::cur);
     initialiser << "};\n";
     vars_cpp << "devInst_t Thread_" << thread_num << "_Devices[" << thread->P_devicel.size() << "] = " << initialiser.str().c_str();
     if (t_devtyp->pPropsD)
     {
        vars_h << "extern devtyp_" << t_devtyp->Name().c_str() << "_props_t Thread_" << thread_num << "_DeviceProperties[" << thread->P_devicel.size() << "];\n";
        initialiser_4.seekp(-1,ios_base::cur);
        initialiser_4 << "};\n";
        vars_cpp << "devtyp_" << t_devtyp->Name().c_str() << "_props_t Thread_" << thread_num << "_DeviceProperties[" << thread->P_devicel.size() << "] = " << initialiser_4.str().c_str();
     }
     if (t_devtyp->pStateD)
     {
         vars_h << "extern devtyp_" << t_devtyp->Name().c_str() << "_state_t Thread_" << thread_num << "_DeviceState[" << thread->P_devicel.size() << "];\n";
         initialiser_5.seekp(-1,ios_base::cur);
         initialiser_5 << "};\n";
         vars_cpp << "devtyp_" << t_devtyp->Name().c_str() << "_state_t Thread_" << thread_num << "_DeviceState[" << thread->P_devicel.size() << "] = " << initialiser_5.str().c_str();
     }
     vars_cpp.close();
}

//------------------------------------------------------------------------------

void P_builder::CompileBins(P_task * task)
{
     string task_dir(par->taskpath+task->Name()+"/");
     if (!task->linked)
     {
        par->Post(811, task->Name());
        return;
     }
     // create all the necessary build directories
     system(static_cast<stringstream*>(&(stringstream(MAKEDIR, ios_base::out | ios_base::ate)<<" "<<task_dir+COMMON_PATH))->str().c_str());
     system(static_cast<stringstream*>(&(stringstream(MAKEDIR, ios_base::out | ios_base::ate)<<" "<<task_dir+TINSEL_PATH))->str().c_str());
     system(static_cast<stringstream*>(&(stringstream(MAKEDIR, ios_base::out | ios_base::ate)<<" "<<task_dir+ORCH_PATH))->str().c_str());
     system(static_cast<stringstream*>(&(stringstream(MAKEDIR, ios_base::out | ios_base::ate)<<" "<<task_dir+BIN_PATH))->str().c_str());
     system(static_cast<stringstream*>(&(stringstream(MAKEDIR, ios_base::out | ios_base::ate)<<" "<<task_dir+BUILD_PATH))->str().c_str());
     // copy static files to their relevant places: softswitch sources,
     if (system(static_cast<stringstream*>(&(stringstream(SYS_COPY, ios_base::out | ios_base::ate)<<" "<<par->taskpath+COMMON_SRC_PATH<<"/* "<<PERMISSION_CPY<<" "<<RECURSIVE_CPY<<" "<<task_dir+COMMON_PATH))->str().c_str()))
     {
        par->Post(807, (task_dir+COMMON_PATH).c_str());
        return;
     }
     // Tinsel code
     if (system(static_cast<stringstream*>(&(stringstream(SYS_COPY, ios_base::out | ios_base::ate)<<" "<<par->taskpath+TINSEL_SRC_PATH<<"/* "<<PERMISSION_CPY<<" "<<RECURSIVE_CPY<<" "<<task_dir+TINSEL_PATH))->str().c_str()))
     {
         par->Post(807, (task_dir+TINSEL_SRC_PATH).c_str());
         return;
     }
     // Orchestrator code for Mothership
     if (system(static_cast<stringstream*>(&(stringstream(SYS_COPY, ios_base::out | ios_base::ate)<<" "<<par->taskpath+ORCH_SRC_PATH<<"/* "<<RECURSIVE_CPY<<" "<<task_dir+ORCH_PATH))->str().c_str()))
     {
         par->Post(807, (task_dir+ORCH_SRC_PATH).c_str());
         return;
     }
     // Makefile
     if (system(static_cast<stringstream*>(&(stringstream(SYS_COPY, ios_base::out | ios_base::ate)<<" "<<par->taskpath+STATIC_SRC_PATH<<"Makefile "<<task_dir+BUILD_PATH))->str().c_str()))
     {
         par->Post(807, (task_dir+BUILD_PATH).c_str());
         return;
     }
     // kludgey line for C++98; this can be greatly simplified in C++11 using to_string.
     // the makefile will have a target "softswitch_%.elf" which will produce a binary with the
     // (virtual) core number appended. Build failures generate a fatal error as expected.
     // if (system(string(COREMAKE_BASE).append(static_cast<ostringstream*>(&(ostringstream()<<c))->str()).c_str()))
     if (system(static_cast<stringstream*>(&(stringstream("(cd ", ios_base::out | ios_base::ate)<<task_dir+BUILD_PATH<<";"<<COREMAKE_BASE<<")"))->str().c_str()))
     {
        par->Post(805);
        return;
     }
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
}

//==============================================================================
#endif

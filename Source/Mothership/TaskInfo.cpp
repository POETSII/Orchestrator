#include "TaskInfo.h"

const unsigned char TaskInfo_t::TASK_IDLE;
const unsigned char TaskInfo_t::TASK_BOOT;
const unsigned char TaskInfo_t::TASK_RDY;
const unsigned char TaskInfo_t::TASK_BARR;
const unsigned char TaskInfo_t::TASK_RUN;
const unsigned char TaskInfo_t::TASK_STOP;
const unsigned char TaskInfo_t::TASK_END;
const unsigned char TaskInfo_t::TASK_ERR;
const map<unsigned char,string> TaskInfo_t::Task_Status({{TaskInfo_t::TASK_IDLE,"TASK_IDLE"},{TaskInfo_t::TASK_BOOT,"TASK_BOOT"},{TaskInfo_t::TASK_RDY,"TASK_RDY"},{TaskInfo_t::TASK_BARR,"TASK_BARR"},{TaskInfo_t::TASK_RUN,"TASK_RUN"},{TaskInfo_t::TASK_STOP,"TASK_STOP"},{TaskInfo_t::TASK_END,"TASK_END"},{TaskInfo_t::TASK_ERR,"TASK_ERR"}});

TaskInfo_t::TaskInfo_t(string name) : TaskName(name), BinPath(), CoreMap(), VCoreMap()
{
    VirtualBox = 0;
    status = TASK_IDLE;

    // Defining the hardware address format, which dictates the spacing between
    // levels of the hierarchy. This should be passed in from the root process
    // when it reads the hardware model, but it's hardcoded for now. <!>
    hardwareAddressFormat.boxWordLength = 0;
    hardwareAddressFormat.boardWordLength = 99;
    hardwareAddressFormat.mailboxWordLength = 99;
    hardwareAddressFormat.coreWordLength = 99;
    hardwareAddressFormat.threadWordLength = 99;
}

TaskInfo_t::~TaskInfo_t()
{
    delete VirtualBox;
    WALKMAP(P_board*,vector<BinPair_t>*,binaries,bBins)
      delete bBins->second;
}

// sets the core value for a virtual core, provided this core number exists
bool TaskInfo_t::setCore(uint32_t vCore, P_core* core)
{
     if (VCoreMap.find(vCore) == VCoreMap.end()) return false; // virtual core exists?
     if (CoreMap.find(core) == CoreMap.end()) // new physical core?
     {
        // Get a P_addr_t for this core.
        P_addr coreSoftAddress;
        core->get_hardware_address()->
            populate_a_software_address(&coreSoftAddress);
        insertCore(vCore, coreSoftAddress); // add a new core in the usual way
        return true;
     }
     CoreMap[core] = vCore;     // otherwise set both maps
     VCoreMap[vCore] = core;
     return true;
}

// sets the virtual core index for a core, provided this core has been mapped.
bool TaskInfo_t::setCore(P_core* core, uint32_t vCore)
{
     if (CoreMap.find(core) == CoreMap.end()) return false; // core exists?
     VCoreMap[vCore] = core;      // set both maps.
     CoreMap[core] = vCore;
     return true;
}

// absolute setter for a core - inserts if none exists, overwrites if it is already in the map
void TaskInfo_t::insertCore(uint32_t vCore, P_addr_t coreID)
{
    // printf("Finding space for a core with %u threads, and with the "
    //        "following address components:\n  box: %u\n  board: %u\n  "
    //        "mailbox: %u\n  core: %u\n",
    //        coreID.A_thread + 1, coreID.A_box, coreID.A_board,
    //        coreID.A_mailbox, coreID.A_core);
    // fflush(stdout);
    if (VirtualBox == 0) // no box yet. Assume this core insertion defines our box number.
    {
        DebugPrint("Creating new 'virtual' box.\n");
        VirtualBox = new P_box("VirtualBox");
        VirtualBox->AutoName(TaskName+"_Box_");

        // Setting up the address of the box. This address will propagate to
        // items (boards, mailboxes, etc.) when they are contained.
        HardwareAddress* boxHardwareAddress = new HardwareAddress(
            &hardwareAddressFormat);  // This will be deleted when the box is
                                      // cleaned up.
        boxHardwareAddress->set_box(coreID.A_box);
        VirtualBox->set_hardware_address(boxHardwareAddress);

        DebugPrint("Inserted VirtualBox %s.\n", VirtualBox->Name().c_str());
    }

    // Defense against multiple cores with different addresses.
    if (coreID.A_box != VirtualBox->get_hardware_address()->get_box())
    {
        DebugPrint("Core ID box address component (%d) does not match "
                   "VirtualBox box address component (%d). Not inserting this "
                   "core.\n",
                   coreID.A_box,
                   VirtualBox->get_hardware_address()->get_box());
        return; // not our box. Ignore.
    }
    P_board* VirtualBoard = 0;
    WALKVECTOR(P_board*, VirtualBox->P_boardv, B)
    {
        if ((*B)->get_hardware_address()->get_board() == coreID.A_board)
        {
            DebugPrint("Using VirtualBoard %s.\n",(*B)->Name().c_str());
            VirtualBoard = (*B);
            break;
        }
    }
    if (!VirtualBoard) // no existing board matches the core. Create a new one.
    {
        DebugPrint("Creating 'virtual' board.\n");
        VirtualBoard = new P_board("VirtualBoard");
        VirtualBox->contain(coreID.A_board, VirtualBoard);
        VirtualBoard->AutoName(VirtualBox->Name()+"_Board_");
        DebugPrint("Inserted VirtualBoard %s\n", VirtualBoard->Name().c_str());
    }
    P_mailbox* VirtualMailbox = 0;
    WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                      unsigned, P_link*,
                      unsigned, P_port*, VirtualBoard->G, MB)
    {
        if (VirtualBoard->G.NodeData(MB)->get_hardware_address()->get_mailbox() == coreID.A_mailbox)
        {
            DebugPrint("Using VirtualMailbox %s\n",
                       VirtualBoard->G.NodeData(MB)->Name().c_str());
            VirtualMailbox = VirtualBoard->G.NodeData(MB);
            break;
        }
    }
    if (!VirtualMailbox) // no existing mailbox matches the core. Create a new one.
    {
        DebugPrint("Creating 'virtual' mailbox.\n");
        VirtualMailbox = new P_mailbox("VirtualMailbox");
        VirtualBoard->contain(coreID.A_mailbox, VirtualMailbox);
        VirtualMailbox->AutoName(VirtualBoard->Name()+"_Mailbox_");
        DebugPrint("Inserted VirtualMailbox %s.\n",
                   VirtualMailbox->Name().c_str());
    }

    softMap_t::iterator C = VCoreMap.find(vCore);
    if (C != VCoreMap.end())
    {
        // core already exists. No need to add.
        if ((C->second->get_hardware_address()->get_box() == VirtualBox->get_hardware_address()->get_box()) &&
            (C->second->get_hardware_address()->get_board() == VirtualBoard->get_hardware_address()->get_board()) &&
            (C->second->get_hardware_address()->get_mailbox() == VirtualMailbox->get_hardware_address()->get_mailbox()) &&
            (C->second->get_hardware_address()->get_core() == coreID.A_core))
        {
            // printf("After all that, the core was already here. Not "
            //        "inserting it.\n");
            // fflush(stdout);
            return;
        }
       // A mapped core with the same virtual number is already in the table; get rid of it.
       CoreMap.erase(C->second);
       removeCore(C->second);
    }
    // insert the new core with its appropriate address fields.
    // printf("Creating 'virtual' core.\n");
    // fflush(stdout);
    P_core* VirtualCore = new P_core("VirtualCore");
    VirtualMailbox->contain(coreID.A_core, VirtualCore);
    VirtualCore->AutoName(VirtualMailbox->Name()+"_Core_");
    // printf("Inserted VirtualCore %s\n", VirtualCore->Name().c_str());
    // fflush(stdout);

    // generate the threads for the core.
    P_thread* VirtualThread;
    for (unsigned thread = 0; thread <= coreID.A_thread; thread++)
    {
        // printf("Creating 'virtual' thread.\n");
        // fflush(stdout);
        VirtualThread = new P_thread("VirtualThread");
        VirtualCore->contain(thread, VirtualThread);
        VirtualThread->AutoName(VirtualCore->Name()+"_Thread_");
        // printf("Inserted VirtualThread %s\n",
        //        VirtualThread->Name().c_str());
        // fflush(stdout);
    }

    // then map to the task.
    VCoreMap[vCore] = VirtualCore;
    CoreMap[VirtualCore] = vCore;
}

// remove a core from the map (by virtual ID)
void TaskInfo_t::deleteCore(uint32_t vCore)
{
     softMap_t::iterator C;
     if ((C = VCoreMap.find(vCore)) == VCoreMap.end()) return; // core isn't mapped
     P_core* core = C->second;
     CoreMap.erase(C->second); // remove from both maps
     VCoreMap.erase(C);
     removeCore(core);          // and then remove from the VirtualBox
}

// remove a core from the map (by physical core)
void TaskInfo_t::deleteCore(P_core* core)
{
     hardMap_t::iterator C;
     if ((C = CoreMap.find(core)) == CoreMap.end()) return; // core isn't mapped
     VCoreMap.erase(C->second); // remove from both maps
     CoreMap.erase(C);
     removeCore(core);           // and then remove from the VirtualBox.
}

// remove a core from the VirtualBox
void TaskInfo_t::removeCore(P_core* core)
{
    std::map<AddressComponent, P_core*>::iterator coreIterator;
    for (coreIterator=core->parent->P_corem.begin();
         coreIterator!=core->parent->P_corem.end(); coreIterator++)
    {
        if (coreIterator->second == core)
        {
            core->parent->P_corem.erase(coreIterator); // very inefficient but rare: remove old mapped core
            break;                                     // and end the search
        }
    }
    delete core; // once removed destroy the object.
}

// Populates the "cores" member of this object using the Virtual hardware
// stack, if "cores" has not already been populated.
void TaskInfo_t::populateCoreVector()
{
    P_board* VirtualBoard;
    P_mailbox* VirtualMailbox;
    P_core* VirtualCore;
    if (!cores.size())
    {
        WALKVECTOR(P_board*, VirtualBox->P_boardv, B)
        {
            VirtualBoard = (*B);
            WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                              unsigned, P_link*,
                              unsigned, P_port*, VirtualBoard->G, MB)
            {
                VirtualMailbox = VirtualBoard->G.NodeData(MB);
                WALKMAP(AddressComponent, P_core*, VirtualMailbox->P_corem, CO)
                {
                    VirtualCore = CO->second;
                    cores.push_back(VirtualCore);
                }
            }
        }
   }
}

// Populates the "threads" member of this object, if it has not already been
// populated.
void TaskInfo_t::populateThreadVector()
{
    P_core* VirtualCore;
    P_thread* VirtualThread;
    if (!threads.size())
    {
        populateCoreVector();
        WALKVECTOR(P_core*, cores, CO)
        {
            VirtualCore = (*CO);
            WALKMAP(AddressComponent, P_thread*, VirtualCore->P_threadm, TH)
            {
                VirtualThread = TH->second;
                threads.push_back(VirtualThread);
            }
        }
    }
}

// Populates the "devices" member of this object, if it has not already been
// populated.
void TaskInfo_t::populateDeviceVector()
{
    P_thread* VirtualThread;
    P_device* VirtualDevice;
    if (!devices.size())
    {
        populateThreadVector();
        WALKVECTOR(P_thread*, threads, TH)
        {
            VirtualThread = (*TH);
            WALKLIST(P_device*, VirtualThread->P_devicel, DV)
            {
                VirtualDevice = (*DV);
                devices.push_back(VirtualDevice);
            }
        }
    }
}

vector<P_core*>& TaskInfo_t::CoresForTask()
{
    populateCoreVector();
    return cores;
}

vector<P_thread*>& TaskInfo_t::ThreadsForTask()
{
    populateThreadVector();
    return threads;
}

vector<P_device*>& TaskInfo_t::DevicesForTask()
{
    populateDeviceVector();
    return devices;
}

vector<BinPair_t>& TaskInfo_t::BinariesForBoard(P_board* board)
{
    P_mailbox* VirtualMailbox;
    if (binaries.find(board) == binaries.end())
    {
        BinPair_t core_bins;
        binaries[board] = new vector<BinPair_t>;
        WALKPDIGRAPHNODES(AddressComponent, P_mailbox*,
                          unsigned, P_link*,
                          unsigned, P_port*, board->G, MB)
        {
            VirtualMailbox = board->G.NodeData(MB);
            WALKMAP(AddressComponent, P_core*, VirtualMailbox->P_corem, CO)
            {
                core_bins.instr = CO->second->instructionBinary;
                core_bins.data = CO->second->dataBinary;
                binaries[board]->push_back(core_bins);
            }
        }
    }
    return *binaries[board];
}

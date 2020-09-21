#include "PlacementLoader.h"

/* Very, very unsafe. */

PlacementLoader::PlacementLoader(Placer* placer,
                                 std::string path):Algorithm(placer)
{
    filePath = path;
    result.method = "load";
    load_file();
}

float PlacementLoader::do_it(GraphI_t* gi)
{
    DevI_t* device;
    std::string threadName;
    HardwareIterator* hwIt;

    /* Go over each device in turn */
    WALKPDIGRAPHNODES(unsigned, DevI_t*, unsigned, EdgeI_t*, unsigned,
                      PinI_t*, gi->G, deviceIterator)
    {
        hwIt = new HardwareIterator(placer->engine);

        device = gi->G.NodeData(deviceIterator);

        /* Ignore if it's not a normal device (we don't map those). */
        if (device->devTyp != 'D') continue;

        /* Get the thread name. */
        threadName = dataFromFile[device->FullName()];

        /* Look naively. */
        while (threadName != hwIt->get_thread()->FullName())
        {
            hwIt->next_thread();
            if (hwIt->has_wrapped())
            {
                printf("We couldnt place device '%s' to thread '%s', because "
                       "we couldn't find the thread.\n",
                       device->FullName().c_str(), threadName.c_str());
                break;
            }
        }

        placer->link(hwIt->get_thread(), device);
        delete hwIt;
    }

    return 0;
}

void PlacementLoader::load_file()
{
    std::string recordBuffer, device, thread;
    std::stringstream recordStream;
    std::ifstream dataStream;
    dataStream.open(filePath);
    if (!dataStream.is_open())
    {
        throw FileOpenException(
            dformat("File: %s. Message: %s",
                    filePath, OSFixes::getSysErrorString(errno).c_str()));
    }

    /* For each line... */
    while(std::getline(dataStream, recordBuffer))
    {
        /* Add a record. */
        device = "";
        thread = "";
        recordStream.str(recordBuffer.c_str());
        recordStream.seekg(0);
        std::getline(recordStream, device, ',');
        std::getline(recordStream, thread);
        dataFromFile[device] = thread;
    }

    dataStream.close();
}

#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILEPARSER_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_HARDWAREFILEMANAGER_HARDWAREFILEPARSER_H

/* Logic for parsing hardware model files, and for generating hardware models
 * from them. Example:
 *
 *     P_engine* engine = new P_engine("My engine");
 *     parser = HardwareFileParser("/path/to/my/hardware/file.uif", engine);
 *     // We're done here.
 *
 * Or, for more points:
 *
 *     P_engine* engine = new P_engine("My engine");
 *     parser = HardwareFileParser;
 *     try
 *     {
 *         parser.loadFile("/path/to/my/hardware/file.uif");
 *         parser.populateHardwareModel(engine);
 *     }
 *     catch (OrchestratorException& exception)
 *     {
 *         printf("%s\n", exception.what())
 *     }
 *     // We're done here.
 *
 * Hardware model files are UIF files that fully define a model of the
 * hardware, which is suitable for the Orchestrator's purposes.
 *
 * Rougly speaking, the conversion from a hardware model file to a hardware
 * model follows this procedure:
 *
 *  1. Load the file.
 *  2. Parse the file using the UIF parser, and generate a token tree. Raises
 *     if the file is syntactically invalid.
 *  3. Parse the token tree, and generate a hardware stack (PoetsEngine
 *     populated with components). Raises if the file is semantically invalid.
 *  ...
 *  From which the programmer can use the hardware model as they desire (for
 *  the most part, this means putting it into the Orchestrator, but do what you
 *  will).
 *
 * The HardwareFileParser class encapsulates this process (I wouldn't normally
 * do it this way, but the recommended approach for using UIF is to use it as a
 * base class, so...). Only one file can be loaded by this parser at a time;
 * loading a second file clobbers the data structure of the first.
 *
 * See the hardware model documentation for a description of the file
 * format. */

#include <numeric>
#include <string.h>
#include <vector>

#include "HardwareAddressFormat.h"
#include "HardwareAddress.h"
#include "HardwareFileNotFoundException.h"
#include "HardwareFileNotLoadedException.h"
#include "HardwareModel.h"
#include "HardwareSyntaxException.h"

#include "dfprintf.h"
#include "jnj.h"  /* JNJ is a wrapper around UIF that provides nicer access to
                   * the UIF data structure. */

class HardwareFileParser: public JNJ
{
public:
    HardwareFileParser();
    HardwareFileParser(const char* filePath, P_engine* engine);
    void loadFile(const char* filePath);
    void populateHardwareModel(P_engine* engine);

private:
    bool isFileLoaded;
    std::string loadedFile;
    bool doesFileExist(const char* filePath);
    static void onSyntaxError(void*, void*, int);
};

#endif

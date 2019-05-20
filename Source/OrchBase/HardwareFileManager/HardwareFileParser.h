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
 *         parser.load_file("/path/to/my/hardware/file.uif");
 *         parser.populate_hardware_model(engine);
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
 * Roughly speaking, the conversion from a hardware model file to a hardware
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
 * As part of this process, the input file is validated. Points that are not
 * currently validated include:
 *
 * - Lines within a section that are simply one word, without the assignment
 *   operator (=) or the prefix (+). These are simply ignored.
 *
 * - Multidimensional input components are not matched against their respective
 *   address word lengths, either using their dimensionality or their value.
 *
 * - Repeatedly-defined values within a section - only the last defined value
 *   is used.
 *
 * The contents of the engine are only replaced if validation succeeds. This
 * replacement requires the engine to be dynamically allocated.
 *
 * See the hardware model documentation for a description of the file
 * format. */

#include <numeric>
#include <string.h>
#include <vector>

#include "Dialect1Deployer.h"
#include "HardwareAddressFormat.h"
#include "HardwareAddress.h"
#include "HardwareFileNotFoundException.h"
#include "HardwareFileNotLoadedException.h"
#include "HardwareModel.h"
#include "HardwareSemanticException.h"
#include "HardwareSyntaxException.h"

#include "dfprintf.h"
#include "flat.h"
#include "jnj.h"  /* JNJ is a wrapper around UIF that provides nicer access to
                   * the UIF data structure. */

class HardwareFileParser: public JNJ
{
public:
    HardwareFileParser();
    HardwareFileParser(const char* filePath, P_engine* engine);
    void load_file(const char* filePath);
    void populate_hardware_model(P_engine* engine);

private:
    bool isFileLoaded;
    std::string loadedFile;
    bool does_file_exist(const char* filePath);
    void set_uif_error_callback();
    static void on_syntax_error(void*, void*, int);

    /* Validation methods. */
    bool validate_section_contents(std::string* errorMessage);
    bool validate_sections(std::string* errorMessage);
    bool provision_deployer(Dialect1Deployer* deployer,
                            std::string* errorMessage);

    /* Node-checking methods. */
    bool is_value_at_node_natural(UIF::Node* recordNode, UIF::Node* valueNode,
                                  std::string variable, std::string value,
                                  std::string sectionName,
                                  std::string* errorMessage);
    bool is_value_at_node_floating(UIF::Node* recordNode, UIF::Node* valueNode,
                                   std::string variable, std::string value,
                                   std::string sectionName,
                                   std::string* errorMessage);
    bool are_values_at_node_natural(UIF::Node* recordNode,
                                    UIF::Node* valueNode, std::string variable,
                                    std::string value, std::string sectionName,
                                    std::string* errorMessage);
    void invalid_variable_message(std::string* errorMessage,
                                  UIF::Node* recordNode,
                                  std::string sectionName,
                                  std::string variable);
};

float str2float(std::string floatLike);
unsigned str2unsigned(std::string unsignedLike);

#endif

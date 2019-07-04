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
 *  2. Parse the file using the UIF parser, and generate a tree. Raises if the
 *     file is syntactically invalid.
 *  3. Traverse the generated tree, and generate a hardware stack (P_engine
 *     populated with components). Raises if the file is semantically invalid.
 *
 *  From which the programmer can use the hardware model as they desire (for
 *  the most part, this means putting it into the Orchestrator, but do what you
 *  will).
 *
 * The HardwareFileParser class encapsulates this process (I wouldn't normally
 * do it this way, but the recommended approach for using UIF is to use it as a
 * base class, so...). Only one file can be loaded by this parser at a time;
 * loading a second file clobbers the data structure of the first.
 *
 * As part of this process, the input file is validated. In dialect 1, things
 * that are not currently validated include:
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
 * The contents of the engine are only defined if validation succeeds. This
 * replacement requires the engine to be dynamically allocated.
 *
 * The the methods of the class that this header declares are defined in
 * multiple files, depending on the dialect they interact with.
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
#include "Validation.h"

#include "dfprintf.h"
#include "flat.h"
#include "jnj.h"  /* JNJ is a wrapper around UIF that provides nicer access to
                   * the UIF data structure. */

/* Some structs and typedefs used to streamline the structures used by the
 * dialect 3 parser for vaidation:
 *
 * - BoardInfo: Holds information on what line in the input file a board was
 *   declared, and where the board object is.
 *
 * - MailboxInfo: Holds information on what line in the input file a mailbox
 *   was declared, and where the mailbox object is.
 *
 * - EdgeInfo: Holds information on the determined weight of an edge, whether
 *   its reverse-direction has been declared (yet), and the line in the input
 *   file where its instance is first mentioned.
 *
 * - BoardName: Holds the name of a board. The first string in the pair is the
 *   name of the box containing the boards. The second string in the pair is
 *   the name of the board in the box. This is a concession to the fact that
 *   board names only need to be unique within their box.
 *
 * - MailboxName: Holds the name of a mailbox. It's relative.
 */
struct BoardInfo{int lineNumber; P_board* memoryAddress;};
struct MailboxInfo{int lineNumber; P_mailbox* memoryAddress;};
struct EdgeInfo{float weight; bool isReverseDefined; unsigned lineNumber;};

typedef std::pair<std::string, std::string> BoardName;
typedef std::string MailboxName;

#define ITEM_ENUM_LENGTH 3
enum item {box, board, mailbox};

class HardwareFileParser: public JNJ
{
public:
    HardwareFileParser();
    HardwareFileParser(const char* filePath, P_engine* engine);
    void load_file(const char* filePath);
    void populate_hardware_model(P_engine* engine);

private:
    /* Dialect-independent methods */
    bool isFileLoaded;
    std::string loadedFile;
    bool does_file_exist(const char* filePath);
    unsigned get_dialect();
    void get_values_as_strings(std::vector<std::string>* toPopulate,
                               UIF::Node* valueNode);
    void invalid_variable_message(std::string* errorMessage,
                                  UIF::Node* recordNode,
                                  std::string sectionName,
                                  std::string variable);

    /* UIF-syntax error methods */
    static void on_syntax_error(void*, void*, int);
    void set_uif_error_callback();

    /* Dialect 1 validation and deployment methods. */
    void d1_populate_hardware_model(P_engine* engine);
    bool d1_provision_deployer(Dialect1Deployer* deployer,
                               std::string* errorMessage);
    bool d1_validate_section_contents(std::string* errorMessage);
    bool d1_validate_sections(std::string* errorMessage);

    /* Dialect 3 validation and deployment members and methods. */
    bool d3_define_box_fields_from_section(P_box* box, UIF::Node* sectionNode);
    bool d3_get_address_from_item_definition(UIF::Node* itemNode,
                                             AddressComponent* address);
    bool d3_get_board_name(UIF::Node* itemNode, BoardName* boardName);
    bool d3_get_explicit_type_from_item_definition(UIF::Node* itemNode,
                                                   std::string* type);
    bool d3_get_section_from_type(std::string itemType, std::string type,
                                  std::string sourceSectionName,
                                  unsigned lineNumber,
                                  UIF::Node** sectionNode);
    bool d3_get_validate_default_types(UIF::Node** globalDefaults);
    bool d3_load_validate_sections();
    void d3_populate_hardware_model(P_engine* engine);
    bool d3_populate_validate_address_format(P_engine* engine);
    bool d3_populate_validate_engine_box(P_engine* engine);
    bool d3_populate_validate_header(P_engine* engine);
    bool d3_validate_types_define_cache();

    /* Holds error messages from validation. */
    std::string d3_errors;

    /* Holds UIF sections that have no types. The key is the "sort" of section
     * it is (i.e. 'header', 'engine_box'), and the value is the UIF node that
     * corresponds to that section. */
    std::map<std::string, UIF::Node*> untypedSections;

    /* Holds UIF sections that have types. The key is the "sort" of section it
     * is, and the value is another map whose value is the "type" of the
     * section, and whose value id the UIF node that corresponds to that
     * section. */
    std::map<std::string, std::map<std::string, UIF::Node*>> typedSections;

    /* Holds, for a (former) section, the type-specific section that defines
     * how items created in the former section behave. The key is the section
     * that defines the items, and the value is the section that defines the
     * properties of that item. If no valid value exists, it is zero. This is
     * just a convenient cache mechanism. Examples:
     *
     * 1. Given:
     *
     *        [board(SomeBoardType)]
     *        +type=SomeMailboxType
     *
     *    Then the first element of the map pair will be a pointer to the
     *    [board(SomeBoardType)] (section) UIF node, and the second will be a
     *    pointer to the [mailbox(SomeMailboxType)] (section) UIF node.
     *
     * 2. Given:
     *
     *        [board(SomeBoardType)]
     *        ... // With no +type field
     *        [default_types]
     *        ...
     *        +mailbox_type=SomeMailboxType
     *
     *    Then the entry in the map will be identical to case 1.
     *
     * 3. Given:
     *
     *        [board(SomeBoardType)]
     *        +type=SomeMailboxType1
     *        [default_types]
     *        ...
     *        +mailbox_type=SomeMailboxType2
     *
     *    Then the first element of the map pair will be a pointer to the
     *    [board(SomeBoardType)] (section) UIF node, and the second will be a
     *    pointer to the [mailbox(SomeMailboxType1)] (section) UIF node.
     *
     * 4. Given:
     *
     *        [board(SomeBoardType)]
     *        ... // With no +type field
     *        [default_types]
     *        ... // With no +mailbox_type field
     *
     *    Then the first element of the map pair will be a pointer to the
     *    [board(SomeBoardType)] (section) UIF node, and the second will be
     *    zero. Note that this is perfectly valid - just that each mailbox
     *    declaration must explicitly define a valid type. */
    std::map<UIF::Node*, UIF::Node*> defaultTypes;

    /* Holds sizes of address components. The key is either "box", "board", or
     * "mailbox", and the value is the number of elements comprising that
     * address component. */
    std::map<std::string, unsigned> addressLengths;

    /* Holds boards that have been declared to exist within a box in
     * [engine_box], but which have not (yet) been created from parsing
     * [engine_board]. */
    std::list<BoardName> undefinedBoards;

    /* Holds all boxes, indexable by their name. The key is the unique name of
     * the box, and the value points to the box object itself. */
    std::map<std::string, P_box*> boxFromName;

    /* Holds information on all boards. The key is the unique name of the
     * board, and the value holds the information about that board. */
    std::map<BoardName, BoardInfo> boardInfoFromName;

    /* Holds information on all mailboxes in the current board. The key is the
     * name of the mailbox in this board, and the value holds the information
     * about that mailbox. */
    std::map<MailboxName, MailboxInfo> mailboxInfoFromName;

    /* Holds the edge that connects to boards together, if any. The two
     * elements of the pair held in the key of the map represent the boards at
     * each end of the edge, and the value of the map holds the edge
     * information. */
    std::map<std::pair<BoardName, BoardName>, EdgeInfo> boardEdges;

    /* Holds the edge that connects to mailboxes together in a given board, if
     * any. The two elements of the pair held in the key of the map represent
     * the mailboxes at each end of the edge, and the value of the map holds
     * the mailbox information. */
    std::map<std::pair<MailboxName, MailboxName>, EdgeInfo> mailboxEdges;
};
#endif

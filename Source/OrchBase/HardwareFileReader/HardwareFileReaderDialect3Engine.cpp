/* Defined behaviour for the dialect 3 functionality of methods that directly
 * modify either the P_engine object, or its address format. */

#include "HardwareFileReader.h"

/* Validate the contents of the packet_address_format section, and populate the
 * format object in the engine with them.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate */
bool HardwareFileReader::d3_populate_validate_address_format(P_engine* engine)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    sectionName = "packet_address_format";

    /* Valid fields for the header section. */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("box");
    validFields.push_back("board");
    validFields.push_back("mailbox");
    validFields.push_back("core");
    validFields.push_back("thread");

    /* Mandatory fields for the header section. Could be indeces that refer to
     * validFields, but this is (probably) easier to read/maintain. */
    std::vector<std::string> mandatoryFields;
    mandatoryFields.push_back("board");
    mandatoryFields.push_back("mailbox");
    mandatoryFields.push_back("core");
    mandatoryFields.push_back("thread");

    /* We live in a world where the box component of the address may or may not
     * be mandatory. */
    if (!IGNORE_BOX_COMPONENT)
    {
        mandatoryFields.push_back("box");
    }

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Staging vectors for holding value nodes and variable nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Staging area for the values of multi-valued records (we need to add them
     * together). */
    std::vector<std::string> values;
    std::vector<std::string>::iterator valueIterator;
    unsigned* accumulationTarget;  /* This will point to a _WordLength member
                                    * of the address format object owned by the
                                    * engine. */

    /* Iterate through all record nodes in this section. */
    std::vector<UIF::Node*> recordNodes;
    GetRecd(untypedSections[sectionName], recordNodes);

    std::vector<UIF::Node*>::iterator recordIterator;
    for (recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complaints */
        if (!(complain_if_variable_not_plus_prefixed(variableNodes[0]) and
              complain_if_variable_name_invalid(variableNodes[0],
                                                &validFields) and
              complain_if_record_is_multivariable(&variableNodes)))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        if (complain_if_variable_true_in_map(variableNodes[0], &fieldsFound))
        {
            anyErrors = true;
            continue;
        }
        variable = variableNodes[0]->str;
        fieldsFound[variable] = true;

        /* Specific logic for each variable. For box, core, and thread complain
         * if (in order):
         *
         * - The record is multi-valued.
         * - The value is not natural. */
        if (variable == "box" or variable == "core" or
            variable == "thread")
        {

            /* Complaints */
            if (!(complain_if_record_is_multivalue(&valueNodes) and
                  complain_if_value_not_natural(valueNodes[0])))
            {
                anyErrors = true;
                continue;
            }

            /* We bind! */
            if (variable == "box")
            {
                engine->addressFormat.boxWordLength =
                    str2unsigned(valueNodes[0]->str);
            }
            else if (variable == "core")
            {
                engine->addressFormat.coreWordLength =
                    str2unsigned(valueNodes[0]->str);
            }
            else /* Must be thread */
            {
                engine->addressFormat.threadWordLength =
                    str2unsigned(valueNodes[0]->str);
            }
        }

        /* For board and mailbox, complain if any of the values are not
         * natural. */
        else if (variable == "board" or variable == "mailbox")
        {
            /* Validate, treating the single-variable and multiple-variable
             * cases separately. */
            get_values_as_strings(&values, valueNodes[0]);
            if (values.size() == 1)
            {
                if (!complain_if_value_not_natural(valueNodes[0]))
                {
                    anyErrors = true;
                    continue;
                }
            }
            else
            {
                if (!complain_if_values_and_children_not_natural(
                        valueNodes[0]))
                {
                    anyErrors = true;
                    continue;
                }
            }

            /* We bind! (is either board or mailbox) */

            /* Determine target */
            accumulationTarget =
                &(variable == "board" ?
                  engine->addressFormat.boardWordLength :
                  engine->addressFormat.mailboxWordLength);
            *accumulationTarget = 0;  /* Reset target */

            /* Add values to target. The total word length is (simply) the sum
             * of its multidimensional constituents. */
            for (valueIterator=values.begin(); valueIterator!=values.end();
                 valueIterator++)
            {
                *accumulationTarget += str2unsigned(*valueIterator);
            }
        }
    }

    /* Ensure mandatory fields have been defined. */
    if (!complain_if_mandatory_field_not_defined(&mandatoryFields,
                                                 &fieldsFound))
    {
        anyErrors = true;
    }

    return !anyErrors;
}

/* Validate the contents of the header section, and populate an engine with
 * them.
 *
 * Returns true if all validation checks pass, and false otherwise. Arguments:
 *
 * - engine: Engine to populate */
bool HardwareFileReader::d3_populate_validate_header(P_engine* engine)
{
    bool anyErrors = false;  /* Innocent until proven guilty. */
    sectionName = "header";

    /* Valid fields for the header section. */
    std::vector<std::string> validFields;
    std::vector<std::string>::iterator fieldIterator;
    validFields.push_back("author");
    validFields.push_back("datetime");
    validFields.push_back("dialect");
    validFields.push_back("file");
    validFields.push_back("hardware");
    validFields.push_back("version");

    /* Mandatory fields for the header section. Could be indeces that refer to
     * validFields, but this is (probably) easier to read/maintain. */
    std::vector<std::string> mandatoryFields;
    mandatoryFields.push_back("datetime");
    mandatoryFields.push_back("dialect");
    mandatoryFields.push_back("version");

    /* Holds fields we've already grabbed (for validation purposes). */
    std::map<std::string, bool> fieldsFound;
    for (fieldIterator=validFields.begin(); fieldIterator!=validFields.end();
         fieldIterator++)
    {
        fieldsFound.insert(std::make_pair(*fieldIterator, false));
    }

    /* Temporary staging vectors for holding value nodes and variable
     * nodes. */
    std::vector<UIF::Node*> valueNodes;
    std::vector<UIF::Node*> variableNodes;

    /* Holds whether or not the engine has been given a name. */
    bool isEngineNamed = false;

    /* Iterate through all record nodes in this section. */
    std::vector<UIF::Node*> recordNodes;
    GetRecd(untypedSections[sectionName], recordNodes);

    for (std::vector<UIF::Node*>::iterator recordIterator=recordNodes.begin();
         recordIterator!=recordNodes.end(); recordIterator++)
    {
        record = *recordIterator;

        /* Get the value and variable nodes. */
        GetVari(record, variableNodes);
        GetValu(record, valueNodes);

        /* Ignore this record if the record has not got a variable/value
         * pair (i.e. if the line is empty, or is just a comment). */
        if (variableNodes.size() == 0 and valueNodes.size() == 0){continue;}

        /* Complaints */
        if (!(complain_if_variable_not_plus_prefixed(variableNodes[0]) and
              complain_if_variable_name_invalid(variableNodes[0],
                                                &validFields) and
              complain_if_record_is_multivariable(&variableNodes) and
              complain_if_record_is_multivalue(&valueNodes)))
        {
            anyErrors = true;
            continue;
        }

        /* Complain if duplicate. NB: We know the variable name is valid if
         * control has reached here. */
        if (complain_if_variable_true_in_map(variableNodes[0], &fieldsFound))
        {
            anyErrors = true;
            continue;
        }
        variable = variableNodes[0]->str;
        fieldsFound[variable] = true;

        /* Specific logic for each variable. */
        if (variable == "author")
        {
            engine->author = valueNodes[0]->str;
        }

        else if (variable == "datetime")
        {
            /* Special validation for this one. */
            if (valueNodes[0]->qop != Lex::Sy_ISTR)
            {
                errors.push_back(dformat(
                    "L%u: Variable '%s' in the '%s' section has value '%s', "
                    "which is not a datetime in the form YYYYMMDDhhmmss.",
                    record->pos, variable.c_str(),
                    valueNodes[0]->str.c_str(), sectionName.c_str()));
                anyErrors = true;
            }

            else
            {
                engine->datetime = str2long(valueNodes[0]->str);
            }

        }

        /* This has already been read and validated, so we don't care. */
        else if (variable == "dialect");

        else if (variable == "file")
        {
            engine->fileOrigin = valueNodes[0]->str;
            engine->Name(valueNodes[0]->str);
            isEngineNamed = true;
        }

        /* Ignore this one for now. */
        else if (variable == "hardware"){}

        else if (variable == "version")
        {
            engine->version = valueNodes[0]->str;
        }
    }

    /* Ensure mandatory fields have been defined. */
    if (!complain_if_mandatory_field_not_defined(&mandatoryFields,
                                                 &fieldsFound))
    {
        anyErrors = true;
    }

    /* If the engine has not been named, give it a default name. */
    if (!isEngineNamed)
    {
        engine->Name(DEFAULT_ENGINE_NAME);
    }

    return !anyErrors;
}

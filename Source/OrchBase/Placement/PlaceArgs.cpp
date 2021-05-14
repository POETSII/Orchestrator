/* Defines the placement argument-holder (see the accompanying header for
 * further information). */

#include <sstream>

#include "PlaceArgs.h"
#include "dprintf.h"
#include "flat.h"

/* Gets the value of a boolean argument. Throws an InvalidArgumentException
 * if the argument is not set, does not exist, or is not a boolean argument. */
bool PlaceArgs::get_bool(std::string name)
{
    /* Is the argument boolean? */
    std::map<std::string, std::string>::iterator typeIt;
    typeIt = validTypes.find(name);
    InvalidArgumentException exc = InvalidArgumentException(
        dformat("Invalid argument '%s' is not boolean.", name.c_str()));
    if (typeIt == validTypes.end()) throw exc;
    if (typeIt->second != "bool") throw exc;

    /* Is the value set? */
    if (args[name].empty())
        throw InvalidArgumentException(
            dformat("Argument '%s' has not been set.", name.c_str()));

    /* Get it and decode it. */
    return (args[name] == "True" or args[name] == "true");
}

/* Gets the value of an unsigned argument. Throws an InvalidArgumentException
 * if the argument is not set, does not exist, or is not an unsigned
 * argument. */
unsigned PlaceArgs::get_uint(std::string name)
{
    /* Is the argument unsigned? */
    std::map<std::string, std::string>::iterator typeIt;
    typeIt = validTypes.find(name);
    InvalidArgumentException exc = InvalidArgumentException(
        dformat("Invalid argument '%s' is not unsigned.", name.c_str()));
    if (typeIt == validTypes.end()) throw exc;
    if (typeIt->second != "unsigned") throw exc;

    /* Is the value set? */
    if (args[name].empty())
        throw InvalidArgumentException(
            dformat("Argument '%s' has not been set.", name.c_str()));

    /* Get it and decode it. */
    return str2uint(args[name]);
}

/* Sets the value of an argument, doing some validation on the type. */
void PlaceArgs::set(std::string name, std::string value)
{
    /* Is the argument sane? */
    std::map<std::string, std::string>::iterator typeIt;
    typeIt = validTypes.find(name);
    if (typeIt == validTypes.end())
        throw InvalidArgumentException(
            dformat("Invalid argument '%s'.", name.c_str()));

    /* Iterate through each type, and do some validation. */
    if (typeIt->second == "bool")
    {
        if (value != "true" and value != "false" and
            value != "True" and value != "False")
        {
            throw InvalidArgumentException(
                dformat("Invalid value '%s' for argument '%s' (it should be a "
                        "boolean).",
                        value.c_str(), name.c_str()));
        }
    }

    if (typeIt->second == "unsigned")
    {
        /* Go through each character in the string until you hit a
         * non-digit. If we encounter any non-digits, or if the string is
         * empty, throw a wobbly. */
        std::string::const_iterator valIt = value.begin();
        while (valIt != value.end() and std::isdigit(*valIt)) valIt++;
        if (valIt != value.end() or value.empty())
        {
            throw InvalidArgumentException(
                dformat("Invalid value '%s' for argument '%s' (it should be "
                        "unsigned).",
                        value.c_str(), name.c_str()));
        }
    }

    /* If we're past validation, set the value. */
    args[name] = value;
}

/* Sets up the `validAlgs` and `validTypes` map. These maps don't change for as
 * long as this PlaceArgs exists. */
void PlaceArgs::setup()
{
    /* ho ho ho */
    validTypes["inpl"] = "bool";  /* Inplace */
    validTypes["iter"] = "unsigned";  /* Iterations */

    validAlgs["sa"].insert("iter");
    validAlgs["sa"].insert("inpl");
    validAlgs["gc"].insert("iter");
    validAlgs["gc"].insert("inpl");
}

/* Given the name of an algorithm, checks the all set arguments. Throws an
 * InvalidArgumentException, and clears all set arguments, if an argument
 * doesn't match for this algorithm. */
void PlaceArgs::validate_args(std::string algName)
{
    /* Search */
    std::set<std::string> badArgs;
    std::map<std::string, std::string>::iterator setArgIt;
    for (setArgIt = args.begin(); setArgIt != args.end(); setArgIt++)
    {
        if (validAlgs[algName].find(setArgIt->first) ==
            validAlgs[algName].end()) badArgs.insert(setArgIt->first);
    }

    /* Report */
    if (badArgs.size() > 0)
    {
        std::stringstream errStream;
        if (badArgs.size() > 1) errStream << "Arguments '";
        else errStream << "Argument '";
        std::set<std::string>::iterator badArgIt;
        bool comma = false;
        for (badArgIt = badArgs.begin(); badArgIt != badArgs.end(); badArgIt++)
        {
            if (comma) errStream << ", ";
            else comma = true;
            errStream << *badArgIt;
        }
        if (badArgs.size() == 1) errStream << "' is ";
        else errStream << "' are ";
        errStream << "not valid for the '" << algName << "' algorithm. "
                  << "Clearing arguments.";
        clear();
        throw InvalidArgumentException(errStream.str());
    }
}

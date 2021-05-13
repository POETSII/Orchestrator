#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_PLACEARGS_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_PLACEARGS_H

/* Holds arguments for placement algorithms. Basically a use of the properties
 * pattern.
 *
 * Some arguments can only be applied to certain algorithms, but we won't know
 * which algorithm the arguments will be applied to until they've all been
 * staged. */

#include <map>
#include <set>
#include <string>

#include "InvalidArgumentException.h"

class PlaceArgs
{
public:
    PlaceArgs(){setup();}
    void copy_to(std::map<std::string, std::string>& copy){copy = args;}
    void clear(){args.clear();}

    /* Getters for arguments of each type. */
    bool get_bool(std::string name);
    unsigned get_uint(std::string name);

    bool is_set(std::string name){return args.find(name) != args.end();}
    void set(std::string name, std::string value);
    void setup();
    std::map<std::string, std::string>::size_type size(){return args.size();}
    void validate_args(std::string algName);

private:
    /* Holds currently-stored arguments from the operator. */
    std::map<std::string, std::string> args;

    /* Maps algorithms to valid arguments. "Static" after `setup` is called. */
    std::map<std::string, std::set<std::string> > validAlgs;

    /* Maps arguments to 'types'. "Static" after `setup` is called. */
    std::map<std::string, std::string> validTypes;
};

#endif

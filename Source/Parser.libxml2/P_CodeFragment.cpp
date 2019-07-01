#include "P_CodeFragment.h"

const set<string> P_CodeFragment::tags;
const set<string> P_CodeFragment::attributes;

P_CodeFragment::P_CodeFragment(xmlTextReaderPtr parser):P_Pparser(parser), code(0)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      isCData = true;
}

P_CodeFragment::~P_CodeFragment()
{
}

int P_CodeFragment::InsertSubObject(string subobj)
{
    if (!isCData) return ERR_INVALID_NODE; // not CDATA? Can't be a code fragment. Abort parsing.
    code = new CFrag(subobj);              // only need to comment this line to stub out functionality
    return ERR_SUCCESS;
}

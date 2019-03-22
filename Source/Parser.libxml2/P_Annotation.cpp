#include "P_Annotation.h"

const set<string> P_Annotation::tags_init()
{
      const char* tags_array[2] = {"Documentation","MetaData"};
      set<string> tmp_tags(tags_array,tags_array+2);
      return tmp_tags;
}

const set<string> P_Annotation::tags(tags_init());

P_Annotation::P_Annotation(xmlTextReaderPtr parser) : P_Pparser(parser)
{
      valid_tags=&tags;
}

P_Annotation::~P_Annotation()
{
}

int P_Annotation::InsertSubObject(string subobj)
{
    if (isText) return ERR_SUCCESS; // simply swallow the text of annotations
    // descend immediately into the subtags; no need to recur.
    if ((subobj == "Documentation") || (subobj == "MetaData"))
    {
       isText = true;
       int error = ParseNextSubObject(subobj);
       isText = false;
       return error;
    }
    return ERR_UNEXPECTED_TAG;
}

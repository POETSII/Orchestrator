#include "P_Annotation.h"

const set<string> P_Annotation::tags_init()
{
      const char* tags_array[3] = {"Documentation","MetaData","HashDigest"};
      set<string> tmp_tags(tags_array,tags_array+3);
      return tmp_tags;
}

const set<string> P_Annotation::attrs_init()
{
      const char* attrs_array[2] = {"type","hash"};
      set<string> tmp_attrs(attrs_array,attrs_array+2);
      return tmp_attrs;
}

const set<string>& P_Annotation::getTags()
{
      static const set<string> tags(tags_init());
      return tags;
}

const set<string>& P_Annotation::getAttributes()
{
      static const set<string> attributes(attrs_init());
      return attributes;
}

P_Annotation::P_Annotation(xmlTextReaderPtr parser):P_Pparser(parser), Hash()
{
      valid_tags=&getTags();
      valid_attributes=&getAttributes();
      HashType = NONE;
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
       // if there is a HashType defined we are (invalidly) inside a
       // HashDigest tag.
       if (HashType != NONE) return ERR_UNEXPECTED_TAG;
       // otherwise just swallow up any text encountered.
       isText = true;
       int error = ParseNextSubObject(subobj);
       isText = false;
       return error;
    }
    if (subobj == "HashDigest")
    {
       if (ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       return ParseNextSubObject(subobj); // an empty element is expected here.
    }
    return ERR_UNEXPECTED_TAG;
}

int P_Annotation::SetObjectProperty(string prop, string value)
{
    // SetObjectProperty need only be invoked for a HashDigest tag.
    // we assume therefore that this is the tag being dealt with.
    if (prop == "type")
    {
       if (value == "structural") HashType = STRUC;
       else if (value == "functional") HashType = FUNC;
       else if (value == "instance") HashType = INST;
       else return -ERR_INVALID_ATTR;
       return ERR_SUCCESS;
    }
    if (prop == "hash")
    {
       Hash = value;
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}

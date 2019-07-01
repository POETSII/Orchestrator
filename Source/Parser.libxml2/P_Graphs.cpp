#include "P_Graphs.h"
#include "OrchBase.h"
#include "stdlib.h"
// #include <iostream>
#include "P_GraphType.h"
#include "P_GraphInstance.h"
#include "P_GraphTypeReference.h"
#include "P_GraphInstanceReference.h"
#include "P_GraphInstanceMetadataPatch.h"

/* lists of allowable tag values. Unfortunately C++98 syntax for initializing
   static const STL containers is extremely oblique. The problem is particularly
   complicated if a static constant member is initialised with a static constant
   member in another translation unit (e.g. if it's in a derived class with a
   base class containing a static member), because the order of initialisation
   of static members is UNDEFINED between translation units. Thus the program
   can crash before it even starts, if the static member of the derived class 
   is inititialised before the static member of the base class (yes, this can
   happen. C++11 improves upon this, although it's not clear that it improves
   upon the problem of generating a static initializer (function). Surely this
   shouldn't be this hard? However as long as the lists are small this probably
   isn't too terribly bad.
*/

// permitted tags for the top-level object are the top-level type declarations
// and the instance definitions.
const set<string> P_Graphs::tags_init()
{
      const char* tags_array[6] = {"GraphType","GraphInstance","GraphTypeReference","GraphInstanceReference","GraphInstanceMetadataPatch"};
      set<string> tmp_tags(tags_array,tags_array+5);
      return tmp_tags;
}

// Only one attribute is relevant for the Graphs object
const set<string> P_Graphs::attrs_init()
{
      const char* attrs_array[1] = {"formatMinorVersion"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string>& P_Graphs::getTags()
{
      static const set<string> tags(tags_init());
      return tags;
}

const set<string>& P_Graphs::getAttributes()
{
      static const set<string> attributes(attrs_init());
      return attributes;
}

P_Graphs::P_Graphs(OrchBase* orchestrator, string infile):P_Pparser()
{
      // Graphs is the top-level tag, and thus this object will be created before
      // the XML file is open. Set or create the orchestrator and open a parser for 
      // the file to be read.
      internalOrch=false;
      SetOrch(orchestrator);
      valid_tags = &getTags();
      valid_attributes = &getAttributes();
      Load(infile);
}

P_Graphs::~P_Graphs()
{
      // if we created an internal orchestrator during construction we are
      // responsible for destroying it.
      if (internalOrch) delete orchDB;
}

int P_Graphs::InsertSubObject(string subobj)
{
    if (subobj == "GraphType")
    {
       P_GraphType gt_subobj(GetParser(), orchDB);
       if (gt_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (!gt_subobj.typdcl) return ERR_INVALID_OBJECT; // didn't set up an Orchestrator object
       return gt_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "GraphInstance")
    {
       P_GraphInstance gi_subobj(GetParser(), orchDB);
       if (gi_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (!gi_subobj.task) return ERR_INVALID_OBJECT; // didn't set up an Orchestrator object
       return gi_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "GraphTypeReference")
    {
       P_GraphTypeReference gtr_obj(orchDB);
       if (gtr_obj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       return gtr_obj.ParseDocument();
    }
    if (subobj == "GraphInstanceReference")
    {
       P_GraphInstanceReference gir_obj(orchDB);
       if (gir_obj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       return gir_obj.ParseDocument();
    }
    if (subobj == "GraphInstanceMetadataPatch")
    {
       P_GraphInstanceMetadataPatch gm_subobj(GetParser(), orchDB);
       if (gm_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       return gm_subobj.ParseNextSubObject(subobj);
    }
    return ERR_UNEXPECTED_TAG;   
}

void P_Graphs::SetOrch(OrchBase* orchestrator)
{
      // if we did not get an orchestrator pointer (e.g. the P_Graphs object was
      // not instantiated from within an existing orchestrator process) then we
      // need to create a dummy (as an MPI 'singleton').
      if (!(orchDB=orchestrator))
      {
	 orchDB = new OrchBase();
	 // orchDB = new OrchBaseDummy(0,0,"OrchBaseDummy","OrchBaseDummy.h");
	 internalOrch=true;
      }
}

// only one property is expected: the minor version
int P_Graphs::SetObjectProperty(string prop, string value)
{
      if (prop != "formatMinorVersion") return -ERR_UNEXPECTED_ATTR;
      minorVersion = atoi(value.c_str());
      return ERR_SUCCESS;
}


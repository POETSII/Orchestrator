#include "P_EdgeInstances.h"
#include "P_EdgeI.h"


const set<string> P_EdgeInstances::tags_init()
{
      const char* tags_array[1] = {"EdgeI"};
      set<string> tmp_tags(tags_array,tags_array+1);
      return tmp_tags;
}

const set<string> P_EdgeInstances::attrs_init()
{
      const char* attrs_array[1] = {"sorted"};
      set<string> tmp_attrs(attrs_array,attrs_array+1);
      return tmp_attrs;
}

const set<string> P_EdgeInstances::tags(tags_init());
const set<string> P_EdgeInstances::attributes(attrs_init());

P_EdgeInstances::P_EdgeInstances(xmlTextReaderPtr parser, P_task* graphinstance): P_Pparser(parser), parent(graphinstance)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      // errorsAreFatal=false; // only during debug when we ignore subtags
}

P_EdgeInstances::~P_EdgeInstances()
{
}

int P_EdgeInstances::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!parent) return ERR_INVALID_NODE; // no Orchestrator task means the parent GraphInstance was badly defined. Abort parsing.
    // tag not in class-specific subset: abort parsing
    if (valid_tags->find(subobj) == valid_tags->end()) return ERR_UNEXPECTED_TAG;
    if (subobj == "EdgeI")
    {
       P_EdgeI ei_subobj(GetParser(), parent);
       if (ei_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       if (ei_subobj.edgeStr == "") // blank edge indicates the properties were immediately resolvable
       {
	  if ((ei_subobj.src.devIdx==0) || (ei_subobj.dst.devIdx==0)) return ERR_INVALID_OBJECT; // either no source or no destination
	  // message types didn't match
	  if (ei_subobj.src.pin->pP_pintyp->pMsg != ei_subobj.src.pin->pP_pintyp->pMsg) return ERR_INVALID_OBJECT;
          unsigned edgeIdx = parent->pD->G.SizeArcs()+1;
	  ei_subobj.src.pin->idx = 2*edgeIdx;  // update the back indices in the pins
	  ei_subobj.dst.pin->idx = 2*edgeIdx+1;
	  // otherwise add to the device graph
          parent->pD->G.InsertArc(edgeIdx,
				  ei_subobj.src.devIdx,
				  ei_subobj.dst.devIdx,
				  ei_subobj.src.pin->pP_pintyp->pMsg,
				  ei_subobj.src.pin->idx, 
				  ei_subobj.src.pin,
				  ei_subobj.dst.pin->idx,
				  ei_subobj.dst.pin);
       }
       return ei_subobj.ParseNextSubObject(subobj);
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}

int P_EdgeInstances::SetObjectProperty(string prop, string value)
{
    // Only one property is expected: whether the list is sorted
    if (prop == "sorted") return ERR_SUCCESS; // (which we don't care about)
    return -ERR_UNEXPECTED_ATTR;
}

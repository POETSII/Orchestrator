#include "P_EdgeI.h"
#include "P_DataDecl.h"
#include "P_DataDef.h"
#include "P_device.h"
#include "P_pin.h"
#include "P_pintyp.h"
#include "P_super.h"
#include "stdlib.h"
#include "CFrag.h"
// #include <iostream>

const set<string> P_EdgeI::tags_init()
{
      const char* tags_array[3] = {"P","S","M"};
      set<string> tmp_tags(tags_array,tags_array+3);
      return tmp_tags;
}

const set<string> P_EdgeI::attrs_init()
{
      const char* attrs_array[4] = {"path","sendIndex","P","S"};
      set<string> tmp_attrs(attrs_array,attrs_array+4);
      return tmp_attrs;
}

const set<string> P_EdgeI::tags(tags_init());
const set<string> P_EdgeI::attributes(attrs_init());

P_EdgeI::P_EdgeI(xmlTextReaderPtr parser, P_task* graph) : P_Pparser(parser), edgeStr("_INVALID_"), gIdx(0), sIdx(0), inProps(""), inState(""), isMetaData(false), parent(graph)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
      src.pin = 0;
      src.devIdx = 0;
      dst.pin = 0;
      dst.devIdx = 0;
      // errorsAreFatal=false; // only during debug when we ignore subtags      
}

P_EdgeI::~P_EdgeI()
{
}

int P_EdgeI::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!src.pin || !dst.pin) return ERR_INVALID_NODE; // no source, no destination implies an invalid path. Abort parsing.
    if (isMetaData) return ERR_SUCCESS;
    if (subobj == "P")
    {
       if (!inProps.empty()) return ERR_UNEXPECTED_TAG;
       // by this point everything should have been set up to be able to generate a value
       P_DataDef dstPropsVal(GetParser(), dst.pin->par);
       // P_DataDef dstPropsVal(GetParser(), dst.pin->pP_pintyp->pProps);
       int error = dstPropsVal.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) dst.pin->pPropsI = dstPropsVal.code;
       // if (error == ERR_SUCCESS) dst.pin->pProps = dstPropsVal.value;
       return error;
    }
    if (subobj == "S")
    {
       if (!inState.empty()) return ERR_UNEXPECTED_TAG;
       // by this point everything should have been set up to be able to generate a value
       P_DataDef dstStateVal(GetParser(), dst.pin->par);
       // P_DataDef dstStateVal(GetParser(), dst.pin->pP_pintyp->pState);
       int error = dstStateVal.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) dst.pin->pStateI = dstStateVal.code;
       // if (error == ERR_SUCCESS) dst.pin->pState = dstStateVal.value;
       return error;
    }
    if (subobj == "M")
    {
       // for the moment just swallow metadata
       isMetaData = true;
       int error = ParseNextSubObject(subobj);
       isMetaData = false;
       return error;
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}

int P_EdgeI::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator object
    // set up the new device instance
    if (prop == "path")
    {
       edgeStr = value;
       size_t srcDstSplit = edgeStr.find_first_of('-');
       string srcStr = edgeStr.substr(srcDstSplit+1);
       string dstStr = edgeStr.substr(0,srcDstSplit);
       size_t srcDevPinSplit = srcStr.find_first_of(':');
       string srcDevStr = srcStr.substr(0,srcDevPinSplit);
       string srcPinStr = srcStr.substr(srcDevPinSplit+1);
       size_t dstDevPinSplit = dstStr.find_first_of(':');
       string dstDevStr = dstStr.substr(0,dstDevPinSplit);
       string dstPinStr = dstStr.substr(dstDevPinSplit+1);
       // now begins the long process of determining what we already have and if
       // necessary creating skeleton objects
       map<string,P_device*>::iterator srcDev= parent->pD->deviceMap.find(srcDevStr);
       map<string,P_device*>::iterator dstDev= parent->pD->deviceMap.find(dstDevStr);
       // first we look up the source and destination device instances
       P_device* source = 0;
       P_device* destination = 0;
       if (srcDevStr.empty() || dstDevStr.empty())
       {
	  if (!parent->pSup)
	  {
	     if (!parent->pP_typdcl) return -ERR_INVALID_ATTR;
	     map<string, P_devtyp*>::iterator dev = parent->pP_typdcl->P_devtypm.begin();
	     while (dev != parent->pP_typdcl->P_devtypm.end())
	     {
	        if (dev->second->isSuper)
		{
		   parent->pSup = new P_super(dev->second->Name());
		   parent->pSup->pP_devtyp = dev->second;
		   parent->pSup->idx = 0xFFFFFFFF; // place supervisor devices at the highest possible offset.
		   break;
		}
	     }
	     if (dev == parent->pP_typdcl->P_devtypm.end()) return -ERR_INVALID_ATTR;
	  }
       }
       if (srcDevStr.empty()) source = parent->pSup;
       else if (srcDev != parent->pD->deviceMap.end()) source = srcDev->second;
       if (dstDevStr.empty()) destination = parent->pSup;
       else if (dstDev != parent->pD->deviceMap.end()) destination = dstDev->second;
       // no device is an absolute error because then there will be no way to look up the
       // pin type.
       if (!source || !destination) return -ERR_INVALID_ATTR;
       // same problem if no device type: can't do anything
       if (!source->pP_devtyp || !destination->pP_devtyp) return -ERR_INVALID_ATTR;
       P_pintyp* outPinTyp;
       P_pintyp* inPinTyp;
       map<string, P_pintyp*>::iterator outPinTIt = source->pP_devtyp->P_pintypOm.find(srcPinStr);
       map<string, P_pintyp*>::iterator inPinTIt = destination->pP_devtyp->P_pintypIm.find(dstPinStr);
       if (outPinTIt == source->pP_devtyp->P_pintypOm.end())
       {
	  outPinTyp = new P_pintyp(source->pP_devtyp, srcPinStr);
	  source->pP_devtyp->P_pintypOm[srcPinStr] = outPinTyp;
       }
       else outPinTyp = outPinTIt->second;
       if (inPinTIt == destination->pP_devtyp->P_pintypIm.end())
       {
	  inPinTyp = new P_pintyp(destination->pP_devtyp, dstPinStr);
	  destination->pP_devtyp->P_pintypIm[dstPinStr] = inPinTyp;
       }
       else inPinTyp = inPinTIt->second;
       if (outPinTyp->pMsg != inPinTyp->pMsg)
       {
	 if (!outPinTyp->pMsg) outPinTyp->pMsg = inPinTyp->pMsg;
	 else if (!inPinTyp->pMsg) inPinTyp->pMsg = outPinTyp->pMsg;
	 else return -ERR_INVALID_OBJECT;
       }
       src.pin = new P_pin(parent->pD,srcStr);
       dst.pin = new P_pin(parent->pD,dstStr);
       src.pin->pP_pintyp = outPinTyp;
       dst.pin->pP_pintyp = inPinTyp;
       src.devIdx = source->idx;
       dst.devIdx = destination->idx;
       edgeStr = "";
       return ERR_SUCCESS;
    }
    if (prop == "sendIndex")
    {
       sIdx = static_cast<unsigned>(strtoul(value.c_str(),NULL,10));
       return ERR_SUCCESS;
    }
    if (prop == "P")
    {
       inProps = value;
       if (dst.pin) dst.pin->pPropsI = new CFrag(inProps);
       return ERR_SUCCESS;
    }
    if (prop == "S")
    {
       inState = value;
       if (dst.pin) dst.pin->pStateI = new CFrag(inState);
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}

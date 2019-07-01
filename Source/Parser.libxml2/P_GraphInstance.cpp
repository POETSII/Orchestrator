#include "P_GraphInstance.h"
#include "P_super.h"
#include "P_DeviceInstances.h"
#include "P_EdgeInstances.h"
#include "P_DataDef.h"
#include "P_devtyp.h"
#include "CFrag.h"


const set<string> P_GraphInstance::tags_init()
{
      const char* tags_array[3] = {"DeviceInstances","EdgeInstances","Properties"};
      set<string> tmp_tags(tags_array,tags_array+3);
      tmp_tags.insert(P_GraphInstance::P_Annotation::getTags().begin(), P_GraphInstance::P_Annotation::getTags().end());
      return tmp_tags;
}

const set<string> P_GraphInstance::attrs_init()
{
      const char* attrs_array[4] = {"id","graphTypeId","supervisorDeviceTypeId","P"};
      set<string> tmp_attrs(attrs_array,attrs_array+4);
      return tmp_attrs;
}

const set<string> P_GraphInstance::tags(tags_init());
const set<string> P_GraphInstance::attributes(attrs_init());

P_GraphInstance::P_GraphInstance(xmlTextReaderPtr parser, OrchBase* orchestrator) : P_Annotation(parser), task(0), properties(""), parent(orchestrator)
{
      valid_tags=&tags;
      valid_attributes=&attributes;
}

P_GraphInstance::~P_GraphInstance()
{
}

int P_GraphInstance::InsertSubObject(string subobj)
{
    // stub insert routine swallows everything internal. 
    // int error = ERR_SUCCESS;
    // return error;
    if (!task) return ERR_INVALID_NODE; // no Orchestrator object means the GraphInstance had no ID. Abort parsing.
    // tag not in class-specific subset: refer to parent class.
    if (valid_tags->find(subobj) == valid_tags->end()) return P_Annotation::InsertSubObject(subobj);
    if (subobj == "DeviceInstances")
    {
       P_DeviceInstances di_subobj(GetParser(), task);
       if (di_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS; // check for sorted attribute
       return di_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "EdgeInstances")
    {
       P_EdgeInstances ei_subobj(GetParser(), task);
       if (ei_subobj.ParseObjectProperties() < 0) return ERR_UNPARSEABLE_PROPS;
       return ei_subobj.ParseNextSubObject(subobj);
    }
    if (subobj == "Properties")
    {
       if (!properties.empty()) return ERR_UNEXPECTED_TAG; // can't have both a tag and a P attribute
       P_DataDef pi_subobj(GetParser(), task->pD);
       int error = pi_subobj.ParseNextSubObject(subobj);
       if (error == ERR_SUCCESS) task->pD->pPropsI = pi_subobj.code; 
       // if (error == ERR_SUCCESS) task->pD->pProps = p_subobj.value;
       return error;
    }
    return ERR_UNEXPECTED_TAG; // should never reach this line
}

int P_GraphInstance::SetObjectProperty(string prop, string value)
{
    if (!parent) return -ERR_UNDEFINED_ORCH; // no associated orchestrator
    // with the ID we can create the task
    if (prop == "id")
    {
       id = value;
       if (!(task = parent->P_taskm[id])) task = parent->P_taskm[id] = new P_task(parent,value);
       // if we have already read a graph type for the instance we can associate it.
       // note here that if the graph type itself has not been read in, this line
       // will create a new entry in the P_typdclm map for a to-be-seen type. When
       // we encounter that later in the XML, the value will be filled in
       if (!graphTypeID.empty())
       {
	  if (!(task->pP_typdcl = parent->P_typdclm[graphTypeID]))
	     task->pP_typdcl = parent->P_typdclm[graphTypeID] = new P_typdcl(parent,graphTypeID);
          // similarly for the supervisor type
          if (!supervisorID.empty())
          {
	     if (!(task->pSup = parent->P_superm[supervisorID]))
	     {
	        task->pSup = parent->P_superm[supervisorID] = new P_super(supervisorID);
		task->pSup->idx = 0xFFFFFFFF;
	     }
	     map<string, P_devtyp*>::iterator dev = task->pP_typdcl->P_devtypm.find(supervisorID);
	     if (dev == task->pP_typdcl->P_devtypm.end())	    
	     {
	        P_devtyp* superType = new P_devtyp(task->pP_typdcl, supervisorID);
	        task->pP_typdcl->P_devtypm[supervisorID] = superType;
	        task->pSup->pP_devtyp = superType;
	     }
	     else task->pSup->pP_devtyp = dev->second;
	  }	  
       }
       return ERR_SUCCESS;
    }
    // once we have a graph type we can associate it with the task
    if (prop == "graphTypeId")
    {
       graphTypeID = value;
       // provided the task already exists
       if (task)
       {
	  if (!(task->pP_typdcl = parent->P_typdclm[graphTypeID]))
	     task->pP_typdcl = parent->P_typdclm[graphTypeID] = new P_typdcl(parent, graphTypeID);
	  // and if we have a supervisor type as well this can also be associated and set up
	  if (!supervisorID.empty())
	  {
	     if (!(task->pSup = parent->P_superm[supervisorID]))
	     {
	        task->pSup = parent->P_superm[supervisorID] = new P_super(supervisorID);
		task->pSup->idx = 0xFFFFFFFF;
	     }
	     map<string, P_devtyp*>::iterator dev = task->pP_typdcl->P_devtypm.find(supervisorID);
	     if (dev == task->pP_typdcl->P_devtypm.end())	    
	     {
	        P_devtyp* superType = new P_devtyp(task->pP_typdcl, supervisorID);
	        task->pP_typdcl->P_devtypm[supervisorID] = superType;
	        task->pSup->pP_devtyp = superType;
	     }
	     else task->pSup->pP_devtyp = dev->second;
	     task->pSup->idx = 0xFFFFFFFF;
	  }
       }
       return ERR_SUCCESS;
    }
    // and once we have a supervisor type we can likewise associate it with the task
    if (prop == "supervisorDeviceTypeId")
    {
       supervisorID = value;
       // under the same constraint that the task exists.
       if (task)
       {
	  if (!(task->pSup = parent->P_superm[supervisorID]))
          {
	     task->pSup = parent->P_superm[supervisorID] = new P_super(supervisorID);
	     task->pSup->idx = 0xFFFFFFFF;
	  }
	  if (!graphTypeID.empty()) // and if we have the graph type as well, we can associate it with a supervisor device type.
	  {
	     map<string, P_devtyp*>::iterator dev = task->pP_typdcl->P_devtypm.find(supervisorID);
	     if (dev == task->pP_typdcl->P_devtypm.end())	    
	     {
	        P_devtyp* superType = new P_devtyp(task->pP_typdcl, supervisorID);
	        task->pP_typdcl->P_devtypm[supervisorID] = superType;
	        task->pSup->pP_devtyp = superType;
	     }
	     else task->pSup->pP_devtyp = dev->second;
	  }
       }
       return ERR_SUCCESS;	 
    }
    if (prop == "P")
    {
       properties = value;
       if (task) task->pD->pPropsI = new CFrag(properties);
       return ERR_SUCCESS;
    }
    return -ERR_UNEXPECTED_ATTR;
}

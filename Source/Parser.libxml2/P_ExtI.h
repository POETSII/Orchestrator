#ifndef _P_EXTI_H_
#define _P_EXTI_H_

#include "P_task.h"
#include "P_Pparser.h"
#include "P_device.h"

class P_ExtI : public P_Pparser 
{
public:

       P_ExtI(xmlTextReaderPtr,P_task*);
       ~P_ExtI();

       int        InsertSubObject(string);
       int        SetObjectProperty(string, string);

       string     id;
       string     devtype;
       P_device*  device;

private:

static const set<string> tags_init();
static const set<string> attrs_init();

static const set<string>tags;
static const set<string>attributes;

             bool isMetaData;
             P_task* parent;
};

#endif

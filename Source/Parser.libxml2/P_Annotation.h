#ifndef _P_ANNOTATION_H_
#define _P_ANNOTATION_H_

#include "P_Pparser.h"

class P_Annotation: public P_Pparser
{
public:
  
       P_Annotation(xmlTextReaderPtr);
       ~P_Annotation();

       int InsertSubObject(string);

protected:
       
static const set<string>tags;
 
static const int ERR_UNDEFINED_ORCH = 0x800;
       
private:

static const set<string>tags_init();
      
inline       int ParseDocument(string) {return ERR_NOT_AT_DOC_TAG;};

 
};

#endif

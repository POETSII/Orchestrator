#ifndef _P_ANNOTATION_H_
#define _P_ANNOTATION_H_

#include "P_Pparser.h"

enum         hashtyp {NONE, STRUC, FUNC, INST};

class P_Annotation: public P_Pparser
{
public:
  
             P_Annotation(xmlTextReaderPtr);
virtual      ~P_Annotation();
 
virtual      bool CheckHash() = 0;
inline const string GetHash() const {return Hash;};
inline const hashtyp GetHashType() const {return HashType;};
virtual      int InsertSubObject(string);
virtual      int SetObjectProperty(string, string);
	     
protected:

static const set<string>& getAttributes();
static const set<string>& getTags();
       
private:

static const set<string> attrs_init();
static const set<string> tags_init();
inline       int ParseDocument(string) {return ERR_NOT_AT_DOC_TAG;};

string       Hash;
hashtyp      HashType;

};

#endif

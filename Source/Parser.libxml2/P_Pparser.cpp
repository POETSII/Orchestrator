#include "P_Pparser.h"
#include "stdlib.h"
#include <iostream>

const int P_Pparser::ERR_SUCCESS;
const int P_Pparser::ERR_UNEXPECTED_TAG;
const int P_Pparser::ERR_UNEXPECTED_ATTR;
const int P_Pparser::ERR_UNEXPECTED_TEXT;
const int P_Pparser::ERR_UNEXPECTED_CDATA;
const int P_Pparser::ERR_INVALID_NODE;
const int P_Pparser::ERR_INVALID_NODE_TYPE;
const int P_Pparser::ERR_INVALID_ATTR;
const int P_Pparser::ERR_INVALID_XML_DOC;
const int P_Pparser::ERR_UNPARSEABLE_PROPS;
const int P_Pparser::ERR_PARSER_NOT_OPEN;
const int P_Pparser::ERR_NOT_AT_DOC_TAG;
const int P_Pparser::ERR_INVALID_OBJECT;
const int P_Pparser::ERR_UNDEFINED_ORCH;
const int P_Pparser::ERR_INVALID_JSON_DOC;
const int P_Pparser::ERR_INVALID_JSON_INIT; 

P_Pparser::P_Pparser(xmlTextReaderPtr p_def, bool errsFatal) : isText(false), isCData(false), errorsAreFatal(errsFatal), parser(p_def), err(0)
{
  
}

P_Pparser::~P_Pparser()
{
  
}

int P_Pparser::ParseDocument(string tag)
{
    string top_tag;
    int node_type;
    err = 0;
    while (!err && (GetNextTag(&node_type, top_tag) == ""));
    if (err || (node_type != XML_READER_TYPE_ELEMENT)) return err |= ERR_INVALID_XML_DOC;
    if (top_tag != tag) return err |= ERR_UNEXPECTED_TAG;
    if (ParseObjectProperties() < 0) return err |= ERR_UNPARSEABLE_PROPS;
    return ParseNextSubObject(top_tag);
}

// The main parser loop - grabs one object at a time and descends into its subobjects.
int P_Pparser::ParseNextSubObject(string tag)
{
    string subtag;
    int subtag_type;
    if (!parser) return err |= ERR_PARSER_NOT_OPEN;
    // an empty element can just be skipped over.
    /* a real 'gotcha' in the libxml2 library; for this one function
      (xmlTextReaderIsEmptyElement) libxml2 is not strictly standards-compliant.
      Technically in XML, a empty element can be 'self-closing' (<tag/>)
      or not <tag></tag>. But libxml2's xmlTextReaderIsEmptyElement only 
      returns true for a self-closing element. See http://www.xmlsoft.org/xmlreader.html
      where the comment is:
      'IsEmptyElement: check if the current node is empty, this is a bit bizarre in the
       sense that <a/> will be considered empty while <a></a> will not.' 
    */ 
    if ((subtag_type = xmlTextReaderIsEmptyElement(parser)) < 0) return err |= ERR_INVALID_NODE;
    else if (subtag_type) return ERR_SUCCESS;
    while ((GetNextTag(&subtag_type, subtag) != tag) || (subtag_type != XML_READER_TYPE_END_ELEMENT))
    {
          // cout << "New subtag " << subtag << " of type " << subtag_type << " for tag " << tag << "\n";
          // cout.flush();
	  if (err && errorsAreFatal) break; // error occurred; abort parsing.
	  if (subtag.empty())
          if (subtag.empty()) continue; // nothing of value found - move on
	  // set up subobjects when valid ones are found.
	  if ((valid_tags && (valid_tags->find(subtag) != valid_tags->end())) ||
	      (isCData && (subtag_type == XML_READER_TYPE_CDATA)) ||
	      (isText && (subtag_type == XML_READER_TYPE_TEXT)))
	       err = InsertSubObject(subtag);
	  // other tags encountered indicate a problem.
	  else err |= ERR_UNEXPECTED_TAG;     
    }
    // cout << "Ending tag " << subtag << " of type " << subtag_type << " for tag " << tag << "\n";
    // cout.flush();
    return err;
}

int P_Pparser::ParseObjectProperties()
{
    int read_state, attr_count;
    string attr;
    string attr_val;
    xmlChar* read_value;
    if (!parser) return -(err |= ERR_PARSER_NOT_OPEN);
    // no attributes, so just exit. (Object should assume global defaults)
    if ((read_state = xmlTextReaderHasAttributes(parser)) < 1) return read_state;
    if ((attr_count = xmlTextReaderAttributeCount(parser)) < 1) return read_state;    
    for (int a = 0; a < attr_count; a++)
    {
        // get the next attribute
        if ((read_state = xmlTextReaderMoveToAttributeNo(parser, a)) < 1) return read_state;
	// skip over namespaces
	if (read_state = xmlTextReaderIsNamespaceDecl(parser))
	{
	   if (read_state < 0) return read_state;
	   continue;
	}
	// attributes cannot be nameless.
	if ((read_value = xmlTextReaderLocalName(parser)) == NULL)
	   return -(err |= ERR_INVALID_ATTR);
	attr =  xmlChartoStr(read_value, attr);
	free(read_value); // must free the xmlChar* representation of the attribute
	// nor can they have an unrecognised name.
	if ((!valid_attributes) || (valid_attributes->find(attr) == valid_attributes->end()))
	   return -(err |= ERR_INVALID_ATTR);
	// but their values can be empty.
	if ((read_value = xmlTextReaderValue(parser)) == NULL) attr_val = "";
	else
	{
	   attr_val = xmlChartoStr(read_value, attr_val);
	   free(read_value); // likewise must free the xmlChar* representation of the value.
	}
	if (read_state = SetObjectProperty(attr, attr_val)) return read_state;
    }
    // position the reader over the main tag so that a test for IsEmptyElement is valid.
    if (xmlTextReaderMoveToElement(parser) < 1)
       return -(err |= ERR_INVALID_NODE);
    return attr_count;
}

// GetNextTag provides the low-level parser machinery to step through the XML
string& P_Pparser::GetNextTag(int* tag_type, string& tag)
{
    int read_state;
    xmlChar* read_value;
    if (!parser)
    {
       err |= ERR_PARSER_NOT_OPEN;
       return tag = "";
    }
    do // swallow whitespace
    {
        // bad XML or end of document
        if ((read_state = xmlTextReaderRead(parser)) < 0) err |= ERR_INVALID_NODE;
        if (read_state < 1) return tag = "";
    }
    while (((*tag_type = xmlTextReaderNodeType(parser)) == XML_READER_TYPE_WHITESPACE) || (*tag_type == XML_READER_TYPE_SIGNIFICANT_WHITESPACE));
    switch(*tag_type)
    {
    case -1:
    case XML_READER_TYPE_ENTITY_REFERENCE:
    case XML_READER_TYPE_ENTITY:
    case XML_READER_TYPE_PROCESSING_INSTRUCTION:
    case XML_READER_TYPE_DOCUMENT_TYPE:
    case XML_READER_TYPE_DOCUMENT_FRAGMENT:
    case XML_READER_TYPE_NOTATION:
    case XML_READER_TYPE_END_ENTITY:  
    // bad node
    err |= ERR_INVALID_NODE_TYPE;
    break;
    
    case XML_READER_TYPE_ATTRIBUTE:
    // attributes should have been read in by the object before asking for the next tag
    err |= ERR_UNEXPECTED_ATTR;
    break;
    
    // nothing to do if the node in question is always empty.
    case XML_READER_TYPE_NONE:
    case XML_READER_TYPE_COMMENT:
    case XML_READER_TYPE_DOCUMENT:
    case XML_READER_TYPE_XML_DECLARATION:
    break;
    
    case XML_READER_TYPE_TEXT:
    if (isText)
    {
       // get the text value when expected 
       if ((read_value = xmlTextReaderValue(parser)) == NULL) break;
       tag = xmlChartoStr(read_value, tag);
       free(read_value); // result of xmlTextReaderValue must be explicitly freed
       return tag;
    }
    // weren't expecting text here though. 
    err |= ERR_UNEXPECTED_TEXT;
    break;
 
    case XML_READER_TYPE_CDATA:
    if (isCData)
    {
       // get code fragments where expected 
       if ((read_value = xmlTextReaderValue(parser)) == NULL) break;
       tag = xmlChartoStr(read_value, tag);
       free(read_value); // result of xmlTextReaderValue must be explicitly freed
       return tag;
    }
    // No code fragment expected for this object 
    err |= ERR_UNEXPECTED_CDATA;
    break;
 
    case XML_READER_TYPE_ELEMENT:
    case XML_READER_TYPE_END_ELEMENT:
    // found a subelement, so get its name. 
    if ((read_value = xmlTextReaderLocalName(parser)) == NULL) break;
    tag = xmlChartoStr(read_value, tag);
    free(read_value); // result of xmlTextReaderLocalName must be explicitly freed
    return tag;
    
    // other values are a bad node type. (This should never happen)
    default:
    err |= ERR_INVALID_NODE_TYPE;
    }
    return tag = ""; // empty string if the tag was invalid or empty.   
}


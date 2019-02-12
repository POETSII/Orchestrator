#include "pidatatype.h"

PIDataType::PIDataType(const QString& name, PIGraphObject *parent) :
    PIGraphBranch(name, "DataType", QVector<int>({DTYPE}), parent), data_type(NULL), base_type(""), value(""), default_value(""), expanded_type(""), num_replications(0)
{
    // we will do something more sensible with this in future when a PoetsDataType is better defined.
    data_type = new PoetsDataType(dynamic_cast<PIGraphObject*>(this));
}

PIDataType::~PIDataType()
{
    delete data_type;
}

void PIDataType::defineObject(QXmlStreamReader* xml_def)
{
     // validate the parsed tag against the possible tags which are datatypes.
     if (valid_elements.contains(xml_def->name().toString()))
     {
        setXmlName(xml_def->name().toString());
        switch (valid_elements[xmlName()])
        {
        case SCALAR:
        // odd arguments for .value (with an empty-string first argument) reflect the fact that
        // in ordinary XML attributes have no NamespaceUri.
        base_type = xml_def->attributes().value("", "type").toString();
        num_replications = -1;
        if (xml_def->attributes().hasAttribute("", "default"))
            default_value = xml_def->attributes().value("", "default").toString();
        break;
        case ARRAY:
        num_replications = (xml_def->attributes().value("", "length").toString()).toInt();
        if (xml_def->attributes().hasAttribute("", "type"))
        {
            base_type = xml_def->attributes().value("", "type").toString();
            if (xml_def->attributes().hasAttribute("", "default"))
                default_value = xml_def->attributes().value("", "default").toString();
        }
        else base_type = "array";
        break;
        case UNION:
        // uses the c keyword union as the base type.
        base_type = "union";
        num_replications = -1;
        // value stores the name of the tag field, if any, for the top-level union declaration.
        if (xml_def->attributes().hasAttribute("", "tagName"))
            value = xml_def->attributes().value("", "tagName").toString();
        else value = QString(name()).append("_tag");
        break;
        case STRUCT:
        base_type = "struct";
        num_replications = -1;
        break;
        case DTYPE:
        // a top-level data definition is considered to be a struct.
        base_type = QString("struct %1").arg(name());
        break;
        default:
        // something unknown generates a comment.
        base_type = QString("/* %1").arg(name());
        }
     }
     PIGraphBranch::defineObject(xml_def);
}

const PIGraphObject* PIDataType::appendSubObject(QXmlStreamReader* xml_def)
{
    PIGraphObject* sub_object = NULL;
    // reading the next element should get a subelement.
    switch (xml_def->tokenType())
    {
    // If we found an EndElement this should be the end of the DataType definition.
    case QXmlStreamReader::EndElement:
    return this;
    case QXmlStreamReader::StartElement:
    // all types could have documentation
    if (xml_def->name() == "Documentation")
    {
        // documentation could be literally anything. As a result, use readElementText
        // to swallow all up to the closing end-element without further parsing.
        _documentation = xml_def->readElementText(QXmlStreamReader::IncludeChildElements);
        return sub_object;
    }
    // non-composite types can't have internal members. Refer to base class.
    else if (!((base_type == "union") || (base_type == "array") || (base_type.contains("struct")))) return PIGraphBranch::appendSubObject(xml_def);
    switch (valid_elements.value(xml_def->name().toString(), DTYPE))
    {
    // valid sub-objects recurse down the XML tree.
    case SCALAR:
    case ARRAY:
    case UNION:
    case STRUCT:
    sub_object = insertSubObject(DTYPE, new PIDataType(xml_def->attributes().value("", "name").toString(), this));
    sub_object->defineObject(xml_def);
    break;
    default:
    // refer other types of token up the inheritance chain.
    return PIGraphBranch::appendSubObject(xml_def);
    }
    return sub_object;
    default:
    // non-start elements can be immediately handled in the base (probably producing an error)
    return PIGraphBranch::appendSubObject(xml_def);
    }
}

const size_t PIDataType::size() const
{
    size_t curr_size = 0;
    if (base_type == "union") // unions iterate through their data members and look for the largest element
    {
       for (QVector<const PIGraphObject*>::const_iterator member = beginConstSubObjects(DTYPE); member != endConstSubObjects(DTYPE); member++) curr_size = std::max(curr_size, static_cast<const PIDataType*>(*member)->size());
    }
    else if (base_type == "struct") // structures iterate through their data members and accumulate sizes
    {
       for (QVector<const PIGraphObject*>::const_iterator member = beginConstSubObjects(DTYPE); member != endConstSubObjects(DTYPE); member++) curr_size += static_cast<const PIDataType*>(*member)->size();
    }
    else if (base_type == "array") // arrays with internal objects take their data member's size (there should be only one)
    {
       curr_size += static_cast<const PIDataType*>(constSubObject(DTYPE,0))->size();
    }
    else curr_size = data_type->type_sizes.value(base_type);
    if (num_replications < 0) return curr_size;
    curr_size *= num_replications;
    return curr_size;
}

const QString& PIDataType::elaborateDataTypeStr(const QString& indent)
{
    // string already assembled - just return it.
    if (expanded_type.endsWith(";\n") || expanded_type.endsWith("*/\n")) return expanded_type;
    // unknown types merely end the comment section.
    if (expanded_type.contains("/*")) return expanded_type.append("*/\n");
    // arrays are built by expanding their internal member
    if (base_type == "array")
    {
       expanded_type = static_cast<PIDataType*>(subObject(DTYPE,0))->elaborateDataTypeStr(indent);
       expanded_type.chop(2); // remove the trailing ;\n from expanded elements.
    }
    else
    {
    expanded_type = base_type;
    // composite types - expand internal members
    if (expanded_type == "union" || expanded_type.contains("struct"))
    {
        // which are declared in order, and indented one more level
        QString internal_def("");
        for (QVector<PIGraphObject*>::iterator sub_var = beginSubObjects(DTYPE); sub_var != endSubObjects(DTYPE); sub_var++) internal_def.append(static_cast<PIDataType*>(*sub_var)->elaborateDataTypeStr(indent + ("   ")));
        expanded_type.append(QString("\n%1{\n%2%3} ").arg(indent).arg(internal_def).arg(indent));
    }
    // everything other than top-level structures uses its name as a data definition
    if (num_replications) expanded_type.append(QString(" %1").arg(name()));
    }
    // arrays append their size suffix
    if (num_replications > 0) expanded_type.append(QString("[%1]").arg(num_replications));
    // 'top and tail' the declaration before returning it.
    expanded_type.prepend(indent);
    expanded_type.append(";\n");
    return expanded_type;
}

// constructs a default value, possibly from subvalues. We perform this operation once, then keep the result.
// this could be problematic if an underlying datatype changes (not likely); bulletproof code would keep some
// sort of flag. However, this seems unnecessary given the overwhelmingly probable usage; data types once defined
// are not likely to change.
const QString& PIDataType::defaultValue() const
{
    // default already built. No need to redo.
    if (!default_value.isEmpty()) return default_value;
    const PIDataType* def_subobject;
    // QString array_internals; // needed to be able to chop values - see below.
    // unions' default value requires specification of a member (the first member)
    if (base_type == "union")
    {
       // so get that first member
       def_subobject = dynamic_cast<const PIDataType*>(constSubObject(DTYPE, 0));
       // and then descend into the member to retrieve the default, repeating as necessary for an array
       // annoyingly the chop function is an in-place operation returning a void. chopped is only available in Qt 5.10 onwards.
       // this necessitates several sequential string-building functions with additional static intermediates.
       if (def_subobject->numElements() > 0)
       {
           QString array_internals = QString("%1, ").arg(def_subobject->defaultValue()).repeated(def_subobject->numElements());
           array_internals.chop(2);
           return default_value = QString("{.%1 = %2}").arg(def_subobject->name()).arg(QString("{%1}").arg(array_internals));
       }
       else return default_value = def_subobject->defaultValue();
       // return default_value = QString("{.%1 = %2}").arg(def_subobject->name()).arg((def_subobject->numElements() > 0) ? QString("{%1}").arg(QString("%1, ").arg(def_subobject->defaultValue()).repeated(def_subobject->numElements()).chopped(2)) : def_subobject->defaultValue());
    }
    // structures should be built up from their subtype defaults.
    if (base_type.contains("struct"))
    {
       QString build_default("");
       for (QVector<const PIGraphObject*>::const_iterator def_member = beginConstSubObjects(DTYPE); def_member != endConstSubObjects(DTYPE); def_member++)
       {
           // get each member in turn and build by recursive descent.
           def_subobject = dynamic_cast<const PIDataType*>(*def_member);
           // expand any internal arrays. Arrays at the top level are not expanded but subelement arrays are - this is treated as a higher-level default.
           if (def_subobject->numElements() > 0 && (!(def_subobject->dataTypeStr() == "array")))
           {
              QString array_internals = QString("%1, ").arg(def_subobject->defaultValue()).repeated(def_subobject->numElements());
              array_internals.chop(2);
              build_default.append(QString("{%1}, ").arg(array_internals));
           }
           // build_default.append(QString("{%1}, ").arg(QString("%1, ").arg(def_subobject->defaultValue()).repeated(def_subobject->numElements()).chopped(2)));
           else build_default.append(QString("%1, ").arg(def_subobject->defaultValue()));
       }
       build_default.chop(2);
       return default_value = QString("{%1}").arg(build_default);
       //return default_value = QString("{%1}").arg(build_default.chopped(2));
    }
    if (base_type == "array") // arrays of non-scalars are expanded from their internal defaults
    {
        QString array_internals = QString("%1, ").arg(dynamic_cast<const PIDataType*>(constSubObject(DTYPE, 0))->defaultValue()).repeated(numElements());
        array_internals.chop(2);
        return default_value = QString("{%1}").arg(array_internals);
    }
    // everything else is a scalar without a preset default, so we fall back on a generic default.
    // arrays at this level of the data structure are NOT expanded; this is left to higher-level default expansions
    // so we can if necessary splice non-default and default values together.
    switch (data_type->vld_types[base_type])
    {
    case PoetsDataType::ISCALAR:
    case PoetsDataType::USCALAR:
    return default_value = "0";
    case PoetsDataType::FSCALAR:
    return default_value = "0.0";
    case PoetsDataType::SSCALAR:
    return default_value = "\"\"";
    // last case should never happen: an unknown type. Leave the default blank.
    default:
    return default_value;
    }
}


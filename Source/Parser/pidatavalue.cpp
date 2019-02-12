#include "pidatavalue.h"
#include <algorithm>

PIDataValue::PIDataValue(const QString& name, PIGraphObject *parent) : PIGraphLeaf(name, parent), elaborated_value(""), data_type(NULL)
{

}

// generates the initialisers for each variable in the data type
const QString& PIDataValue::elaborateDataValueStr(void)
{
    // abort any attempt to elaborate a value if there's no basis for elaboration (no value or no data type).
    // if the value has already been elaborated don't try to do it again. A bit awkward here. As is this will
    // fail if the type or value has been changed after a data value has already been elaborated. The other
    // option, equally ugly, is to make data_type private and access it via a method that resets elaborated_value,
    // and likewise make setValue reset elaborated_value.
    if (value().isEmpty() || (data_type == NULL) || !elaborated_value.isEmpty()) return elaborated_value;
    // get the whole JSON definition for the object(s)
    QJsonParseError j_val_err;
    QJsonDocument json_doc = QJsonDocument::fromJson(value().toUtf8(), &j_val_err);
    // we should not get a JSON array. If we do there is an error, just as if a JSON parse error occurred. If so, exit.
    if (j_val_err.error || json_doc.isArray() || json_doc.isNull()) return elaborated_value;
    QJsonObject top_obj;
    if (!json_doc.isEmpty()) top_obj = json_doc.object();
    // have to go through the data type's members rather than the JSON values because undefined values take the default.
    for (QVector<const PIGraphObject*>::const_iterator sub_obj = data_type->beginConstSubObjects(PIDataType::DTYPE); sub_obj < data_type->endConstSubObjects(PIDataType::DTYPE); sub_obj++)
    {
        elaborated_value.append(QString("%1, ").arg(elaborateSubValue(top_obj, dynamic_cast<const PIDataType*>(*sub_obj))));
    }
    // remove the trailing comma-space
    elaborated_value.chop(2);
    return elaborated_value = QString("{%1}").arg(elaborated_value);
}

// generates the initialiser list for a given declared data type.
QString PIDataValue::elaborateSubValue(const QJsonObject& in_obj, const PIDataType* sub_type)
{
    // Composites should be enclosed in braces. Scalars should remain bare.
    QString enclose = (sub_type->numElements() > 0 || sub_type->numSubObjects(PIDataType::DTYPE)) ? "{%1}" : "%1";
    QString elements("");
    QString subobj_key = subFieldInObj(in_obj, sub_type);
    QJsonValue in_val;
    QJsonArray in_subarray;
    QJsonObject in_subobj;
    if (!in_obj.isEmpty()) in_val = in_obj[subobj_key];
    // No value indicates we take the default.
    // To accommodate arrays, we repeat for the number of elements in the array and add a comma-space. We then remove the last comma-space.
    // Hence the buildup of string-construction operations here.
    if (in_val.isUndefined() || in_val.isNull())
    {
       elements = QString("%1, ").arg(sub_type->defaultValue()).repeated(abs(sub_type->numElements()));
       elements.chop(2);
       return enclose.arg(elements);
    }
    // return enclose.arg(QString("%1, ").arg(sub_type->defaultValue()).repeated(abs(sub_type->numElements())).chopped(2));
    // array types (which have positive definite numElements) should have a JSON array initialiser. If they don't there is some sort of error.
    if (sub_type->numElements() > 0 && !in_val.isArray()) return QString("");
    switch (sub_type->dataType()->vld_types[sub_type->dataTypeStr()])
    {
    case PoetsDataType::ISCALAR:
    if (sub_type->numElements() > 0)
    {
       // arrays need to be expanded and any missing elements filled with default values. Note that if the JSON array contains too many elements
       // this will take the first numElements() values. We may want to issue a warning when this happens.
       in_subarray = in_val.toArray();
       // more string construction operators here to: convert to an int, extract the value from the JSON array as a double, using the default from the datatype if the extracted value is invalid.
       for (int elem = 0; elem < std::min(in_subarray.size(), sub_type->numElements()); elem++) elements.append(QString("%1, ").arg(QVariant(in_subarray[elem].toDouble(sub_type->defaultValue().toDouble())).toInt()));
       // and finally append any missing defaults to the end. Remove the trailing comma-space with the chop(2) operation.
       QString default_elems(QString("%1, ").arg(sub_type->defaultValue()).repeated(std::max((sub_type->numElements() - in_subarray.size()), 0)));
       elements.append(default_elems);
       elements.chop(2);
       return enclose.arg(elements);
       // return enclose.arg(elements.append(QString("%1, ").arg(sub_type->defaultValue()).repeated(max((sub_type->numElements() - in_subarray.size()), 0))).chopped(2));
    }
    // crude conversion to an int can be improved (and bounds-checked?) in future and should be migrated to PoetsDataType as a static method.
    else return enclose.arg(QVariant(in_val.toDouble(sub_type->defaultValue().toInt())).toInt());
    case PoetsDataType::USCALAR:
    if (sub_type->numElements() > 0)
    {
       // arrays need to be expanded and any missing elements filled with default values. Note that if the JSON array contains too many elements
       // this will take the first numElements() values. We may want to issue a warning when this happens.
       in_subarray = in_val.toArray();
       // more string construction operators here to: convert to an int, extract the value from the JSON array as a double, using the default from the datatype if the extracted value is invalid.
       for (int elem = 0; elem < std::min(in_subarray.size(), sub_type->numElements()); elem++) elements.append(QString("%1, ").arg(QVariant(in_subarray[elem].toDouble(sub_type->defaultValue().toDouble())).toUInt()));
       // and finally append any missing defaults to the end. Remove the trailing comma-space with the chop(2) operation.
       QString default_elems(QString("%1, ").arg(sub_type->defaultValue()).repeated(std::max((sub_type->numElements() - in_subarray.size()), 0)));
       elements.append(default_elems);
       elements.chop(2);
       return enclose.arg(elements);
       // return enclose.arg(elements.append(QString("%1, ").arg(sub_type->defaultValue()).repeated(max((sub_type->numElements() - in_subarray.size()), 0))).chopped(2));
    }
    // crude conversion to a uint can be improved (and bounds-checked?) in future and should be migrated to PoetsDataType as a static method.
    else return enclose.arg(QVariant(in_val.toDouble(sub_type->defaultValue().toUInt())).toUInt());
    case PoetsDataType::FSCALAR:
    if (sub_type->numElements() > 0)
    {
       // expand the array as above.
       in_subarray = in_val.toArray();
       for (int elem = 0; elem < std::min(in_subarray.size(), sub_type->numElements()); elem++) elements.append(QString("%1, ").arg(in_subarray[elem].toDouble(sub_type->defaultValue().toDouble())));
       QString default_elems(QString("%1, ").arg(sub_type->defaultValue()).repeated(std::max((sub_type->numElements() - in_subarray.size()), 0)));
       elements.append(default_elems);
       elements.chop(2);
       return enclose.arg(elements);
       // return enclose.arg(elements.append(QString("%1, ").arg(sub_type->defaultValue()).repeated(max((sub_type->numElements() - in_subarray.size()), 0))).chopped(2));
    }
    else return enclose.arg(in_val.toDouble(sub_type->defaultValue().toDouble()));
    // strings are enclosed in double quotes after being extracted.
    case PoetsDataType::SSCALAR:
    if (sub_type->numElements() > 0)
    {
       // expand the array as above.
       in_subarray = in_val.toArray();
       for (int elem = 0; elem < std::min(in_subarray.size(), sub_type->numElements()); elem++) elements.append(QString("\"%1\", ").arg(in_subarray[elem].toString(sub_type->defaultValue())));
       QString default_elems(QString("\"%1\", ").arg(sub_type->defaultValue()).repeated(std::max((sub_type->numElements() - in_subarray.size()), 0)));
       elements.append(default_elems);
       elements.chop(2);
       return enclose.arg(elements);
       // return enclose.arg(elements.append(QString("\"%1\", ").arg(sub_type->defaultValue()).repeated(max((sub_type->numElements() - in_subarray.size()), 0))).chopped(2));
    }
    else
    {
        elements = QString("\"%1\", ").arg(in_val.toString(sub_type->defaultValue())).repeated(abs(sub_type->numElements()));
        elements.chop(2);
        return enclose.arg(elements);
    }
    // return enclose.arg(QString("\"%1\", ").arg(in_val.toString(sub_type->defaultValue())).repeated(abs(sub_type->numElements())).chopped(2));
    case PoetsDataType::STRUCT:
    if (sub_type->numElements() > 0)
    {
       // expand the array as above.
       in_subarray = in_val.toArray();
       for (int elem = 0; elem < std::min(in_subarray.size(), sub_type->numElements()); elem++)
       {
           in_subobj = in_subarray[elem].toObject();
           QString sub_elements("");
           // recursively expand subobjects. This will automatically insert the braces around the subobject.
           for (QVector<const PIGraphObject*>::const_iterator sub_obj = sub_type->beginConstSubObjects(PIDataType::DTYPE); sub_obj != sub_type->endConstSubObjects(PIDataType::DTYPE); sub_type++)
               sub_elements.append(QString("%1, ").arg(elaborateSubValue(in_subobj, dynamic_cast<const PIDataType*>(*sub_obj))));
           // then add it to the element array initialiser. Enclose each structure initialiser with braces.
           sub_elements.chop(2);
           elements.append(QString("{%1}, ").arg(sub_elements));
           // elements.append(QString("{%1}, ").arg(sub_elements.chopped(2)));
       }
       // then insert default values as needed to fill the array. As long as the entire default is being used the subtype can brace-enclose it.
       QString default_elems(QString("%1, ").arg(sub_type->defaultValue()).repeated(std::max((sub_type->numElements() - in_subarray.size()), 0)));
       elements.append(default_elems);
       elements.chop(2);
       return enclose.arg(elements);
       // return enclose.arg(elements.append(QString("%1, ").arg(sub_type->defaultValue()).repeated(max((sub_type->numElements() - in_subarray.size()), 0))).chopped(2));
    }
    else
    {
       // a singleton tuple (structure) object is expanded just like elements of an array.
       in_subobj = in_val.toObject(QJsonObject());
       for (QVector<const PIGraphObject*>::const_iterator sub_obj = sub_type->beginConstSubObjects(PIDataType::DTYPE); sub_obj != sub_type->endConstSubObjects(PIDataType::DTYPE); sub_type++)
           elements.append(QString("%1, ").arg(elaborateSubValue(in_subobj, dynamic_cast<const PIDataType*>(*sub_obj))));
       elements.chop(2);
       return enclose.arg(elements);
       // return enclose.arg(elements.chopped(2));
    }
    case PoetsDataType::UNION:
    {
    // extract the member name from the found key
    QString sub_member_name(subobj_key.split('.')[1]);
    QString sub_elem(".%1 = %2");
    // build an object from which the recursive descent can extract the value to set.
    in_subobj.insert(sub_member_name, in_val);
    // retrieve the submember
    const PIDataType* sub_member = dynamic_cast<const PIDataType*>(sub_type->constSubObject(PIDataType::DTYPE, sub_member_name));
    if (sub_type->numElements() > 0)
    {
        // expand the array as above.
        in_subarray = in_val.toArray();
        for (int elem = 0; elem < std::min(in_subarray.size(), sub_type->numElements()); elem++)
        {
            elements.append(QString("{%1}, ").arg(sub_elem.arg(sub_member_name).arg(elaborateSubValue(in_subobj, sub_member))));
        }
        // then insert default values as needed to fill the array. Like structs, union defaults will brace-enclose the returned value.
        QString default_elems(QString("%1, ").arg(sub_type->defaultValue()).repeated(std::max((sub_type->numElements() - in_subarray.size()), 0)));
        elements.append(default_elems);
        elements.chop(2);
        return enclose.arg(elements);
        // return enclose.arg(elements.append(QString("%1, ").arg(sub_type->defaultValue()).repeated(max((sub_type->numElements() - in_subarray.size()), 0))).chopped(2));
    }
    // singleton unions recurse down the subelement to be set.
    return enclose.arg(sub_elem.arg(sub_member_name).arg(elaborateSubValue(in_subobj, sub_member)));
    }
    // if the type doesn't match a valid POETS type then don't even try to initialise; there is an error.
    default:
    return QString("");
    }
}

// determines if a key for a subtype is present in a JSON initialiser.
QString PIDataValue::subFieldInObj(const QJsonObject& in_obj, const PIDataType* sub_type) const
{
     if (sub_type->dataTypeStr() == "union")
     {
        // with unions we need to search through looking for a match for the union name and its associated member.
        QString found_value;
        for (QVector<const PIGraphObject*>::const_iterator member = sub_type->beginConstSubObjects(PIDataType::DTYPE); member != sub_type->endConstSubObjects(PIDataType::DTYPE); sub_type++)
        {
            if (in_obj.contains(found_value = (sub_type->name() + QString(".%1").arg((*member)->name())))) return found_value;
        }
     }
     // by returning the base name of a union we can guarantee the key search will miss in the JSON object. Other type keys will be found or not found as may be the case.
     return sub_type->name();
}

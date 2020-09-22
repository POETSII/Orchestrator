#!/bin/bash
FILES=$(echo P_builder.{cpp,h})
for FILE in $FILES; do
    sed -i "s|pT->pD->|pGi->|g" -i "$FILE"
    sed -i "s|pT->pP_typdcl|pGi->pT|g" -i "$FILE"
    sed -i "s|pT->filename|pGi->par->filename|g" -i "$FILE"
    sed -i "s|P_task|GraphI_t|g" -i "$FILE"
    sed -i "s|P_message|EdgeI_t|g" -i "$FILE"
    sed -i "s|pT|pGi|g" -i "$FILE"
    sed -i "s|pP_devtyp|pT|g" -i "$FILE"
    sed -i "s|P_device|DevI_t|g" -i "$FILE"
    sed -i "s|P_devtyp|DevT_t|g" -i "$FILE"
    sed -i "s|P_pin|PinI_t|g" -i "$FILE"
    sed -i "s|P_message|EdgeI_t|g" -i "$FILE"
    sed -i "s|DevI_tv|DevI_v|g" -i "$FILE"
    sed -i "s|DevT_tv|DevT_v|g" -i "$FILE"
    sed -i "s|Task|Graph instance|g" -i "$FILE"
    sed -i "s|taskToCores|giToCores|g" -i "$FILE"
    sed -i "s|POETS::|OSFixes::|g" -i "$FILE"
done

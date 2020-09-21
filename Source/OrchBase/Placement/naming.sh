#!/bin/bash
FILES=$(echo *.{cpp,h} {Algorithms,Constraints,Exceptions}/*)
for FILE in $FILES; do
    sed -i "s|task->pD->|gi->|g" -i "$FILE"
    sed -i "s|task->pP_typdcl|gi->pT|g" -i "$FILE"
    sed -i "s|task->filename|gi->par->filename|g" -i "$FILE"
    sed -i "s|P_task|GraphI_t|g" -i "$FILE"
    sed -i "s|task|gi|g" -i "$FILE"
    sed -i "s|pP_devtyp|pT|g" -i "$FILE"
    sed -i "s|P_device|DevI_t|g" -i "$FILE"
    sed -i "s|P_devtyp|DevT_t|g" -i "$FILE"
    sed -i "s|P_pin|PinI_t|g" -i "$FILE"
    sed -i "s|P_message|EdgeI_t|g" -i "$FILE"
    sed -i "s|DevI_tv|DevI_v|g" -i "$FILE"
    sed -i "s|DevT_tv|DevT_v|g" -i "$FILE"
    sed -i "s|NoTask|NoGI|g" -i "$FILE"
    sed -i "s|TaskFinder|GraphFinder|g" -i "$FILE"
    sed -i "s|placedTasks|placedGraphs|g" -i "$FILE"
    sed -i "s|Task|Graph instance|g" -i "$FILE"
    sed -i "s|POETS::|OSFixes::|g" -i "$FILE"
done

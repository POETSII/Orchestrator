#ifndef __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINTCATEGORY_H
#define __ORCHESTRATOR_SOURCE_ORCHBASE_PLACEMENT_CONSTRAINTCATEGORY_H

/* Defines an enumerated type to organise constraints of different categories,
 * so that they can be searched and compared.
 *
 * See the placement documentation for further information. */

enum constraintCategory {maxDevicesPerThread,
                         maxThreadsPerCore,
                         fixDevice};
#endif

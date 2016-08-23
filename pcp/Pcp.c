#include "Pcp.h"
/*{
#include <pcp/pmapi.h>
}*/
int pcp_context = -1;
int lookupMetric(char *metric, pmAtomValue *atom, int type, int inst) {
   // Create context
   if(pcp_context == -1) {
      pcp_context = pmNewContext(PM_CONTEXT_LOCAL, "local:");
   }
   // Still no context, bail out
   if(pcp_context < 0) {
      return -1;
   }

   pmID pmid;

   // Get the PMID.
   if(pmLookupName(1, &metric, &pmid) < 0) {
      return -1;
   }

   // Do the fetch
   pmResult *result;
   if(pmFetch(1, &pmid, &result) < 0) {
      return -1;
   }

   // Extract
   pmValueSet *res = result[0].vset[0];
   int i;
   for(i = 0;i < inst; i++){
     pmExtractValue(res->valfmt, &res->vlist[i], type, &atom[i], type);
   }
   pmFreeResult(result);
  //  pmDestroyContext(pcp_context);

   return 0;
}

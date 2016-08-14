#include "metric_repository.h"

int pcp_init(char *host){
  sts = pmNewContext(PM_CONTEXT_HOST, host);
  if (sts < 0){
    fprintf(stderr, "Error connecting to pmcd on %s:%s\n", host, pmErrStr(sts));
    exit(1);
  }
}

int pcp_fetch(){
  sts = pmLookupName(METRIC_COUNT, metrics, &pmid[0]);
  if (sts < 0){
    fprintf(stderr, "Error looking up for following metrics\n");
    exit(1);
  }
  sts = pmFetch(2, pmid, &result);
  if (sts < 0) {
  fprintf(stderr, "Error fetching %s\n", pmErrStr(sts));
  exit(1);
  }

  int i;
  for(i = 0; i < METRIC_COUNT; i++){
    sts = pmLookupDesc(pmid[i], &desc[i]);
    if (sts < 0) {
    fprintf(stderr, "Error getting descriptor for %s: %s\n", metrics[i], pmErrStr(sts));
    exit(1);
    }
  }
}

int pcp_get_metric_result(char *metric_name, pmValueSet *vset, pmDesc *des){
  int i;
  for(i = 0; i < METRIC_COUNT; i++){
    if(strcmp(metrics[i], metric_name) == 0){
      sts = 0;
      break;
    }
    sts = -1;
  }
  if (sts < 0) {
  fprintf(stderr, "Could not find metric: [%s] in the result\n", metric_name);
  exit(1);
  }
  *vset = *result->vset[i];
  *des = desc[i];

  return sts;
}

// int main(){
//   pcp_init("local:");
//   pcp_fetch();
//   pmValueSet vsets;
//   pmDesc dess;
//   sts = pcp_get_metric_result("hinv.ncpu", &vsets, &dess);
//
//   pmPrintValue(stdout, vsets.valfmt, dess.type, &vsets.vlist[0], 1);
//   printf("\n" );
//   return 0;
// }

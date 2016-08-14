#include <pcp/pmapi.h>

#define METRIC_COUNT 2
char *metrics[] = {"mem.freemem","hinv.ncpu"};
pmID pmid[METRIC_COUNT];
pmDesc desc[METRIC_COUNT];
pmResult *result;
int sts;
int pcp_init(char *host);

int pcp_fetch();

int pcp_get_metric_result(char *metric, pmValueSet *vset, pmDesc *des);

/* The ThreadsWindowData structure is passed from
 * showthds() to ShowHelpBox() to MenuThreads()
 */

typedef struct {
  uint   execitem;                      /* currently highlighted thread      */
  uchar *pAllInfo;                      /* thread information block          */
} ThreadsWindowData;


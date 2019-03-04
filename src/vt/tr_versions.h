#ifndef _TR_VERSIONS_H_
#define _TR_VERSIONS_H_

#define LEVEL_FORMAT_PC         (0)
#define LEVEL_FORMAT_PSX        (1)
#define LEVEL_FORMAT_DC         (2)
#define LEVEL_FORMAT_OPENTOMB   (3)   // Maybe some day...

#define TR_I            (0)
#define TR_I_DEMO       (1)
#define TR_I_UB         (2)
#define TR_II           (3)
#define TR_II_DEMO      (4)
#define TR_III          (5)
#define TR_IV           (6)
#define TR_IV_DEMO      (7)
#define TR_V            (8)
#define TR_UNKNOWN      (127)

#define IS_TR_I(v)      ((v) < TR_II)
#define IS_TR_II(v)     ((v) >= TR_II && (v) < TR_III)
#define IS_TR_III(v)    ((v) >= TR_III && (v) < TR_IV)
#define IS_TR_IV(v)     ((v) >= TR_IV && (v) < TR_V)
#define IS_TR_V(v)      ((v) == TR_V)

#endif // _TR_VERSIONS_H_

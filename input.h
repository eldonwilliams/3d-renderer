#include "tigr.h"
typedef struct {
    int locked;
    int dx, dy;
} MouseLockState;

void updateLockedMouseMac(Tigr *screen, MouseLockState *m);
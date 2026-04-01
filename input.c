#ifdef __APPLE__
#include "input.h"
#include "tigr.h"
#include <ApplicationServices/ApplicationServices.h>
#include <objc/message.h>
#include <objc/objc.h>
#include <objc/runtime.h>

void updateLockedMouseMac(Tigr *screen, MouseLockState *m) {
  CGPoint p = CGEventGetLocation(CGEventCreate(NULL));

  CGGetLastMouseDelta(&m->dx, &m->dy);
  if (m->locked) {
    CGAssociateMouseAndMouseCursorPosition(false);
  } else {
    CGAssociateMouseAndMouseCursorPosition(true);
  }
}
#endif

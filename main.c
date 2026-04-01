#include "egraphics.h"
#include "input.h"
#include "tigr.h"
#include <stdio.h>
#include <stdlib.h>

MouseLockState mouseLockState = {0, 0, 0};
void movementLoop(Camera *camera, Tigr *screen) {
  Vec3 *cameraPos = &camera->position;

  if (tigrKeyHeld(screen, TK_SPACE)) {
    cameraPos->y -= 0.1;
  }
  if (tigrKeyHeld(screen, TK_SHIFT)) {
    cameraPos->y += 0.1;
  }

  if (tigrKeyHeld(screen, 'W')) {
    moveCamera(camera, (Vec3){0, 0, 0.1});
  }

  if (tigrKeyHeld(screen, 'S')) {
    moveCamera(camera, (Vec3){0, 0, -0.1});
  }

  if (tigrKeyHeld(screen, 'A')) {
    moveCamera(camera, (Vec3){-0.1, 0, 0});
  }

  if (tigrKeyHeld(screen, 'D')) {
    moveCamera(camera, (Vec3){0.1, 0, 0});
  }

  if (tigrKeyHeld(screen, TK_ESCAPE)) {
    *camera = (Camera){.position = {0, 0, 0}, .rotation = {0, 0, 0}};
  }

  if (tigrKeyDown(screen, 'C')) {
    mouseLockState.locked = !mouseLockState.locked;
  }

  updateLockedMouseMac(screen, &mouseLockState);
  if (mouseLockState.locked) {
    camera->rotation.y += mouseLockState.dx / 100.0;
    camera->rotation.x -= mouseLockState.dy / 100.0;
  }
}

int main() {
  Tigr *screen = tigrWindow(800, 800, "Eldon's 3D Renderer", TIGR_RETINA);
  float t = 0;
  Camera camera = {.position = {0, 0, 0}, .rotation = {0, 0, 0}};

  Vec3 *cameraPos = &camera.position;

  while (!tigrClosed(screen)) {
    tigrClear(screen, tigrRGB(0, 0, 0));

    for (int x = -5; x < 5; x++) {
      for (int z = -5; z < 5; z++) {
        drawObjectForCamera(screen, translate(cube, (Vec3){x * 2, -1, z *
        2}),
                            camera);
      }
    }

    drawObjectForCamera(
        screen, translate(rotateAroundY(cube, t), (Vec3){0, -4, 4}), camera);
    
    drawObjectForCamera(
        screen, translate(rotateAroundY(createTorus(10, 6, 1.5f, 0.75f), 0), (Vec3){0, -4, -4}), camera);

    movementLoop(&camera, screen);

    tigrPrint(screen, tfont, 120, 110, tigrRGB(0xff, 0xff, 0xff),
              "3D Rendering?");
    tigrPrint(screen, tfont, 120, 140, tigrRGB(0xff, 0xff, 0xff), "FPS: %.2f",
              1.0 / tigrTime());
    tigrPrint(screen, tfont, 120, 170, tigrRGB(0xff, 0xff, 0xff),
              "CAMERA: %.2f, %.2f, %.2f\n       %.2f, %.2f, %.2f",
              camera.position.x, camera.position.y, camera.position.z,
              camera.rotation.x, camera.rotation.y, camera.rotation.z);

    tigrUpdate(screen);
    t += 0.005;
  }
  tigrFree(screen);
  return 0;
}

#include "tigr.h"

// Structs

typedef struct {
  float x, y, z;
} Vec3;

typedef struct {
  float x, y;
} Vec2;

typedef struct {
  Vec3 *vertices;
  int vertexCount;
  int **faces;
  int faceCount;
} Object;

typedef struct {
	Vec3 position;
	Vec3 rotation;
} Camera;

// Primitives

extern const Object cube;
Object createTorus(int segU, int segV, float innerRadius, float outerRadius);

// Camera Manipulation

// Moves the camera by using the forward vector (based on the camera's rotation) and the right vector (perpendicular to the forward vector)
void moveCamera(Camera *camera, Vec3 translation);

// Transformations

// Projects a world space point to viewport space (x and y in range [-1, 1], z
// is depth)
int project(Vec3 point, Vec2 *out);

// Projects a viewport space to screen space for a given buffer
Vec2 viewportToScreen(Vec2 point, Tigr *dest);

// Transforms a world space point to camera space
Vec3 worldToCamera(Vec3 point, Camera camera);

// Object transformations

// Makes a copy of a given object
Object makeCopy(Object obj);

// Scales a object by a given factor
Object scale(Object obj, float scale);

// Translates a object by a given vector
Object translate(Object obj, Vec3 translation);

// Rotates an object around the Y axis
Object rotateAroundY(Object obj, float angle);

// Rotates an object around the X axis
Object rotateAroundX(Object obj, float angle);

// Rotates an object around the Z axis
Object rotateAroundZ(Object obj, float angle);

// Drawing Functions

// Draws a wireframe of the given object to the destination buffer
void drawObject(Tigr *dest, Object obj);
void drawObjectForCamera(Tigr *dest, Object obj, Camera camera);

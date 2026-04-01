#include "egraphics.h"
#include "math.h"
#include "tigr.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

const Object cube = {.vertices =
                         (Vec3[]){
                             {-1, -1, -1}, // 0
                             {1, -1, -1},  // 1
                             {1, 1, -1},   // 2
                             {-1, 1, -1},  // 3
                             {-1, -1, 1},  // 4
                             {1, -1, 1},   // 5
                             {1, 1, 1},    // 6
                             {-1, 1, 1}    // 7
                         },
                     .vertexCount = 8,
                     .faces =
                         (int *[]){
                             (int[]){0, 2, 1}, (int[]){0, 3, 2}, // back  (-z)
                             (int[]){4, 5, 6}, (int[]){4, 6, 7}, // front (+z)
                             (int[]){0, 4, 7}, (int[]){0, 7, 3}, // left  (-x)
                             (int[]){1, 6, 5}, (int[]){1, 2, 6}, // right (+x)
                             (int[]){3, 6, 2}, (int[]){3, 7, 6}, // top   (+y)
                             (int[]){0, 1, 5}, (int[]){0, 5, 4}  // bottom(-y)
                         },
                     .faceCount = 12};

// AI
static Vec3 vec3Add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static Vec3 vec3Sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static Vec3 vec3Scale(Vec3 v, float s) {
    return (Vec3){v.x * s, v.y * s, v.z * s};
}

static float vec3Dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static Vec3 vec3Cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

static Vec3 computeObjectCenter(const Object *obj) {
    Vec3 sum = {0, 0, 0};

    for (int i = 0; i < obj->vertexCount; i++) {
        sum = vec3Add(sum, obj->vertices[i]);
    }

    if (obj->vertexCount == 0) {
        return sum;
    }

    return vec3Scale(sum, 1.0f / (float)obj->vertexCount);
}

void fixObjectWinding(Object *obj) {
    if (!obj || !obj->vertices || !obj->faces) {
        return;
    }

    Vec3 center = computeObjectCenter(obj);

    for (int i = 0; i < obj->faceCount; i++) {
        int *face = obj->faces[i];
        if (!face) {
            continue;
        }

        int i0 = face[0];
        int i1 = face[1];
        int i2 = face[2];

        if (i0 < 0 || i0 >= obj->vertexCount ||
            i1 < 0 || i1 >= obj->vertexCount ||
            i2 < 0 || i2 >= obj->vertexCount) {
            continue;
        }

        Vec3 a = obj->vertices[i0];
        Vec3 b = obj->vertices[i1];
        Vec3 c = obj->vertices[i2];

        Vec3 ab = vec3Sub(b, a);
        Vec3 ac = vec3Sub(c, a);
        Vec3 normal = vec3Cross(ab, ac);

        Vec3 triCenter = vec3Scale(vec3Add(vec3Add(a, b), c), 1.0f / 3.0f);
        Vec3 outward = vec3Sub(triCenter, center);

        // If normal points inward, flip winding.
        if (vec3Dot(normal, outward) < 0.0f) {
            int temp = face[1];
            face[1] = face[2];
            face[2] = temp;
        }
    }
}
Object createTorus(int segU, int segV, float R, float r) {
  Object obj;

  obj.vertexCount = segU * segV;
  obj.vertices = malloc(sizeof(Vec3) * obj.vertexCount);

  // Generate vertices
  for (int i = 0; i < segU; i++) {
    float theta = (2.0f * M_PI * i) / segU;

    for (int j = 0; j < segV; j++) {
      float phi = (2.0f * M_PI * j) / segV;

      int index = i * segV + j;

      float x = (R + r * cosf(phi)) * cosf(theta);
      float y = (R + r * cosf(phi)) * sinf(theta);
      float z = r * sinf(phi);

      obj.vertices[index] = (Vec3){x, y, z};
    }
  }

  // Generate faces (quads), ensuring they have correct winding order for backface culling
  obj.faceCount = segU * segV;
  obj.faces = malloc(sizeof(int *) * obj.faceCount);

	for (int i = 0; i < segU; i++) {
		for (int j = 0; j < segV; j++) {
			int index = i * segV + j;
			int nextI = (i + 1) % segU;
			int nextJ = (j + 1) % segV;

			obj.faces[index] = malloc(sizeof(int) * 3);
			obj.faces[index][0] = index;
			obj.faces[index][1] = nextI * segV + j;
			obj.faces[index][2] = nextI * segV + nextJ;
		}
	}  
  
	// fixObjectWinding(&obj);
  return obj;
}
// AI

void moveCamera(Camera *camera, Vec3 translation) {
  float cosY = cos(camera->rotation.y);
  float sinY = sin(camera->rotation.y);

  Vec3 forward = {sinY, 0, cosY};
  Vec3 right = {cosY, 0, -sinY};

  camera->position.x += forward.x * translation.z + right.x * translation.x;
  camera->position.y += translation.y;
  camera->position.z += forward.z * translation.z + right.z * translation.x;
}

int project(Vec3 point, Vec2 *out) {
  if (point.z <= 0.1) {
    return 0; // Behind the near plane
  }
  out->x = point.x / point.z;
  out->y = point.y / point.z;
  return 1;
}

Vec2 viewportToScreen(Vec2 point, Tigr *dest) {
  return (Vec2){.x = (point.x + 1) / 2 * dest->w,
                .y = (point.y + 1) / 2 * dest->h};
}

Vec3 rotateX(Vec3 p, float a) {
  float c = cos(a), s = sin(a);
  return (Vec3){p.x, p.y * c - p.z * s, p.y * s + p.z * c};
}

Vec3 rotateY(Vec3 p, float a) {
  float c = cos(a), s = sin(a);
  return (Vec3){p.x * c + p.z * s, p.y, -p.x * s + p.z * c};
}

Vec3 rotateZ(Vec3 p, float a) {
  float c = cos(a), s = sin(a);
  return (Vec3){p.x * c - p.y * s, p.x * s + p.y * c, p.z};
}

Vec3 worldToCamera(Vec3 point, Camera camera) {
  point.x -= camera.position.x;
  point.y -= camera.position.y;
  point.z -= camera.position.z;

  point =
      rotateX(rotateY(rotateZ(point, -camera.rotation.z), -camera.rotation.y),
              -camera.rotation.x);

  return point;
}

Object makeCopy(Object obj) {
  Object copy;
  copy.vertexCount = obj.vertexCount;
  copy.faceCount = obj.faceCount;
  copy.vertices = malloc(sizeof(Vec3) * obj.vertexCount);
  copy.faces = malloc(sizeof(int *) * obj.faceCount);
  for (int i = 0; i < obj.vertexCount; i++) {
    copy.vertices[i] = obj.vertices[i];
  }
  for (int i = 0; i < obj.faceCount; i++) {
    copy.faces[i] = malloc(sizeof(int) * 3);
    for (int j = 0; j < 3; j++) {
      copy.faces[i][j] = obj.faces[i][j];
    }
  }
  return copy;
}

Object scale(Object obj, float scale) {
  Object result = makeCopy(obj);
  for (int i = 0; i < obj.vertexCount; i++) {
    result.vertices[i].x *= scale;
    result.vertices[i].y *= scale;
    result.vertices[i].z *= scale;
  }
  return result;
}

Object translate(Object obj, Vec3 translation) {
  Object result = makeCopy(obj);
  for (int i = 0; i < obj.vertexCount; i++) {
    result.vertices[i].x += translation.x;
    result.vertices[i].y += translation.y;
    result.vertices[i].z += translation.z;
  }
  return result;
}

Object rotateAroundZ(Object obj, float angle) {
  Object result = makeCopy(obj);
  float cosA = cos(angle);
  float sinA = sin(angle);
  for (int i = 0; i < obj.vertexCount; i++) {
    float x = result.vertices[i].x;
    float y = result.vertices[i].y;
    result.vertices[i].x = x * cosA - y * sinA;
    result.vertices[i].y = x * sinA + y * cosA;
  }
  return result;
}

Object rotateAroundY(Object obj, float angle) {
  Object result = makeCopy(obj);
  float cosA = cos(angle);
  float sinA = sin(angle);
  for (int i = 0; i < obj.vertexCount; i++) {
    float x = result.vertices[i].x;
    float z = result.vertices[i].z;
    result.vertices[i].x = x * cosA - z * sinA;
    result.vertices[i].z = x * sinA + z * cosA;
  }
  return result;
}

Object rotateAroundX(Object obj, float angle) {
  Object result = makeCopy(obj);
  float cosA = cos(angle);
  float sinA = sin(angle);
  for (int i = 0; i < obj.vertexCount; i++) {
    float y = result.vertices[i].y;
    float z = result.vertices[i].z;
    result.vertices[i].y = y * cosA - z * sinA;
    result.vertices[i].z = y * sinA + z * cosA;
  }
  return result;
}

// Draws the edge but clips to screen bounds
void drawEdge(Tigr *dest, Vec2 a, Vec2 b) {
  const float LIM = 100000; // Arbitrary large value to prevent overflow
  if (a.x > LIM || a.x < -LIM || a.y > LIM || a.y < -LIM || b.x > LIM ||
      b.x < -LIM || b.y > LIM || b.y < -LIM) {
    return;
  }

  if ((a.x < 0 && b.x < 0) || (a.x > dest->w && b.x > dest->w) ||
      (a.y < 0 && b.y < 0) || (a.y > dest->h && b.y > dest->h)) {
    return;
  }
  tigrLine(dest, a.x, a.y, b.x, b.y, tigrRGB(0xff, 0xff, 0xff));
}

static float edgeFunction(Vec2 a, Vec2 b, Vec2 p) {
  return (p.x - a.x) * (b.y - a.y) - (p.y - a.y) * (b.x - a.x);
}

void fillFace(Tigr *bmp, Vec2 p0, Vec2 p1, Vec2 p2, TPixel color) {
  // Normalize winding so inside test is always >= 0
  float area = edgeFunction(p0, p1, p2);
  if (area == 0.0f)
    return;

  if (area < 0.0f) {
    Vec2 tmp = p1;
    p1 = p2;
    p2 = tmp;
    area = -area;
  }

  int minX = (int)floorf(fminf(p0.x, fminf(p1.x, p2.x)));
  int maxX = (int)ceilf(fmaxf(p0.x, fmaxf(p1.x, p2.x)));
  int minY = (int)floorf(fminf(p0.y, fminf(p1.y, p2.y)));
  int maxY = (int)ceilf(fmaxf(p0.y, fmaxf(p1.y, p2.y)));

  if (minX < 0)
    minX = 0;
  if (minY < 0)
    minY = 0;
  if (maxX >= bmp->w)
    maxX = bmp->w - 1;
  if (maxY >= bmp->h)
    maxY = bmp->h - 1;

  if (minX > maxX || minY > maxY)
    return;

  Vec2 start = {minX + 0.5f, minY + 0.5f};

  float w0_row = edgeFunction(p1, p2, start);
  float w1_row = edgeFunction(p2, p0, start);
  float w2_row = edgeFunction(p0, p1, start);

  // Correct increments
  float w0_dx = (p2.y - p1.y);
  float w1_dx = (p0.y - p2.y);
  float w2_dx = (p1.y - p0.y);

  float w0_dy = -(p2.x - p1.x);
  float w1_dy = -(p0.x - p2.x);
  float w2_dy = -(p1.x - p0.x);

  for (int y = minY; y <= maxY; y++) {
    float w0 = w0_row;
    float w1 = w1_row;
    float w2 = w2_row;

    TPixel *row = &bmp->pix[y * bmp->w + minX];

    for (int x = minX; x <= maxX; x++) {
      if (w0 >= 0.0f && w1 >= 0.0f && w2 >= 0.0f) {
        *row = color;
      }

      w0 += w0_dx;
      w1 += w1_dx;
      w2 += w2_dx;
      row++;
    }

    w0_row += w0_dy;
    w1_row += w1_dy;
    w2_row += w2_dy;
  }
}

void drawObject(Tigr *dest, Object object) {
  Vec3 *vertices = object.vertices;
  for (int i = 0; i < object.faceCount; i++) {
    int *face = object.faces[i];
    Vec2 p1, p2, p3;
    int projectionFlag = 0;
    projectionFlag |= project(vertices[face[0]], &p1);
    projectionFlag |= project(vertices[face[1]], &p2);
    projectionFlag |= project(vertices[face[2]], &p3);

    p1 = viewportToScreen(p1, dest);
    p2 = viewportToScreen(p2, dest);
    p3 = viewportToScreen(p3, dest);

    drawEdge(dest, p1, p2);
    drawEdge(dest, p2, p3);
    drawEdge(dest, p3, p1);
  }
}

float area2(Vec2 a, Vec2 b, Vec2 c) {
  return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

Vec3 cross(Vec3 a, Vec3 b) {
  return (Vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
                a.x * b.y - a.y * b.x};
}

Vec3 sub(Vec3 a, Vec3 b) { return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z}; }

int dot(Vec3 a, Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 normalize(Vec3 v) {
  float length = sqrt(dot(v, v));
  return (Vec3){v.x / length, v.y / length, v.z / length};
}

Vec3 scaleVec(Vec3 v, float s) { return (Vec3){v.x * s, v.y * s, v.z * s}; }

const Vec3 lightDir = {1, 1, 0}; // Light coming from the camera direction

void drawObjectForCamera(Tigr *dest, Object object, Camera camera) {
  Vec3 *vertices = object.vertices;

  Vec3 forward = {
		cos(camera.rotation.x) * sin(camera.rotation.y),
		-sin(camera.rotation.x),
		cos(camera.rotation.x) * cos(camera.rotation.y)
	};

  for (int i = 0; i < object.faceCount; i++) {
    int *face = object.faces[i];

    Vec3 ab = sub(vertices[face[1]], vertices[face[0]]);
    Vec3 ac = sub(vertices[face[2]], vertices[face[0]]);
    Vec3 normal = scaleVec(cross(ac, ab), 1.0f);
    float intensity = dot(forward, normal) / (sqrt(dot(normal, normal)) * sqrt(dot(forward, forward)));
    // if (intensity <= 0) {
    //   continue; // Backface culling and no lighting
    // }

    Vec2 p1, p2, p3;
    int projectionFlag = 0;
    projectionFlag |= project(worldToCamera(vertices[face[0]], camera), &p1);
    projectionFlag |= project(worldToCamera(vertices[face[1]], camera), &p2);
    projectionFlag |= project(worldToCamera(vertices[face[2]], camera), &p3);

    if (projectionFlag == 0) {
    p1 = viewportToScreen(p1, dest);
    p2 = viewportToScreen(p2, dest);
    p3 = viewportToScreen(p3, dest);

    // fillFace(dest, p1, p2, p3, tigrRGB(((i * 1.0) / object.faceCount) * 255, 0x00, 0x00));
      continue;
    }

    if (area2(p1, p2, p3) >= 0) {
    p1 = viewportToScreen(p1, dest);
    p2 = viewportToScreen(p2, dest);
    p3 = viewportToScreen(p3, dest);

    // fillFace(dest, p1, p2, p3, tigrRGB(((i * 1.0) / object.faceCount) * 255, 0x00, 0x00));
      continue; // Backface culling
    }

    p1 = viewportToScreen(p1, dest);
    p2 = viewportToScreen(p2, dest);
    p3 = viewportToScreen(p3, dest);

    fillFace(dest, p1, p2, p3, tigrRGB(0x00, ((i * 1.0) / object.faceCount) * 255, 0x00));
    drawEdge(dest, p1, p2);
    drawEdge(dest, p2, p3);
    drawEdge(dest, p3, p1);
  }
}

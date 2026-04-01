int clamp(int v, int min, int max) {
  if (v < min)
    return min;
  if (v > max)
    return max;
  return v;
}

float clampf(float v, float min, float max) {
  if (v < min)
    return min;
  if (v > max)
    return max;
  return v;
}

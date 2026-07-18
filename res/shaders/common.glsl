#define PI 3.14159265358979f
#define TAU (2.f * PI)
#define PHI 1.6180339f
#define GOLDEN_ANGLE (TAU * (2.f - PHI))

#define sq(x) (x*x)
#define saturate(x) clamp(x, 0.f, 1.f)
#define safe_sqrt(x) sqrt(max(x, 0.f))

vec2 complexMultiply(vec2 n1, vec2 n2) {
  // (a + bi) * (c + di) = ac + adi + bci + bdi^2
  // Since i^2 = -1:
  // Real part:      ac - bd
  // Imaginary part: ad + bc
  return vec2(
    n1.x * n2.x - n1.y * n2.y,
    n1.x * n2.y + n1.y * n2.x
  );
}

vec2 complexExp(vec2 a) {
  return vec2(cos(a.y), sin(a.y)) * exp(a.x);
}


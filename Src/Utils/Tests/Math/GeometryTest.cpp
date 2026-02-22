#include <iostream>
#include <cassert>
#include <cmath>
#include "Tools/Math/Geometry.h"

// Simple approx equality for floats
bool approx(float a, float b, float epsilon = 0.001f) {
  return std::abs(a - b) < epsilon;
}

bool approx(const Vector2f& a, const Vector2f& b, float epsilon = 0.001f) {
  return (a - b).norm() < epsilon;
}

int main() {
  std::cout << "Testing Geometry::raycastCircle..." << std::endl;

  Geometry::Circle circle(Vector2f(0.f, 0.f), 10.f);
  Vector2f rayBase;
  Vector2f rayDirection;
  Vector2f result1, result2;
  int intersections;

  // Case 1: No intersection (ray outside, pointing away)
  rayBase = Vector2f(20.f, 0.f);
  rayDirection = Vector2f(1.f, 0.f); // pointing away
  intersections = Geometry::raycastCircle(circle, rayBase, rayDirection, result1, result2);
  assert(intersections == 0);
  std::cout << "Case 1 Passed: No intersection (pointing away)" << std::endl;

  // Case 2: No intersection (ray outside, passing by)
  rayBase = Vector2f(20.f, 20.f);
  rayDirection = Vector2f(-1.f, 0.f); // passing above
  intersections = Geometry::raycastCircle(circle, rayBase, rayDirection, result1, result2);
  assert(intersections == 0);
  std::cout << "Case 2 Passed: No intersection (passing by)" << std::endl;

  // Case 3: Tangent
  rayBase = Vector2f(20.f, 10.f);
  rayDirection = Vector2f(-1.f, 0.f); // tangent at (0, 10)
  intersections = Geometry::raycastCircle(circle, rayBase, rayDirection, result1, result2);
  assert(intersections == 1);
  assert(approx(result1, Vector2f(0.f, 10.f)));
  assert(approx(result2, Vector2f(0.f, 10.f)));
  std::cout << "Case 3 Passed: Tangent" << std::endl;

  // Case 4: Two intersections (passing through center)
  rayBase = Vector2f(20.f, 0.f);
  rayDirection = Vector2f(-1.f, 0.f); // through center
  intersections = Geometry::raycastCircle(circle, rayBase, rayDirection, result1, result2);
  // Expected: intersections at (10, 0) and (-10, 0)
  // Wait, raycastCircle returns points relative to rayBase? No, absolute positions.
  // The implementation logic suggests it returns 2 intersections.
  // result1 should be the closer one?
  // u1 is projection of u (center - rayBase) on rayDirection.
  // center=(0,0), rayBase=(20,0), u=(-20,0). rayDir=(-1,0). u.dot(rayDir) = 20. u1 = (-1,0)*20 = (-20,0).
  // m = sqrt(100 - 0) = 10.
  // result1 = rayBase + u1 + unitDir * m = (20,0) + (-20,0) + (-1,0)*10 = (-10, 0).
  // result2 = rayBase + u1 - unitDir * m = (20,0) + (-20,0) - (-1,0)*10 = (10, 0).
  // Wait, order might be different. Let's check logic.
  // But definitely 2 intersections.

  // With the BUG (missing return), it might return garbage or crash.
  // Or it might return result of last expression? No, C++ doesn't work like that.
  // But raycastCircle implementation ends with:
  /*
  } else { // ray begins inside circle, so only one instersect
    result1 = rayBase + u1 + unitDir * m;
  }
  */
  // If u.norm() > circle.radius (our case), it executes the 'if' block.
  // And falls through to end of function without return.

  std::cout << "Running Case 4 (Two intersections)..." << std::endl;
  intersections = Geometry::raycastCircle(circle, rayBase, rayDirection, result1, result2);
  // assert(intersections == 2); // This will likely fail or return garbage.
  if (intersections == 2) {
      std::cout << "Case 4 Passed: 2 intersections returned (Unexpected if bug exists)" << std::endl;
  } else {
      std::cout << "Case 4 Failed: returned " << intersections << " (Expected 2)" << std::endl;
  }

  // Case 5: Ray starts inside
  rayBase = Vector2f(5.f, 0.f);
  rayDirection = Vector2f(1.f, 0.f);
  intersections = Geometry::raycastCircle(circle, rayBase, rayDirection, result1, result2);
  // Expected: intersection at (10, 0).
  // result1 should be (10,0).
  // It enters the 'else' block.
  // And falls through without return.

  std::cout << "Running Case 5 (Inside)..." << std::endl;
  // assert(intersections == 1); // This will likely fail.
  if (intersections == 1) {
      std::cout << "Case 5 Passed: 1 intersection returned (Unexpected if bug exists)" << std::endl;
  } else {
      std::cout << "Case 5 Failed: returned " << intersections << " (Expected 1)" << std::endl;
  }

  return 0;
}

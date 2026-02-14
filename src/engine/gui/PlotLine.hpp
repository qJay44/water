#pragma once

#include <functional>
#include <vector>

class PlotLine {
public:
  PlotLine(const std::string& label, std::function<float(float)> formula);

  void update(float xmin, float xmax, int points = 100);
  void render(const vec4& color = vec4(0.2f, 0.7f, 1.0f, 1.0f), float weight = 1.f);
  void render(const vec3& color = vec3(0.2f, 0.7f, 1.0f), float weight = 1.f);

private:
  std::string label;
  std::function<float(float)> formula;

  std::vector<float> xData;
  std::vector<float> yData;
};


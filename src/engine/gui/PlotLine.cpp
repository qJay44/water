// #include "PlotLine.hpp"

// #include "implot.h"

// PlotLine::PlotLine(const std::string& label, std::function<float(float)> formula)
//   : label(label), formula(formula) {
//   update(0.f, 5.f, 100);
// }

// void PlotLine::update(float xmin, float xmax, int points) {
//   xData.resize(points);
//   yData.resize(points);
//   float step = (xmax - xmin) / (points - 1);

//   for (int i = 0; i < points; i++) {
//     float x = xmin + (i * step);
//     xData[i] = x;
//     yData[i] = formula(x);
//   }
// }

// void PlotLine::render(const vec4& color, float weight) {
//   ImPlot::SetNextLineStyle(color, weight);
//   ImPlot::PlotLine(label.c_str(), xData.data(), yData.data(), xData.size());
// }

// void PlotLine::render(const vec3& color, float weight) {
//   render(ImVec4(color, 1.f), weight);
// }


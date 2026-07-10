#pragma once

#include "nlohmann/json.hpp"
#include <fstream>

namespace water {
  void loadPreset(auto& cfg, std::string_view name) {
    fspath path = fspath("res/data/water") / name;

    std::ifstream f(path);

    if (f.is_open()) {
      nlohmann::json j;
      f >> j;
      j.get_to(cfg);
      f.close();
    } else {
      warning("[Water::loadPreset] Could not open the file [{}]", path.string());
    }
  }

  void savePreset(auto& cfg, std::string_view name) {
    fspath path = fspath("res/data/water") / name;
  std::filesystem::create_directories(path.parent_path());

    std::ofstream f(path);

    if (f.is_open()) {
      nlohmann::json j = cfg;
      f << j.dump(2);
      f.close();
    } else {
      error("[Water::savePreset] Could not open the file [{}]", path.string());
    }
  }

} // namespace water


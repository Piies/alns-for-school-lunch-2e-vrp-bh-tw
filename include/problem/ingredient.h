#pragma once

#include <vector>
#include <string>


struct Ingredient{
  std::string name;
  double tray_to_kg_ration;

  Ingredient(std::string name, double tray_to_kg_ration) : name(name), tray_to_kg_ration(tray_to_kg_ration) {};
};

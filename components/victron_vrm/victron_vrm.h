#pragma once
#include "esphome/core/component.h"
#include "esphome/components/smart_display/plugin_api.h"
#include "model.h"

namespace esphome::victron_vrm {

// The plugin: an ESPHome component and a tessera::Plugin. It registers its tile types in setup().
class VictronVrm : public Component, public tessera::Plugin {
 public:
  void setup() override;
  Model model(JsonObjectConst data) const;
  float get_setup_priority() const override { return setup_priority::DATA; }
};

}  // namespace esphome::victron_vrm

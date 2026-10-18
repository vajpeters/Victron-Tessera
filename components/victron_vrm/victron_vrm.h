#pragma once
#include "esphome/core/component.h"
#include "esphome/components/smart_display/plugin_api.h"
#include "model.h"
#include "esphome/components/text_sensor/text_sensor.h"
#include "esphome/components/switch/switch.h"
#include "esphome/components/number/number.h"

namespace esphome::victron_vrm {

// The plugin: an ESPHome component and a tessera::Plugin. It registers its tile types in setup().
class VictronVrm : public Component, public tessera::Plugin {
 public:
  void setup() override;
  bool settings(tessera::SettingsPage &page) override;
  void set_reading(unsigned metric, text_sensor::TextSensor *value, text_sensor::TextSensor *unit) {
    readings_[metric] = value; units_[metric] = unit;
  }
  void set_source_time(text_sensor::TextSensor *value) { source_time_ = value; }
  void set_battery_reverse(switch_::Switch *value) { battery_reverse_ = value; }
  void set_grid_reverse(switch_::Switch *value) { grid_reverse_ = value; }
  void set_max_age(number::Number *value) { max_age_ = value; }
  Model snapshot() const;
  float get_setup_priority() const override { return setup_priority::DATA; }
 private:
  text_sensor::TextSensor *readings_[COUNT]{}, *units_[COUNT]{}, *source_time_{};
  switch_::Switch *battery_reverse_{}, *grid_reverse_{};
  number::Number *max_age_{};
};

}  // namespace esphome::victron_vrm

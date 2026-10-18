#include "victron_vrm.h"
#include "flow_view.h"
#include <algorithm>
#include <cstdio>
#include <string>

namespace esphome::victron_vrm {

namespace {
void set_attribute(Model &model, Metric metric, JsonObjectConst attributes, const char *value_key,
                   const char *unit_key, const char *fallback_unit) {
  const JsonVariantConst raw = attributes[value_key];
  if (raw.isNull()) return;
  const JsonVariantConst configured_unit = attributes[unit_key];
  const char *unit = configured_unit.is<const char *>() ? configured_unit.as<const char *>() : fallback_unit;
  if (raw.is<const char *>()) {
    model.set(metric, raw.as<const char *>(), unit);
    return;
  }
  if (raw.is<float>() || raw.is<double>() || raw.is<int>() || raw.is<long>())
    model.set(metric, raw.as<double>(), unit);
}

void set_entity(Model &model, Metric metric, JsonObjectConst entities, const char *key, bool battery = false) {
  const JsonObjectConst source = entities[key].as<JsonObjectConst>();
  if (source.isNull()) return;
  const JsonObjectConst attributes = source["attributes"].as<JsonObjectConst>();
  const JsonVariantConst unit_value = attributes["unit_of_measurement"];
  const char *unit = unit_value.is<const char *>() ? unit_value.as<const char *>() : nullptr;
  const JsonVariantConst state = source["state"];
  const Metric target = battery && unit && std::strcmp(unit, "%") ? BATTERY : metric;
  if (state.is<const char *>()) model.set(target, state.as<const char *>(), unit);
  else if (!state.isNull()) model.set(target, state.as<double>(), unit);
  const JsonVariantConst updated = source["updated"];
  uint32_t moment = 0;
  if (updated.is<const char *>()) {
    double parsed = 0;
    if (numeric(updated.as<const char *>(), parsed) && parsed > 0 && parsed <= UINT32_MAX) moment = static_cast<uint32_t>(parsed);
  } else if (!updated.isNull()) {
    const double parsed = updated.as<double>();
    if (std::isfinite(parsed) && parsed > 0 && parsed <= UINT32_MAX) moment = static_cast<uint32_t>(parsed);
  }
  if (moment && (!model.at || moment < model.at)) model.at = moment;
  if (source["wait"].is<const char *>()) model.disconnected = true;
}
}  // namespace

Model VictronVrm::model(JsonObjectConst data) const {
  Model m;
  const JsonObjectConst attributes = data["attributes"].as<JsonObjectConst>();
  const JsonVariantConst soc = data["state"];
  const JsonVariantConst soc_unit = attributes["unit_of_measurement"];
  const char *unit = soc_unit.is<const char *>() ? soc_unit.as<const char *>() : "%";
  if (soc.is<const char *>()) m.set(SOC, soc.as<const char *>(), unit);
  else if (!soc.isNull()) m.set(SOC, soc.as<double>(), unit);
  set_attribute(m, BATTERY, attributes, "battery_power", "battery_power_unit", "W");
  set_attribute(m, SOLAR, attributes, "solar_power", "solar_power_unit", "W");
  set_attribute(m, LOAD, attributes, "load_power", "load_power_unit", "W");
  set_attribute(m, GRID, attributes, "grid_power", "grid_power_unit", "W");
  set_attribute(m, VOLTAGE, attributes, "battery_voltage", "battery_voltage_unit", "V");
  set_attribute(m, CURRENT, attributes, "battery_current", "battery_current_unit", "A");
  set_attribute(m, CRITICAL_LOAD, attributes, "critical_load_power", "critical_load_power_unit", "W");
  const JsonVariantConst source_time = attributes["source_time"];
  if (source_time.is<const char *>()) m.set_time(source_time.as<const char *>());
  else if (!source_time.isNull()) m.set_time(source_time.as<double>());
  const JsonObjectConst entities = data["entities"].as<JsonObjectConst>();
  if (!entities.isNull()) {
    set_entity(m, GRID, entities, "grid");
    set_entity(m, LOAD, entities, "ac_load");
    set_entity(m, CRITICAL_LOAD, entities, "critical_load");
    set_entity(m, SOC, entities, "battery", true);
    set_entity(m, SOLAR, entities, "solar");
  }
  m.disconnected = m.disconnected || data["wait"].is<const char *>();
  return m;
}
// Every object belongs to the current tile/card. Updates only paint existing labels.
class DashboardView {
 public:
  void create(lv_obj_t *parent, int width, int height, const tessera::Plugin *plugin, bool detail) {
    plugin_ = plugin; width_ = width;
    flow_ = width >= ui::mm(110) && height >= ui::mm(75);
    if (flow_) { flow_view_.create(parent, width, height, plugin); return; }
    const int body = ui::line_height(Font::BODY), gap = ui::px(4);
    footer_ = ui::label(parent, Font::BODY, theme::MUTED);
    lv_obj_set_size(footer_, width, body); lv_obj_set_pos(footer_, 0, std::max(0, height - body));
    const int room = std::max(0, height - body - gap);
    columns_ = width >= ui::mm(detail ? 45 : 60) ? 2 : 1;
    const int cell_min = body + ui::line_height(Font::TITLE) + ui::px(8);
    const int rows = std::max(1, room / std::max(1, cell_min));
    count_ = std::min(detail ? 7 : 5, rows * columns_);
    if (count_ < columns_) columns_ = 1;
    cell_height_ = room / std::max(1, (count_ + columns_ - 1) / columns_);
    cell_width_ = std::max(1, (width - gap * (columns_ - 1)) / columns_);
    for (int i = 0; i < count_; ++i) {
      const int x = (i % columns_) * (cell_width_ + gap), y = (i / columns_) * cell_height_;
      names_[i] = ui::label(parent, Font::BODY, theme::MUTED);
      lv_obj_set_size(names_[i], cell_width_, body); lv_obj_set_pos(names_[i], x, y);
      values_[i] = ui::label(parent, Font::TITLE, theme::INK);
      lv_obj_set_width(values_[i], cell_width_); lv_obj_set_pos(values_[i], x, y + body);
      bars_[i] = ui::block(parent, theme::TRACK);
      lv_obj_set_size(bars_[i], cell_width_, std::max(1, ui::px(2)));
      lv_obj_set_pos(bars_[i], x, y + cell_height_ - gap);
    }
  }
  void theme() {
    if (flow_) { flow_view_.theme(); return; }
    ui::set_color(footer_, theme::MUTED);
    for (int i = 0; i < count_; ++i) {
      ui::set_color(names_[i], theme::MUTED); ui::set_color(values_[i], theme::INK);
      ui::set_fill(bars_[i], tessera::Tone::NORMAL, theme::TRACK);
    }
  }
  void paint(const Model &m, uint32_t now) {
    if (flow_) { flow_view_.paint(m, now); return; }
    const Sample battery = m.get(BATTERY), grid = m.get(GRID);
    const char *battery_key = !battery.valid ? "battery_power" :
      battery.value > 30 ? "charging" : battery.value < -30 ? "discharging" : "idle";
    std::string names[7] = {plugin_->text("soc"), plugin_->text("solar"), plugin_->text("load"),
      plugin_->text(grid.valid ? (grid.value > 0 ? "import" : grid.value < 0 ? "export" : "grid") : "grid"),
      plugin_->text(battery_key), plugin_->text("voltage"), plugin_->text("current")};
    std::string values[7] = {value(m.get(SOC), "%"), value(m.get(SOLAR), "W"), value(m.get(LOAD), "W"),
      value(grid, "W", 0, true), value(battery, "W", 0, true), value(m.get(VOLTAGE), "V", 1), value(m.get(CURRENT), "A", 1)};
    const Health health = m.health(now);
    for (int i = 0; i < count_; ++i) {
      ui::set_text(names_[i], names[i]); Font face = Font::BODY;
      for (Font f : {Font::VALUE, Font::HEADLINE, Font::TITLE, Font::BODY}) {
        if (ui::text_width(values[i], f) <= cell_width_ &&
            ui::line_height(f) <= cell_height_ - ui::line_height(Font::BODY) - ui::px(8)) {
          face = f; break;
        }
      }
      ui::set_font(values_[i], face); ui::set_text(values_[i], values[i]);
      ui::set_fill(bars_[i], health == Health::OK ? tessera::Tone::ACCENT : tessera::Tone::NORMAL, theme::TRACK);
    }
    std::string footer = plugin_->text(health_key(health));
    if (m.at && now && m.at <= now && now - m.at < 86400u) {
      const std::string with_time = footer + " " + tessera::clock_text(m.at);
      if (ui::text_width(with_time, Font::BODY) <= width_) footer = with_time;
    }
    ui::set_text(footer_, footer);
    ui::set_color(footer_, health == Health::OLD ? theme::INK : theme::MUTED);
  }
 private:
  bool flow_ = false;
  FlowView flow_view_;
  const tessera::Plugin *plugin_ = nullptr;
  int width_ = 0, count_ = 0, columns_ = 1, cell_width_ = 0, cell_height_ = 0;
  lv_obj_t *footer_ = nullptr, *names_[7]{}, *values_[7]{}, *bars_[7]{};
};
class OverviewTile : public tessera::Tile {
 public:
  explicit OverviewTile(VictronVrm *plugin) : plugin_(plugin) {}
  void create(const tessera::TileContext &c) override {
    tile_ = c.tile; entity_ = c.entity; view_.create(c.parent, c.width, c.height, plugin_, false);
  }
  void on_state(JsonObjectConst data) override { model_ = plugin_->model(data); dirty_ = true; }
  void on_tick(uint32_t now) override {
    const Health health = model_.health(now);
    if (!dirty_ && health == health_) return;
    dirty_ = false; health_ = health; view_.paint(model_, now);
  }
  void on_theme() override { view_.theme(); }
  void on_tap() override { tessera::open_card(plugin_->plugin_id(), "dashboard", entity_.c_str(), tile_); }
 private:
  VictronVrm *plugin_;
  int tile_ = -1;
  tessera::Text entity_;
  Model model_;
  Health health_ = Health::EMPTY;
  bool dirty_ = true;
  DashboardView view_;
};
class DashboardCard : public tessera::Card {
 public:
  explicit DashboardCard(VictronVrm *plugin) : plugin_(plugin) {}
  void open(const tessera::CardContext &c) override { view_.create(c.parent, c.width, c.height, plugin_, true); }
  void on_state(JsonObjectConst data) override { model_ = plugin_->model(data); dirty_ = true; }
  void on_tick(uint32_t now) override {
    const Health health = model_.health(now);
    if (!dirty_ && health == health_) return;
    dirty_ = false; health_ = health; view_.paint(model_, now);
  }
  void on_theme() override { view_.theme(); }
 private:
  VictronVrm *plugin_;
  Model model_;
  Health health_ = Health::EMPTY;
  bool dirty_ = true;
  DashboardView view_;
};
void VictronVrm::setup() {
  add_tile("overview", [this]() { return new OverviewTile(this); });
  add_card("dashboard", [this]() { return new DashboardCard(this); }, true);
}
}  // namespace esphome::victron_vrm

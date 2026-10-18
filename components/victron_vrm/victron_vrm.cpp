#include "victron_vrm.h"
#include "esphome/components/api/api_server.h"
#include <algorithm>
#include <cstdio>
#include <string>

namespace esphome::victron_vrm {
using tessera::Font;
namespace ui = tessera::ui;

Model VictronVrm::snapshot() const {
  Model m;
  for (unsigned i = 0; i < COUNT; ++i) {
    if (readings_[i] && units_[i] && readings_[i]->has_state() && units_[i]->has_state())
      m.set(static_cast<Metric>(i), readings_[i]->state.c_str(), units_[i]->state.c_str());
  }
  if (source_time_ && source_time_->has_state()) m.set_time(source_time_->state.c_str());
  m.signs(battery_reverse_ && battery_reverse_->state, grid_reverse_ && grid_reverse_->state);
  if (max_age_ && max_age_->has_state() && std::isfinite(max_age_->state))
    m.max_age = static_cast<uint32_t>(max_age_->state);
  // No clients is a definite loss of the HA feed. A connected client alone does not prove source freshness.
  m.disconnected = !api::global_api_server || !api::global_api_server->is_connected();
  return m;
}
static std::string value(Sample sample, const char *unit, unsigned precision = 0, bool absolute = false) {
  if (!sample.valid) return "--";
  char text[48]; const double v = absolute ? std::fabs(sample.value) : sample.value;
  if (!strcmp(unit, "W") && std::fabs(v) >= 1000)
    snprintf(text, sizeof(text), std::fabs(v) >= 100000 ? "%.0f kW" : "%.1f kW", v / 1000);
  else snprintf(text, sizeof(text), "%.*f %s", static_cast<int>(precision), v, unit);
  return text;
}
static const char *health_key(Health h) {
  switch (h) {
    case Health::OLD: return "old";
    case Health::PARTIAL: return "partial";
    case Health::UNKNOWN_TIME: return "unknown_time";
    case Health::CLOCK: return "clock";
    case Health::OK: return "measured";
    default: return "empty";
  }
}

// Every object belongs to the current tile/card. Updates only paint existing labels.
class DashboardView {
 public:
  void create(lv_obj_t *parent, int width, int height, const tessera::Plugin *plugin, bool detail) {
    plugin_ = plugin; width_ = width;
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
    ui::set_color(footer_, theme::MUTED);
    for (int i = 0; i < count_; ++i) {
      ui::set_color(names_[i], theme::MUTED); ui::set_color(values_[i], theme::INK);
      ui::set_fill(bars_[i], tessera::Tone::NORMAL, theme::TRACK);
    }
  }
  void paint(const Model &m, uint32_t now) {
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
  const tessera::Plugin *plugin_ = nullptr;
  int width_ = 0, count_ = 0, columns_ = 1, cell_width_ = 0, cell_height_ = 0;
  lv_obj_t *footer_ = nullptr, *names_[7]{}, *values_[7]{}, *bars_[7]{};
};
class OverviewTile : public tessera::Tile {
 public:
  explicit OverviewTile(VictronVrm *plugin) : plugin_(plugin) {}
  void create(const tessera::TileContext &c) override {
    tile_ = c.tile; view_.create(c.parent, c.width, c.height, plugin_, false);
  }
  void on_tick(uint32_t now) override { view_.paint(plugin_->snapshot(), now); }
  void on_theme() override { view_.theme(); on_tick(tessera::epoch()); }
  void on_tap() override { tessera::open_card(plugin_->plugin_id(), "dashboard", "", tile_); }
 private:
  VictronVrm *plugin_;
  int tile_ = -1;
  DashboardView view_;
};
class DashboardCard : public tessera::Card {
 public:
  explicit DashboardCard(VictronVrm *plugin) : plugin_(plugin) {}
  void open(const tessera::CardContext &c) override { view_.create(c.parent, c.width, c.height, plugin_, true); }
  void on_tick(uint32_t now) override { view_.paint(plugin_->snapshot(), now); }
  void on_theme() override { view_.theme(); on_tick(tessera::epoch()); }
 private:
  VictronVrm *plugin_;
  DashboardView view_;
};
void VictronVrm::setup() {
  add_tile("overview", [this]() { return new OverviewTile(this); });
  add_card("dashboard", [this]() { return new DashboardCard(this); }, true);
}
bool VictronVrm::settings(tessera::SettingsPage &page) {
  page.toggle(text("reverse_battery"), [this] { return battery_reverse_ && battery_reverse_->state; },
    [this](bool on) { if (battery_reverse_) { if (on) battery_reverse_->turn_on(); else battery_reverse_->turn_off(); } });
  page.toggle(text("reverse_grid"), [this] { return grid_reverse_ && grid_reverse_->state; },
    [this](bool on) { if (grid_reverse_) { if (on) grid_reverse_->turn_on(); else grid_reverse_->turn_off(); } });
  page.number(text("max_age"), 60, 86400, 60, "s", [this] { return max_age_ ? static_cast<int>(max_age_->state) : 900; },
    [this](int age) { if (max_age_) { auto call = max_age_->make_call(); call.set_value(age); call.perform(); } });
  return true;
}
}  // namespace esphome::victron_vrm

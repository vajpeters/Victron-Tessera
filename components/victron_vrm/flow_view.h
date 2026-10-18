#pragma once
#include "victron_vrm.h"
#include <algorithm>
#include <cstdio>
#include <string>

namespace esphome::victron_vrm {
using tessera::Font;
namespace ui = tessera::ui;

inline std::string value(Sample sample, const char *unit, unsigned precision = 0, bool absolute = false) {
  if (!sample.valid) return "--";
  char text[48]; const double v = absolute ? std::fabs(sample.value) : sample.value;
  if (!strcmp(unit, "W") && std::fabs(v) >= 1000)
    snprintf(text, sizeof(text), std::fabs(v) >= 100000 ? "%.0f kW" : "%.1f kW", v / 1000);
  else snprintf(text, sizeof(text), "%.*f %s", static_cast<int>(precision), v, unit);
  return text;
}
inline const char *health_key(Health h) {
  switch (h) {
    case Health::OLD: return "old";
    case Health::PARTIAL: return "partial";
    case Health::UNKNOWN_TIME: return "unknown_time";
    case Health::CLOCK: return "clock";
    case Health::OK: return "measured";
    default: return "empty";
  }
}

// A Victron-style arrangement using only mapped measurements and native theme colours.
class FlowView {
 public:
  void create(lv_obj_t *parent, int width, int height, const tessera::Plugin *plugin) {
    plugin_ = plugin; width_ = width;
    const int gap = ui::px(20), body = ui::line_height(Font::BODY);
    const int icon = ui::line_height(Font::ICON_SMALL), header = std::max(ui::line_height(Font::HEADLINE), icon);
    const int room = height - body - gap;
    const int gutter = std::max(gap, width / 28);
    const int cw = (width - gutter * 2) / 3, ch = room * 34 / 100;
    const int bottom = room - ch, mid = (ch + bottom) / 2;
    const int center = width / 2, left = cw / 2, right = width - cw / 2;
    const int iw = std::min(cw / 2, room / 5), ih = std::min(room / 5, bottom - ch - gap * 2);
    // Connections are created first, behind cards and the inverter.
    line(parent, left, ch, left, mid);
    line(parent, left, mid, right, mid);
    line(parent, center, ch, center, mid - ih / 2);
    line(parent, right, ch, right, mid);
    line(parent, center, mid + ih / 2, center, bottom);
    line(parent, center, bottom - gap, right, bottom - gap);
    line(parent, right, bottom - gap, right, bottom);
    const int xs[5] = {0, cw + gutter, 2 * (cw + gutter), cw + gutter, 2 * (cw + gutter)};
    const int ys[5] = {0, 0, 0, bottom, bottom};
    const uint32_t glyphs[5] = {0xF0D3E, 0xF06A5, 0xF002A, 0xF0079, 0xF0A72};
    const theme::Role accents[5] = {theme::INK, theme::MARK_GREEN, theme::MARK_AMBER,
                                  theme::MARK_BLUE, theme::MARK_AMBER};
    for (int i = 0; i < 5; ++i) {
      auto &c = cards_[i]; c.width = cw - gap * 2; c.height = ch;
      c.frame = box(parent, xs[i], ys[i], cw, ch, theme::LINE, ui::px(6));
      c.surface = box(c.frame, 1, 1, cw - 2, ch - 2, theme::CARD, ui::px(6));
      // Let Tessera's per-tile background colour softly tint the Victron panels.
      lv_obj_set_style_bg_opa(c.surface, LV_OPA_80, 0);
      const int tx = gap - 1, ty = gap - 1;
      c.value_height = ch - body * 2 - gap * 3 - header;
      c.icon = label(c.surface, tx, ty, icon, Font::ICON_SMALL, accents[i]);
      ui::set_text(c.icon, ui::icon(glyphs[i]));
      c.title = label(c.surface, tx + icon + gap / 2, ty, c.width - icon - gap / 2, Font::HEADLINE, theme::INK);
      c.value_width = c.width - icon - gap / 2;
      c.value_x = tx + icon + gap / 2; c.value_y = ty + header + gap / 2;
      c.value = label(c.surface, tx, c.value_y, c.width, Font::TITLE, theme::INK);
      c.unit = label(c.surface, tx, c.value_y, c.width, Font::TITLE, theme::INK);
      c.rule = box(c.surface, 0, ch - body * 2 - gap - 1, cw - 2, 1, theme::LINE, 0);
      c.note = label(c.surface, tx, ch - body * 2 - gap / 2 - 1, c.width, Font::BODY, theme::MUTED);
      c.extra = label(c.surface, tx, ch - body - gap / 2 - 1, c.width, Font::BODY, theme::MUTED);
    }
    inverter_ = box(parent, center - iw / 2, mid - ih / 2, iw, ih, theme::ACCENT, ui::px(6));
    inverter_band_ = box(inverter_, 0, ih / 2, iw, ui::px(3), theme::MARK_AMBER, 0);
    inverter_icon_ = label(inverter_, 0, ui::px(4), iw,
                           Font::ICON, theme::ON_ACCENT);
    lv_obj_set_style_text_align(inverter_icon_, LV_TEXT_ALIGN_CENTER, 0);
    ui::set_text(inverter_icon_, ui::icon(0xF095B));
    inverter_text_ = label(inverter_, ui::px(2), ih - body - ui::px(4),
                           iw - ui::px(4), Font::BODY, theme::ON_ACCENT);
    lv_obj_set_style_text_align(inverter_text_, LV_TEXT_ALIGN_CENTER, 0);
    ui::set_text(inverter_text_, plugin_->text("inverter"));
    // Direction marks show measured direction, never an inferred power balance.
    const int top_start = ch, top_end = mid - ih / 2;
    const int low_start = mid + ih / 2, low_end = bottom;
    const int centers[5] = {left, center, right, center, right};
    for (int i = 0; i < 5; ++i) {
      const int start = i < 3 ? top_start : low_start;
      const int end = i < 3 ? top_end : low_end;
      arrow_fits_[i] = end - start >= icon + ui::px(2);
      arrows_[i] = label(parent, centers[i] - icon / 2, (start + end - icon) / 2,
                         icon, Font::ICON_SMALL, theme::ACCENT);
    }
    // PV feeds the lower horizontal branch; its direction mark follows that branch.
    lv_obj_set_pos(arrows_[4], (center + right - icon) / 2, bottom - gap - icon / 2);
    arrow_fits_[4] = right - center >= icon + gap && gap >= (icon + 1) / 2;
    footer_ = label(parent, 0, height - body, width, Font::BODY, theme::MUTED);
    theme();
  }
  void theme() {
    for (auto &c : cards_) {
      ui::set_fill(c.frame, tessera::Tone::NORMAL, theme::LINE);
      ui::set_fill(c.surface, tessera::Tone::NORMAL, theme::CARD);
      ui::set_fill(c.rule, tessera::Tone::NORMAL, theme::LINE);
      ui::set_color(c.title, theme::INK); ui::set_color(c.value, theme::INK); ui::set_color(c.unit, theme::INK);
      ui::set_color(c.note, theme::MUTED); ui::set_color(c.extra, theme::MUTED);
    }
    const theme::Role accents[5] = {theme::INK, theme::MARK_GREEN, theme::MARK_AMBER, theme::MARK_BLUE, theme::MARK_AMBER};
    for (int i = 0; i < 5; ++i) ui::set_color(cards_[i].icon, accents[i]);
    // Keep the solar palette, blended toward the theme's ink for readable icons.
    const auto solar_ink = lv_color_mix(ui::color(theme::ENERGY_SOLAR), ui::color(theme::INK), LV_OPA_80);
    lv_obj_set_style_text_color(cards_[2].icon, solar_ink, 0);
    lv_obj_set_style_text_color(cards_[4].icon, solar_ink, 0);
    for (int i = 0; i < line_count_; ++i) ui::set_fill(lines_[i], tessera::Tone::NORMAL, theme::LINE);
    ui::set_fill(inverter_, tessera::Tone::NORMAL, theme::ACCENT);
    ui::set_fill(inverter_band_, tessera::Tone::NORMAL, theme::MARK_AMBER);
    ui::set_color(inverter_icon_, theme::ON_ACCENT); ui::set_color(inverter_text_, theme::ON_ACCENT);
    for (auto *arrow : arrows_) ui::set_color(arrow, theme::ACCENT_ICON);
    ui::set_color(footer_, theme::MUTED);
  }
  void paint(const Model &m, uint32_t now) {
    const Sample battery = m.get(BATTERY), grid = m.get(GRID);
    const Metric metrics[5] = {GRID, LOAD, CRITICAL_LOAD, SOC, SOLAR};
    const char *keys[5] = {"grid", "load", "critical_load", "soc", "solar"};
    for (int i = 0; i < 5; ++i) {
      auto &c = cards_[i];
      const std::string title = plugin_->text(keys[i]);
      ui::set_font(c.title, ui::text_width(title, Font::HEADLINE) <= c.value_width ? Font::HEADLINE : Font::TITLE);
      ui::set_text(c.title, title);
      std::string text = value(m.get(metrics[i]), i == 3 ? "%" : "W", i == 3 ? 1 : 0, i == 0);
      const auto split = text.find_last_of(' ');
      const std::string number = split == std::string::npos ? text : text.substr(0, split);
      const std::string unit = split == std::string::npos ? "" : text.substr(split + 1);
      Font face = Font::BODY, unit_face = Font::BODY;
      bool fits = false;
      const int spacing = unit.empty() ? 0 : ui::px(6);
      for (Font f : {Font::DIGITS, Font::VALUE, Font::HEADLINE, Font::TITLE, Font::BODY}) {
        if (!ui::covers(number, f) || ui::line_height(f) > c.value_height) continue;
        for (Font u : {Font::VALUE, Font::HEADLINE, Font::TITLE, Font::BODY}) {
          if ((!unit.empty() && ui::line_height(u) > c.value_height) ||
              ui::text_width(number, f) + spacing + ui::text_width(unit, u) > c.value_width) continue;
          face = f; unit_face = u; fits = true; break;
        }
        if (fits) break;
      }
      const int number_width = std::min(c.value_width, ui::text_width(number, face));
      const int unit_width = unit.empty() ? 0 : std::max(0, c.value_width - number_width - spacing);
      const int row_height = std::max(ui::line_height(face), unit.empty() ? 0 : ui::line_height(unit_face));
      ui::set_font(c.value, face); ui::set_text(c.value, number);
      lv_obj_set_width(c.value, number_width);
      lv_obj_set_pos(c.value, c.value_x, c.value_y + row_height - ui::line_height(face));
      ui::set_font(c.unit, unit_face); ui::set_text(c.unit, unit);
      lv_obj_set_width(c.unit, std::max(1, unit_width));
      lv_obj_set_pos(c.unit, c.value_x + number_width + (unit.empty() ? 0 : spacing),
                     c.value_y + row_height - ui::line_height(unit_face));
      ui::set_text(c.note, ""); ui::set_text(c.extra, "");
    }
    ui::set_text(cards_[0].note, plugin_->text(!grid.valid ? "not_mapped" : grid.value > 0 ? "import" : grid.value < 0 ? "export" : "idle"));
    ui::set_text(cards_[2].note, m.get(CRITICAL_LOAD).valid ? "" : plugin_->text("not_mapped"));
    const char *state = !battery.valid ? "battery_power" : battery.value > 30 ? "charging" : battery.value < -30 ? "discharging" : "idle";
    ui::set_text(cards_[3].note, std::string(plugin_->text(state)) + "  " + value(battery, "W", 0, true));
    ui::set_text(cards_[3].extra, value(m.get(VOLTAGE), "V", 1) + "   " + value(m.get(CURRENT), "A", 1));
    const Health health = m.health(now);
    const bool fresh = health == Health::OK;
    const Sample flows[5] = {grid, m.get(LOAD), m.get(CRITICAL_LOAD), battery, m.get(SOLAR)};
    for (int i = 0; i < 5; ++i) {
      const bool active = arrow_fits_[i] && fresh && flows[i].valid && std::fabs(flows[i].value) > 1;
      const bool down = i == 0 ? grid.value > 0 : i == 3 ? battery.value > 0 : false;
      ui::set_text(arrows_[i], active ? ui::icon(i == 4 ? 0xF004D : down ? 0xF0045 : 0xF005D) : "");
    }
    std::string footer = plugin_->text(health_key(health));
    if (m.at && now && m.at <= now && now - m.at < 86400u) {
      const auto timed = footer + " " + tessera::clock_text(m.at);
      if (ui::text_width(timed, Font::BODY) <= width_) footer = timed;
    }
    ui::set_text(footer_, footer);
  }
 private:
  struct CardParts {
    lv_obj_t *frame{}, *surface{}, *icon{}, *title{}, *value{}, *unit{}, *rule{}, *note{}, *extra{};
    int width{}, height{}, value_height{}, value_width{}, value_x{}, value_y{};
  } cards_[5];
  const tessera::Plugin *plugin_{};
  int width_{}, line_count_{};
  bool arrow_fits_[5]{};
  lv_obj_t *lines_[7]{}, *arrows_[5]{}, *inverter_{}, *inverter_icon_{}, *inverter_band_{}, *inverter_text_{}, *footer_{};
  static lv_obj_t *box(lv_obj_t *parent, int x, int y, int w, int h, theme::Role color, int radius) {
    auto *obj = ui::block(parent, color); lv_obj_set_pos(obj, x, y); lv_obj_set_size(obj, w, h);
    lv_obj_set_style_radius(obj, radius, 0); return obj;
  }
  static lv_obj_t *label(lv_obj_t *parent, int x, int y, int w, Font font, theme::Role color) {
    auto *obj = ui::label(parent, font, color); lv_obj_set_pos(obj, x, y); lv_obj_set_width(obj, w); return obj;
  }
  void line(lv_obj_t *parent, int x1, int y1, int x2, int y2) {
    lines_[line_count_++] = box(parent, std::min(x1, x2), std::min(y1, y2),
                               std::max(ui::px(2), std::abs(x2 - x1)),
                               std::max(ui::px(2), std::abs(y2 - y1)), theme::LINE, 0);
  }
};

}  // namespace esphome::victron_vrm

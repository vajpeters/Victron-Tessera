#pragma once
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>

namespace esphome::victron_vrm {
enum Metric : unsigned { SOC, BATTERY, SOLAR, LOAD, GRID, VOLTAGE, CURRENT, CRITICAL_LOAD, COUNT };
struct Sample { double value = 0; bool valid = false; };
inline bool numeric(const char *raw, double &out) {
  if (!raw || !*raw) return false;
  char *end = nullptr;
  out = std::strtod(raw, &end);
  if (end == raw) return false;
  while (*end && std::isspace(static_cast<unsigned char>(*end))) ++end;
  return !*end && std::isfinite(out);
}
enum class Health { EMPTY, PARTIAL, OLD, UNKNOWN_TIME, CLOCK, OK };
class Model {
 public:
  static constexpr uint32_t MAX_AGE = 900;
  std::array<Sample, COUNT> samples{};
  uint32_t at = 0;
  bool disconnected = false;
  void set(Metric metric, const char *raw, const char *unit) {
    if (metric >= COUNT) return;
    samples[metric] = {};
    double number = 0, scale = 1;
    if (!numeric(raw, number) || !unit) return;
    if (metric == SOC) {
      if (std::strcmp(unit, "%") || number < 0 || number > 100) return;
    } else if ((metric >= BATTERY && metric <= GRID) || metric == CRITICAL_LOAD) {
      if (!std::strcmp(unit, "kW")) scale = 1000;
      else if (!std::strcmp(unit, "MW")) scale = 1000000;
      else if (std::strcmp(unit, "W")) return;
      if ((metric == SOLAR || metric == LOAD || metric == CRITICAL_LOAD) && number < 0) return;
    } else if (metric == VOLTAGE) {
      if (!std::strcmp(unit, "mV")) scale = .001;
      else if (std::strcmp(unit, "V")) return;
      if (number < 0) return;
    } else if (metric == CURRENT) {
      if (!std::strcmp(unit, "mA")) scale = .001;
      else if (std::strcmp(unit, "A")) return;
    }
    number *= scale;
    if (std::isfinite(number)) samples[metric] = {number, true};
  }
  void set(Metric metric, double number, const char *unit) {
    if (metric >= COUNT || !std::isfinite(number) || !unit) return;
    char raw[32];
    std::snprintf(raw, sizeof(raw), "%.12g", number);
    set(metric, raw, unit);
  }
  void set_time(const char *raw) {
    double epoch = 0;
    if (numeric(raw, epoch) && epoch > 0 && epoch <= UINT32_MAX && std::floor(epoch) == epoch)
      at = static_cast<uint32_t>(epoch);
  }
  void set_time(double epoch) {
    if (std::isfinite(epoch) && epoch > 0 && epoch <= UINT32_MAX && std::floor(epoch) == epoch)
      at = static_cast<uint32_t>(epoch);
  }
  Sample get(Metric metric) const { return samples[metric]; }
  Health health(uint32_t now) const {
    bool any = false;
    for (const auto &s : samples) any = any || s.valid;
    if (!any) return Health::EMPTY;
    if (disconnected) return Health::OLD;
    if (at && now && ((at > now && at - now > 300u) || (now > at && now - at > MAX_AGE))) return Health::OLD;
    if (!get(SOC).valid || !get(BATTERY).valid || !get(SOLAR).valid || !get(LOAD).valid) return Health::PARTIAL;
    if (!at) return Health::UNKNOWN_TIME;
    if (!now) return Health::CLOCK;
    return Health::OK;
  }
};
}  // namespace esphome::victron_vrm

#include "../components/victron_vrm/model.h"
#include <cassert>
#include <iostream>
using namespace esphome::victron_vrm;
static Model normal() {
  Model m;
  m.set(SOC, "78.5", "%"); m.set(BATTERY, "-.45", "kW");
  m.set(SOLAR, "1.2", "kW"); m.set(LOAD, "1800", "W"); m.set(GRID, "150", "W");
  m.set_time("1792303200");
  return m;
}
int main() {
  constexpr uint32_t now=1792303260;
  auto m=normal();
  assert(m.get(BATTERY).value == -450 && m.get(SOLAR).value == 1200);
  assert(m.health(now) == Health::OK);
  m.set(SOC,"0","%"); m.set(SOLAR,"0","W"); m.set(GRID,"0","W");
  assert(m.get(SOC).valid && m.get(SOC).value == 0 && m.get(SOLAR).valid && m.get(GRID).valid);
  for (const char *bad : {"unknown","unavailable","false","True","","nan","inf","1.2 kW","1,2", "--"}) {
    m=normal(); m.set(BATTERY,bad,"W"); assert(!m.get(BATTERY).valid);
    assert(m.health(now)==Health::OK); // battery SOC is still a valid battery source
  }
  m=normal(); m.set(SOLAR,"12","kWh"); assert(!m.get(SOLAR).valid);
  m=normal(); m.set(LOAD,"1200","VA"); assert(!m.get(LOAD).valid);
  m=normal(); m.set(BATTERY,"12",nullptr); assert(!m.get(BATTERY).valid);
  m=normal(); m.set(SOC,"101","%"); assert(!m.get(SOC).valid);
  m=normal(); m.set(SOC,"-1","%"); assert(!m.get(SOC).valid);
  m=normal(); m.set(SOLAR,"-1","W"); assert(!m.get(SOLAR).valid);
  m=normal(); m.set(VOLTAGE,"52400","mV"); assert(std::fabs(m.get(VOLTAGE).value-52.4)<.001);
  m.set(CURRENT,"-8600","mA"); assert(std::fabs(m.get(CURRENT).value+8.6)<.001);
  m=normal(); m.set(GRID,"-2.5","kW"); assert(m.get(GRID).value==-2500);
  m=normal(); m.set(SOLAR,"1.2","MW"); assert(m.get(SOLAR).value==1200000);
  // Critical loads are independently mapped and optional, never inferred from AC consumption.
  m=normal(); assert(!m.get(CRITICAL_LOAD).valid && m.health(now)==Health::OK);
  m=normal(); m.samples[BATTERY] = {}; assert(m.get(SOC).valid && m.health(now)==Health::OK);
  m=normal(); m.samples[SOC] = {}; assert(m.get(BATTERY).valid && m.health(now)==Health::OK);
  m.set(CRITICAL_LOAD,".65","kW");
  assert(m.get(CRITICAL_LOAD).valid && m.get(CRITICAL_LOAD).value==650);
  assert(m.get(LOAD).value==1800);
  m.set(CRITICAL_LOAD,"0","W");
  assert(m.get(CRITICAL_LOAD).valid && m.get(CRITICAL_LOAD).value==0);
  for (const char *bad : {"-1", "unavailable"}) {
    m.set(CRITICAL_LOAD,bad,"W");
    assert(!m.get(CRITICAL_LOAD).valid && m.health(now)==Health::OK);
  }
  m.set(CRITICAL_LOAD,"3","kWh"); assert(!m.get(CRITICAL_LOAD).valid);
  m.set(CRITICAL_LOAD,".001","MW");
  assert(m.get(CRITICAL_LOAD).valid && m.get(CRITICAL_LOAD).value==1000);
  // A finite source value may overflow while converting to watts; clear the previous reading.
  m=normal(); m.set(SOLAR,"1e308","MW");
  assert(!m.get(SOLAR).valid && m.health(now)==Health::PARTIAL);
  m=normal(); assert(m.get(BATTERY).valid);
  m.set(BATTERY,"unavailable","W");
  assert(!m.get(BATTERY).valid && m.health(now)==Health::OK);
  m.samples[SOC] = {}; assert(m.health(now)==Health::PARTIAL);
  m=normal(); m.at=0; assert(m.health(now)==Health::UNKNOWN_TIME);
  assert(m.health(0)==Health::UNKNOWN_TIME);
  // Repeated identical states do not establish the age of an underlying measurement.
  m.set(SOLAR,"1.2","kW");
  assert(m.get(SOLAR).valid && m.get(SOLAR).value==1200);
  assert(m.health(now+86400)==Health::UNKNOWN_TIME);
  m=normal(); assert(m.health(0)==Health::CLOCK);
  m=normal(); assert(m.health(now+901)==Health::OLD);
  m=normal(); m.at=now+300; assert(m.health(now)==Health::OK);
  m.at=now+301; assert(m.health(now)==Health::OLD);
  m=normal(); m.disconnected=true; assert(m.health(now)==Health::OLD);
  m=normal(); m.set(GRID,"unavailable","W"); assert(m.health(now)==Health::OK); // off-grid is optional
  m=normal(); m.set(VOLTAGE,"unavailable","V"); assert(m.health(now)==Health::OK);
  m=normal(); assert(m.health(m.at+Model::MAX_AGE)==Health::OK);
  assert(m.health(m.at+Model::MAX_AGE+1)==Health::OLD);
  m=normal(); m.at=0; m.set_time("1792303200000"); assert(m.at==0);
  m.set_time("2026-10-18T12:00:00Z"); assert(m.at==0);
  m.set_time("nan"); assert(m.at==0);
  m.set_time("-1"); assert(m.at==0);
  m.set_time("1792303200.5"); assert(m.at==0);
  m=Model{}; assert(m.health(now)==Health::EMPTY);
  std::cout << "Model tests passed: units, missing values, optional sensors and source age.\n";
}

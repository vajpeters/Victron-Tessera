"""Home Assistant entity mappings supplied by Tessera's setup UI, one installation per screen."""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import smart_display, text_sensor, switch, number
from esphome.const import CONF_ID

DEPENDENCIES = ["smart_display", "api"]
ns = cg.esphome_ns.namespace("victron_vrm")
VictronVrm = ns.class_("VictronVrm", cg.Component)
METRICS = ("soc", "battery", "solar", "load", "grid", "voltage", "current", "critical_load")
schema = {cv.GenerateID(): cv.declare_id(VictronVrm)}
for key in (*METRICS, *(m + "_unit" for m in METRICS), "source_time"):
    schema[cv.Required(key)] = cv.use_id(text_sensor.TextSensor)
schema[cv.Required("battery_reverse")] = cv.use_id(switch.Switch)
schema[cv.Required("grid_reverse")] = cv.use_id(switch.Switch)
schema[cv.Required("max_age")] = cv.use_id(number.Number)
CONFIG_SCHEMA = cv.Schema(schema).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await smart_display.register_plugin(var, __file__)
    for index, name in enumerate(METRICS):
        cg.add(var.set_reading(index, await cg.get_variable(config[name]), await cg.get_variable(config[name + "_unit"])))
    cg.add(var.set_source_time(await cg.get_variable(config["source_time"])))
    cg.add(var.set_battery_reverse(await cg.get_variable(config["battery_reverse"])))
    cg.add(var.set_grid_reverse(await cg.get_variable(config["grid_reverse"])))
    cg.add(var.set_max_age(await cg.get_variable(config["max_age"])))

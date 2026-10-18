"""Victron dashboard data supplied by the Home Assistant entity selected on each tile."""
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import smart_display
from esphome.const import CONF_ID

DEPENDENCIES = ["smart_display", "api"]
ns = cg.esphome_ns.namespace("victron_vrm")
VictronVrm = ns.class_("VictronVrm", cg.Component)
schema = {cv.GenerateID(): cv.declare_id(VictronVrm)}
CONFIG_SCHEMA = cv.Schema(schema).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await smart_display.register_plugin(var, __file__)

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import i2c
from esphome.const import CONF_ID

CODEOWNERS = ["@ThatOneNeji"]
DEPENDENCIES = ["i2c"]
MULTI_CONF = True

qwiic_keypad_ns = cg.esphome_ns.namespace("qwiic_keypad")
QwiicKeypadComponent = qwiic_keypad_ns.class_(
    "QwiicKeypadComponent", cg.Component, i2c.I2CDevice
)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(QwiicKeypadComponent),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x4B))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import i2c, key_provider
from esphome.const import CONF_ID, CONF_ON_KEY, CONF_TRIGGER_ID

CODEOWNERS = ["@ThatOneNeji"]
DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["key_provider"]
MULTI_CONF = True

qwiic_keypad_ns = cg.esphome_ns.namespace("qwiic_keypad")
QwiicKeypadComponent = qwiic_keypad_ns.class_(
    "QwiicKeypadComponent", key_provider.KeyProvider, cg.Component, i2c.I2CDevice
)
QwiicKeypadKeyTrigger = qwiic_keypad_ns.class_(
    "QwiicKeypadKeyTrigger", automation.Trigger.template(cg.uint8)
)

CONF_QWIIC_KEYPAD_ID = "qwiic_keypad_id"
# Defined here rather than imported: esphome.components.const only gained CONF_KEYS after our minimum version
CONF_KEYS = "keys"
CONF_RELEASE_DELAY = "release_delay"

# The key labels, in physical position order (row by row); must match QWIIC_KEYPAD_LAYOUT in registers.h
KEYPAD_LAYOUT = "123456789*0#"


def check_keys(value):
    value = cv.string(value)
    for ch in value:
        if not ch.isascii():
            # Each key is reported as one byte, so only ASCII characters can be key codes
            raise cv.Invalid(f"Key code {ch!r} is not an ASCII character")
    if len(value) != len(KEYPAD_LAYOUT):
        raise cv.Invalid(
            f"The number of key codes must equal the number of buttons ({len(KEYPAD_LAYOUT)})"
        )
    return value


CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(QwiicKeypadComponent),
            cv.Optional(CONF_KEYS, default=KEYPAD_LAYOUT): check_keys,
            # The device never reports a release, so a key is released this long after its press. Keep it above the
            # API's batch_delay (100ms): the API only sends an entity's latest state per batch, so a shorter press
            # would reach Home Assistant as OFF only.
            cv.Optional(
                CONF_RELEASE_DELAY, default="200ms"
            ): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_ON_KEY): automation.validate_automation(
                {
                    cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(
                        QwiicKeypadKeyTrigger
                    ),
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
    .extend(i2c.i2c_device_schema(0x4B))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)

    cg.add(var.set_keys(config[CONF_KEYS]))
    cg.add(var.set_release_delay(config[CONF_RELEASE_DELAY].total_milliseconds))
    for conf in config.get(CONF_ON_KEY, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID])
        cg.add(var.register_key_trigger(trigger))
        await automation.build_automation(trigger, [(cg.uint8, "x")], conf)

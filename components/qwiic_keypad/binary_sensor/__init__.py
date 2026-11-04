import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.const import CONF_COL, CONF_ID, CONF_KEY, CONF_ROW

from .. import CONF_QWIIC_KEYPAD_ID, QwiicKeypadComponent, qwiic_keypad_ns

DEPENDENCIES = ["qwiic_keypad"]

QwiicKeypadBinarySensor = qwiic_keypad_ns.class_(
    "QwiicKeypadBinarySensor", binary_sensor.BinarySensor
)


def check_button(obj):
    if CONF_ROW in obj or CONF_COL in obj:
        if CONF_KEY in obj:
            raise cv.Invalid("You can't provide both a key and a position")
        if CONF_ROW not in obj:
            raise cv.Invalid("Missing row")
        if CONF_COL not in obj:
            raise cv.Invalid("Missing col")
    elif CONF_KEY not in obj:
        raise cv.Invalid("Missing key or position")
    elif len(obj[CONF_KEY]) != 1:
        raise cv.Invalid("Key must be one character")
    return obj


CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema(QwiicKeypadBinarySensor).extend(
        {
            cv.GenerateID(CONF_QWIIC_KEYPAD_ID): cv.use_id(QwiicKeypadComponent),
            # The keypad has a fixed 4 x 3 layout, so positions can be range-checked (matrix_keypad can't)
            cv.Optional(CONF_ROW): cv.int_range(min=0, max=3),
            cv.Optional(CONF_COL): cv.int_range(min=0, max=2),
            cv.Optional(CONF_KEY): cv.string,
        }
    ),
    check_button,
)


async def to_code(config):
    if CONF_KEY in config:
        var = cg.new_Pvariable(config[CONF_ID], config[CONF_KEY][0])
    else:
        var = cg.new_Pvariable(config[CONF_ID], config[CONF_ROW], config[CONF_COL])
    await binary_sensor.register_binary_sensor(var, config)
    keypad = await cg.get_variable(config[CONF_QWIIC_KEYPAD_ID])
    cg.add(keypad.register_listener(var))

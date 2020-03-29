set(COMPONENT_SRCS Output.cpp Bsp.cpp Logic.cpp Input.cpp DHT_sensor_library/DHT.cpp blink.cpp Network.cpp IRremote/esp32_rmt_common.c IRremote/RMTLib.cpp IRremote/rmtlib_nec.c)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

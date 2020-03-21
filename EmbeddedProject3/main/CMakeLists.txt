set(COMPONENT_SRCS Output.cpp Bsp.cpp Logic.cpp Input.cpp DHT_sensor_library/DHT.cpp blink.cpp Network.cpp IRremote/ir_NEC.cpp IRremote/ir_Panasonic.cpp IRremote/ir_RC5_RC6.cpp IRremote/ir_Samsung.cpp IRremote/ir_Sanyo.cpp IRremote/ir_Sharp.cpp IRremote/ir_Sony.cpp IRremote/ir_Template.cpp IRremote/ir_Whynter.cpp IRremote/irPronto.cpp IRremote/irRecv.cpp IRremote/IRremote.cpp)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

set(COMPONENT_SRCS Output.cpp Bsp.cpp Logic.cpp Input.cpp DHT_sensor_library/DHT.cpp blink.cpp IRremote/IRremote.cpp IRremote/ir_Aiwa.cpp IRremote/ir_Denon.cpp IRremote/ir_Dish.cpp IRremote/ir_JVC.cpp IRremote/ir_Lego_PF.cpp IRremote/ir_LG.cpp IRremote/ir_Mitsubishi.cpp IRremote/ir_NEC.cpp IRremote/ir_Panasonic.cpp IRremote/ir_RC5_RC6.cpp IRremote/ir_Samsung.cpp IRremote/ir_Sanyo.cpp IRremote/ir_Sharp.cpp IRremote/ir_Sony.cpp IRremote/ir_Template.cpp IRremote/ir_Whynter.cpp IRremote/irPronto.cpp IRremote/irRecv.cpp IRremote/irSend.cpp Network.cpp)
set(COMPONENT_ADD_INCLUDEDIRS ".")

register_component()

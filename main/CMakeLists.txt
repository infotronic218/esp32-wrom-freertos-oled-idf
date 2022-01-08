 # Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.c" "ssd1306.c" "ssd1306_i2c.c" "ssd1306_spi.c" )
set(COMPONENT_ADD_INCLUDEDIRS "")
#idf_component_register(SRCS "main.c" "ssd1306.c" "ssd1306_i2c.c" "ssd1306_spi.c"  INCLUDE_DIRS "./")
#set(COMPONENT_SRCS main.c ssd1306.c ssd1306_i2c.c ssd1306_spi.c)
#set(COMPONENT_ADD_INCLUDEDIRS ".")

#register_component()

register_component()

idf_component_register(SRCS "util.cpp" "main.cpp"
                    INCLUDE_DIRS "."
                    REQUIRES arduino blufi FastLED RPCServer
                    EMBED_FILES "index.htm")
add_definitions(-DESP32)
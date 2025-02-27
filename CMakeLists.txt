# Check cmake version meets our requirements
cmake_minimum_required(VERSION 3.3)

# Declare project, which will configure compiler for us
project(ExtractModule)

#Finds ROOT and Falaise packages and defines cmake variables
find_package(Falaise REQUIRED)
find_package(ROOT    REQUIRED)

include_directories(include ${Bayeux_INCLUDE_DIRS})

file(GLOB SOURCES "src/*.cc" "src/dicts/*.cc")
add_library(ExtractModule SHARED ${SOURCES} )

target_link_libraries(ExtractModule PUBLIC Falaise::Falaise ${ROOT_LIBRARIES} ROOT::Geom)

install(TARGETS ExtractModule DESTINATION ${CMAKE_INSTALL_PLUGINDIR})

configure_file("extract.conf.in" "extract.conf" @ONLY)
configure_file("variant.profile.in" "variant.profile" @ONLY)

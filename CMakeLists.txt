# Boilerplate combined from IDF and MDF examples
cmake_minimum_required(VERSION 3.5)

# Do I need both here?
# include($ENV{IDF_PATH}/tools/cmake/project.cmake)
include($ENV{MDF_PATH}/project.cmake)

# Not sure if renaming this is necessary
project(mesh_mac_spam)
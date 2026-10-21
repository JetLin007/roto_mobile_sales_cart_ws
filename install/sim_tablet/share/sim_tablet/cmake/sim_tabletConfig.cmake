# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_sim_tablet_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED sim_tablet_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(sim_tablet_FOUND FALSE)
  elseif(NOT sim_tablet_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(sim_tablet_FOUND FALSE)
  endif()
  return()
endif()
set(_sim_tablet_CONFIG_INCLUDED TRUE)

# output package information
if(NOT sim_tablet_FIND_QUIETLY)
  message(STATUS "Found sim_tablet: 0.1.0 (${sim_tablet_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'sim_tablet' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${sim_tablet_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(sim_tablet_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${sim_tablet_DIR}/${_extra}")
endforeach()

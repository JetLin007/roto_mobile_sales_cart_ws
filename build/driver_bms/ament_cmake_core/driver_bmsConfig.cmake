# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_driver_bms_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED driver_bms_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(driver_bms_FOUND FALSE)
  elseif(NOT driver_bms_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(driver_bms_FOUND FALSE)
  endif()
  return()
endif()
set(_driver_bms_CONFIG_INCLUDED TRUE)

# output package information
if(NOT driver_bms_FIND_QUIETLY)
  message(STATUS "Found driver_bms: 0.1.0 (${driver_bms_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'driver_bms' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${driver_bms_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(driver_bms_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${driver_bms_DIR}/${_extra}")
endforeach()

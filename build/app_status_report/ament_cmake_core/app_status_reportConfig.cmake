# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_app_status_report_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED app_status_report_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(app_status_report_FOUND FALSE)
  elseif(NOT app_status_report_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(app_status_report_FOUND FALSE)
  endif()
  return()
endif()
set(_app_status_report_CONFIG_INCLUDED TRUE)

# output package information
if(NOT app_status_report_FIND_QUIETLY)
  message(STATUS "Found app_status_report: 0.1.0 (${app_status_report_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'app_status_report' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${app_status_report_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(app_status_report_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${app_status_report_DIR}/${_extra}")
endforeach()

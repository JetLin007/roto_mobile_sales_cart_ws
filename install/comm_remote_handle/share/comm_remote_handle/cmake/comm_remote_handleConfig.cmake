# generated from ament/cmake/core/templates/nameConfig.cmake.in

# prevent multiple inclusion
if(_comm_remote_handle_CONFIG_INCLUDED)
  # ensure to keep the found flag the same
  if(NOT DEFINED comm_remote_handle_FOUND)
    # explicitly set it to FALSE, otherwise CMake will set it to TRUE
    set(comm_remote_handle_FOUND FALSE)
  elseif(NOT comm_remote_handle_FOUND)
    # use separate condition to avoid uninitialized variable warning
    set(comm_remote_handle_FOUND FALSE)
  endif()
  return()
endif()
set(_comm_remote_handle_CONFIG_INCLUDED TRUE)

# output package information
if(NOT comm_remote_handle_FIND_QUIETLY)
  message(STATUS "Found comm_remote_handle: 0.1.0 (${comm_remote_handle_DIR})")
endif()

# warn when using a deprecated package
if(NOT "" STREQUAL "")
  set(_msg "Package 'comm_remote_handle' is deprecated")
  # append custom deprecation text if available
  if(NOT "" STREQUAL "TRUE")
    set(_msg "${_msg} ()")
  endif()
  # optionally quiet the deprecation message
  if(NOT ${comm_remote_handle_DEPRECATED_QUIET})
    message(DEPRECATION "${_msg}")
  endif()
endif()

# flag package as ament-based to distinguish it after being find_package()-ed
set(comm_remote_handle_FOUND_AMENT_PACKAGE TRUE)

# include all config extra files
set(_extras "")
foreach(_extra ${_extras})
  include("${comm_remote_handle_DIR}/${_extra}")
endforeach()

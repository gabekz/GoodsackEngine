# colorize CMake output

# code adapted from stackoverflow: http://stackoverflow.com/a/19578320
# from post authored by https://stackoverflow.com/users/2556117/fraser

#------------------------------------------------------------------------------
# message types from https://cmake.org/cmake/help/latest/command/message.html
#
# FATAL_ERROR
# CMake Error, stop processing and generation.
#
# The cmake(1) executable will return a non-zero exit code.
#
# SEND_ERROR
# CMake Error, continue processing, but skip generation.
#
# WARNING
# CMake Warning, continue processing.
#
# AUTHOR_WARNING
# CMake Warning (dev), continue processing.
#
# DEPRECATION
# CMake Deprecation Error or Warning if variable CMAKE_ERROR_DEPRECATED or
# CMAKE_WARN_DEPRECATED is enabled, respectively, else no message.
#
# (none) or NOTICE
# Important message printed to stderr to attract user's attention.
#
# STATUS
# The main interesting messages that project users might be interested in.
# Ideally these should be concise, no more than a
# single line, but still informative.
#
# VERBOSE
# Detailed informational messages intended for project users. These messages
# should provide additional details that won't be of interest in most cases, but
# which may be useful to those building the project when they want deeper insight
# into what's happening.
#
# DEBUG
# Detailed informational messages intended for developers working on the
# project itself as opposed to users who just want to build it.
# These messages will not typically be of interest to other users building
# the project and will often be closely related to internal implementation details.
#
# TRACE
# Fine-grained messages with very low-level implementation details.
# Messages using this log level would normally only be temporary and
# would expect to be removed before releasing the project, packaging up the files, etc.
#------------------------------------------------------------------------------

macro(define_colors)
  string(ASCII 27 Esc)
  set(ColourReset "${Esc}[m")
  set(ColourBold "${Esc}[1m")
  set(Red "${Esc}[31m")
  set(Green "${Esc}[32m")
  set(Yellow "${Esc}[33m")
  set(Blue "${Esc}[34m")
  set(Magenta "${Esc}[35m")
  set(Cyan "${Esc}[36m")
  set(White "${Esc}[37m")
  set(BoldRed "${Esc}[1;31m")
  set(BoldGreen "${Esc}[1;32m")
  set(BoldYellow "${Esc}[1;33m")
  set(BoldBlue "${Esc}[1;34m")
  set(BoldMagenta "${Esc}[1;35m")
  set(BoldCyan "${Esc}[1;36m")
  set(BoldWhite "${Esc}[1;37m")
endmacro()

#function(message)
#  list(GET ARGV 0 MessageType)
#  if(MessageType STREQUAL FATAL_ERROR OR MessageType STREQUAL SEND_ERROR)
#    list(REMOVE_AT ARGV 0)
#    _message(${MessageType} "${BoldRed}${ARGV}${ColourReset}")
#  elseif(MessageType STREQUAL WARNING)
#    list(REMOVE_AT ARGV 0)
#    _message(${MessageType} "${BoldYellow}${ARGV}${ColourReset}")
#  elseif(MessageType STREQUAL AUTHOR_WARNING)
#    list(REMOVE_AT ARGV 0)
#    _message(${MessageType} "${BoldCyan}${ARGV}${ColourReset}")
#  elseif(MessageType STREQUAL STATUS)
#    list(REMOVE_AT ARGV 0)
#    _message(${MessageType} "${Green}${ARGV}${ColourReset}")
#  else()
#    _message("${ARGV}")
#  endif()
#endfunction()
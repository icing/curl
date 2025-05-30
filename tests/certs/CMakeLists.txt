#***************************************************************************
#                                  _   _ ____  _
#  Project                     ___| | | |  _ \| |
#                             / __| | | | |_) | |
#                            | (__| |_| |  _ <| |___
#                             \___|\___/|_| \_\_____|
#
# Copyright (C) Daniel Stenberg, <daniel@haxx.se>, et al.
#
# This software is licensed as described in the file COPYING, which
# you should have received as part of this distribution. The terms
# are also available at https://curl.se/docs/copyright.html.
#
# You may opt to use, copy, modify, merge, publish, distribute and/or sell
# copies of the Software, and permit persons to whom the Software is
# furnished to do so, under the terms of the COPYING file.
#
# This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
# KIND, either express or implied.
#
# SPDX-License-Identifier: curl
#
###########################################################################
find_program(SH_EXECUTABLE "sh")
mark_as_advanced(SH_EXECUTABLE)
if(SH_EXECUTABLE)
  # Get 'CERTCONFIGS', 'GENERATEDCERTS', 'SRPFILES' variables
  curl_transform_makefile_inc("Makefile.inc" "${CMAKE_CURRENT_BINARY_DIR}/Makefile.inc.cmake")
  include("${CMAKE_CURRENT_BINARY_DIR}/Makefile.inc.cmake")

  add_custom_target(clean-certs
    COMMAND ${CMAKE_COMMAND} -E remove ${GENERATEDCERTS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

  add_custom_target(build-certs
    DEPENDS ${CERTCONFIGS} ${SRPFILES}
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/genroot.sh" test
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/genserv.sh" test-localhost test
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/genserv.sh" test-localhost.nn test
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/genserv.sh" test-localhost0h test
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/genserv.sh" test-localhost-san-first test
    COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/scripts/genserv.sh" test-localhost-san-last test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endif()

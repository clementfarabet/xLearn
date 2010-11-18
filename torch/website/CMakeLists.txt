SET(hlpfiles index.hlp download.hlp credits.hlp sourceforge.hlp)
SET(generatedhtmlfiles)

FOREACH(hlpfile ${hlpfiles})
  SET(hlpfile "${CMAKE_CURRENT_SOURCE_DIR}/${hlpfile}")
  GET_FILENAME_COMPONENT(_file_ ${hlpfile} NAME_WE)
  
  ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${_file_}.html"
    COMMAND ${LUA_EXECUTABLE}
    ARGS "${Torch_HLP2HTML_LUA}" "${hlpfile}" "${CMAKE_CURRENT_SOURCE_DIR}/webtemplate.html" \"\" "${CMAKE_CURRENT_BINARY_DIR}"
    DEPENDS "${LUA_EXECUTABLE}" "${hlpfile}" "${CMAKE_CURRENT_SOURCE_DIR}/webtemplate.html")
  SET(generatedhtmlfiles ${generatedhtmlfiles} "${CMAKE_CURRENT_BINARY_DIR}/${_file_}.html")
ENDFOREACH(hlpfile)

# copy CSS file
ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/webtorch.css"
  COMMAND ${CMAKE_COMMAND} ARGS "-E" "copy" "${CMAKE_CURRENT_SOURCE_DIR}/webtorch.css" "${CMAKE_CURRENT_BINARY_DIR}/webtorch.css"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/webtorch.css")
SET(generatedhtmlfiles ${generatedhtmlfiles} "${CMAKE_CURRENT_BINARY_DIR}/webtorch.css")

# copy Torch logo file
ADD_CUSTOM_COMMAND(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/torchlogo.png"
  COMMAND ${CMAKE_COMMAND} ARGS "-E" "copy" "${CMAKE_CURRENT_SOURCE_DIR}/torchlogo.png" "${CMAKE_CURRENT_BINARY_DIR}/torchlogo.png"
  DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/torchlogo.png")
SET(generatedhtmlfiles ${generatedhtmlfiles} "${CMAKE_CURRENT_BINARY_DIR}/torchlogo.png")

ADD_CUSTOM_TARGET(website ALL DEPENDS ${generatedhtmlfiles})

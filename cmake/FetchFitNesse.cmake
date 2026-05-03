# Downloads the latest fitnesse-standalone.jar from GitHub releases.
# Invoked as a CMake script: cmake -DLIB_DIR=<path> -P FetchFitNesse.cmake

if(NOT DEFINED LIB_DIR)
    message(FATAL_ERROR "LIB_DIR must be defined")
endif()

set(API_URL "https://api.github.com/repos/unclebob/fitnesse/releases/latest")
set(API_JSON "${LIB_DIR}/fitnesse_latest.json")
set(JAR_FILE "${LIB_DIR}/fitnesse-standalone.jar")

message(STATUS "Querying latest FitNesse release from GitHub...")
file(DOWNLOAD "${API_URL}" "${API_JSON}"
    HTTPHEADER "Accept: application/vnd.github+json"
    STATUS _status)
list(GET _status 0 _code)
if(NOT _code EQUAL 0)
    message(FATAL_ERROR "GitHub API query failed: ${_status}")
endif()

file(READ "${API_JSON}" _json)
file(REMOVE "${API_JSON}")

string(REGEX MATCH
    "\"browser_download_url\": \"([^\"]*-standalone\\.jar)\""
    _match "${_json}")
set(_url "${CMAKE_MATCH_1}")
if(NOT _url)
    message(FATAL_ERROR "Could not find standalone jar URL in release JSON")
endif()

string(REGEX MATCH "v[0-9]+" _version "${_url}")
message(STATUS "Downloading FitNesse ${_version}...")
file(DOWNLOAD "${_url}" "${JAR_FILE}"
    SHOW_PROGRESS
    STATUS _status)
list(GET _status 0 _code)
if(NOT _code EQUAL 0)
    message(FATAL_ERROR "Download failed: ${_status}")
endif()

message(STATUS "Saved to ${JAR_FILE}")

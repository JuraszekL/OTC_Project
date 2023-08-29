# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.
file(GLOB_RECURSE UI_SOURCES ui/*.c)
file(GLOB_RECURSE UI_WIFI_COMPONENTS ui/ui_wifi_list.c ui/ui_wifi_popup.c ui/ui_styles.c)

idf_component_register(
    SRCS "main.c" "display.c" "touchpad.c" "animations.c" "ui_task.c" "wifi.c"
    "clock.c" "const_text.c" "online_requests.c" "sdcard.c" "spiffs_task.c" ${UI_SOURCES}
    ${UI_WIFI_COMPONENTS} # list the source files of this component
    INCLUDE_DIRS "include" "ui" # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)

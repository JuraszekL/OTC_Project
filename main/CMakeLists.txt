# See the build system documentation in IDF programming guide
# for more information about component CMakeLists.txt files.
file(GLOB_RECURSE UI_SOURCES ui/*.c ui/screens/*.c ui/components/*.c ui/fonts/*.c ui/images/*.c)
file(GLOB_RECURSE MAIN_SOURCES *.c)

idf_component_register(
	SRCS ${UI_SOURCES} ${MAIN_SOURCES}
    INCLUDE_DIRS "include" "ui" # optional, add here public include directories
    PRIV_INCLUDE_DIRS   # optional, add here private include directories
    REQUIRES            # optional, list the public requirements (component names)
    PRIV_REQUIRES       # optional, list the private requirements
)
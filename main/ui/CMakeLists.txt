SET(SOURCES screens/ui_InitScreen.c
    ui.c
    ui_helpers.c
    images/ui_img_all3_png.c
    images/ui_img_all2_png.c
    fonts/ui_font_Varino12.c
    fonts/ui_font_Varino18.c
    fonts/ui_font_Varino30.c)

add_library(ui ${SOURCES})

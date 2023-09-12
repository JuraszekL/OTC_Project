#include "ui.h"

/**************************************************************
 *
 * Public function definitions
 *
 ***************************************************************/

/* some generic functions used by multiple screens */
void UI_ScreenCreate(lv_obj_t **screen){

    *screen = lv_obj_create(NULL);
    lv_obj_add_style(*screen, &UI_ScreenStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_clear_flag(*screen, LV_OBJ_FLAG_SCROLLABLE);
}

void UI_BackButtonCreate(lv_obj_t **screen, lv_obj_t **button){

	lv_obj_t *back_but_label;

	*button = lv_btn_create(*screen);
    lv_obj_add_style(*button, &UI_ButtonStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_x(*button, 20);
    lv_obj_set_y(*button, -20);
    lv_obj_set_align(*button, LV_ALIGN_BOTTOM_LEFT);

    back_but_label = lv_label_create(*button);
    lv_obj_add_style(back_but_label, &UI_ButtonLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(back_but_label, "%c", ICON_LEFT_ARROW);
}

void UI_HorizontalLineCreate(lv_obj_t **screen, lv_obj_t **line){

	*line = lv_obj_create(*screen);
	lv_obj_add_style(*line, &UI_HorizontalLineStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
}

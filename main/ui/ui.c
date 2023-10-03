#include "ui.h"
/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define UI_BUTTON_WIDTH				80
#define UI_BUTTON_HEIGHT			60

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

void UI_ButtonCreate(lv_obj_t **parent, lv_obj_t **button, char icon){

	lv_obj_t *back_but_label;

	*button = lv_btn_create(*parent);
    lv_obj_add_style(*button, &UI_ButtonStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(*button, UI_BUTTON_WIDTH);
    lv_obj_set_height(*button, UI_BUTTON_HEIGHT);

    back_but_label = lv_label_create(*button);
    lv_obj_add_style(back_but_label, &UI_ButtonLabelStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(back_but_label, "%c", icon);
}

void UI_BackButtonCreate(lv_obj_t **parent, lv_obj_t **button){

	UI_ButtonCreate(parent, button, ICON_LEFT_ARROW);

    lv_obj_set_x(*button, 20);
    lv_obj_set_y(*button, -20);
    lv_obj_set_align(*button, LV_ALIGN_BOTTOM_LEFT);
}

void UI_HorizontalLineCreate(lv_obj_t **screen, lv_obj_t **line){

	*line = lv_obj_create(*screen);
	lv_obj_add_style(*line, &UI_HorizontalLineStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
}

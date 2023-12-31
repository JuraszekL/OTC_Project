#include "ui.h"

/**************************************************************
 *
 *	Definitions
 *
 ***************************************************************/
#define UI_BUTTON_WIDTH				80
#define UI_BUTTON_HEIGHT			60

#define POPUP_DEFAULT_WIDTH			200
#define POPUP_DEFAULT_HEIGHT		150

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

	lv_obj_t *button_icon;

	*button = lv_btn_create(*parent);
    lv_obj_add_style(*button, &UI_ButtonStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_width(*button, UI_BUTTON_WIDTH);
    lv_obj_set_height(*button, UI_BUTTON_HEIGHT);

    button_icon = lv_label_create(*button);
    lv_obj_add_style(button_icon, &UI_Icon24Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_text_fmt(button_icon, "%c", icon);
    lv_obj_set_align(button_icon, LV_ALIGN_CENTER);
}

void UI_BackButtonCreate(lv_obj_t **parent, lv_obj_t **button){

	UI_ButtonCreate(parent, button, ICON_LEFT_ARROW);

    lv_obj_set_x(*button, 20);
    lv_obj_set_y(*button, -20);
    lv_obj_set_align(*button, LV_ALIGN_BOTTOM_LEFT);
}

void UI_CheckboxCreate(lv_obj_t **parent, lv_obj_t **checkbox, char *label, lv_align_t label_align){

	lv_obj_t *checkbox_label;

	*checkbox = lv_checkbox_create(*parent);
	lv_obj_add_style(*checkbox, &UI_CheckboxStyle, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_style(*checkbox, &UI_CheckboxStyle, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_add_style(*checkbox, &UI_CheckboxStyle, LV_PART_MAIN | LV_STATE_CHECKED);
	lv_obj_add_style(*checkbox, &UI_CheckboxStyle, LV_PART_INDICATOR | LV_STATE_CHECKED);
	lv_checkbox_set_text(*checkbox, "");

	if(label){

		checkbox_label = lv_label_create(*checkbox);
		lv_obj_add_style(checkbox_label, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
		lv_label_set_text(checkbox_label, label);
		if(LV_ALIGN_OUT_RIGHT_MID == label_align){

			lv_obj_set_style_text_align(checkbox_label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN | LV_STATE_DEFAULT);
			lv_obj_align_to(checkbox_label, *checkbox, label_align, 2, 0);
		}
		else if(LV_ALIGN_OUT_BOTTOM_MID == label_align){

			lv_obj_set_style_text_align(checkbox_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
			lv_obj_align_to(checkbox_label, *checkbox, label_align, -3, 2);
		}
	}
}

void UI_PopupCreate(UI_PopupObj_t *popup){

	if(0 == popup) return;

	popup->background = lv_obj_create(lv_layer_top());
	lv_obj_set_size(popup->background, LV_PCT(100), LV_PCT(100));
	lv_obj_set_style_bg_color(popup->background, UI_CurrentTheme.background_color_base, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(popup->background, 150, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(popup->background, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_clear_flag(popup->background, LV_OBJ_FLAG_SCROLLABLE);

	popup->panel = lv_obj_create(popup->background);
	lv_obj_set_size(popup->panel, POPUP_DEFAULT_WIDTH, POPUP_DEFAULT_HEIGHT);
	lv_obj_set_align(popup->panel, LV_ALIGN_CENTER);
	lv_obj_set_style_bg_color(popup->panel, UI_CurrentTheme.background_color_ext, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(popup->panel, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_color(popup->panel, UI_CurrentTheme.main_color_base, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(popup->panel, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_clear_flag(popup->panel, LV_OBJ_FLAG_SCROLLABLE);

	popup->label = lv_label_create(popup->panel);
	lv_obj_add_style(popup->label, &UI_Text16Style, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_align(popup->label, LV_ALIGN_TOP_MID);
    lv_obj_set_style_text_align(popup->label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_label_set_long_mode(popup->label, LV_LABEL_LONG_SCROLL);
}

void UI_KeyboardCreate(lv_obj_t **parent, lv_obj_t **keyboard){

	*keyboard = lv_keyboard_create(*parent);
	lv_obj_set_align(*keyboard, LV_ALIGN_BOTTOM_MID);
	lv_obj_set_y(*keyboard, 240);
	lv_obj_set_size(*keyboard, LV_PCT(100), LV_PCT(50));

	// keyboard background
	lv_obj_set_style_bg_color(*keyboard, UI_CurrentTheme.background_color_base, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(*keyboard, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

	// background for normal keys
	lv_obj_set_style_bg_color(*keyboard, UI_CurrentTheme.background_color_ext, LV_PART_ITEMS | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(*keyboard, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_DEFAULT);

	// background for checked keys
	lv_obj_set_style_bg_color(*keyboard, UI_CurrentTheme.background_color_base, LV_PART_ITEMS | LV_STATE_CHECKED);
	lv_obj_set_style_bg_opa(*keyboard, LV_OPA_COVER, LV_PART_ITEMS | LV_STATE_CHECKED);

	// border for all keys
	lv_obj_set_style_border_width(*keyboard, 1, LV_PART_ITEMS);
	lv_obj_set_style_border_color(*keyboard, UI_CurrentTheme.main_color_base, LV_PART_ITEMS);
	lv_obj_set_style_border_opa(*keyboard, LV_OPA_60, LV_PART_ITEMS);

	// text color for all keys
	lv_obj_set_style_text_color(*keyboard, UI_CurrentTheme.main_color_ext, LV_PART_ITEMS);
	lv_obj_set_style_text_color(*keyboard, UI_CurrentTheme.main_color_ext, LV_PART_ITEMS | LV_STATE_CHECKED);
}

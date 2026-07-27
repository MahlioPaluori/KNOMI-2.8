#include "ui_overlay/status_bar/status_bar.h"

#include <lvgl.h>

namespace {
struct StatusBarWidgets {
    lv_obj_t *root;
    lv_obj_t *left;
    lv_obj_t *leftCenter;
    lv_obj_t *center;
    lv_obj_t *rightCenter;
    lv_obj_t *right;
    lv_obj_t *statusMessageLabel;
    lv_obj_t *printerStateLabel;
    lv_obj_t *clockLabel;
};

StatusBarWidgets s_widgets = {
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
};

void style_container(lv_obj_t *obj) {
    if (!obj) {
        return;
    }
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_all(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_radius(obj, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_scrollbar_mode(obj, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t *create_zone_label(lv_obj_t *parent, const char *text) {
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    return label;
}
}

void StatusBar::create(lv_obj_t *parent) {
    if (!parent) {
        return;
    }

    lv_obj_t *root = lv_obj_create(parent);
    lv_obj_set_size(root, LV_PCT(100), LV_PCT(100));
    style_container(root);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(root, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_left(root, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(root, 6, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(root, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_pad_column(root, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t *left = lv_obj_create(root);
    style_container(left);
    lv_obj_set_size(left, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(left, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *left_center = lv_obj_create(root);
    style_container(left_center);
    lv_obj_set_size(left_center, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left_center, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(left_center, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *center = lv_obj_create(root);
    style_container(center);
    lv_obj_set_size(center, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_grow(center, 1);
    lv_obj_set_flex_flow(center, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(center, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *right_center = lv_obj_create(root);
    style_container(right_center);
    lv_obj_set_size(right_center, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right_center, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right_center, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *right = lv_obj_create(root);
    style_container(right);
    lv_obj_set_size(right, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(right, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *status_message_label = create_zone_label(left, "");
    lv_obj_t *printer_state_label = create_zone_label(center, "");
    lv_obj_t *clock_label = create_zone_label(right, "");

    s_widgets.root = root;
    s_widgets.left = left;
    s_widgets.leftCenter = left_center;
    s_widgets.center = center;
    s_widgets.rightCenter = right_center;
    s_widgets.right = right;
    s_widgets.statusMessageLabel = status_message_label;
    s_widgets.printerStateLabel = printer_state_label;
    s_widgets.clockLabel = clock_label;

    StatusBar::setStatusMessage("ONLINE");
    StatusBar::setPrinterState("READY");
    StatusBar::setClock("12:34");
}

void StatusBar::destroy(void) {
    if (s_widgets.root) {
        lv_obj_del(s_widgets.root);
    }
    s_widgets.root = nullptr;
    s_widgets.left = nullptr;
    s_widgets.leftCenter = nullptr;
    s_widgets.center = nullptr;
    s_widgets.rightCenter = nullptr;
    s_widgets.right = nullptr;
    s_widgets.statusMessageLabel = nullptr;
    s_widgets.printerStateLabel = nullptr;
    s_widgets.clockLabel = nullptr;
}

void StatusBar::show(void) {
    if (s_widgets.root) {
        lv_obj_clear_flag(s_widgets.root, LV_OBJ_FLAG_HIDDEN);
    }
}

void StatusBar::hide(void) {
    if (s_widgets.root) {
        lv_obj_add_flag(s_widgets.root, LV_OBJ_FLAG_HIDDEN);
    }
}

lv_obj_t *StatusBar::getRoot(void) {
    return s_widgets.root;
}

void StatusBar::setStatusMessage(const char *text) {
    lv_label_set_text(s_widgets.statusMessageLabel, text);
    lv_obj_invalidate(s_widgets.statusMessageLabel);
}

void StatusBar::setPrinterState(const char *state) {
    lv_label_set_text(s_widgets.printerStateLabel, state);
    lv_obj_invalidate(s_widgets.printerStateLabel);
}

void StatusBar::setClock(const char *time) {
    lv_label_set_text(s_widgets.clockLabel, time);
    lv_obj_invalidate(s_widgets.clockLabel);
}

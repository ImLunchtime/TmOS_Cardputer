#include "apps/calculator/app_calculator.h"
#include <cstdio>
#include <cstdlib>
#include <string>
#include <stack>
#include <cctype>
#include <cmath>

AppCalculator::AppCalculator() {}

AppCalculator::~AppCalculator() {}

void AppCalculator::onClose() {
    if (root_) {
        lv_obj_clean(root_);
        root_ = nullptr;
    }
}

static double applyOp(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0 ? a / b : 0;
    }
    return 0;
}

static int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

static double evaluate_expression(const char* expr) {
    std::stack<double> values;
    std::stack<char> ops;
    
    for (int i = 0; expr[i]; i++) {
        if (isspace(expr[i])) continue;
        
        if (isdigit(expr[i]) || expr[i] == '.') {
            char* end;
            double val = strtod(&expr[i], &end);
            values.push(val);
            i += (end - &expr[i]) - 1;
        } else if (expr[i] == '(') {
            ops.push('(');
        } else if (expr[i] == ')') {
            while (!ops.empty() && ops.top() != '(') {
                double val2 = values.top(); values.pop();
                double val1 = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(val1, val2, op));
            }
            if (!ops.empty()) ops.pop();
        } else {
            while (!ops.empty() && precedence(ops.top()) >= precedence(expr[i])) {
                double val2 = values.top(); values.pop();
                double val1 = values.top(); values.pop();
                char op = ops.top(); ops.pop();
                values.push(applyOp(val1, val2, op));
            }
            ops.push(expr[i]);
        }
    }
    
    while (!ops.empty()) {
        double val2 = values.top(); values.pop();
        double val1 = values.top(); values.pop();
        char op = ops.top(); ops.pop();
        values.push(applyOp(val1, val2, op));
    }
    
    return values.empty() ? 0 : values.top();
}

void AppCalculator::calculate() {
    const char* text = lv_textarea_get_text(ta_input_);
    if (!text || strlen(text) == 0) return;

    double result = evaluate_expression(text);
    
    // Format result to string
    char buf[256];
    // check if integer
    if (floor(result) == result) {
        snprintf(buf, sizeof(buf), "%s = %.0f\n", text, result);
    } else {
        snprintf(buf, sizeof(buf), "%s = %.2f\n", text, result);
    }
    
    lv_textarea_add_text(ta_history_, buf);
    lv_textarea_set_text(ta_input_, "");
}

void AppCalculator::on_btn_clicked(lv_event_t* e) {
    AppCalculator* app = (AppCalculator*)lv_event_get_user_data(e);
    app->calculate();
}

void AppCalculator::on_input_ready(lv_event_t* e) {
    AppCalculator* app = (AppCalculator*)lv_event_get_user_data(e);
    app->calculate();
}

void AppCalculator::onOpen(lv_obj_t* window_root) {
    root_ = window_root;
    lv_obj_set_flex_flow(root_, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(root_, 0, 0);
    
    // History/Answer Area (Top, fills remaining space)
    ta_history_ = lv_textarea_create(root_);
    lv_obj_set_width(ta_history_, lv_pct(100));
    lv_obj_set_flex_grow(ta_history_, 1);
    // Make history read-only
    lv_obj_clear_flag(ta_history_, LV_OBJ_FLAG_CLICK_FOCUSABLE); 
    lv_textarea_set_cursor_click_pos(ta_history_, false);
    lv_textarea_set_placeholder_text(ta_history_, "History");
    
    // Bottom Container (Input + Button)
    lv_obj_t* bottom_cont = lv_obj_create(root_);
    lv_obj_set_width(bottom_cont, lv_pct(100));
    lv_obj_set_height(bottom_cont, 24);
    lv_obj_set_flex_flow(bottom_cont, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(bottom_cont, 0, 0);
    lv_obj_set_style_border_width(bottom_cont, 0, 0);
    
    // Input Area
    ta_input_ = lv_textarea_create(bottom_cont);
    lv_obj_set_flex_grow(ta_input_, 1);
    lv_obj_set_height(ta_input_, lv_pct(100));
    lv_textarea_set_placeholder_text(ta_input_, "Formula...");
    lv_textarea_set_one_line(ta_input_, true);
    lv_obj_add_event_cb(ta_input_, on_input_ready, LV_EVENT_READY, this);
    
    // Calc Button
    btn_calc_ = lv_btn_create(bottom_cont);
    lv_obj_set_width(btn_calc_, 24);
    lv_obj_set_height(btn_calc_, lv_pct(100));
    lv_obj_add_event_cb(btn_calc_, on_btn_clicked, LV_EVENT_CLICKED, this);
    
    lv_obj_t* label = lv_label_create(btn_calc_);
    lv_label_set_text(label, LV_SYMBOL_OK);
    lv_obj_center(label);

    // Apply system font (SimHei 12) to all elements
    ui_theme::apply_small_text_recursive(root_);

    // Focus the textarea so keyboard works immediately
    lv_group_t* g = lv_group_get_default();
    if (g) {
        lv_group_add_obj(g, ta_input_);
        lv_group_add_obj(g, btn_calc_);
        lv_group_focus_obj(ta_input_);
    }
}

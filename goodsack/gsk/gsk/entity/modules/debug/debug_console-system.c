#include "debug_console-system.h"

#include "core/device/device.h"
#include "core/graphics/ui/gui_canvas.h"

#include "runtime/gsk_runtime_wrapper.h"

#include "console/console_cmd.h"
#include "console/console_var.h"

#include "util/logger.h"
#include "util/maths.h"
#include "util/sysdefs.h"

gsk_GuiText *s_debug_text = NULL;
gsk_GuiCanvas *s_canvas   = NULL;

static u8 is_shown = FALSE;

static u8 last_status = FALSE;

static void
init(gsk_Entity ent)
{
    if (ent.index != 0) { return; }

    gsk_Renderer *p_renderer = ent.ecs->renderer;

    gsk_GuiCanvas canvas_debug = gsk_gui_canvas_create(1920, 1080);
    gsk_scene_add_canvas(p_renderer->sceneL[p_renderer->activeScene],
                         canvas_debug,
                         "CANVAS_DEBUG");

    gsk_GuiCanvas *p_canvas = gsk_scene_get_canvas_by_name(
      p_renderer->sceneL[p_renderer->activeScene], "CANVAS_DEBUG");

    s_canvas = p_canvas;

#if 1

    // console panel

    gsk_GuiElement *element_panel =
      gsk_gui_element_create(GskGuiElementAnchorType_Top,
                             (vec2) {0, 0},
                             (vec2) {p_canvas->canvas_size[0], 120},
                             (vec3) {0, 0, 1},
                             NULL,
                             NULL);
    gsk_gui_canvas_add_element(p_canvas, element_panel);

    // vec2 test = {0, 80};
    // glm_vec2_copy(test, element_panel->size);

    // console text

    gsk_GuiText *gui_text_debug =
      gsk_gui_text_create(">",
                          GskGuiElementAnchorType_TopLeft,
                          (vec2) {10.0f, -60.0f},
                          (vec3) {0.0f, 1.0f, 0.0f},
                          256);
    gsk_gui_canvas_add_text(p_canvas, gui_text_debug);

    s_debug_text = gui_text_debug;

#else
    for (int i = 0; i < p_canvas->elements_count; i++)
    {
        gsk_GuiElement *p_element = p_canvas->elements[i];
        if (p_element->max_elements > 1)
        {
            s_debug_text = p_element;
            break;
        }
    }
#endif

    if (s_debug_text == NULL) { LOG_CRITICAL("debug text not found"); }
}

static void
update(gsk_Entity ent)
{
    if (ent.index != 0) { return; }

#if 0
    if (glfwGetKey(ent.ecs->renderer->window, GLFW_KEY_GRAVE_ACCENT) ==
        GLFW_PRESS)
    {
        is_shown = (is_shown == FALSE) ? TRUE : FALSE;
    }
#endif

    s_canvas->is_visible = is_shown;

    const gsk_DebugContext *p_debug_context = ent.ecs->renderer->debugContext;
    s_canvas->is_visible                    = p_debug_context->is_active;
    if (p_debug_context->is_active == FALSE) { return; }

    gsk_Input input = gsk_device_getInput();

    if (input.cursor_state.is_locked == TRUE) { return; }

    gsk_gui_text_modify(s_debug_text, gsk_runtime_get_input_textfield());

    if (glfwGetKey(ent.ecs->renderer->window, GLFW_KEY_F1) == GLFW_PRESS)
    {
        gsk_gui_text_modify(s_debug_text, "");
        gsk_runtime_clear_input_textfield();
    }

    if (glfwGetKey(ent.ecs->renderer->window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        gsk_console_cmd_execute(s_debug_text->str_text);

        gsk_gui_text_modify(s_debug_text, "");
        gsk_runtime_clear_input_textfield();
    }
}

void
s_debug_console_system_init(gsk_ECS *ecs)
{
    gsk_ecs_system_register(ecs,
                            ((gsk_ECSSystem) {
                              .init   = (gsk_ECSSubscriber)init,
                              .update = (gsk_ECSSubscriber)update,
                            }));
}
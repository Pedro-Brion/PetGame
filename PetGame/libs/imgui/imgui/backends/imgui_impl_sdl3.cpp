// dear imgui: Platform Backend for SDL3
// This needs to be used along with a Renderer (e.g. SDL_GPU, DirectX11, OpenGL3, Vulkan..)
// (Info: SDL3 is a cross-platform general purpose library for handling windows, inputs, graphics context creation, etc.)

// Implemented features:
//  [X] Platform: Clipboard support.
//  [X] Platform: Mouse support. Can discriminate Mouse/TouchScreen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass ImGuiKey values to all key functions e.g. ImGui::IsKeyPressed(ImGuiKey_Space). [Legacy SDL_SCANCODE_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility (ImGuiBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: IME support.

// You can use unmodified imgui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire imgui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2025-04-22: IME: honor ImGuiPlatformImeData->WantTextInput as an alternative way to call SDL_StartTextInput(), without IME being necessarily visible.
//  2025-04-09: Don't attempt to call SDL_CaptureMouse() on drivers where we don't call SDL_GetGlobalMouseState(). (#8561)
//  2025-03-30: Update for SDL3 api changes: Revert SDL_GetClipboardText() memory ownership change. (#8530, #7801)
//  2025-03-21: Fill gamepad inputs and set ImGuiBackendFlags_HasGamepad regardless of ImGuiConfigFlags_NavEnableGamepad being set.
//  2025-03-10: When dealing with OEM keys, use scancodes instead of translated keycodes to choose ImGuiKey values. (#7136, #7201, #7206, #7306, #7670, #7672, #8468)
//  2025-02-26: Only start SDL_CaptureMouse() when mouse is being dragged, to mitigate issues with e.g.Linux debuggers not claiming capture back. (#6410, #3650)
//  2025-02-24: Avoid calling SDL_GetGlobalMouseState() when mouse is in relative mode.
//  2025-02-18: Added ImGuiMouseCursor_Wait and ImGuiMouseCursor_Progress mouse cursor support.
//  2025-02-10: Using SDL_OpenURL() in platform_io.Platform_OpenInShellFn handler.
//  2025-01-20: Made ImGui_ImplSDL3_SetGamepadMode(ImGui_ImplSDL3_GamepadMode_Manual) accept an empty array.
//  2024-10-24: Emscripten: SDL_EVENT_MOUSE_WHEEL event doesn't require dividing by 100.0f on Emscripten.
//  2024-09-03: Update for SDL3 api changes: SDL_GetGamepads() memory ownership revert. (#7918, #7898, #7807)
//  2024-08-22: moved some OS/backend related function pointers from ImGuiIO to ImGuiPlatformIO:
//               - io.GetClipboardTextFn    -> platform_io.Platform_GetClipboardTextFn
//               - io.SetClipboardTextFn    -> platform_io.Platform_SetClipboardTextFn
//               - io.PlatformSetImeDataFn  -> platform_io.Platform_SetImeDataFn
//  2024-08-19: Storing SDL_WindowID inside ImGuiViewport::PlatformHandle instead of SDL_Window*.
//  2024-08-19: ImGui_ImplSDL3_ProcessEvent() now ignores events intended for other SDL windows. (#7853)
//  2024-07-22: Update for SDL3 api changes: SDL_GetGamepads() memory ownership change. (#7807)
//  2024-07-18: Update for SDL3 api changes: SDL_GetClipboardText() memory ownership change. (#7801)
//  2024-07-15: Update for SDL3 api changes: SDL_GetProperty() change to SDL_GetPointerProperty(). (#7794)
//  2024-07-02: Update for SDL3 api changes: SDLK_x renames and SDLK_KP_x removals (#7761, #7762).
//  2024-07-01: Update for SDL3 api changes: SDL_SetTextInputRect() changed to SDL_SetTextInputArea().
//  2024-06-26: Update for SDL3 api changes: SDL_StartTextInput()/SDL_StopTextInput()/SDL_SetTextInputRect() functions signatures.
//  2024-06-24: Update for SDL3 api changes: SDL_EVENT_KEY_DOWN/SDL_EVENT_KEY_UP contents.
//  2024-06-03; Update for SDL3 api changes: SDL_SYSTEM_CURSOR_ renames.
//  2024-05-15: Update for SDL3 api changes: SDLK_ renames.
//  2024-04-15: Inputs: Re-enable calling SDL_StartTextInput()/SDL_StopTextInput() as SDL3 no longer enables it by default and should play nicer with IME.
//  2024-02-13: Inputs: Fixed gamepad support. Handle gamepad disconnection. Added ImGui_ImplSDL3_SetGamepadMode().
//  2023-11-13: Updated for recent SDL3 API changes.
//  2023-10-05: Inputs: Added support for extra ImGuiKey values: F13 to F24 function keys, app back/forward keys.
//  2023-05-04: Fixed build on Emscripten/iOS/Android. (#6391)
//  2023-04-06: Inputs: Avoid calling SDL_StartTextInput()/SDL_StopTextInput() as they don't only pertain to IME. It's unclear exactly what their relation is to IME. (#6306)
//  2023-04-04: Inputs: Added support for io.AddMouseSourceEvent() to discriminate ImGuiMouseSource_Mouse/ImGuiMouseSource_TouchScreen. (#2702)
//  2023-02-23: Accept SDL_GetPerformanceCounter() not returning a monotonically increasing value. (#6189, #6114, #3644)
//  2023-02-07: Forked "imgui_impl_sdl2" into "imgui_impl_sdl3". Removed version checks for old feature. Refer to imgui_impl_sdl2.cpp for older changelog.

#include "imgui.h"
#ifndef IMGUI_DISABLE
#include "imgui_impl_sdl3.h"

// Clang warnings with -Weverything
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wimplicit-int-float-conversion"  // warning: implicit conversion from 'xxx' to 'float' may lose precision
#endif

// SDL
#include <SDL3/SDL.h>
#include <stdio.h>              // for snprintf()
#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif
#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#endif

#if !defined(__EMSCRIPTEN__) && !defined(__ANDROID__) && !(defined(__APPLE__) && TARGET_OS_IOS) && !defined(__amigaos4__)
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    1
#else
#define SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE    0
#endif

// FIXME-LEGACY: remove when SDL 3.1.3 preview is released.
#ifndef SDLK_APOSTROPHE
#define SDLK_APOSTROPHE SDLK_QUOTE
#endif
#ifndef SDLK_GRAVE
#define SDLK_GRAVE SDLK_BACKQUOTE
#endif

// SDL Data
struct ImGui_ImplSDL3_Data
{
    SDL_Window*             Window;
    SDL_WindowID            WindowID;
    SDL_Renderer*           Renderer;
    Uint64                  Time;
    char*                   ClipboardTextData;
    char                    BackendPlatformName[48];

    // IME handling
    SDL_Window*             ImeWindow;

    // Mouse handling
    Uint32                  MouseWindowID;
    int                     MouseButtonsDown;
    SDL_Cursor*             MouseCursors[ImGuiMouseCursor_COUNT];
    SDL_Cursor*             MouseLastCursor;
    int                     MousePendingLeaveFrame;
    bool                    MouseCanUseGlobalState;
    bool                    MouseCanUseCapture;

    // Gamepad handling
    ImVector<SDL_Gamepad*>      Gamepads;
    ImGui_ImplSDL3_GamepadMode  GamepadMode;
    bool                        WantUpdateGamepadsList;

    /**
 * @brief Constructs an ImGui_ImplSDL3_Data object and initializes all members to zero.
 */
ImGui_ImplSDL3_Data()   { memset((void*)this, 0, sizeof(*this)); }
};

// Backend data stored in io.BackendPlatformUserData to allow support for multiple Dear ImGui contexts
// It is STRONGLY preferred that you use docking branch with multi-viewports (== single Dear ImGui context + multiple windows) instead of multiple Dear ImGui contexts.
// FIXME: multi-context support is not well tested and probably dysfunctional in this backend.
/**
 * @brief Retrieves the backend data for the current Dear ImGui context.
 *
 * @return Pointer to the ImGui_ImplSDL3_Data structure for the current context, or nullptr if no context is active.
 */
static ImGui_ImplSDL3_Data* ImGui_ImplSDL3_GetBackendData()
{
    return ImGui::GetCurrentContext() ? (ImGui_ImplSDL3_Data*)ImGui::GetIO().BackendPlatformUserData : nullptr;
}

/**
 * @brief Retrieves the current text from the system clipboard.
 *
 * Frees any previously stored clipboard text buffer, obtains the latest clipboard text using SDL, stores it in the backend state, and returns a pointer to the text.
 *
 * @return const char* Pointer to the clipboard text, or nullptr if unavailable.
 */
static const char* ImGui_ImplSDL3_GetClipboardText(ImGuiContext*)
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    if (bd->ClipboardTextData)
        SDL_free(bd->ClipboardTextData);
    bd->ClipboardTextData = SDL_GetClipboardText();
    return bd->ClipboardTextData;
}

/**
 * @brief Sets the system clipboard text to the specified string.
 *
 * @param text Null-terminated UTF-8 string to set as clipboard contents.
 */
static void ImGui_ImplSDL3_SetClipboardText(ImGuiContext*, const char* text)
{
    SDL_SetClipboardText(text);
}

/**
 * @brief Manages IME (Input Method Editor) state and text input area for a given ImGui viewport.
 *
 * Starts or stops SDL text input based on IME visibility or text input requests, and sets the text input rectangle for IME composition. Associates the IME window with the specified viewport as needed.
 */
static void ImGui_ImplSDL3_PlatformSetImeData(ImGuiContext*, ImGuiViewport* viewport, ImGuiPlatformImeData* data)
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    SDL_WindowID window_id = (SDL_WindowID)(intptr_t)viewport->PlatformHandle;
    SDL_Window* window = SDL_GetWindowFromID(window_id);
    if ((!(data->WantVisible || data->WantTextInput) || bd->ImeWindow != window) && bd->ImeWindow != nullptr)
    {
        SDL_StopTextInput(bd->ImeWindow);
        bd->ImeWindow = nullptr;
    }
    if (data->WantVisible)
    {
        SDL_Rect r;
        r.x = (int)data->InputPos.x;
        r.y = (int)data->InputPos.y;
        r.w = 1;
        r.h = (int)data->InputLineHeight;
        SDL_SetTextInputArea(window, &r, 0);
        bd->ImeWindow = window;
    }
    if (data->WantVisible || data->WantTextInput)
        SDL_StartTextInput(window);
}

// Not static to allow third-party code to use that if they want to (but undocumented)
ImGuiKey ImGui_ImplSDL3_KeyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode);
/**
 * @brief Maps SDL keycode and scancode to the corresponding ImGuiKey.
 *
 * Converts SDL3 key events to ImGuiKey values, supporting keypad, function, navigation, and OEM keys. Falls back to scancode mapping for keys not directly represented by SDL keycodes.
 *
 * @param keycode SDL keycode from the key event.
 * @param scancode SDL scancode from the key event.
 * @return ImGuiKey value corresponding to the SDL key event, or ImGuiKey_None if unmapped.
 */
ImGuiKey ImGui_ImplSDL3_KeyEventToImGuiKey(SDL_Keycode keycode, SDL_Scancode scancode)
{
    // Keypad doesn't have individual key values in SDL3
    switch (scancode)
    {
        case SDL_SCANCODE_KP_0: return ImGuiKey_Keypad0;
        case SDL_SCANCODE_KP_1: return ImGuiKey_Keypad1;
        case SDL_SCANCODE_KP_2: return ImGuiKey_Keypad2;
        case SDL_SCANCODE_KP_3: return ImGuiKey_Keypad3;
        case SDL_SCANCODE_KP_4: return ImGuiKey_Keypad4;
        case SDL_SCANCODE_KP_5: return ImGuiKey_Keypad5;
        case SDL_SCANCODE_KP_6: return ImGuiKey_Keypad6;
        case SDL_SCANCODE_KP_7: return ImGuiKey_Keypad7;
        case SDL_SCANCODE_KP_8: return ImGuiKey_Keypad8;
        case SDL_SCANCODE_KP_9: return ImGuiKey_Keypad9;
        case SDL_SCANCODE_KP_PERIOD: return ImGuiKey_KeypadDecimal;
        case SDL_SCANCODE_KP_DIVIDE: return ImGuiKey_KeypadDivide;
        case SDL_SCANCODE_KP_MULTIPLY: return ImGuiKey_KeypadMultiply;
        case SDL_SCANCODE_KP_MINUS: return ImGuiKey_KeypadSubtract;
        case SDL_SCANCODE_KP_PLUS: return ImGuiKey_KeypadAdd;
        case SDL_SCANCODE_KP_ENTER: return ImGuiKey_KeypadEnter;
        case SDL_SCANCODE_KP_EQUALS: return ImGuiKey_KeypadEqual;
        default: break;
    }
    switch (keycode)
    {
        case SDLK_TAB: return ImGuiKey_Tab;
        case SDLK_LEFT: return ImGuiKey_LeftArrow;
        case SDLK_RIGHT: return ImGuiKey_RightArrow;
        case SDLK_UP: return ImGuiKey_UpArrow;
        case SDLK_DOWN: return ImGuiKey_DownArrow;
        case SDLK_PAGEUP: return ImGuiKey_PageUp;
        case SDLK_PAGEDOWN: return ImGuiKey_PageDown;
        case SDLK_HOME: return ImGuiKey_Home;
        case SDLK_END: return ImGuiKey_End;
        case SDLK_INSERT: return ImGuiKey_Insert;
        case SDLK_DELETE: return ImGuiKey_Delete;
        case SDLK_BACKSPACE: return ImGuiKey_Backspace;
        case SDLK_SPACE: return ImGuiKey_Space;
        case SDLK_RETURN: return ImGuiKey_Enter;
        case SDLK_ESCAPE: return ImGuiKey_Escape;
        //case SDLK_APOSTROPHE: return ImGuiKey_Apostrophe;
        case SDLK_COMMA: return ImGuiKey_Comma;
        //case SDLK_MINUS: return ImGuiKey_Minus;
        case SDLK_PERIOD: return ImGuiKey_Period;
        //case SDLK_SLASH: return ImGuiKey_Slash;
        case SDLK_SEMICOLON: return ImGuiKey_Semicolon;
        //case SDLK_EQUALS: return ImGuiKey_Equal;
        //case SDLK_LEFTBRACKET: return ImGuiKey_LeftBracket;
        //case SDLK_BACKSLASH: return ImGuiKey_Backslash;
        //case SDLK_RIGHTBRACKET: return ImGuiKey_RightBracket;
        //case SDLK_GRAVE: return ImGuiKey_GraveAccent;
        case SDLK_CAPSLOCK: return ImGuiKey_CapsLock;
        case SDLK_SCROLLLOCK: return ImGuiKey_ScrollLock;
        case SDLK_NUMLOCKCLEAR: return ImGuiKey_NumLock;
        case SDLK_PRINTSCREEN: return ImGuiKey_PrintScreen;
        case SDLK_PAUSE: return ImGuiKey_Pause;
        case SDLK_LCTRL: return ImGuiKey_LeftCtrl;
        case SDLK_LSHIFT: return ImGuiKey_LeftShift;
        case SDLK_LALT: return ImGuiKey_LeftAlt;
        case SDLK_LGUI: return ImGuiKey_LeftSuper;
        case SDLK_RCTRL: return ImGuiKey_RightCtrl;
        case SDLK_RSHIFT: return ImGuiKey_RightShift;
        case SDLK_RALT: return ImGuiKey_RightAlt;
        case SDLK_RGUI: return ImGuiKey_RightSuper;
        case SDLK_APPLICATION: return ImGuiKey_Menu;
        case SDLK_0: return ImGuiKey_0;
        case SDLK_1: return ImGuiKey_1;
        case SDLK_2: return ImGuiKey_2;
        case SDLK_3: return ImGuiKey_3;
        case SDLK_4: return ImGuiKey_4;
        case SDLK_5: return ImGuiKey_5;
        case SDLK_6: return ImGuiKey_6;
        case SDLK_7: return ImGuiKey_7;
        case SDLK_8: return ImGuiKey_8;
        case SDLK_9: return ImGuiKey_9;
        case SDLK_A: return ImGuiKey_A;
        case SDLK_B: return ImGuiKey_B;
        case SDLK_C: return ImGuiKey_C;
        case SDLK_D: return ImGuiKey_D;
        case SDLK_E: return ImGuiKey_E;
        case SDLK_F: return ImGuiKey_F;
        case SDLK_G: return ImGuiKey_G;
        case SDLK_H: return ImGuiKey_H;
        case SDLK_I: return ImGuiKey_I;
        case SDLK_J: return ImGuiKey_J;
        case SDLK_K: return ImGuiKey_K;
        case SDLK_L: return ImGuiKey_L;
        case SDLK_M: return ImGuiKey_M;
        case SDLK_N: return ImGuiKey_N;
        case SDLK_O: return ImGuiKey_O;
        case SDLK_P: return ImGuiKey_P;
        case SDLK_Q: return ImGuiKey_Q;
        case SDLK_R: return ImGuiKey_R;
        case SDLK_S: return ImGuiKey_S;
        case SDLK_T: return ImGuiKey_T;
        case SDLK_U: return ImGuiKey_U;
        case SDLK_V: return ImGuiKey_V;
        case SDLK_W: return ImGuiKey_W;
        case SDLK_X: return ImGuiKey_X;
        case SDLK_Y: return ImGuiKey_Y;
        case SDLK_Z: return ImGuiKey_Z;
        case SDLK_F1: return ImGuiKey_F1;
        case SDLK_F2: return ImGuiKey_F2;
        case SDLK_F3: return ImGuiKey_F3;
        case SDLK_F4: return ImGuiKey_F4;
        case SDLK_F5: return ImGuiKey_F5;
        case SDLK_F6: return ImGuiKey_F6;
        case SDLK_F7: return ImGuiKey_F7;
        case SDLK_F8: return ImGuiKey_F8;
        case SDLK_F9: return ImGuiKey_F9;
        case SDLK_F10: return ImGuiKey_F10;
        case SDLK_F11: return ImGuiKey_F11;
        case SDLK_F12: return ImGuiKey_F12;
        case SDLK_F13: return ImGuiKey_F13;
        case SDLK_F14: return ImGuiKey_F14;
        case SDLK_F15: return ImGuiKey_F15;
        case SDLK_F16: return ImGuiKey_F16;
        case SDLK_F17: return ImGuiKey_F17;
        case SDLK_F18: return ImGuiKey_F18;
        case SDLK_F19: return ImGuiKey_F19;
        case SDLK_F20: return ImGuiKey_F20;
        case SDLK_F21: return ImGuiKey_F21;
        case SDLK_F22: return ImGuiKey_F22;
        case SDLK_F23: return ImGuiKey_F23;
        case SDLK_F24: return ImGuiKey_F24;
        case SDLK_AC_BACK: return ImGuiKey_AppBack;
        case SDLK_AC_FORWARD: return ImGuiKey_AppForward;
        default: break;
    }

    // Fallback to scancode
    switch (scancode)
    {
    case SDL_SCANCODE_GRAVE: return ImGuiKey_GraveAccent;
    case SDL_SCANCODE_MINUS: return ImGuiKey_Minus;
    case SDL_SCANCODE_EQUALS: return ImGuiKey_Equal;
    case SDL_SCANCODE_LEFTBRACKET: return ImGuiKey_LeftBracket;
    case SDL_SCANCODE_RIGHTBRACKET: return ImGuiKey_RightBracket;
    case SDL_SCANCODE_NONUSBACKSLASH: return ImGuiKey_Oem102;
    case SDL_SCANCODE_BACKSLASH: return ImGuiKey_Backslash;
    case SDL_SCANCODE_SEMICOLON: return ImGuiKey_Semicolon;
    case SDL_SCANCODE_APOSTROPHE: return ImGuiKey_Apostrophe;
    case SDL_SCANCODE_COMMA: return ImGuiKey_Comma;
    case SDL_SCANCODE_PERIOD: return ImGuiKey_Period;
    case SDL_SCANCODE_SLASH: return ImGuiKey_Slash;
    default: break;
    }
    return ImGuiKey_None;
}

/**
 * @brief Updates ImGui modifier key states based on SDL key modifier flags.
 *
 * Sets the current state of Ctrl, Shift, Alt, and Super modifiers in ImGui according to the provided SDL_Keymod bitmask.
 *
 * @param sdl_key_mods Bitmask of SDL key modifier flags.
 */
static void ImGui_ImplSDL3_UpdateKeyModifiers(SDL_Keymod sdl_key_mods)
{
    ImGuiIO& io = ImGui::GetIO();
    io.AddKeyEvent(ImGuiMod_Ctrl, (sdl_key_mods & SDL_KMOD_CTRL) != 0);
    io.AddKeyEvent(ImGuiMod_Shift, (sdl_key_mods & SDL_KMOD_SHIFT) != 0);
    io.AddKeyEvent(ImGuiMod_Alt, (sdl_key_mods & SDL_KMOD_ALT) != 0);
    io.AddKeyEvent(ImGuiMod_Super, (sdl_key_mods & SDL_KMOD_GUI) != 0);
}


/**
 * @brief Returns the ImGui main viewport if the given SDL window ID matches the backend window.
 *
 * @param window_id SDL window ID to check.
 * @return ImGuiViewport* Pointer to the main ImGui viewport if the window ID matches, otherwise nullptr.
 */
static ImGuiViewport* ImGui_ImplSDL3_GetViewportForWindowID(SDL_WindowID window_id)
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    return (window_id == bd->WindowID) ? ImGui::GetMainViewport() : nullptr;
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
/**
 * @brief Processes an SDL3 event and updates ImGui input state accordingly.
 *
 * Handles relevant SDL3 events such as mouse, keyboard, text input, window focus, and gamepad connection changes, translating them into ImGui input events. Returns true if the event was processed by ImGui, false otherwise.
 *
 * @param event Pointer to the SDL_Event to process.
 * @return true if the event was handled by ImGui, false otherwise.
 */
bool ImGui_ImplSDL3_ProcessEvent(const SDL_Event* event)
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplSDL3_Init()?");
    ImGuiIO& io = ImGui::GetIO();

    switch (event->type)
    {
        case SDL_EVENT_MOUSE_MOTION:
        {
            if (ImGui_ImplSDL3_GetViewportForWindowID(event->motion.windowID) == nullptr)
                return false;
            ImVec2 mouse_pos((float)event->motion.x, (float)event->motion.y);
            io.AddMouseSourceEvent(event->motion.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
            io.AddMousePosEvent(mouse_pos.x, mouse_pos.y);
            return true;
        }
        case SDL_EVENT_MOUSE_WHEEL:
        {
            if (ImGui_ImplSDL3_GetViewportForWindowID(event->wheel.windowID) == nullptr)
                return false;
            //IMGUI_DEBUG_LOG("wheel %.2f %.2f, precise %.2f %.2f\n", (float)event->wheel.x, (float)event->wheel.y, event->wheel.preciseX, event->wheel.preciseY);
            float wheel_x = -event->wheel.x;
            float wheel_y = event->wheel.y;
            io.AddMouseSourceEvent(event->wheel.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
            io.AddMouseWheelEvent(wheel_x, wheel_y);
            return true;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
        {
            if (ImGui_ImplSDL3_GetViewportForWindowID(event->button.windowID) == nullptr)
                return false;
            int mouse_button = -1;
            if (event->button.button == SDL_BUTTON_LEFT) { mouse_button = 0; }
            if (event->button.button == SDL_BUTTON_RIGHT) { mouse_button = 1; }
            if (event->button.button == SDL_BUTTON_MIDDLE) { mouse_button = 2; }
            if (event->button.button == SDL_BUTTON_X1) { mouse_button = 3; }
            if (event->button.button == SDL_BUTTON_X2) { mouse_button = 4; }
            if (mouse_button == -1)
                break;
            io.AddMouseSourceEvent(event->button.which == SDL_TOUCH_MOUSEID ? ImGuiMouseSource_TouchScreen : ImGuiMouseSource_Mouse);
            io.AddMouseButtonEvent(mouse_button, (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN));
            bd->MouseButtonsDown = (event->type == SDL_EVENT_MOUSE_BUTTON_DOWN) ? (bd->MouseButtonsDown | (1 << mouse_button)) : (bd->MouseButtonsDown & ~(1 << mouse_button));
            return true;
        }
        case SDL_EVENT_TEXT_INPUT:
        {
            if (ImGui_ImplSDL3_GetViewportForWindowID(event->text.windowID) == nullptr)
                return false;
            io.AddInputCharactersUTF8(event->text.text);
            return true;
        }
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
        {
            if (ImGui_ImplSDL3_GetViewportForWindowID(event->key.windowID) == nullptr)
                return false;
            ImGui_ImplSDL3_UpdateKeyModifiers((SDL_Keymod)event->key.mod);
            //IMGUI_DEBUG_LOG("SDL_EVENT_KEY_%s : key=%d ('%s'), scancode=%d ('%s'), mod=%X\n",
            //    (event->type == SDL_EVENT_KEY_DOWN) ? "DOWN" : "UP  ", event->key.key, SDL_GetKeyName(event->key.key), event->key.scancode, SDL_GetScancodeName(event->key.scancode), event->key.mod);
            ImGuiKey key = ImGui_ImplSDL3_KeyEventToImGuiKey(event->key.key, event->key.scancode);
            io.AddKeyEvent(key, (event->type == SDL_EVENT_KEY_DOWN));
            io.SetKeyEventNativeData(key, event->key.key, event->key.scancode, event->key.scancode); // To support legacy indexing (<1.87 user code). Legacy backend uses SDLK_*** as indices to IsKeyXXX() functions.
            return true;
        }
        case SDL_EVENT_WINDOW_MOUSE_ENTER:
        {
            if (ImGui_ImplSDL3_GetViewportForWindowID(event->window.windowID) == nullptr)
                return false;
            bd->MouseWindowID = event->window.windowID;
            bd->MousePendingLeaveFrame = 0;
            return true;
        }
        // - In some cases, when detaching a window from main viewport SDL may send SDL_WINDOWEVENT_ENTER one frame too late,
        //   causing SDL_WINDOWEVENT_LEAVE on previous frame to interrupt drag operation by clear mouse position. This is why
        //   we delay process the SDL_WINDOWEVENT_LEAVE events by one frame. See issue #5012 for details.
        // FIXME: Unconfirmed whether this is still needed with SDL3.
        case SDL_EVENT_WINDOW_MOUSE_LEAVE:
        {
            if (ImGui_ImplSDL3_GetViewportForWindowID(event->window.windowID) == nullptr)
                return false;
            bd->MousePendingLeaveFrame = ImGui::GetFrameCount() + 1;
            return true;
        }
        case SDL_EVENT_WINDOW_FOCUS_GAINED:
        case SDL_EVENT_WINDOW_FOCUS_LOST:
        {
            if (ImGui_ImplSDL3_GetViewportForWindowID(event->window.windowID) == nullptr)
                return false;
            io.AddFocusEvent(event->type == SDL_EVENT_WINDOW_FOCUS_GAINED);
            return true;
        }
        case SDL_EVENT_GAMEPAD_ADDED:
        case SDL_EVENT_GAMEPAD_REMOVED:
        {
            bd->WantUpdateGamepadsList = true;
            return true;
        }
    }
    return false;
}

/**
 * @brief Sets platform-specific handles for an ImGui viewport using the given SDL window.
 *
 * Assigns the SDL window ID to the viewport's PlatformHandle and, on supported platforms, sets PlatformHandleRaw to the native window handle (e.g., HWND on Windows, Cocoa window pointer on macOS).
 */
static void ImGui_ImplSDL3_SetupPlatformHandles(ImGuiViewport* viewport, SDL_Window* window)
{
    viewport->PlatformHandle = (void*)(intptr_t)SDL_GetWindowID(window);
    viewport->PlatformHandleRaw = nullptr;
#if defined(_WIN32) && !defined(__WINRT__)
    viewport->PlatformHandleRaw = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
#elif defined(__APPLE__) && defined(SDL_VIDEO_DRIVER_COCOA)
    viewport->PlatformHandleRaw = SDL_GetPointerProperty(SDL_GetWindowProperties(window), SDL_PROP_WINDOW_COCOA_WINDOW_POINTER, nullptr);
#endif
}

/**
 * @brief Initializes the Dear ImGui SDL3 platform backend.
 *
 * Sets up ImGui to use SDL3 for windowing, input, clipboard, IME, mouse cursor, and gamepad support. Configures backend capabilities, platform handlers, and loads system mouse cursors. Associates the backend with the provided SDL_Window and optional SDL_Renderer.
 *
 * @param window SDL_Window to associate with ImGui.
 * @param renderer Optional SDL_Renderer for renderer-specific integrations.
 * @param sdl_gl_context Unused in this function; provided for compatibility with other backends.
 * @return true on successful initialization.
 */
static bool ImGui_ImplSDL3_Init(SDL_Window* window, SDL_Renderer* renderer, void* sdl_gl_context)
{
    ImGuiIO& io = ImGui::GetIO();
    IMGUI_CHECKVERSION();
    IM_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");
    IM_UNUSED(sdl_gl_context); // Unused in this branch

    const int ver_linked = SDL_GetVersion();

    // Setup backend capabilities flags
    ImGui_ImplSDL3_Data* bd = IM_NEW(ImGui_ImplSDL3_Data)();
    snprintf(bd->BackendPlatformName, sizeof(bd->BackendPlatformName), "imgui_impl_sdl3 (%u.%u.%u; %u.%u.%u)",
        SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION, SDL_VERSIONNUM_MAJOR(ver_linked), SDL_VERSIONNUM_MINOR(ver_linked), SDL_VERSIONNUM_MICRO(ver_linked));
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = bd->BackendPlatformName;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;           // We can honor GetMouseCursor() values (optional)
    io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;            // We can honor io.WantSetMousePos requests (optional, rarely used)

    bd->Window = window;
    bd->WindowID = SDL_GetWindowID(window);
    bd->Renderer = renderer;

    // Check and store if we are on a SDL backend that supports SDL_GetGlobalMouseState() and SDL_CaptureMouse()
    // ("wayland" and "rpi" don't support it, but we chose to use a white-list instead of a black-list)
    bd->MouseCanUseGlobalState = false;
    bd->MouseCanUseCapture = false;
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
    const char* sdl_backend = SDL_GetCurrentVideoDriver();
    const char* capture_and_global_state_whitelist[] = { "windows", "cocoa", "x11", "DIVE", "VMAN" };
    for (const char* item : capture_and_global_state_whitelist)
        if (strncmp(sdl_backend, item, strlen(item)) == 0)
            bd->MouseCanUseGlobalState = bd->MouseCanUseCapture = true;
#endif

    ImGuiPlatformIO& platform_io = ImGui::GetPlatformIO();
    platform_io.Platform_SetClipboardTextFn = ImGui_ImplSDL3_SetClipboardText;
    platform_io.Platform_GetClipboardTextFn = ImGui_ImplSDL3_GetClipboardText;
    platform_io.Platform_SetImeDataFn = ImGui_ImplSDL3_PlatformSetImeData;
    platform_io.Platform_OpenInShellFn = [](ImGuiContext*, const char* url) { return SDL_OpenURL(url) == 0; };

    // Gamepad handling
    bd->GamepadMode = ImGui_ImplSDL3_GamepadMode_AutoFirst;
    bd->WantUpdateGamepadsList = true;

    // Load mouse cursors
    bd->MouseCursors[ImGuiMouseCursor_Arrow] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    bd->MouseCursors[ImGuiMouseCursor_TextInput] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_TEXT);
    bd->MouseCursors[ImGuiMouseCursor_ResizeAll] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNS] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NS_RESIZE);
    bd->MouseCursors[ImGuiMouseCursor_ResizeEW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_EW_RESIZE);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNESW] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NESW_RESIZE);
    bd->MouseCursors[ImGuiMouseCursor_ResizeNWSE] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NWSE_RESIZE);
    bd->MouseCursors[ImGuiMouseCursor_Hand] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_POINTER);
    bd->MouseCursors[ImGuiMouseCursor_Wait] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT);
    bd->MouseCursors[ImGuiMouseCursor_Progress] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_PROGRESS);
    bd->MouseCursors[ImGuiMouseCursor_NotAllowed] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_NOT_ALLOWED);

    // Set platform dependent data in viewport
    // Our mouse update function expect PlatformHandle to be filled for the main viewport
    ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui_ImplSDL3_SetupPlatformHandles(main_viewport, window);

    // From 2.0.5: Set SDL hint to receive mouse click events on window focus, otherwise SDL doesn't emit the event.
    // Without this, when clicking to gain focus, our widgets wouldn't activate even though they showed as hovered.
    // (This is unfortunately a global SDL setting, so enabling it might have a side-effect on your application.
    // It is unlikely to make a difference, but if your app absolutely needs to ignore the initial on-focus click:
    // you can ignore SDL_EVENT_MOUSE_BUTTON_DOWN events coming right after a SDL_WINDOWEVENT_FOCUS_GAINED)
#ifdef SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH
    SDL_SetHint(SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH, "1");
#endif

    // From 2.0.22: Disable auto-capture, this is preventing drag and drop across multiple windows (see #5710)
#ifdef SDL_HINT_MOUSE_AUTO_CAPTURE
    SDL_SetHint(SDL_HINT_MOUSE_AUTO_CAPTURE, "0");
#endif

    return true;
}

/**
 * @brief Initializes the Dear ImGui SDL3 backend for use with OpenGL rendering.
 *
 * @param window SDL_Window to associate with the ImGui context.
 * @param sdl_gl_context OpenGL context handle for the SDL window.
 * @return true if initialization was successful, false otherwise.
 */
bool ImGui_ImplSDL3_InitForOpenGL(SDL_Window* window, void* sdl_gl_context)
{
    IM_UNUSED(sdl_gl_context); // Viewport branch will need this.
    return ImGui_ImplSDL3_Init(window, nullptr, sdl_gl_context);
}

/**
 * @brief Initializes the Dear ImGui SDL3 backend for use with a Vulkan renderer.
 *
 * @param window Pointer to the SDL_Window to be used with Vulkan rendering.
 * @return true if initialization succeeds, false otherwise.
 */
bool ImGui_ImplSDL3_InitForVulkan(SDL_Window* window)
{
    return ImGui_ImplSDL3_Init(window, nullptr, nullptr);
}

/**
 * @brief Initializes the Dear ImGui SDL3 backend for use with Direct3D rendering.
 *
 * @param window Pointer to the SDL_Window to be used with ImGui.
 * @return true if initialization was successful, false otherwise.
 *
 * @note This function is only supported on Windows platforms.
 */
bool ImGui_ImplSDL3_InitForD3D(SDL_Window* window)
{
#if !defined(_WIN32)
    IM_ASSERT(0 && "Unsupported");
#endif
    return ImGui_ImplSDL3_Init(window, nullptr, nullptr);
}

/**
 * @brief Initializes the Dear ImGui SDL3 backend for use with Metal rendering.
 *
 * @param window Pointer to the SDL_Window to be used with Metal.
 * @return true if initialization was successful, false otherwise.
 */
bool ImGui_ImplSDL3_InitForMetal(SDL_Window* window)
{
    return ImGui_ImplSDL3_Init(window, nullptr, nullptr);
}

/**
 * @brief Initializes the Dear ImGui SDL3 backend for use with an SDL_Renderer.
 *
 * Sets up platform integration for Dear ImGui using the provided SDL_Window and SDL_Renderer.
 *
 * @param window Pointer to the SDL_Window to be used by ImGui.
 * @param renderer Pointer to the SDL_Renderer to be used by ImGui.
 * @return true if initialization was successful, false otherwise.
 */
bool ImGui_ImplSDL3_InitForSDLRenderer(SDL_Window* window, SDL_Renderer* renderer)
{
    return ImGui_ImplSDL3_Init(window, renderer, nullptr);
}

/**
 * @brief Initializes the Dear ImGui SDL3 backend for use with SDL GPU rendering.
 *
 * @param window Pointer to the SDL_Window to be used by ImGui.
 * @return true if initialization was successful, false otherwise.
 */
bool ImGui_ImplSDL3_InitForSDLGPU(SDL_Window* window)
{
    return ImGui_ImplSDL3_Init(window, nullptr, nullptr);
}

/**
 * @brief Initializes the Dear ImGui SDL3 backend for a custom or unspecified rendering backend.
 *
 * @param window Pointer to the SDL_Window to be used by ImGui.
 * @return true if initialization succeeds, false otherwise.
 *
 * @details
 * This function sets up the ImGui SDL3 platform backend for use with a rendering backend not explicitly supported by other initialization helpers. Use this when integrating with a custom graphics API or when none of the provided backend-specific init functions are appropriate.
 */
bool ImGui_ImplSDL3_InitForOther(SDL_Window* window)
{
    return ImGui_ImplSDL3_Init(window, nullptr, nullptr);
}

static void ImGui_ImplSDL3_CloseGamepads();

/**
 * @brief Shuts down the Dear ImGui SDL3 platform backend and releases associated resources.
 *
 * Frees clipboard text, destroys mouse cursors, closes gamepad handles, clears backend flags, and deletes backend state.
 */
void ImGui_ImplSDL3_Shutdown()
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    IM_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");
    ImGuiIO& io = ImGui::GetIO();

    if (bd->ClipboardTextData)
        SDL_free(bd->ClipboardTextData);
    for (ImGuiMouseCursor cursor_n = 0; cursor_n < ImGuiMouseCursor_COUNT; cursor_n++)
        SDL_DestroyCursor(bd->MouseCursors[cursor_n]);
    ImGui_ImplSDL3_CloseGamepads();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(ImGuiBackendFlags_HasMouseCursors | ImGuiBackendFlags_HasSetMousePos | ImGuiBackendFlags_HasGamepad);
    IM_DELETE(bd);
}

/**
 * @brief Updates ImGui mouse input state from SDL, including capture and global position.
 *
 * Handles mouse capture when dragging, updates mouse position based on SDL focus and global mouse state, and optionally sets the OS mouse position if requested by ImGui navigation settings.
 */
static void ImGui_ImplSDL3_UpdateMouseData()
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    ImGuiIO& io = ImGui::GetIO();

    // We forward mouse input when hovered or captured (via SDL_EVENT_MOUSE_MOTION) or when focused (below)
#if SDL_HAS_CAPTURE_AND_GLOBAL_MOUSE
    // - SDL_CaptureMouse() let the OS know e.g. that our drags can extend outside of parent boundaries (we want updated position) and shouldn't trigger other operations outside.
    // - Debuggers under Linux tends to leave captured mouse on break, which may be very inconvenient, so to mitigate the issue we wait until mouse has moved to begin capture.
    if (bd->MouseCanUseCapture)
    {
        bool want_capture = false;
        for (int button_n = 0; button_n < ImGuiMouseButton_COUNT && !want_capture; button_n++)
            if (ImGui::IsMouseDragging(button_n, 1.0f))
                want_capture = true;
        SDL_CaptureMouse(want_capture);
    }

    SDL_Window* focused_window = SDL_GetKeyboardFocus();
    const bool is_app_focused = (bd->Window == focused_window);
#else
    SDL_Window* focused_window = bd->Window;
    const bool is_app_focused = (SDL_GetWindowFlags(bd->Window) & SDL_WINDOW_INPUT_FOCUS) != 0; // SDL 2.0.3 and non-windowed systems: single-viewport only
#endif
    if (is_app_focused)
    {
        // (Optional) Set OS mouse position from Dear ImGui if requested (rarely used, only when io.ConfigNavMoveSetMousePos is enabled by user)
        if (io.WantSetMousePos)
            SDL_WarpMouseInWindow(bd->Window, io.MousePos.x, io.MousePos.y);

        // (Optional) Fallback to provide mouse position when focused (SDL_EVENT_MOUSE_MOTION already provides this when hovered or captured)
        const bool is_relative_mouse_mode = SDL_GetWindowRelativeMouseMode(bd->Window);
        if (bd->MouseCanUseGlobalState && bd->MouseButtonsDown == 0 && !is_relative_mouse_mode)
        {
            // Single-viewport mode: mouse position in client window coordinates (io.MousePos is (0,0) when the mouse is on the upper-left corner of the app window)
            float mouse_x_global, mouse_y_global;
            int window_x, window_y;
            SDL_GetGlobalMouseState(&mouse_x_global, &mouse_y_global);
            SDL_GetWindowPosition(focused_window, &window_x, &window_y);
            io.AddMousePosEvent(mouse_x_global - window_x, mouse_y_global - window_y);
        }
    }
}

/**
 * @brief Updates the OS mouse cursor to match the current ImGui cursor state.
 *
 * Hides the OS cursor if ImGui is drawing its own cursor or requests no cursor. Otherwise, sets and shows the OS cursor to match the ImGui cursor type, unless cursor changes are disabled by configuration.
 */
static void ImGui_ImplSDL3_UpdateMouseCursor()
{
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
        return;
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();

    ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
    if (io.MouseDrawCursor || imgui_cursor == ImGuiMouseCursor_None)
    {
        // Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
        SDL_HideCursor();
    }
    else
    {
        // Show OS mouse cursor
        SDL_Cursor* expected_cursor = bd->MouseCursors[imgui_cursor] ? bd->MouseCursors[imgui_cursor] : bd->MouseCursors[ImGuiMouseCursor_Arrow];
        if (bd->MouseLastCursor != expected_cursor)
        {
            SDL_SetCursor(expected_cursor); // SDL function doesn't have an early out (see #6113)
            bd->MouseLastCursor = expected_cursor;
        }
        SDL_ShowCursor();
    }
}

/**
 * @brief Closes all opened SDL_Gamepad handles and clears the gamepad list.
 *
 * In automatic gamepad modes, this function closes each SDL_Gamepad instance managed by the backend. In manual mode, it only clears the internal gamepad list without closing handles.
 */
static void ImGui_ImplSDL3_CloseGamepads()
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    if (bd->GamepadMode != ImGui_ImplSDL3_GamepadMode_Manual)
        for (SDL_Gamepad* gamepad : bd->Gamepads)
            SDL_CloseGamepad(gamepad);
    bd->Gamepads.resize(0);
}

/**
 * @brief Sets the gamepad input mode for the backend.
 *
 * In manual mode, uses the provided array of SDL_Gamepad pointers as the active gamepads. In automatic modes, triggers an update to detect connected gamepads.
 *
 * @param mode The desired gamepad mode (manual or automatic).
 * @param manual_gamepads_array Array of SDL_Gamepad pointers to use in manual mode.
 * @param manual_gamepads_count Number of gamepads in the manual array.
 */
void ImGui_ImplSDL3_SetGamepadMode(ImGui_ImplSDL3_GamepadMode mode, SDL_Gamepad** manual_gamepads_array, int manual_gamepads_count)
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    ImGui_ImplSDL3_CloseGamepads();
    if (mode == ImGui_ImplSDL3_GamepadMode_Manual)
    {
        IM_ASSERT(manual_gamepads_array != nullptr || manual_gamepads_count <= 0);
        for (int n = 0; n < manual_gamepads_count; n++)
            bd->Gamepads.push_back(manual_gamepads_array[n]);
    }
    else
    {
        IM_ASSERT(manual_gamepads_array == nullptr && manual_gamepads_count <= 0);
        bd->WantUpdateGamepadsList = true;
    }
    bd->GamepadMode = mode;
}

/**
 * @brief Updates the ImGui key event state for a specific gamepad button across all connected gamepads.
 *
 * Merges the state of the specified SDL gamepad button from all connected gamepads and updates the corresponding ImGui key event.
 *
 * @param bd Backend data containing the list of connected gamepads.
 * @param io ImGui IO object to update key events.
 * @param key ImGuiKey corresponding to the gamepad button.
 * @param button_no SDL gamepad button to query.
 */
static void ImGui_ImplSDL3_UpdateGamepadButton(ImGui_ImplSDL3_Data* bd, ImGuiIO& io, ImGuiKey key, SDL_GamepadButton button_no)
{
    bool merged_value = false;
    for (SDL_Gamepad* gamepad : bd->Gamepads)
        merged_value |= SDL_GetGamepadButton(gamepad, button_no) != 0;
    io.AddKeyEvent(key, merged_value);
}

/**
 * @brief Clamps a floating-point value to the range [0.0f, 1.0f].
 *
 * @param v The value to clamp.
 * @return float The clamped value within [0.0f, 1.0f].
 */
static inline float Saturate(float v) { return v < 0.0f ? 0.0f : v  > 1.0f ? 1.0f : v; }
/**
 * @brief Updates the ImGui analog key event for a specific gamepad axis.
 *
 * Merges the normalized analog values from all connected gamepads for the given axis, applies a dead zone, and updates the corresponding ImGui analog key event.
 *
 * @param key ImGuiKey corresponding to the analog input.
 * @param axis_no SDL gamepad axis to read.
 * @param v0 Minimum raw axis value for normalization (dead zone start).
 * @param v1 Maximum raw axis value for normalization (full activation).
 */
static void ImGui_ImplSDL3_UpdateGamepadAnalog(ImGui_ImplSDL3_Data* bd, ImGuiIO& io, ImGuiKey key, SDL_GamepadAxis axis_no, float v0, float v1)
{
    float merged_value = 0.0f;
    for (SDL_Gamepad* gamepad : bd->Gamepads)
    {
        float vn = Saturate((float)(SDL_GetGamepadAxis(gamepad, axis_no) - v0) / (float)(v1 - v0));
        if (merged_value < vn)
            merged_value = vn;
    }
    io.AddKeyAnalogEvent(key, merged_value > 0.1f, merged_value);
}

/**
 * @brief Updates the list of connected gamepads and their input states for ImGui.
 *
 * Scans for connected gamepads if needed, opens handles, and updates ImGui's backend flags and input state for all supported gamepad buttons and analog axes. Handles both automatic and manual gamepad management modes.
 */
static void ImGui_ImplSDL3_UpdateGamepads()
{
    ImGuiIO& io = ImGui::GetIO();
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();

    // Update list of gamepads to use
    if (bd->WantUpdateGamepadsList && bd->GamepadMode != ImGui_ImplSDL3_GamepadMode_Manual)
    {
        ImGui_ImplSDL3_CloseGamepads();
        int sdl_gamepads_count = 0;
        SDL_JoystickID* sdl_gamepads = SDL_GetGamepads(&sdl_gamepads_count);
        for (int n = 0; n < sdl_gamepads_count; n++)
            if (SDL_Gamepad* gamepad = SDL_OpenGamepad(sdl_gamepads[n]))
            {
                bd->Gamepads.push_back(gamepad);
                if (bd->GamepadMode == ImGui_ImplSDL3_GamepadMode_AutoFirst)
                    break;
            }
        bd->WantUpdateGamepadsList = false;
        SDL_free(sdl_gamepads);
    }

    io.BackendFlags &= ~ImGuiBackendFlags_HasGamepad;
    if (bd->Gamepads.Size == 0)
        return;
    io.BackendFlags |= ImGuiBackendFlags_HasGamepad;

    // Update gamepad inputs
    const int thumb_dead_zone = 8000;           // SDL_gamepad.h suggests using this value.
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadStart,       SDL_GAMEPAD_BUTTON_START);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadBack,        SDL_GAMEPAD_BUTTON_BACK);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadFaceLeft,    SDL_GAMEPAD_BUTTON_WEST);           // Xbox X, PS Square
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadFaceRight,   SDL_GAMEPAD_BUTTON_EAST);           // Xbox B, PS Circle
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadFaceUp,      SDL_GAMEPAD_BUTTON_NORTH);          // Xbox Y, PS Triangle
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadFaceDown,    SDL_GAMEPAD_BUTTON_SOUTH);          // Xbox A, PS Cross
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadDpadLeft,    SDL_GAMEPAD_BUTTON_DPAD_LEFT);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadDpadRight,   SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadDpadUp,      SDL_GAMEPAD_BUTTON_DPAD_UP);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadDpadDown,    SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadL1,          SDL_GAMEPAD_BUTTON_LEFT_SHOULDER);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadR1,          SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadL2,          SDL_GAMEPAD_AXIS_LEFT_TRIGGER,  0.0f, 32767);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadR2,          SDL_GAMEPAD_AXIS_RIGHT_TRIGGER, 0.0f, 32767);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadL3,          SDL_GAMEPAD_BUTTON_LEFT_STICK);
    ImGui_ImplSDL3_UpdateGamepadButton(bd, io, ImGuiKey_GamepadR3,          SDL_GAMEPAD_BUTTON_RIGHT_STICK);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadLStickLeft,  SDL_GAMEPAD_AXIS_LEFTX,  -thumb_dead_zone, -32768);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadLStickRight, SDL_GAMEPAD_AXIS_LEFTX,  +thumb_dead_zone, +32767);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadLStickUp,    SDL_GAMEPAD_AXIS_LEFTY,  -thumb_dead_zone, -32768);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadLStickDown,  SDL_GAMEPAD_AXIS_LEFTY,  +thumb_dead_zone, +32767);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadRStickLeft,  SDL_GAMEPAD_AXIS_RIGHTX, -thumb_dead_zone, -32768);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadRStickRight, SDL_GAMEPAD_AXIS_RIGHTX, +thumb_dead_zone, +32767);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadRStickUp,    SDL_GAMEPAD_AXIS_RIGHTY, -thumb_dead_zone, -32768);
    ImGui_ImplSDL3_UpdateGamepadAnalog(bd, io, ImGuiKey_GamepadRStickDown,  SDL_GAMEPAD_AXIS_RIGHTY, +thumb_dead_zone, +32767);
}

/**
 * @brief Retrieves the window size and framebuffer scale for an SDL window.
 *
 * Calculates the logical window size and the framebuffer scale ratio (pixel size to window size) for the given SDL window. If the window is minimized, the size is set to zero.
 *
 * @param window SDL window to query.
 * @param out_size Optional output for the logical window size.
 * @param out_framebuffer_scale Optional output for the framebuffer scale (pixel size divided by window size).
 */
static void ImGui_ImplSDL3_GetWindowSizeAndFramebufferScale(SDL_Window* window, ImVec2* out_size, ImVec2* out_framebuffer_scale)
{
    int w, h;
    int display_w, display_h;
    SDL_GetWindowSize(window, &w, &h);
    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        w = h = 0;
    SDL_GetWindowSizeInPixels(window, &display_w, &display_h);
    if (out_size != nullptr)
        *out_size = ImVec2((float)w, (float)h);
    if (out_framebuffer_scale != nullptr)
        *out_framebuffer_scale = (w > 0 && h > 0) ? ImVec2((float)display_w / w, (float)display_h / h) : ImVec2(1.0f, 1.0f);
}

/**
 * @brief Prepares the Dear ImGui SDL3 backend for a new frame.
 *
 * Updates display size, framebuffer scale, and delta time based on the current SDL window state. Handles pending mouse leave events, updates mouse data and cursor shape, and processes gamepad input for the current frame.
 */
void ImGui_ImplSDL3_NewFrame()
{
    ImGui_ImplSDL3_Data* bd = ImGui_ImplSDL3_GetBackendData();
    IM_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call ImGui_ImplSDL3_Init()?");
    ImGuiIO& io = ImGui::GetIO();

    // Setup main viewport size (every frame to accommodate for window resizing)
    ImGui_ImplSDL3_GetWindowSizeAndFramebufferScale(bd->Window, &io.DisplaySize, &io.DisplayFramebufferScale);

    // Setup time step (we could also use SDL_GetTicksNS() available since SDL3)
    // (Accept SDL_GetPerformanceCounter() not returning a monotonically increasing value. Happens in VMs and Emscripten, see #6189, #6114, #3644)
    static Uint64 frequency = SDL_GetPerformanceFrequency();
    Uint64 current_time = SDL_GetPerformanceCounter();
    if (current_time <= bd->Time)
        current_time = bd->Time + 1;
    io.DeltaTime = bd->Time > 0 ? (float)((double)(current_time - bd->Time) / frequency) : (float)(1.0f / 60.0f);
    bd->Time = current_time;

    if (bd->MousePendingLeaveFrame && bd->MousePendingLeaveFrame >= ImGui::GetFrameCount() && bd->MouseButtonsDown == 0)
    {
        bd->MouseWindowID = 0;
        bd->MousePendingLeaveFrame = 0;
        io.AddMousePosEvent(-FLT_MAX, -FLT_MAX);
    }

    ImGui_ImplSDL3_UpdateMouseData();
    ImGui_ImplSDL3_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    ImGui_ImplSDL3_UpdateGamepads();
}

//-----------------------------------------------------------------------------

#if defined(__clang__)
#pragma clang diagnostic pop
#endif

#endif // #ifndef IMGUI_DISABLE

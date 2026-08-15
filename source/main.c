#include "utils/init.h"
#include "utils/glutil.h"

#include <psp2/kernel/threadmgr.h>

#include <falso_jni/FalsoJNI.h>
#include <so_util/so_util.h>

#include "reimpl/controls.h"

#include "audio.h"

int _newlib_heap_size_user = 256 * 1024 * 1024;

#ifdef USE_SCELIBC_IO
int sceLibcHeapSize = 4 * 1024 * 1024;
#endif

so_module so_mod;
extern void *g_CMvApp_instance;
extern void *g_CMvPlayer_instance;
extern void *g_CGsGraphics_instance;

extern int settings_capframerate;

int (* _ZN6CMvApp10EvKeyPressEi)(void *this, int keycode);
int (* _ZN6CMvApp12EvKeyReleaseEi)(void *this, int keycode);

int (* _ZN6CMvApp14EvPointerPressEP15MC_PointerEvent)(void *this, void *event);
int (* _ZN6CMvApp16EvPointerReleaseEP15MC_PointerEvent)(void *this, void *event);
int (* _ZN6CMvApp13EvPointerMoveEP15MC_PointerEvent)(void *this, void *event);

void (* _ZN12CMvCharacter6FullHPEv)(void *this);
void (* _ZN12CMvCharacter5SetSPEib)(void *this, int param1, int param2);

int pressL1 = 0;
int pressR1 = 0;
int pressSelect = 0;

int main() {
    soloader_init_all();

    int (*JNI_OnLoad)(void *jvm) = (void *)so_symbol(&so_mod, "JNI_OnLoad");
    void (*NativeInit)(void*, void*, int, int) = (void *)so_symbol(&so_mod, "Java_com_gamevil_nexus2_Natives_NativeInit");
    void (*NativeResize)(void*, void*, int, int) = (void *)so_symbol(&so_mod, "Java_com_gamevil_nexus2_Natives_NativeResize");
    void (*NativeRender)(void*, void*) = (void *)so_symbol(&so_mod, "Java_com_gamevil_nexus2_Natives_NativeRender");

    _ZN12CMvCharacter6FullHPEv = (void *)so_symbol(&so_mod, "_ZN12CMvCharacter6FullHPEv");
    _ZN12CMvCharacter5SetSPEib = (void *)so_symbol(&so_mod, "_ZN12CMvCharacter5SetSPEib");
    _ZN6CMvApp10EvKeyPressEi = (void *)so_symbol(&so_mod, "_ZN6CMvApp10EvKeyPressEi");
    _ZN6CMvApp12EvKeyReleaseEi = (void *)so_symbol(&so_mod, "_ZN6CMvApp12EvKeyReleaseEi");

    _ZN6CMvApp14EvPointerPressEP15MC_PointerEvent = (void *)so_symbol(&so_mod, "_ZN6CMvApp14EvPointerPressEP15MC_PointerEvent");
    _ZN6CMvApp16EvPointerReleaseEP15MC_PointerEvent = (void *)so_symbol(&so_mod, "_ZN6CMvApp16EvPointerReleaseEP15MC_PointerEvent");
    _ZN6CMvApp13EvPointerMoveEP15MC_PointerEvent = (void *)so_symbol(&so_mod, "_ZN6CMvApp13EvPointerMoveEP15MC_PointerEvent");

    JNI_OnLoad(&jvm);

    audio_init();
    gl_init();

    if(settings_capframerate) {
        eglSwapInterval(0, 2);
    }

    NativeInit(&jvm, NULL, 960, 544);
    NativeResize(&jvm, NULL, 960, 544);

    while (1) {
        controls_poll();
        NativeRender(&jvm, NULL);
        gl_swap();
    }
    sceKernelExitDeleteThread(0);
}

void controls_handler_key(int32_t keycode, ControlsAction action) {
    if (g_CMvApp_instance) {
        int32_t avk = vita_to_control(keycode);

        switch (action) {
            case CONTROLS_ACTION_DOWN:
                if(keycode == AKEYCODE_BUTTON_L1) { pressL1 = 1; }
                if(keycode == AKEYCODE_BUTTON_R1) { pressR1 = 1; }
                if(keycode == AKEYCODE_BUTTON_SELECT) { pressSelect = 1; }

                if(pressL1 && pressR1 && pressSelect){
                    if(g_CMvPlayer_instance){
                        l_debug("Cheat - Setting players HP and SP to 100");                       
                        _ZN12CMvCharacter6FullHPEv(g_CMvPlayer_instance);
                        _ZN12CMvCharacter5SetSPEib(g_CMvPlayer_instance, 999, 0);
                    }
                }

                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, avk);
                break;
            case CONTROLS_ACTION_UP:
                if(keycode == AKEYCODE_BUTTON_L1) { pressL1 = 0; }
                if(keycode == AKEYCODE_BUTTON_R1) { pressR1 = 0; }
                if(keycode == AKEYCODE_BUTTON_SELECT) { pressSelect = 0; }
                _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, avk);
                break;
        }
    }
}

void controls_handler_touch(int32_t id, float x, float y, ControlsAction action) {
    if (g_CMvApp_instance) {
        // the game defaults to an internal resolution of 400x240, so we need to scale the touch coordinates accordingly
        float xx = x * ((float)400 / (float)960); 
        float yy = y * ((float)240 / (float)544);

        int touches[2] = { (int)xx, (int)yy };

        switch (action) {
            case CONTROLS_ACTION_DOWN:
                l_debug("controls_handler_touch: PointerPress at (%f, %f)", xx, yy);
                _ZN6CMvApp14EvPointerPressEP15MC_PointerEvent(g_CMvApp_instance, touches);
                break;
            case CONTROLS_ACTION_UP:
                l_debug("controls_handler_touch: PointerRelease at (%f, %f)", x, y);
                _ZN6CMvApp16EvPointerReleaseEP15MC_PointerEvent(g_CMvApp_instance, touches);
                break;
            case CONTROLS_ACTION_MOVE:
                l_debug("controls_handler_touch: PointerMove at (%f, %f)", x, y);
                _ZN6CMvApp13EvPointerMoveEP15MC_PointerEvent(g_CMvApp_instance, touches);
                break;
        }
    } else {
        l_debug("controls_handler_touch: g_GVUISystem_instance is NULL, cannot handle touch event");
    }
}

void controls_handler_analog(ControlsStickId which, float x, float y, ControlsAction action) {
    if(which == CONTROLS_STICK_LEFT) {

        if(action == CONTROLS_ACTION_MOVE){
            if(x  > 0.5) {
                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, -4);
            } else if(x < -0.5) {
                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, -3);
            }
            
            if(y > 0.5) {
                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, -2);
            } else if(y < -0.5) {
                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, -1);
            } 
        }

        if(action == CONTROLS_ACTION_UP){
            _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, -1);
            _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, -2);
            _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, -3);
            _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, -4);
        }
    }

    if(which == CONTROLS_STICK_RIGHT) {

        if(action == CONTROLS_ACTION_MOVE){
            if(x  > 0.5) {
                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, 51);
            } else if(x < -0.5) {
                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, 49);
            }
            
            if(y > 0.5) {
                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, 55);
            } else if(y < -0.5) {
                _ZN6CMvApp10EvKeyPressEi(g_CMvApp_instance, 57);
            } 
        }

        if(action == CONTROLS_ACTION_UP){
            _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, 49); // use skill 1
            _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, 51); // use skill 2
            _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, 55); // use skill 3
            _ZN6CMvApp12EvKeyReleaseEi(g_CMvApp_instance, 57); // use skill 4
        }
    }

}

int32_t vita_to_control(int32_t vita_button) {
    switch (vita_button) {
        /*
        Nothing      : 0
        Move Up      : -1
        Move Down    : -2
        Move Left    : -3
        Move Right   : -4
        ???          : -5
        Show Map     : -6
        Attack       : -7
        Unknown Menu : -8
        Change Volume Up   : -13
        Change Volume Down      : -14
        Show Menu    : -16
        Quick Save   : -10
        Unknown Menu 2 : -11
        Change Skill Row: 35
        Use Skill 1 : 49
        run north?   : 50
        Use Skill 2 : 51
        run south?   : 52
        Attack      : 53 
        run east?   : 54
        Use Skill 3 : 55
        run south?   : 56
        Use Skill 4 : 57
        */

        case AKEYCODE_DPAD_UP: return -1;
        case AKEYCODE_DPAD_DOWN: return -2;
        case AKEYCODE_DPAD_LEFT: return -3;
        case AKEYCODE_DPAD_RIGHT: return -4;
        case AKEYCODE_BUTTON_A: return -7;
        //case AKEYCODE_BUTTON_X: return -13;
        case AKEYCODE_BUTTON_Y: return -16;
        case AKEYCODE_BUTTON_START: return -6;
        case AKEYCODE_BUTTON_L1: return 0; 
        
        case AKEYCODE_BUTTON_B: return -12;
        case AKEYCODE_BUTTON_R1: return 11; 

        //case AKEYCODE_BUTTON_SELECT: return -10;
        
        default: return 0;
    }
}
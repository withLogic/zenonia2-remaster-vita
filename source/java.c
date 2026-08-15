#include <falso_jni/FalsoJNI.h>
#include <falso_jni/FalsoJNI_Impl.h>
#include <falso_jni/FalsoJNI_Logger.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "reimpl/asset_manager.h"
#include "utils/logger.h"
#include "audio.h"

/*
 * JNI Methods
 */

static void stub_void(jmethodID id, va_list args) { (void)id; (void)args; }
static jint stub_int(jmethodID id, va_list args) { (void)id; (void)args; return 0; }
static jboolean stub_bool(jmethodID id, va_list args) { (void)id; (void)args; return JNI_FALSE; }
static jobject stub_obj(jmethodID id, va_list args) { (void)id; (void)args; return NULL; }
static jobject stub_bytearray(jmethodID id, va_list args) {
    (void)id; (void)args;
    return (jobject)jda_alloc(0, FIELD_TYPE_BYTE);
}

static jobject readAssete_impl(jmethodID id, va_list args) {
    jstring jstr = va_arg(args, jstring);

    const char* filename = GetStringUTFChars(NULL, jstr, NULL);
    if (!filename) {
        return NULL;
    }

    sceClibPrintf("[readAssete] INFO: Asset open : %s\n", filename);

    AAsset* asset = AAssetManager_open(NULL, filename, AASSET_MODE_BUFFER);
    if (!asset) {
        ReleaseStringUTFChars(NULL, jstr, filename);
        return NULL;
    }

    int length = AAsset_getLength(asset);
    
    if (length <= 0) {
        AAsset_close(asset);
        ReleaseStringUTFChars(NULL, jstr, filename);
        return NULL;
    }

    jbyteArray jarr = NewByteArray(NULL, length);
    if (!jarr) {
        AAsset_close(asset);
        ReleaseStringUTFChars(NULL, jstr, filename);
        return NULL;
    }

    void* buffer = malloc(length);
    if (buffer) {
        AAsset_read(asset, buffer, length);
        SetByteArrayRegion(NULL, jarr, 0, length, (jbyte*)buffer);
        free(buffer);
    }

    AAsset_close(asset);
    ReleaseStringUTFChars(NULL, jstr, filename);

    return (jobject)jarr;
}

static jint isAssetExist_impl(jmethodID id, va_list args) {
    (void)id;

    jstring jstr = va_arg(args, jstring);
    
    if (!jstr) return 0;
    const char* filename = GetStringUTFChars(NULL, jstr, NULL);
    
    if (!filename) {
        return 0;
    }

    AAsset* asset = AAssetManager_open(NULL, filename, AASSET_MODE_UNKNOWN);
    int length = 0;
    
    if (asset) {
        length = AAsset_getLength(asset);
        AAsset_close(asset);
    } else {
        sceClibPrintf("[isAssetExist] ERROR: '%s' not found\n", filename);
    }

    ReleaseStringUTFChars(NULL, jstr, filename);
    
    return (jint)length;
}

static void OnSoundPlay_impl(jmethodID id, va_list args) {
    (void)id;

    jint sndID   = va_arg(args, jint);
    jint vol     = va_arg(args, jint);
    jboolean isLoop = (jboolean)va_arg(args, jint);

    sceClibPrintf("[OnSoundPlay] sndID=%d vol=%d isLoop=%d\n", sndID, vol, isLoop);

    char filepath[256];
    snprintf(filepath, sizeof(filepath), DATA_PATH "/res/raw/s%03d.wav", sndID);
    audio_play_sound(sndID, vol, isLoop);

    sceClibPrintf("[OnSoundPlay] sndID=%s \n", filepath);
}

static void OnStopSound_imp(jmethodID id, va_list args) {
    (void)id;

    sceClibPrintf("[OnStopSound] Stopping all sounds\n");
    audio_stop_sound();
}

static int SetSpeed_imp(jmethodID id, va_list args) {
    (void)id;

    jint speed = va_arg(args, jint);
    sceClibPrintf("[SetSpeed] Setting speed to %d\n", speed);

    return 1;
}

NameToMethodID nameToMethodId[] = {
    { 100, "OnEvent", METHOD_TYPE_VOID },
    { 101, "TrackEventDispatch", METHOD_TYPE_VOID },
    { 102, "TrackPageViewDispatch", METHOD_TYPE_VOID },
    { 103, "clearPlayerName", METHOD_TYPE_VOID },
    { 104, "getGLOptionLinear", METHOD_TYPE_VOID },
    { 105, "isNetAvailable", METHOD_TYPE_VOID },
    { 106, "openStoreWithProductId", METHOD_TYPE_VOID },
    { 107, "requestIAP", METHOD_TYPE_VOID },
    { 108, "setReplyID", METHOD_TYPE_VOID },
    { 109, "getAbsolueFilePath", METHOD_TYPE_VOID },
    { 110, "getPWByte", METHOD_TYPE_VOID },
    { 111, "getRePWByte", METHOD_TYPE_VOID },
    { 112, "hideEditView", METHOD_TYPE_VOID },
    { 113, "saveGLOptionLinear", METHOD_TYPE_VOID },
    { 114, "showKeyboardForMsg", METHOD_TYPE_VOID },
    { 115, "showKeyboardForNumber", METHOD_TYPE_VOID },
    { 116, "OnUIStatusChange", METHOD_TYPE_VOID },
    { 117, "OnSoundPlay", METHOD_TYPE_VOID },
    { 118, "OnStopSound", METHOD_TYPE_VOID },
    { 119, "OnVibrate", METHOD_TYPE_VOID },
    { 120, "SetSpeed", METHOD_TYPE_INT },
    { 121, "hideLoadingDialog", METHOD_TYPE_VOID },
    { 122, "showLoadingDialog", METHOD_TYPE_VOID },
    { 123, "OnShowSaveButton", METHOD_TYPE_VOID },
    { 124, "saveIDData", METHOD_TYPE_VOID },
    { 125, "savePWData", METHOD_TYPE_VOID },
    { 126, "showKeyboardForID", METHOD_TYPE_VOID },
    { 127, "showKeyboardForPassword", METHOD_TYPE_VOID },
    { 128, "showKeyboardForRePassword", METHOD_TYPE_VOID },
    { 129, "getPWFromData", METHOD_TYPE_VOID },
    { 130, "getPasswordHash", METHOD_TYPE_VOID },
    { 131, "isAssetExist", METHOD_TYPE_INT },
    { 132, "isFileExist", METHOD_TYPE_VOID },
    { 133, "readAssete", METHOD_TYPE_OBJECT },
    { 134, "getPhoneNumber", METHOD_TYPE_VOID },
    { 135, "getVersion", METHOD_TYPE_VOID },
    { 136, "OnOpenMiniMap", METHOD_TYPE_VOID },
    { 137, "getIDByte", METHOD_TYPE_VOID },
    { 138, "getIDFromData", METHOD_TYPE_VOID },
    { 139, "getMsgByte", METHOD_TYPE_VOID },
    { 140, "getNumberByte", METHOD_TYPE_VOID },
    { 141, "getPhoneModel", METHOD_TYPE_VOID },
    { 142, "getPlayerNameByte", METHOD_TYPE_VOID }
};

MethodsBoolean methodsBoolean[] = {
    { 100, stub_bool },
    { 101, stub_bool },
    { 102, stub_bool },
    { 103, stub_bool },
    { 104, stub_bool },
    { 105, stub_bool },
    { 106, stub_bool },
    { 107, stub_bool },
    { 108, stub_bool },
    { 109, stub_bool },
    { 110, stub_bool },
    { 111, stub_bool },
    { 112, stub_bool },
    { 113, stub_bool },
    { 114, stub_bool },
    { 115, stub_bool },
    { 116, stub_bool },
    { 117, stub_bool },
    { 118, stub_bool },
    { 119, stub_bool },
    { 120, stub_bool },
    { 121, stub_bool },
    { 122, stub_bool },
    { 123, stub_bool },
    { 124, stub_bool },
    { 125, stub_bool },
    { 126, stub_bool },
    { 127, stub_bool },
    { 128, stub_bool },
    { 129, stub_bool },
    { 130, stub_bool },
    { 131, stub_bool },
    { 132, stub_bool },
    { 133, stub_bool },
    { 134, stub_bool },
    { 135, stub_bool },
    { 136, stub_bool },
    { 137, stub_bool },
    { 138, stub_bool },
    { 139, stub_bool },
    { 140, stub_bool },
    { 141, stub_bool },
    { 142, stub_bool }
};
MethodsByte methodsByte[] = {
    { 100, stub_void },
    { 101, stub_void },
    { 102, stub_void },
    { 103, stub_void },
    { 104, stub_void },
    { 105, stub_void },
    { 106, stub_void },
    { 107, stub_void },
    { 108, stub_void },
    { 109, stub_void },
    { 110, stub_void },
    { 111, stub_void },
    { 112, stub_void },
    { 113, stub_void },
    { 114, stub_void },
    { 115, stub_void },
    { 116, stub_void },
    { 117, stub_void },
    { 118, stub_void },
    { 119, stub_void },
    { 120, stub_void },
    { 121, stub_void },
    { 122, stub_void },
    { 123, stub_void },
    { 124, stub_void },
    { 125, stub_void },
    { 126, stub_void },
    { 127, stub_void },
    { 128, stub_void },
    { 129, stub_void },
    { 130, stub_void },
    { 131, stub_void },
    { 132, stub_void },
    { 133, stub_void },
    { 134, stub_void },
    { 135, stub_void },
    { 136, stub_void },
    { 137, stub_void },
    { 138, stub_void },
    { 139, stub_void },
    { 140, stub_void },
    { 141, stub_void },
    { 142, stub_void }
};
MethodsChar methodsChar[] = {
    { 100, stub_void },
    { 101, stub_void },
    { 102, stub_void },
    { 103, stub_void },
    { 104, stub_void },
    { 105, stub_void },
    { 106, stub_void },
    { 107, stub_void },
    { 108, stub_void },
    { 109, stub_void },
    { 110, stub_void },
    { 111, stub_void },
    { 112, stub_void },
    { 113, stub_void },
    { 114, stub_void },
    { 115, stub_void },
    { 116, stub_void },
    { 117, stub_void },
    { 118, stub_void },
    { 119, stub_void },
    { 120, stub_void },
    { 121, stub_void },
    { 122, stub_void },
    { 123, stub_void },
    { 124, stub_void },
    { 125, stub_void },
    { 126, stub_void },
    { 127, stub_void },
    { 128, stub_void },
    { 129, stub_void },
    { 130, stub_void },
    { 131, stub_void },
    { 132, stub_void },
    { 133, stub_void },
    { 134, stub_void },
    { 135, stub_void },
    { 136, stub_void },
    { 137, stub_void },
    { 138, stub_void },
    { 139, stub_void },
    { 140, stub_void },
    { 141, stub_void },
    { 142, stub_void }
};
MethodsDouble methodsDouble[] = {
    { 100, stub_int },
    { 101, stub_int },
    { 102, stub_int },
    { 103, stub_int },
    { 104, stub_int },
    { 105, stub_int },
    { 106, stub_int },
    { 107, stub_int },
    { 108, stub_int },
    { 109, stub_int },
    { 110, stub_int },
    { 111, stub_int },
    { 112, stub_int },
    { 113, stub_int },
    { 114, stub_int },
    { 115, stub_int },
    { 116, stub_int },
    { 117, stub_int },
    { 118, stub_int },
    { 119, stub_int },
    { 120, stub_int },
    { 121, stub_int },
    { 122, stub_int },
    { 123, stub_int },
    { 124, stub_int },
    { 125, stub_int },
    { 126, stub_int },
    { 127, stub_int },
    { 128, stub_int },
    { 129, stub_int },
    { 130, stub_int },
    { 131, stub_int },
    { 132, stub_int },
    { 133, stub_int },
    { 134, stub_int },
    { 135, stub_int },
    { 136, stub_int },
    { 137, stub_int },
    { 138, stub_int },
    { 139, stub_int },
    { 140, stub_int },
    { 141, stub_int },
    { 142, stub_int }
};
MethodsFloat methodsFloat[] = {
    { 100, stub_int },
    { 101, stub_int },
    { 102, stub_int },
    { 103, stub_int },
    { 104, stub_int },
    { 105, stub_int },
    { 106, stub_int },
    { 107, stub_int },
    { 108, stub_int },
    { 109, stub_int },
    { 110, stub_int },
    { 111, stub_int },
    { 112, stub_int },
    { 113, stub_int },
    { 114, stub_int },
    { 115, stub_int },
    { 116, stub_int },
    { 117, stub_int },
    { 118, stub_int },
    { 119, stub_int },
    { 120, stub_int },
    { 121, stub_int },
    { 122, stub_int },
    { 123, stub_int },
    { 124, stub_int },
    { 125, stub_int },
    { 126, stub_int },
    { 127, stub_int },
    { 128, stub_int },
    { 129, stub_int },
    { 130, stub_int },
    { 131, stub_int },
    { 132, stub_int },
    { 133, stub_int },
    { 134, stub_int },
    { 135, stub_int },
    { 136, stub_int },
    { 137, stub_int },
    { 138, stub_int },
    { 139, stub_int },
    { 140, stub_int },
    { 141, stub_int },
    { 142, stub_int }
};
MethodsInt methodsInt[] = {
    { 100, stub_int },
    { 101, stub_int },
    { 102, stub_int },
    { 103, stub_int },
    { 104, stub_int },
    { 105, stub_int },
    { 106, stub_int },
    { 107, stub_int },
    { 108, stub_int },
    { 109, stub_int },
    { 110, stub_int },
    { 111, stub_int },
    { 112, stub_int },
    { 113, stub_int },
    { 114, stub_int },
    { 115, stub_int },
    { 116, stub_int },
    { 117, stub_int },
    { 118, stub_int },
    { 119, stub_int },
    { 120, SetSpeed_imp },
    { 121, stub_int },
    { 122, stub_int },
    { 123, stub_int },
    { 124, stub_int },
    { 125, stub_int },
    { 126, stub_int },
    { 127, stub_int },
    { 128, stub_int },
    { 129, stub_int },
    { 130, stub_int },
    { 131, isAssetExist_impl },
    { 132, stub_int },
    { 133, stub_int },
    { 134, stub_int },
    { 135, stub_int },
    { 136, stub_int },
    { 137, stub_int },
    { 138, stub_int },
    { 139, stub_int },
    { 140, stub_int },
    { 141, stub_int },
    { 142, stub_int }
};
MethodsLong methodsLong[] = {
    { 100, stub_int },
    { 101, stub_int },
    { 102, stub_int },
    { 103, stub_int },
    { 104, stub_int },
    { 105, stub_int },
    { 106, stub_int },
    { 107, stub_int },
    { 108, stub_int },
    { 109, stub_int },
    { 110, stub_int },
    { 111, stub_int },
    { 112, stub_int },
    { 113, stub_int },
    { 114, stub_int },
    { 115, stub_int },
    { 116, stub_int },
    { 117, stub_int },
    { 118, stub_int },
    { 119, stub_int },
    { 120, stub_int },
    { 121, stub_int },
    { 122, stub_int },
    { 123, stub_int },
    { 124, stub_int },
    { 125, stub_int },
    { 126, stub_int },
    { 127, stub_int },
    { 128, stub_int },
    { 129, stub_int },
    { 130, stub_int },
    { 131, stub_int },
    { 132, stub_int },
    { 133, stub_int },
    { 134, stub_int },
    { 135, stub_int },
    { 136, stub_int },
    { 137, stub_int },
    { 138, stub_int },
    { 139, stub_int },
    { 140, stub_int },
    { 141, stub_int },
    { 142, stub_int }
};
MethodsObject methodsObject[] = {
    { 100, stub_obj },
    { 101, stub_obj },
    { 102, stub_obj },
    { 103, stub_obj },
    { 104, stub_obj },
    { 105, stub_obj },
    { 106, stub_obj },
    { 107, stub_obj },
    { 108, stub_obj },
    { 109, stub_obj },
    { 110, stub_obj },
    { 111, stub_obj },
    { 112, stub_obj },
    { 113, stub_obj },
    { 114, stub_obj },
    { 115, stub_obj },
    { 116, stub_obj },
    { 117, stub_obj },
    { 118, stub_obj },
    { 119, stub_obj },
    { 120, stub_obj },
    { 121, stub_obj },
    { 122, stub_obj },
    { 123, stub_obj },
    { 124, stub_obj },
    { 125, stub_obj },
    { 126, stub_obj },
    { 127, stub_obj },
    { 128, stub_obj },
    { 129, stub_obj },
    { 130, stub_obj },
    { 131, stub_obj },
    { 132, stub_obj },
    { 133, readAssete_impl },
    { 134, stub_obj },
    { 135, stub_obj },
    { 136, stub_bytearray },
    { 137, stub_bytearray },
    { 138, stub_bytearray },
    { 139, stub_bytearray },
    { 140, stub_bytearray },
    { 141, stub_bytearray },
    { 142, stub_bytearray }
};
MethodsShort methodsShort[] = {
    { 100, stub_void },
    { 101, stub_void },
    { 102, stub_void },
    { 103, stub_void },
    { 104, stub_void },
    { 105, stub_void },
    { 106, stub_void },
    { 107, stub_void },
    { 108, stub_void },
    { 109, stub_void },
    { 110, stub_void },
    { 111, stub_void },
    { 112, stub_void },
    { 113, stub_void },
    { 114, stub_void },
    { 115, stub_void },
    { 116, stub_void },
    { 117, stub_void },
    { 118, stub_void },
    { 119, stub_void },
    { 120, stub_void },
    { 121, stub_void },
    { 122, stub_void },
    { 123, stub_void },
    { 124, stub_void },
    { 125, stub_void },
    { 126, stub_void },
    { 127, stub_void },
    { 128, stub_void },
    { 129, stub_void },
    { 130, stub_void },
    { 131, stub_void },
    { 132, stub_void },
    { 133, stub_void },
    { 134, stub_void },
    { 135, stub_void },
    { 136, stub_void },
    { 137, stub_void },
    { 138, stub_void },
    { 139, stub_void },
    { 140, stub_void },
    { 141, stub_void },
    { 142, stub_void }
};
MethodsVoid methodsVoid[] = {
    { 100, stub_void },
    { 101, stub_void },
    { 102, stub_void },
    { 103, stub_void },
    { 104, stub_void },
    { 105, stub_void },
    { 106, stub_void },
    { 107, stub_void },
    { 108, stub_void },
    { 109, stub_void },
    { 110, stub_void },
    { 111, stub_void },
    { 112, stub_void },
    { 113, stub_void },
    { 114, stub_void },
    { 115, stub_void },
    { 116, stub_void },
    { 117, OnSoundPlay_impl },
    { 118, OnStopSound_imp },
    { 119, stub_void },
    { 120, stub_void },
    { 121, stub_void },
    { 122, stub_void },
    { 123, stub_void },
    { 124, stub_void },
    { 125, stub_void },
    { 126, stub_void },
    { 127, stub_void },
    { 128, stub_void },
    { 129, stub_void },
    { 130, stub_void },
    { 131, stub_void },
    { 132, stub_void },
    { 133, stub_void },
    { 134, stub_void },
    { 135, stub_void },
    { 136, stub_void },
    { 137, stub_void },
    { 138, stub_void },
    { 139, stub_void },
    { 140, stub_void },
    { 141, stub_void },
    { 142, stub_void }
};

/*
 * JNI Fields
 */

// System-wide constant that applications sometimes request
// https://developer.android.com/reference/android/content/Context.html#WINDOW_SERVICE
char WINDOW_SERVICE[] = "window";

// System-wide constant that's often used to determine Android version
// https://developer.android.com/reference/android/os/Build.VERSION.html#SDK_INT
// Possible values: https://developer.android.com/reference/android/os/Build.VERSION_CODES
const int SDK_INT = 19; // Android 4.4 / KitKat

NameToFieldID nameToFieldId[] = {
	{ 0, "WINDOW_SERVICE", FIELD_TYPE_OBJECT },
	{ 1, "SDK_INT", FIELD_TYPE_INT },
};

FieldsBoolean fieldsBoolean[] = {};
FieldsByte fieldsByte[] = {};
FieldsChar fieldsChar[] = {};
FieldsDouble fieldsDouble[] = {};
FieldsFloat fieldsFloat[] = {};
FieldsInt fieldsInt[] = {
	{ 1, SDK_INT },
};
FieldsObject fieldsObject[] = {
	{ 0, WINDOW_SERVICE },
};
FieldsLong fieldsLong[] = {};
FieldsShort fieldsShort[] = {};

__FALSOJNI_IMPL_CONTAINER_SIZES

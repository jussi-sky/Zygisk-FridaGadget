#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>
#include <jni.h>
#include <sys/types.h>
#include <malloc.h>
#include <cstring>
#include <dlfcn.h>
#include "zygisk.hpp"
#include <pthread.h>

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "Zygisk-FridaGadget", __VA_ARGS__)

#ifdef __LP64__
constexpr const char* kZygoteNiceName = "zygote64";
constexpr const char* nextLoadSo = "/system/lib64/libminitool.so";
//constexpr const char* nextLoadSo = "/data/data/com.ford.fordpasscn/libfridagadget64.so";
#else
constexpr const char* kZygoteNiceName = "zygote";
constexpr const char* nextLoadSo = "/system/lib/libminitool.so";
#endif

static char nice_process_name[256] = {0};
static char package_name[256] = {0};

static bool isApp(int uid) {
    if (uid < 0) {
        return false;
    }
    int appId = uid % 100000;

    // limit only regular app, or strange situation will happen, such as zygote process not start (dead for no reason and leave no clues?)
    // https://android.googlesource.com/platform/frameworks/base/+/android-9.0.0_r8/core/java/android/os/UserHandle.java#151
    return appId >= 10000 && appId <= 19999;
}

static void* gadget(void* args){

    sleep(10);
    void *handle = dlopen(nextLoadSo, RTLD_LAZY);
    if (!handle) {
        LOGD(" %s loaded in libgadget error %s", nice_process_name, dlerror());
    } else {
        LOGD(" %s load ' %s ' success ", nice_process_name,nextLoadSo);
    }
    return nullptr;
}

class MyModule : public zygisk::ModuleBase {

private:
    Api *api;
    JNIEnv *env;
    jint my_uid = 0;

public:

    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;

    }

    void preAppSpecialize(AppSpecializeArgs *args) override {

        my_uid = args->uid;
        if (!isApp(my_uid)) {
            return;
        }

        const char *tablePath = (env->GetStringUTFChars(args->nice_name, 0));
        sprintf(nice_process_name, "%s", tablePath);
        delete tablePath;

        if (!args->app_data_dir) {
            LOGD(" forkAndSpecializePre appDataDir null");
            return;
        }

        const char *app_data_dir = env->GetStringUTFChars(args->app_data_dir, NULL);
        if (app_data_dir == nullptr) {
            return;
        }
        LOGD("app_data_dir %s",app_data_dir);

        int user = 0;
        if (sscanf(app_data_dir, "/data/%*[^/]/%d/%s", &user, package_name) != 2) {
            if (sscanf(app_data_dir, "/data/%*[^/]/%s", package_name) != 1) {
                package_name[0] = '\0';
                LOGD("can't parse %s", app_data_dir);
            }
        }
        env->ReleaseStringUTFChars(args->app_data_dir, app_data_dir);

    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        LOGD("=========== jussi ==========");

        if (!isApp(my_uid)) {
            return;
        }

//        if (!strstr(nice_process_name, "com.ford.fordpasscn")
//        && !strstr(nice_process_name, "com.jussi.sslpinning")
//        && !strstr(nice_process_name, "com.cmit.irs")
//            ) {
//            LOGD("nice process name: %s", nice_process_name);
//            return;
//        }

        char *app_list;
        const char *filepath = "/data/local/tmp/app.list";
        const char *disable = "/data/local/tmp/finstaller/fs/disable";
        FILE *fp = nullptr;
        FILE *fp_disable = nullptr;
        fp_disable = fopen(disable, "r");

        if(fp_disable!=nullptr){
            LOGD("frida is disabled");
            return;
        }

        fp = fopen(filepath, "r");
        if (fp != nullptr) {

            fseek(fp, 0, SEEK_END);
            int fileLen = ftell(fp);
            app_list = (char *) malloc(sizeof(char) * (fileLen + 1));
            fseek(fp, 0, SEEK_SET);
            size_t count = fread(app_list, 1, fileLen, fp);
            app_list[count] = '\0';
            fclose(fp);

            LOGD("app list: %s", app_list);
            LOGD("package name: %s", package_name);
        } else {
            app_list = "";
        }

        if (!strstr(app_list, package_name)) {
            return;
        }

        LOGD(" nice_process_name=%s, pkg=%s,uid=%d, isApp= %d",
             nice_process_name, package_name, my_uid,
             isApp(my_uid));

        //添加这种机制，就可以提前设置进程名， 从而让frida 的gadget 能够识别到
        jclass java_Process = env->FindClass("android/os/Process");
        if (java_Process != nullptr && isApp(my_uid)) {
            jmethodID mtd_setArgV0 = env->GetStaticMethodID(java_Process, "setArgV0",
                                                            "(Ljava/lang/String;)V");
            jstring name = env->NewStringUTF(nice_process_name);
            env->CallStaticVoidMethod(java_Process, mtd_setArgV0, name);

            pthread_t tid;
            int ret = pthread_create(&tid, NULL, gadget, NULL);
            if (ret != 0)
            {
                LOGD("pthread_create error: error_code=%d" ,ret);
            }
        }
    }
};

static void companion_handler(int i) {
    LOGD("Jussi\n");
}

REGISTER_ZYGISK_MODULE(MyModule)
REGISTER_ZYGISK_COMPANION(companion_handler)

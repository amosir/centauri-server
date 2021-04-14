#if PLATFORM_ANDROID
#include "PluginJniHelper.h"
#include <android/log.h>
#include "Android/AndroidApplication.h"
#include <string.h>
#if 1
#define  LOG_TAG    "PluginJniHelper"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#else
#define  LOGD(...)
#endif



using namespace std;

extern "C"
{
    
    
    //////////////////////////////////////////////////////////////////////////
    // java vm helper function
    //////////////////////////////////////////////////////////////////////////
    
    static jclass _getClassIDByLoader(const char *className) {
        if (NULL == className) {
            return NULL;
        }
        
        JNIEnv* env = CentauriPay::PluginJniHelper::getEnv();
        
        jstring _jstrClassName = env->NewStringUTF(className);
        
        jclass _clazz = FAndroidApplication::FindJavaClass(className);
        
        if (NULL == _clazz) {
            LOGD("Classloader failed to find class of %s", className);
        }
        
        env->DeleteLocalRef(_jstrClassName);
        
        return _clazz;
    }
    
    static jclass _getClassID(const char *className, JNIEnv *env)
    {
        JNIEnv *pEnv = env;
        jclass ret = FAndroidApplication::FindJavaClass(className);
        
        //        do
        //        {
        //            if (Centauri::PluginJniHelper::classloader)
        //            {
        //                ret = _getClassIDByLoader(className);
        //                break;
        //            }
        //
        //            if (! pEnv)
        //            {
        //                pEnv = Centauri::PluginJniHelper::getEnv();
        //                if (! pEnv)
        //                {
        //                    break;
        //                }
        //            }
        //
        //            ret = FAndroidApp;
        //            if (! ret)
        //            {
        //                 LOGD("Failed to find class of %s", className);
        //                break;
        //            }
        //        } while (0);
        
        return ret;
    }
}

namespace CentauriPay {
    
    jmethodID PluginJniHelper::loadclassMethod_methodID = NULL;
    jobject PluginJniHelper::classloader = NULL;
    
    JNIEnv* PluginJniHelper::getEnv()
    {
        return FAndroidApplication::GetJavaEnv();
    }
    
    bool PluginJniHelper::getStaticMethodInfo(PluginJniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode)
    {
        if ((NULL == className) ||
            (NULL == methodName) ||
            (NULL == paramCode)) {
            return false;
        }
        
        JNIEnv *pEnv = PluginJniHelper::getEnv();
        if (!pEnv) {
            LOGD("Failed to get JNIEnv");
            return false;
        }
        
        jclass classID = _getClassID(className, pEnv);
        if (! classID) {
            LOGD("Failed to find class %s", className);
            return false;
        }
        
        jmethodID methodID = pEnv->GetStaticMethodID(classID, methodName, paramCode);
        if (! methodID) {
            if(pEnv->ExceptionCheck())
            {
                pEnv->ExceptionClear();
            }
            LOGD("Failed to find static method id of %s", methodName);
            return false;
        }
        
        methodinfo.classID = classID;
        methodinfo.env = pEnv;
        methodinfo.methodID = methodID;
        return true;
    }
    
    bool PluginJniHelper::getMethodInfo(PluginJniMethodInfo &methodinfo, const char *className, const char *methodName, const char *paramCode)
    {
        if ((NULL == className) ||
            (NULL == methodName) ||
            (NULL == paramCode)) {
            return false;
        }
        
        JNIEnv *pEnv = PluginJniHelper::getEnv();
        if (!pEnv) {
            return false;
        }
        
        jclass classID = _getClassID(className, pEnv);
        if (! classID) {
            LOGD("Failed to find class %s", className);
            return false;
        }
        
        jmethodID methodID = pEnv->GetMethodID(classID, methodName, paramCode);
        if (! methodID) {
            if(pEnv->ExceptionCheck())
            {
                pEnv->ExceptionClear();
            }
            LOGD("Failed to find method id of %s", methodName);
            return false;
        }
        
        methodinfo.classID = classID;
        methodinfo.env = pEnv;
        methodinfo.methodID = methodID;
        
        return true;
    }
    
    string PluginJniHelper::jstring2string(jstring jstr)
    {
        if (jstr == NULL) {
            return "";
        }
        
        JNIEnv *pEnv = PluginJniHelper::getEnv();
        if (! pEnv) {
            return NULL;
        }
        
        const char* chars = pEnv->GetStringUTFChars(jstr, NULL);
        std::string ret(chars);
        pEnv->ReleaseStringUTFChars(jstr, chars);
        
        return ret;
    }
    
    std::map<std::string, std::string> PluginJniHelper::JSONObject2Map(jobject json)
    {
        std::map<std::string, std::string> stdmap;
        
        JNIEnv* env = getEnv();
        
        jclass c_json = FAndroidApplication::FindJavaClass("org/json/JSONObject");
        jclass c_iterator = FAndroidApplication::FindJavaClass("java/util/Iterator");
        
        jmethodID m_keys = env->GetMethodID(c_json, "keys", "()Ljava/util/Iterator;");
        jmethodID m_hasNext = env->GetMethodID(c_iterator, "hasNext", "()Z");
        jmethodID m_next = env->GetMethodID(c_iterator, "next", "()Ljava/lang/Object;");
        jmethodID m_getString = env->GetMethodID(c_json, "getString", "(Ljava/lang/String;)Ljava/lang/String;");
        
        jstring jKeyString = NULL;
        jstring jValueString = NULL;
        
        jobject jKeys = env->CallObjectMethod(json, m_keys);
        while(env->CallBooleanMethod(jKeys, m_hasNext))
        {
            jKeyString = (jstring)(env->CallObjectMethod(jKeys, m_next));
            jValueString = (jstring)(env-> CallObjectMethod(json, m_getString, jKeyString));
            
            stdmap.insert(std::make_pair(jstring2string(jKeyString), jstring2string(jValueString)));
        }
        
        env->DeleteLocalRef(jKeys);
        if(jKeyString)
            env->DeleteLocalRef(jKeyString);
        if(jValueString)
            env->DeleteLocalRef(jValueString);
        env->DeleteLocalRef(c_json);
        env->DeleteLocalRef(c_iterator);
        
        return stdmap;
    }
    
    bool PluginJniHelper::setClassLoaderFrom(jobject nativeactivityinstance) {
        PluginJniMethodInfo _getclassloaderMethod;
        
        
        jobject _c = FAndroidApplication::GetClassLoader();
        
        if (NULL == _c) {
            return false;
        }
        
        PluginJniMethodInfo _m;
        if (!PluginJniHelper::getMethodInfo_DefaultClassLoader(_m,
                                                               "java/lang/ClassLoader",
                                                               "loadClass",
                                                               "(Ljava/lang/String;)Ljava/lang/Class;")) {
            return false;
        }
        
        PluginJniHelper::classloader = _c;
        PluginJniHelper::loadclassMethod_methodID = _m.methodID;
        
        return true;
    }
    
    bool PluginJniHelper::getMethodInfo_DefaultClassLoader(PluginJniMethodInfo &methodinfo,
                                                           const char *className,
                                                           const char *methodName,
                                                           const char *paramCode)
    {
        if ((NULL == className) ||
            (NULL == methodName) ||
            (NULL == paramCode)) {
            return false;
        }
        
        JNIEnv *pEnv = PluginJniHelper::getEnv();
        if (!pEnv) {
            return false;
        }
        
        jclass classID =FAndroidApplication::FindJavaClass(className);
        if (! classID) {
            LOGD("Failed to find class %s", className);
            return false;
        }
        
        jmethodID methodID = pEnv->GetMethodID(classID, methodName, paramCode);
        if (! methodID) {
            LOGD("Failed to find method id of %s", methodName);
            return false;
        }
        
        methodinfo.classID = classID;
        methodinfo.env = pEnv;
        methodinfo.methodID = methodID;
        
        pEnv->DeleteLocalRef(classID);
        return true;
    }
    
    jobject PluginJniHelper::createJavaMapObject(std::map<std::string, std::string>* paramMap)
    {
        JNIEnv* env = getEnv();
        jclass class_Hashtable = FAndroidApplication::FindJavaClass("java/util/Hashtable");
        jmethodID construct_method = env->GetMethodID( class_Hashtable, "<init>","()V");
        jobject obj_Map = env->NewObject( class_Hashtable, construct_method, "");
        if (paramMap != NULL)
        {
            jmethodID add_method= env->GetMethodID( class_Hashtable,"put","(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
            for (std::map<std::string, std::string>::const_iterator it = paramMap->begin(); it != paramMap->end(); ++it)
            {
                jstring first = env->NewStringUTF(it->first.c_str());
                jstring second = env->NewStringUTF(it->second.c_str());
                env->CallObjectMethod(obj_Map, add_method, first, second);
                env->DeleteLocalRef(first);
                env->DeleteLocalRef(second);
            }
        }
        env->DeleteLocalRef(class_Hashtable);
        return obj_Map;
    }
    
    jobject PluginJniHelper::createJavaObject(std::string clsName)
    {
        JNIEnv* env = getEnv();
        
        jclass targetClass = FAndroidApplication::FindJavaClass(clsName.c_str());
        
        //查找该类的构造方法
        jmethodID mid = env->GetMethodID(targetClass, "<init>", "()V");
        
        //生成JniTest类的对象， 返回该对象的引用
        jobject obj =  env->NewObject(targetClass, mid, 100);
		env->DeleteLocalRef(targetClass);
		return obj;
    }
    
    void PluginJniHelper::setJObjectField(jobject obj,std::string fldName,std::string value)
    {
        JNIEnv* env = getEnv();
        jfieldID fld;
        jclass targetClass = env->GetObjectClass(obj);
        fld = env->GetFieldID(targetClass,fldName.c_str(),"Ljava/lang/String;");
        //4.将jstring类型转换成字符指针
        jstring new_str = env->NewStringUTF(value.c_str());
        env->SetObjectField(obj,fld,new_str);
        env->DeleteLocalRef(new_str);
		env->DeleteLocalRef(targetClass);
    }
    
    void PluginJniHelper::setJObjectFieldBoolean(jobject obj,std::string fldName,bool value)
    {
        JNIEnv* env = getEnv();
        jfieldID fld;
        jclass targetClass = env->GetObjectClass(obj);
        fld = env->GetFieldID(targetClass,fldName.c_str(),"Z");
        env->SetBooleanField(obj,fld,value);
		env->DeleteLocalRef(targetClass);
    }
    
    string PluginJniHelper::readJObjectField(jobject obj,std::string fldName)
    {
        JNIEnv* env = getEnv();
        jfieldID fld;
        jclass targetClass = env->GetObjectClass(obj);
        fld = env->GetFieldID(targetClass,fldName.c_str(),"Ljava/lang/String;");
		env->DeleteLocalRef(targetClass);
        return jstring2string((jstring)(env->GetObjectField(obj,fld)));
    }
    
    int PluginJniHelper::readIntField(jobject obj, std::string fldName)
    {
        JNIEnv* env = getEnv();
        jfieldID fld;
        jclass targetClass = env->GetObjectClass(obj);
        fld = env->GetFieldID(targetClass,fldName.c_str(),"I");
        env->DeleteLocalRef(targetClass);
        return env->GetIntField(obj,fld);
    }
} // end of namespace cocos2d
#endif


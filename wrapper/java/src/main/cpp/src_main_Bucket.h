/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class src_main_Bucket */

#ifndef _Included_src_main_Bucket
#define _Included_src_main_Bucket
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     src_main_Bucket
 * Method:    Destroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_src_main_Bucket_Destroy
  (JNIEnv *, jobject);

/*
 * Class:     src_main_Bucket
 * Method:    Get
 * Signature: (Lsrc/main/UniqueId;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_src_main_Bucket_Get
  (JNIEnv *, jobject, jobject);

/*
 * Class:     src_main_Bucket
 * Method:    Put
 * Signature: (Ljava/lang/String;Ljava/lang/String;)Lsrc/main/UniqueId;
 */
JNIEXPORT jobject JNICALL Java_src_main_Bucket_Put
  (JNIEnv *, jobject, jstring, jstring);

/*
 * Class:     src_main_Bucket
 * Method:    DestroyBlob
 * Signature: (Lsrc/main/UniqueId;)V
 */
JNIEXPORT void JNICALL Java_src_main_Bucket_DestroyBlob
  (JNIEnv *, jobject, jobject);

#ifdef __cplusplus
}
#endif
#endif

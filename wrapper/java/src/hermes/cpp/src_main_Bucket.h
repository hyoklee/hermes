/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class src_main_java_Bucket */

#ifndef _Included_src_main_java_Bucket
#define _Included_src_main_java_Bucket
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     src_main_java_Bucket
 * Method:    lock
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_hermes_java_Bucket_lock(JNIEnv *, jobject, jint);

/*
 * Class:     src_main_java_Bucket
 * Method:    unlock
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_hermes_java_Bucket_unlock(JNIEnv *, jobject, jint);

/*
 * Class:     src_main_java_Bucket
 * Method:    getContainedBlobIds
 * Signature: ()Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_hermes_java_Bucket_getContainedBlobIds(JNIEnv *,
                                                                      jobject);

/*
 * Class:     src_main_java_Bucket
 * Method:    destroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_hermes_java_Bucket_destroy(JNIEnv *, jobject);

/*
 * Class:     src_main_java_Bucket
 * Method:    getBlobId
 * Signature: (Ljava/lang/String;)Lsrc/main/java/UniqueId;
 */
JNIEXPORT jobject JNICALL Java_hermes_java_Bucket_getBlobId(JNIEnv *, jobject,
                                                            jstring);

/*
 * Class:     src_main_java_Bucket
 * Method:    get
 * Signature: (Lsrc/main/java/UniqueId;)Lsrc/main/java/Blob;
 */
JNIEXPORT jobject JNICALL Java_hermes_java_Bucket_get(JNIEnv *, jobject,
                                                      jobject);

/*
 * Class:     src_main_java_Bucket
 * Method:    put
 * Signature: (Ljava/lang/String;Lsrc/main/java/Blob;)Lsrc/main/java/UniqueId;
 */
JNIEXPORT jobject JNICALL Java_hermes_java_Bucket_put(JNIEnv *, jobject,
                                                      jstring, jobject);

/*
 * Class:     src_main_java_Bucket
 * Method:    lockBlob
 * Signature: (Lsrc/main/java/UniqueId;I)V
 */
JNIEXPORT void JNICALL Java_hermes_java_Bucket_lockBlob(JNIEnv *, jobject,
                                                        jobject, jint);

/*
 * Class:     src_main_java_Bucket
 * Method:    unlockBlob
 * Signature: (Lsrc/main/java/UniqueId;I)V
 */
JNIEXPORT void JNICALL Java_hermes_java_Bucket_unlockBlob(JNIEnv *, jobject,
                                                          jobject, jint);

/*
 * Class:     src_main_java_Bucket
 * Method:    destroyBlob
 * Signature: (Lsrc/main/java/UniqueId;)V
 */
JNIEXPORT void JNICALL Java_hermes_java_Bucket_destroyBlob(JNIEnv *, jobject,
                                                           jobject);

#ifdef __cplusplus
}
#endif
#endif

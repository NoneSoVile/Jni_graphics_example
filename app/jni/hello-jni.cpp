

class MainClass {
    public:
    MainClass(){};
    ~MainClass(){};
    int SomeFunc() { return 5; }
};

#include <string.h>
#include <jni.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <baseGraphics/NvAssetLoader.h>
#include "hello-jni.h"
#include <android/asset_manager_jni.h>
#define  LOG_TAG    "JNI"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#include <android/log.h>
#include <baseGraphics/NvMatrix.h>
#include <baseGraphics/NvMath.h>
#include <baseGraphics/NvGLSLProgram.h>
#include <baseGraphics/NvImageGL.h>
nv::matrix4<float>  cc;
using nv::matrix4f;
using nv::vec4f;

float m_PreviousX, m_PreviousY, m_DeltaX, m_DeltaY;
float mAngleY = 0.0f, mAngleX = 0.0f;
int m_ScreenWidth, m_ScreenHeight;
int m_TexID;

void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error
            = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
    }
}


void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}



class BaseShader : public NvGLSLProgram
{
public:
    BaseShader(const char *vertexProgramPath,
               const char *fragmentProgramPath) :
            positionAHandle(-1)
    {
        bool success = setSourceFromFiles(vertexProgramPath, fragmentProgramPath);
        LOGI("setSourceFromFiles result = %d", success);
        positionAHandle = getAttribLocation("a_Position");
        texCoordAHandle = getAttribLocation("a_TexCoord");
        colorAHandle = getAttribLocation("a_Color");
        texUHandle = getUniformLocation("u_tTex");

        checkGlError("BaseShader");
    }

    GLint positionAHandle;
    GLint texCoordAHandle;
    GLint colorAHandle;
    GLint texUHandle;
};

BaseShader* mTriangleProgram;
GLuint gvPositionHandle;

bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    mTriangleProgram = new BaseShader("shaders/triangle.vert", "shaders/triangle.frag");
    gvPositionHandle = mTriangleProgram->getAttribLocation("a_Position");
    checkGlError("glGetAttribLocation");

    glViewport(0, 0, w, h);

    m_ScreenWidth = w;
    m_ScreenHeight = h;

    m_TexID =
            NvImageGL::UploadTextureFromDDSFile("textures/android_log.dds");
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
    checkGlError("xx");

    return true;
}


void updateTouchParameters(int previousX, int previousY, int deltaX, int deltaY)
{
    LOGI("touchEvent(%d, %d, %d, %d)", previousX, previousY, deltaX, deltaY);

    m_PreviousX = previousX;
    m_PreviousY = previousY;
    m_DeltaX = deltaX;
    m_DeltaY = deltaY;
    mAngleX += deltaY * 0.01f;
    mAngleY -= deltaX * 0.01f;

}

static const float TRIANGLE_COORDS[] = {
        0.0f, 1.0f, -0.0f, 1.0f,         0, 1.0f,
        -1.0f, -1.0f, -0.0f, 1.0f,          0.0f, 0.0f,
        1.0f,  -1.0f, -0.0f, 1.0f,         1.0f, 0.0f, };

void renderFrame() {
    cc.make_identity();
    matrix4f tmp;
    cc = nv::rotationY(tmp, mAngleY);
    tmp.make_identity();
    cc = cc *  nv::rotationX(tmp, mAngleX);

    glClearColor(0.0, 0, 0, 1.0f);
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    mTriangleProgram->enable();
    checkGlError("glUseProgram");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_TexID);
    glUniform1i(mTriangleProgram->texUHandle, 0);
    mTriangleProgram->setUniformMatrix4fv("uMVP", cc._array, 1, GL_FALSE);
    glVertexAttribPointer(mTriangleProgram->positionAHandle, 4, GL_FLOAT, GL_FALSE, 6* sizeof(float), TRIANGLE_COORDS);
    glVertexAttribPointer(mTriangleProgram->texCoordAHandle, 2, GL_FLOAT, GL_FALSE, 6* sizeof(float), TRIANGLE_COORDS + 4);
    glEnableVertexAttribArray(mTriangleProgram->positionAHandle);
    glEnableVertexAttribArray(mTriangleProgram->texCoordAHandle);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(mTriangleProgram->positionAHandle);
    glDisableVertexAttribArray(mTriangleProgram->texCoordAHandle);
    mTriangleProgram->disable();
}

#ifdef __cplusplus
extern "C" {
#endif


    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_initScreen(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_renderFrame(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_touchParameters(JNIEnv * env, jobject obj, jfloat mPreviousX, jfloat mPreviousY, jfloat mDeltaX, jfloat mDeltaY);
    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_initAssetManager(JNIEnv * env, jobject o, jobject obj);

    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_initScreen(JNIEnv * env, jobject obj,  jint width, jint height)
    {
        MainClass *localObj = new MainClass();
        LOGI("Testing OBJ CREATION %d \n", localObj->SomeFunc());
        setupGraphics(width, height);
    }

    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_renderFrame(JNIEnv * env, jobject obj)
    {
       renderFrame();
    }

    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_touchParameters(JNIEnv * env, jobject obj, jfloat mPreviousX, jfloat mPreviousY, jfloat mDeltaX, jfloat mDeltaY)
    {
        updateTouchParameters(mPreviousX, mPreviousY, mDeltaX, mDeltaY);
    }

JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_initAssetManager(JNIEnv * env, jobject o, jobject obj)
{
    AAssetManager* mgr = AAssetManager_fromJava(env, obj);
    if(mgr!=NULL)
    {
        NvAssetLoaderInit(mgr);
        LOGI(" %s", "AAssetManager!=NULL");
    }

}
#ifdef __cplusplus
}
#endif
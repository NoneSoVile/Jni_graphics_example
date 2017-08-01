

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

#include "hello-jni.h"

#define  LOG_TAG    "JNI"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#include <android/log.h>
#include <baseGraphics/NvMatrix.h>

#include <baseGraphics/NvMath.h>
nv::matrix4<float>  cc;

float m_PreviousX, m_PreviousY, m_DeltaX, m_DeltaY;
int m_ScreenWidth, m_ScreenHeight;

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

GLuint loadShader(GLenum shaderType, const char* pSource) {
    GLuint shader = glCreateShader(shaderType);
    if (shader) {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen) {
                char* buf = (char*) malloc(infoLen);
                if (buf) {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                            shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

static const char gVertexShader[] =
    "attribute vec4 vPosition;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "  vColor = vPosition + vec4(0.0, 0.2, 0.2, 0.2);\n"

    "}\n";

static const char gFragmentShader[] =
    "precision mediump float;\n"
    "varying vec4 vColor;\n"
    "void main() {\n"
    //"  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
    "  gl_FragColor = vColor;\n"
    "}\n";
GLuint gProgram;
GLuint gvPositionHandle;

bool setupGraphics(int w, int h) {
    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    LOGI("setupGraphics(%d, %d)", w, h);
    gProgram = createProgram(gVertexShader, gFragmentShader);
    if (!gProgram) {
        LOGE("Could not create program.");
        return false;
    }
    gvPositionHandle = glGetAttribLocation(gProgram, "vPosition");
    checkGlError("glGetAttribLocation");
    LOGI("glGetAttribLocation(\"vPosition\") = %d\n",
            gvPositionHandle);

    glViewport(0, 0, w, h);
    checkGlError("glViewport");

    m_ScreenWidth = w;
    m_ScreenHeight = h;

    return true;
}


void updateTouchParameters(int previousX, int previousY, int deltaX, int deltaY)
{
    LOGI("touchEvent(%d, %d, %d, %d)", previousX, previousY, deltaX, deltaY);

    m_PreviousX = previousX;
    m_PreviousY = previousY;
    m_DeltaX = deltaX;
    m_DeltaY = deltaY;
}

GLfloat gTriangleVertices[] = { 0.0f, 1.0f,
                               -1.0f,-1.0f,
                                1.0f,-1.0f };

void renderFrame() {
    static float grey;
    cc.make_identity();
    nv::vec4f xxx;

    grey += 0.001f;
    if (grey > 1.0f) {
        grey = 0.0f;
    }
    glClearColor(grey, grey, grey, 1.0f);
    checkGlError("glClearColor");
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    glUseProgram(gProgram);
    checkGlError("glUseProgram");

    if ((m_DeltaX > 0) || (m_DeltaX < 0))
    {
        gTriangleVertices[0] = (m_PreviousX / (float)m_ScreenWidth) - 1.0;
    }
    if ((m_DeltaY > 0) || (m_DeltaY < 0))
    {
        gTriangleVertices[1] = (m_PreviousY / (float)m_ScreenHeight) - 1.0;
    }


    //glVertexAttribPointer(gvPositionHandle, 3, GL_FLOAT, GL_FALSE, 0, bananaVerts);
    glVertexAttribPointer(gvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
    checkGlError("glVertexAttribPointer");
    glEnableVertexAttribArray(gvPositionHandle);
    checkGlError("glEnableVertexAttribArray");
    glDrawArrays(GL_TRIANGLES, 0, 3);
    checkGlError("glDrawArrays");
}

#ifdef __cplusplus
extern "C" {
#endif


    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_initScreen(JNIEnv * env, jobject obj,  jint width, jint height);
    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_renderFrame(JNIEnv * env, jobject obj);
    JNIEXPORT void JNICALL Java_com_example_jni_GL2JNILib_touchParameters(JNIEnv * env, jobject obj, jfloat mPreviousX, jfloat mPreviousY, jfloat mDeltaX, jfloat mDeltaY);

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
#ifdef __cplusplus
}
#endif
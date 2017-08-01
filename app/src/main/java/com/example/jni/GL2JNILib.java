
package com.example.jni;

// Wrapper for native library

public class GL2JNILib {

    static {
        System.loadLibrary("hello-jni");
    }

    /**
     * @param width the current view width
     * @param height the current view height
     */
     public static native void initScreen(int width, int height);
     public static native void renderFrame();
     public static native void touchParameters(float mPreviousX, float mPreviousY, float mDeltaX, float mDeltaY);
}

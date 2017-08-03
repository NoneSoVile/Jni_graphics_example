attribute vec4 a_Position;
varying vec4 vColor;
uniform mediump mat4 uMVP;
void main() {
   gl_Position = uMVP*a_Position;
    vColor = a_Position + vec4(0.0, 0.2, 0.2, 0.2);
    }
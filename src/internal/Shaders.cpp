#include "Shaders.hpp"

const GLchar* g_cosmicVsh = R"(
attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color;

#ifdef GL_ES
varying lowp vec4 v_fragmentColor;
varying mediump vec2 v_texCoord;
varying mediump vec2 v_screenTexCoord;
#else
varying vec4 v_fragmentColor;
varying vec2 v_texCoord;
varying vec2 v_screenTexCoord;
#endif

uniform vec2 u_screenSize;

void main() {
    gl_Position = CC_MVPMatrix * a_position;
    v_fragmentColor = a_color;
    v_texCoord = a_texCoord;

    vec4 pos = CC_MVPMatrix * a_position;
    v_screenTexCoord = (pos.xy / pos.w) * 0.5 + 0.5;

    float aspect = u_screenSize.x / u_screenSize.y;
    v_screenTexCoord.x *= aspect;
}
)";

const GLchar* g_cosmicFsh = R"(
#ifdef GL_ES
precision mediump float;
#endif

varying vec2 v_texCoord;
varying vec2 v_screenTexCoord;
varying vec4 v_fragmentColor;

uniform float u_time;
uniform sampler2D u_texture0; // original
uniform sampler2D CC_Texture1; // cosmic texture
uniform sampler2D CC_Texture2; // normal map
uniform sampler2D CC_Texture3; // star overlay

void main() {
    vec2 base = mod(v_screenTexCoord / 0.15 + vec2(-u_time * 0.04,  u_time * 0.02), 1.0);
    vec2 norm = mod(v_screenTexCoord / 0.15 + vec2( u_time * 0.06, -u_time * 0.03), 1.0);

    vec2 star1 = mod(v_screenTexCoord / 0.15 + vec2( u_time * 0.04, -u_time * 0.025), 1.0);
    vec2 star2 = mod(v_screenTexCoord / 0.12  + vec2(-u_time * 0.03,  u_time * 0.035), 1.0);
    vec2 star3 = mod(v_screenTexCoord / 0.18  + vec2( u_time * 0.02,  u_time * 0.05), 1.0);

    vec4 baseCol = texture2D(CC_Texture1, base);
    vec4 normCol = texture2D(CC_Texture2, norm);
    vec4 star1Col = texture2D(CC_Texture3, star1);
    vec4 star2Col = texture2D(CC_Texture3, star2);
    vec4 star3Col = texture2D(CC_Texture3, star3);

    vec4 overCol = (star1Col + star2Col * 0.8 + star3Col * 0.6) / 2.0;

    vec3 combinedRGB = mix(baseCol.rgb, overCol.rgb, 0.6);
    combinedRGB *= normCol.rgb * 2.0;

    float spriteAlpha = texture2D(u_texture0, v_texCoord).a;
    combinedRGB *= spriteAlpha;

    gl_FragColor = vec4(combinedRGB, spriteAlpha);
}
)";

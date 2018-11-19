#version 330 core

precision lowp float;

out vec4 fragColor;

uniform float iTime;
uniform float iTimeDelta;
uniform vec2 iResolution;
uniform vec3 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

const float dx = 0.5;
const float dt = dx * dx * 0.5;
const int ITER = 1;
const float siz = 0.1;
const int FIELD = 1;
const float vf = 0.005;
const float mul = 20.0;
const float e = 0.0025;

//2D Vector field visualizer by nmz (twitter: @stormoid)

/*
 There is already a shader here on shadertoy for 2d vector field viz,
 but I found it to be hard to use so I decided to write my own.
 Heavily modified by me to make it work as an interactive vector field
 for my fluid sim.
 */

void main()
{
    vec2 p = gl_FragCoord.xy / iResolution.y;
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 mou = iMouse.xy / iResolution.y;
    p *= mul;
    mou *= mul;
    
    float fO = 0.0;
    fO += texture( iChannel1, uv ).r + texture( iChannel1, uv ).g + texture( iChannel1, uv ).b;
    fO *= 0.333;
    
    vec2 ep = vec2( e, 0 );
    vec2 rz= vec2( 0 );
    
    float t0 = 0.0, t1 = 0.0, t2 = 0.0;
    t0 += texture( iChannel0, uv ).a * dt * vf;
    t1 += texture( iChannel0, uv + ep.xy ).a * dt * vf;
    t2 += texture( iChannel0, uv + ep.yx ).a * dt * vf;
    vec2 g = vec2( ( t1 - t0 ), ( t2 - t0 ) ) / ep.xx;
    vec2 t = vec2( -g.y, g.x );
    
    p += 0.9 * t + g * 0.3;
    rz += t;
    
    vec2 fld = rz;
    
    float o = 0.0;
    
    o = texture( iChannel0, uv ).a * 0.99;
    fO += o;
    
    if( uv.y < 0.00 || uv.x < 0.00 || uv.x > 1.0 || uv.y > 1.0 ) o *= 0.0;
    
    fragColor = vec4( fld, 0, fO );
    
}


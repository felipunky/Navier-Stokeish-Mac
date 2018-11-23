#version 330 core
precision mediump float;
out vec4 fragColor;

uniform float iTime;
uniform float iTimeDelta;
uniform vec2 iResolution;
uniform vec3 iMouse;
uniform sampler2D iChannel0;
uniform sampler2D iChannel1;

float scr( vec2 uv, vec2 mou )
{
    
    return length( uv - mou );
    
}

float dis( vec2 p, vec2 uv, vec2 mou )
{
    
    float fin = smoothstep( 0.025, 0.025 - 0.005, scr( uv, mou ) );
    
    return fin;
    
}

void main( )
{
    
    vec2 p = gl_FragCoord.xy / iResolution.y;
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 mou = iMouse.xy / iResolution.y;
    
    float xpo = 1.0 / iResolution.x;
    float ypo = 1.0 / iResolution.y;
    
    vec4 cen = texture( iChannel0, uv );
    float top = texture( iChannel0, vec2( uv.x, uv.y + ypo ) ).r;
    float lef = texture( iChannel0, vec2( uv.x - xpo, uv.y ) ).r;
    float rig = texture( iChannel0, vec2( uv.x + xpo, uv.y ) ).r;
    float dow = texture( iChannel0, vec2( uv.x, uv.y - ypo ) ).r;
    
    float dist = dis( uv, p, mou );
    
    float tot = 0.0;
    
    float fac = -( cen.a - 0.5 ) * 2.0 + ( top + lef + rig + dow - 2.0 );
    
    float tex = texture( iChannel1, uv ).r;
    float texO = texture( iChannel1, uv ).g;
    
    tot += fac;
    //if( iMouse.z > 0.5 )
    //tot += dist; // mouse
    tot += tex;
    tot *= 0.98; // damping
    tot *= step(0.1, iTime); // hacky way of clearing the buffer
    tot = 0.5 + tot * 0.5;
    tot = clamp(tot, 0., 1.);
    
    //fragColor = vec4( mix( vec3( tot, 0.1, 0.4 ), vec3( 0 ), tex ), cen.r );
    
    fragColor = vec4( vec3( tot ), cen.r );
    
}


#version 330 core
out vec4 fragColor;

uniform float iTime;
uniform vec2 iResolution;
uniform vec2 iMouse;

// EPS defines the epsilon that we use as a minimum for going out of our trace
// function ray or for defining our shading's function sha 3D treshold
#define EPS   1e-3

// The STEPS integer stores the number of rays that we shoot at our scene,
// more means a better resolution(specially at the edges) but it also
// messes up our frame rate as it means many more calculations
#define STEPS  256

// The FAR float macro defines where should we stop tracing according to the
// distance from our camera to the 3D scene
#define FAR    10.

// We need this for our hash function
#define HASHSCALE1 .1031

#define PI acos( -1.0 )
#define TPI PI * 2.0

// Dave Hoskin's hash one out two in
float hash(vec2 p)
{
    vec3 p3 = fract(vec3(p.xyx) * HASHSCALE1);
    p3 += dot(p3, p3.yzx + 19.19);
    return fract((p3.x + p3.y) * p3.z);
}

// Divides the 2D space in tiles than those tiles are asigned a random colour
// than we interpolate using GLSL's mix() function to interpolate to combine
// the different random values of each tile into a 2D texture.
float noise(vec2 uv)
{
    
    uv += iTime * 0.7;
    vec2 lv = fract(uv);
    lv = lv * lv * (3.0 - 2.0 * lv);
    vec2 id = floor(uv);
    
    float bl = hash(id);
    float br = hash(id + vec2(1, 0));
    float b = mix(bl, br, lv.x);
    
    float tl = hash(id + vec2(0, 1));
    float tr = hash(id + vec2(1));
    float t = mix(tl, tr, lv.x);
    
    float c = mix(b, t, lv.y);
    
    return c;
    
}


float fbm(vec2 uv)
{
    
    const int OCTAVES = 10;
    
    uv *= 1.4;
    
    float result = 0.0;
    float m = 0.0;
    
    float amplitude = 1.0;
    float freq = 1.0;
    
    for (int i = 0; i < OCTAVES; i++)
    {
        result += cos(noise(uv * freq)) * amplitude; uv += iTime * 0.03;
        m += amplitude;
        amplitude *= 0.45; //lacunarity
        freq *= 2.0; //gain
    }
    
    return result / m;
    
}

float fbmO(vec2 uv)
{
    
    const int OCTAVES = 1;
    
    uv *= 1.4;
    
    float amplitude = 1.0;
    float freq = 1.0;
    
    float result = cos(noise(uv * freq)) * amplitude; uv += iTime * 0.03;
    
    return result;
    
}

// Uncomment to see a sphere that goes according to the path
//#define SPHERE

// Constructs a 2*2 matrix that enables us to rotate in 2D, see:
// https://thebookofshaders.com/08/ for more information on how to
// implement this
mat2 rot(float a)
{
    
    return mat2(cos(a), -sin(a),
                sin(a), cos(a)
                );
    
}

// iq's smooth maximum it returns a smoothed version of max, meaning that it
// gets rid of the discontinuties of the max function see the link below for
// the smooth min implementation which is in principle the same, here we just
// apply it to max instead
// http://iquilezles.org/www/articles/smin/smin.htm
float smax(float a, float b, float k)
{
    
    return log(exp(k*a) + exp(k*b)) / k;
    
}

// This function returns a mass sum of the noise function we just
// defined but we assign an amplitude and a frequency
// https://www.shadertoy.com/view/lsf3zB
float hei(vec2 uv)
{
    
    return (1.45) * fbm(uv);
    
}

float heiO(vec2 uv)
{
    
    return (1.45) * fbmO(uv);
    
}

// https://www.shadertoy.com/view/MlXSWX
// The path is a 2D sinusoid that varies over time, depending upon the
// frequencies, and amplitudes.
vec2 path(in float z)
{
    float a = 44.0;
    float b = a * 0.5;
    float s = sin(z / a)*cos(z / b); return vec2(s*b, 0.);
}

// Defines a Signed Distance Function if its inside the surface it returs 0
// else it returns a positive number, although this is a float that we need
// to output it is important for our shading to return a 2nd value therefore
// it is a vec2, this way we can change our shading according to the index
// that we assign to the SDF
// https://en.wikipedia.org/wiki/Signed_distance_function
vec2 map(vec3 p)
{
    
    
    float c = p.y + hei(p.xz);
    
    return vec2(c, 0.0);
    
}

// We define the perpendiculars according to sampling the Signed Distance
// Function and doing Numerical Differentiation aka we find the derivatives
// https://en.wikipedia.org/wiki/Numerical_differentiation
vec3 norm(vec3 p)
{
    
    vec2 e = vec2(EPS, 0.0);
    return normalize(vec3(map(p + e.xyy).x - map(p - e.xyy).x,
                          map(p + e.yxy).x - map(p - e.yxy).x,
                          map(p + e.yyx).x - map(p - e.yyx).x
                          )
                     );
    
}

// We trace a ray from its Ray Origin(ro) and to its Ray Direction(rd) if we
// get close enough to our Signed Distance Function we stop, this distance is
// defined by EPS aka epsilon. We also stop if the distance of the ray is more
// than the defined maximum length aka FAR
float ray(vec3 ro, vec3 rd, out float d)
{
    
    d = 0.0; float t = 0.0;
    for (int i = 0; i < STEPS; ++i)
    {
        
        // We make our steps smaller so that we don't get any artifacts from
        // the raymarching
        d = 0.5 * map(ro + rd * t).x;
        if (abs(d) < EPS || t > FAR) break;
        
        t += d;
        
    }
    
    return t;
    
}


// We compute the colours according to different simulated phenomena such as
// diffuse, ambient, specularity
// Variable definitions:
// col = to the output RGB channels we are calculating
// d = our Signed Distance Function
// t = our ray's distance
// p = our point in space
// n = our numerical gradient aka derivatives aka perpendicular of our surface
// lig = our lights position, note that we must normalize as we dont want a
// direction but only a point in space
// amb = our ambient light, we use our y direction in the normals to fake a
// sun's parallel rays, in here as we use a geometry that is upside down,
// meaning the top, we must define a negative ambient and use it when our
// material's id is the top surface
// dif = we use the dot product from our normals and our light to get the
// diffuse component we must use the max function to not get a value less
// than 0 as this is incorrect
// spe = our specular component we use the same process of our diffuse
// component but instead we over load it by the clamp and power functions to
// get a much brighter result that simulates the bright reflection of a light
// into a surface
// col /= vec3( 120.0 / ( 8.0 + t * t * 0.05 ) ); is a fogging function, it
// takes into accound the ray variable t to get a distance from our camera
vec3 shad(vec3 ro, vec3 rd)
{
    
    float d = 0.0, t = ray(ro, rd, d);
    vec3 col = vec3(0);
    vec3 p = ro + rd * t;
    vec3 n = norm(p);
    vec3 lig = normalize(vec3(0.0, 1.5, (iTime * 0.2) + 1.0));
    lig.y += heiO(ro.xz);
    
    float amb = 0.5 + 0.5 * n.y;
    float ambO = 0.5 + 0.5 * -n.y;
    float dif = max(0.0, dot(lig, n));
    float spe = pow(clamp(dot(lig, reflect(rd, n)), 0.0, 1.0), 16.0);
    
    float tex = heiO(p.xz);
    
    col += 0.5 * vec3(24, 49, 89) / 256.0;
    
    vec3 fint = mix(vec3(1.0), vec3(0), tex);
    vec3 foa = mix(vec3(0.5), vec3(0), hei(p.xz * 10.0));
    
    col += 0.3 * dif;
    col += 0.3 * amb;
    col += 0.1 * spe;
    
    col += -0.05 + fint;
    col += -0.05 + foa;
    
    col *= 1.0 / (1.0 + t * t * 0.1);
    
    col *= sqrt(col);
    
    return col;
    
}

void main()
{
    // Normalized pixel coordinates (from -1 to 1)
    vec2 uv = (-iResolution.xy + 2.0 * gl_FragCoord.xy) / iResolution.y;
    vec2 mou = iMouse / iResolution;
    mou = -mou;
    
    vec3 ro = vec3(0.0, -0.8 - mou.y, iTime * 0.2);
    
    /*
     // Camera lookat
     vec3 ww = normalize( vec3( 0 ) - ro );
     // Camera up
     vec3 uu = normalize( cross( vec3( 0, 1, 0 ), ww ) );
     // Camera side
     vec3 vv = normalize( cross( ww, uu ) );
     // Add it to the ray direction
     vec3 rd = normalize( uv.x * uu + uv.y * vv - 1.5 * ww );
     */
    vec3 rd = normalize(vec3(uv, 1.0));
    
    ro.xz *= rot(mou.x * TPI);
    rd.xz *= rot(mou.x * TPI);
    
    ro.y *= heiO(ro.xz);
    
    float d = 0.0, t = ray(ro, rd, d);
    vec3 p = ro + rd * t;
    vec3 n = norm(p);
    
    vec3 col = d < EPS ? shad(ro, rd) : mix(vec3(0), vec3(0, 0, 0.09), uv.y - 0.4);
    
    // Output to screen
    fragColor = vec4(col, 1.0);
}

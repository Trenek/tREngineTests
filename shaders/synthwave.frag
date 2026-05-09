#version 450

layout(location = 0) in vec2 fragCoord;

layout(location = 0) out vec4 fragColor;

layout(set = 0, binding = 0) readonly uniform UniformBufferObject {
    vec2 iResolution;
    float iTime;
    float iTimeDelta;
    vec4 pad0;
    vec4 pad1;
    vec4 pad2;
} ubo;

#define RESOLUTION ubo.iResolution
#define TIME ubo.iTime
#define TIME_DELTA ubo.iTimeDelta

const float AA = 1.0;
const bool VAPORWAVE = true;
//#define stereo 1. // -1. for cross-eyed (defaults to parallel view)
#define speed 10.0
const bool wave_thing = true;
const bool city = false;

//you can add any sound texture in iChannel0 to turn it into a cool audio visualizer 
// (it looks better with lower speeds though)
//you should commment out or remove the following line to enable it (it's disabled mainly for performance reasons):
#define disable_sound_texture_sampling

#ifndef disable_sound_texture_sampling
    #undef speed 
    // lower value of speed when using as audio visualizer
    #define speed 5.0
#endif

//self-explainatory
#define audio_vibration_amplitude 0.125

float jTime;

#ifdef disable_sound_texture_sampling
#define textureMirror(a, b) vec4(0)
#else
vec4 textureMirror(sampler2D tex, vec2 c){
    vec2 cf = fract(c);
    return texture(tex, mix(cf, 1.0 - cf, mod(floor(c), 2.0)));
}
#endif

float amp(const vec2 p){
    return smoothstep(1.,8.,abs(p.x));   
}

float pow512(const float a){ return pow(a, 512); }
float pow1d5(const float a){ return pow(a, 1.5); }

float hash21(const vec2 co){
    return fract(sin(dot(co, vec2(1.9898, 7.233))) * 45758.5433);
}
float hash(const vec2 uv){
    const float a = amp(uv);
    const float w = 
        wave_thing == false ? 1.0 :
        a > 0.0             ? (1.0 - 0.4 * pow512(0.51 + 0.49 * sin((0.02 * (uv.y + 0.5 * uv.x) - jTime) * 2)))
                            : 0.0;
    return (
        (a > 0.0 ? a * pow1d5(hash21(uv)) * w : 0.0) -
        (textureMirror(iChannel0, vec2((29 * uv.x + uv.y) * 0.03125, 1.0)).x) * audio_vibration_amplitude
    );
}

float edgeMin(const float dx, const vec2 da, const vec2 db, vec2 uv) {
    uv.x += 5.0;

    const vec3 c = fract((round(vec3(uv, uv.x + uv.y))) * (vec3(0, 1, 2) + 0.61803398875));
    const float a1 = textureMirror(iChannel0, vec2(c.y, 0.0)).x > 0.6 ? 0.15 : 1.0;
    const float a2 = textureMirror(iChannel0, vec2(c.x, 0.0)).x > 0.6 ? 0.15 : 1.0;
    const float a3 = textureMirror(iChannel0, vec2(c.z, 0.0)).x > 0.6 ? 0.15 : 1.0;

    return min(min((1.0 - dx) * db.y * a3, da.x * a2), da.y * a1);
}

vec2 trinoise(vec2 uv){
    const float sq = sqrt(3.0 / 2.0);
    uv.x *= sq;
    uv.y -= 0.5 * uv.x;

    vec2 d = fract(uv);
    uv -= d;

    bool c = dot(d, vec2(1.0)) > 1.0;

    vec2 dd = 1.0 - d;
    vec2 da = c ? dd : d;
    vec2 db = c ? d : dd;
    
    float nn = hash(uv + float(c));
    float n2 = hash(uv + vec2(1, 0));
    float n3 = hash(uv + vec2(0, 1));
    
    float nmid = mix(n2, n3, d.y);
    float ns = mix(nn, c ? n2 : n3, da.y);
    float dx = da.x / db.y;

    return vec2(mix(ns, nmid, dx), edgeMin(dx, da, db, uv + d));
}

vec2 map(vec3 p){
    vec2 n = trinoise(p.xz);
    return vec2(p.y - 2 * n.x, n.y);
}

vec3 grad(vec3 p){
    const vec2 e = vec2(0.005, 0.0);
    const float a = map(p).x;

    return (vec3(
        map(p + e.xyy).x,
        map(p + e.yxy).x,
        map(p + e.yyx).x
    ) - a) / e.x;
}

vec2 intersect(vec3 ro, vec3 rd){
    float d = 0.0;
    float h = 0.0;
    vec3 p = vec3(0.0);

    // look nice with 50 iterations
    for (int i = 0; i < 5000 && d <= 150.0 && p.y <= 2.0; i += 1) {
        p = ro + d * rd;
        vec2 s = map(p);
        h = s.x;
        d += h * 0.5;

        if (abs(h) < 0.003 * d) {
            return vec2(d, s.y);
        }
    }
    
    return vec2(-1);
}


void addsun(vec3 rd,vec3 ld,inout vec3 col){
	float sun = smoothstep(0.21, 0.2, distance(rd, ld));
    
    if (sun > 0.0) {
        float yd = (rd.y - ld.y);

        float a = sin(3.1 * exp(-yd * 14.0)); 

        sun *= smoothstep(-0.8, 0.0, a);
        col = mix(col, vec3(1.0, 0.8, 0.4) * 0.75, sun);
    }
}

float starnoise(vec3 rd){
    float c = 0.0;
    vec3 p = normalize(rd) * 300.0;
	for (float i = 0.0; i < 4.0; i += 1) {
        vec3 q = fract(p) - 0.5;
        vec3 id = floor(p);
        float c2 = smoothstep(0.5, 0.0, length(q));
        c2 *= step(hash21(id.xz / id.y), 0.06 - i * i * 0.005);
        c += c2;
        p = p * 0.6 + 0.5 * p * mat3(
            3.0 / 5.0, 0, 4.0 / 5.0, 
            0, 1, 0, 
            -4.0 / 5.0, 0, 3.0 / 5.0
        );
    }
    c *= c;
    float g = dot(sin(rd * 10.512), cos(rd.yzx * 10.512));
    c *= smoothstep(-3.14, -0.9, g) * 0.5 + 0.5 * smoothstep(-0.3, 1.0, g);

    return c * c;
}

vec3 gsky(vec3 rd,vec3 ld,bool mask){
    float haze = exp2(-5.0 * (abs(rd.y) - 0.2 * dot(rd, ld)));

    float st = mask ? starnoise(rd) * (1.0 - min(haze, 1.0)) : 0.0;
    vec3 back = 
        vec3(0.4, 0.1, 0.7) * 
        (1.0 - 0.5 * textureMirror(iChannel0, vec2(0.5 + 0.05 * rd.x / rd.y, 0.0)).x *
        exp2(-0.1 * abs(length(rd.xz) / rd.y)) *
        max(sign(rd.y), 0.0));

    if (city) {
        float x = round(rd.x * 30);
        float h = hash21(vec2(x - 166));
        bool building = (h * h * 0.125 * exp2(- x * x * x * x * 0.0025) > rd.y);
        if (mask && building) {
            back *= 0.0;
            haze = 0.8;
            mask = mask && !building;
        }
    }
    vec3 col = clamp(mix(back, vec3(0.7, 0.1, 0.4), haze) + st, 0.0, 1.0);
    if (mask) {
        addsun(rd,ld,col);
    }
    return col;  
}

void main() {
    fragColor = vec4(0);
    vec2 myFragCoord = fragCoord * RESOLUTION;

    for (float x = 0.0; x < 1.0; x += 1.0 / AA)
    for (float y = 0.0; y < 1.0; y += 1.0 / AA) {
        vec2 uv = (2 * (myFragCoord + vec2(x, y)) - RESOLUTION) / RESOLUTION.y;
        
        const float shutter_speed = 0.25; // for motion blur

        float dt = fract(hash21(AA * (myFragCoord + vec2(x, y))) + TIME) * shutter_speed;
        jTime = mod(TIME - dt * TIME_DELTA, 4000.0);
        vec3 ro = vec3(0.0, 1.0, -20000 + jTime * speed);
        
#ifdef stereo
        ro += stereo * vec3(0.2 * (float(uv.x > 0.0) - 0.5), 0.0, 0.0); 
        const float de = 0.9;
        uv.x = uv.x + 0.5 * (uv.x > 0.0 ? -de : de);
        uv *= 2.0;
#endif
            
        vec3 rd = normalize(vec3(uv, 4.0 / 3.0));
        
        vec2 i = intersect(ro, rd);
        float d = i.x;
        
        vec3 ld = normalize(vec3(0.0, 0.125 + 0.05 * sin(0.1 * jTime), 1.0));

        vec3 fog = d > 0.0 ? exp2(-d * vec3(0.14, 0.1, 0.28)) : vec3(0.0);
        vec3 sky = gsky(rd, ld, d < 0.0);
        
        vec3 p = ro + d * rd;
        vec3 n = normalize(grad(p));
        
        float diff = dot(n, ld) + 0.1 * n.y;
        vec3 col = vec3(0.1, 0.11, 0.18) * diff;
        
        vec3 rfd = reflect(rd, n); 
        vec3 rfcol = gsky(rfd, ld, true);
        
        col = mix(col, rfcol, 0.05 + 0.95 * pow(max(1.0 + dot(rd, n), 0.0), 5.0));
        if (VAPORWAVE) {
            col = mix(col, vec3(0.5, 0.5, 1.0), smoothstep(0.05, 0.0, i.y));
            col = pow(mix(sky, col, fog), vec3(1.7));
        }
        else {
            col = mix(col, vec3(0.8, 0.1, 0.92), smoothstep(0.05, 0.0, i.y));
            col = mix(sky, col, fog);
        }
        if (d < 0.0) {
            d=1e6;
        }

        d = min(d, 10.0);
        fragColor += vec4(clamp(col, 0.0, 1.0), d < 0.0 ? 0.0 : 0.1 + exp2(-d));
    }

    fragColor /= pow(AA, 2);
}

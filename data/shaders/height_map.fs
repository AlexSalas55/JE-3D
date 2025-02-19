varying vec3 v_position;
varying vec3 v_world_position;
varying vec3 v_normal;
varying vec2 v_uv;
varying vec4 v_color;

uniform vec4 u_color;
uniform sampler2D u_texture;
uniform float u_time;

void main()
{
    vec2 uv = v_uv;
    float heightmap = texture2D(u_texture, uv).x;

    vec3 final_color = vec3(1.0);
    
    if (heightmap < 0.1) {
        final_color = vec3(0.8, 0.85, 0.9);    // Hielo azulado claro
    } else
    if (heightmap < 0.2) {
        final_color = vec3(0.85, 0.88, 0.92);  // Hielo más blanquecino
    } else
    if (heightmap < 0.3) {
        final_color = vec3(0.88, 0.9, 0.93);   // Nieve compacta con tono azulado
    } else
    if (heightmap < 0.45) {
        final_color = vec3(0.9, 0.92, 0.95);   // Nieve semi-compacta
    } else
    if (heightmap < 0.6) {
        final_color = vec3(0.92, 0.93, 0.96);  // Nieve ligera
    } else
    if (heightmap < 0.75) {
        final_color = vec3(0.94, 0.95, 0.97);  // Nieve fresca con toque azulado
    } else
    if (heightmap < 0.85) {
        final_color = vec3(0.96, 0.96, 0.98);  // Nieve blanca casi pura
    } else {
        final_color = vec3(0.98, 0.98, 1.0);   // Nieve blanca pura con sutil toque azul
    }

    gl_FragColor = u_color * vec4(final_color, 1.0);

}
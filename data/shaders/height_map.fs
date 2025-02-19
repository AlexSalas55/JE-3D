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

    vec3 final_color;
    float blend = 0.1; // Factor de suavizado

    if (heightmap < 0.1) {
        final_color = vec3(0.8, 0.85, 0.9);    // Hielo azulado claro
    } else if (heightmap < 0.2) {
        float t = smoothstep(0.1 - blend, 0.1 + blend, heightmap);
        final_color = mix(
            vec3(0.8, 0.85, 0.9),              // Hielo azulado claro
            vec3(0.85, 0.88, 0.92),            // Hielo más blanquecino
            t
        );
    } else if (heightmap < 0.3) {
        float t = smoothstep(0.2 - blend, 0.2 + blend, heightmap);
        final_color = mix(
            vec3(0.85, 0.88, 0.92),            // Hielo más blanquecino
            vec3(0.88, 0.9, 0.93),             // Nieve compacta con tono azulado
            t
        );
    } else if (heightmap < 0.45) {
        float t = smoothstep(0.3 - blend, 0.3 + blend, heightmap);
        final_color = mix(
            vec3(0.88, 0.9, 0.93),             // Nieve compacta con tono azulado
            vec3(0.9, 0.92, 0.95),             // Nieve semi-compacta
            t
        );
    } else if (heightmap < 0.6) {
        float t = smoothstep(0.45 - blend, 0.45 + blend, heightmap);
        final_color = mix(
            vec3(0.9, 0.92, 0.95),             // Nieve semi-compacta
            vec3(0.92, 0.93, 0.96),            // Nieve ligera
            t
        );
    } else if (heightmap < 0.75) {
        float t = smoothstep(0.6 - blend, 0.6 + blend, heightmap);
        final_color = mix(
            vec3(0.92, 0.93, 0.96),            // Nieve ligera
            vec3(0.94, 0.95, 0.97),            // Nieve fresca con toque azulado
            t
        );
    } else if (heightmap < 0.85) {
        float t = smoothstep(0.75 - blend, 0.75 + blend, heightmap);
        final_color = mix(
            vec3(0.94, 0.95, 0.97),            // Nieve fresca con toque azulado
            vec3(0.96, 0.96, 0.98),            // Nieve blanca casi pura
            t
        );
    } else {
        float t = smoothstep(0.85 - blend, 0.85 + blend, heightmap);
        final_color = mix(
            vec3(0.96, 0.96, 0.98),            // Nieve blanca casi pura
            vec3(0.98, 0.98, 1.0),             // Nieve blanca pura con sutil toque azul
            t
        );
    }

    gl_FragColor = u_color * vec4(final_color, 1.0);

}
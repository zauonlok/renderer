#include "phong_shader.h"
#include <math.h>
#include "../geometry.h"
#include "../graphics.h"
#include "../image.h"

vec4_t phong_vertex_shader(int nth_vertex, void *attribs_,
                           void *varyings_, void *uniforms_) {
    phong_attribs_t *attribs = (phong_attribs_t*)attribs_;
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    phong_uniforms_t *uniforms = (phong_uniforms_t*)uniforms_;

    /* query model position  */
    vec3_t in_position = attribs->positions[nth_vertex];
    vec4_t position = vec4_from_vec3(in_position, 1.0f);

    /* setup clip position */
    mat4_t mvp_matrix = uniforms->mvp_matrix;
    vec4_t clip_coord = mat4_mul_vec4(mvp_matrix, position);

    /* setup view position */
    {
        mat4_t mv_matrix = uniforms->mv_matrix;
        vec4_t view_pos = mat4_mul_vec4(mv_matrix, position);
        varyings->vs_out_view_pos[nth_vertex] = vec3_from_vec4(view_pos);
    }

    /* setup texture coords */
    {
        vec2_t in_texcoord = attribs->texcoords[nth_vertex];
        varyings->vs_out_texcoords[nth_vertex] = in_texcoord;
    }

    return clip_coord;
}

void phong_interp_varyings(void *varyings_, vec3_t weights) {
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    vec2_t *vs_out_texcoords = varyings->vs_out_texcoords;
    vec3_t *vs_out_view_pos = varyings->vs_out_view_pos;
    varyings->fs_in_texcoord = gfx_interp_vec2(vs_out_texcoords, weights);
    varyings->fs_in_view_pos = gfx_interp_vec3(vs_out_view_pos, weights);
}

static float max_float(float a, float b) {
    return a > b ? a : b;
}

vec4_t phong_fragment_shader(void *varyings_, void *uniforms_) {
    phong_varyings_t *varyings = (phong_varyings_t*)varyings_;
    phong_uniforms_t *uniforms = (phong_uniforms_t*)uniforms_;

    /* for convenience */
    vec2_t in_texcoord = varyings->fs_in_texcoord;
    image_t *diffuse_map = uniforms->diffuse_map;
    image_t *normal_map = uniforms->normal_map;
    image_t *specular_map = uniforms->specular_map;

    /* sample textures */
    vec4_t in_diffuse = gfx_sample_texture(diffuse_map, in_texcoord);
    vec4_t in_normal = gfx_sample_texture(normal_map, in_texcoord);
    vec4_t in_specular_ = gfx_sample_texture(specular_map, in_texcoord);
    float in_specular = in_specular_.x;  /* the specular map is monochrome */

    vec3_t normal, light;
    vec3_t ambient, diffuse, specular;
    vec4_t color;

    /* transform normal */
    {
        /* for normal map, (r,g,b) -> (x,y,z), tga uses bgr color order */
        vec4_t normal_4f = vec4_new(in_normal.z * 2.0f - 1.0f,
                                    in_normal.y * 2.0f - 1.0f,
                                    in_normal.x * 2.0f - 1.0f,
                                    0.0f);
        normal_4f = mat4_mul_vec4(uniforms->normal_matrix, normal_4f);
        normal = vec3_normalize(vec3_from_vec4(normal_4f));
    }
    /* transform light */
    {
        vec4_t light_4f = vec4_from_vec3(uniforms->light_direction, 0.0f);
        light_4f = mat4_mul_vec4(uniforms->view_matrix, light_4f);
        light = vec3_normalize(vec3_from_vec4(light_4f));
    }

    /* calculate ambient color */
    {
        vec3_t light_ambient = uniforms->light_ambient;
        ambient = vec3_new(light_ambient.x * in_diffuse.x,
                           light_ambient.y * in_diffuse.y,
                           light_ambient.z * in_diffuse.z);
    }
    /* calculate diffuse color */
    {
        vec3_t light_diffuse = uniforms->light_diffuse;
        float factor = max_float(-vec3_dot(normal, light), 0.0f);
        diffuse = vec3_new(factor * light_diffuse.x * in_diffuse.x,
                           factor * light_diffuse.y * in_diffuse.y,
                           factor * light_diffuse.z * in_diffuse.z);
    }
    /* calculate specular color */
    {
        vec3_t reflected = gfx_reflect_light(light, normal);
        vec3_t view_pos = varyings->fs_in_view_pos;
        /* in view space, the position of the camera is always at (0,0,0) */
        vec3_t view_dir = vec3_normalize(view_pos);
        float closeness = max_float(-vec3_dot(reflected, view_dir), 0.0f);
        float factor = (float)pow(closeness, uniforms->shininess);
        vec3_t light_specular = uniforms->light_specular;
        specular = vec3_new(
            factor * light_specular.x * in_specular,
            factor * light_specular.y * in_specular,
            factor * light_specular.z * in_specular
        );
    }

    /* using the Phong lighting model */
    color = vec4_new(
        ambient.x + diffuse.x + specular.x,
        ambient.y + diffuse.y + specular.y,
        ambient.z + diffuse.z + specular.z,
        1.0f
    );
    return color;
}

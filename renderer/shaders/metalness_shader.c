#include <assert.h>
#include <stdlib.h>
#include "../core/api.h"
#include "cache_helper.h"
#include "metalness_shader.h"
#include "pbr_helper.h"

/* low-level api */

vec4_t metalness_vertex_shader(void *attribs_, void *varyings_,
                               void *uniforms_) {
    metalness_attribs_t *attribs = (metalness_attribs_t*)attribs_;
    metalness_varyings_t *varyings = (metalness_varyings_t*)varyings_;
    metalness_uniforms_t *uniforms = (metalness_uniforms_t*)uniforms_;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t world_pos = mat4_mul_vec4(uniforms->model_matrix, local_pos);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->viewproj_matrix, world_pos);

    mat3_t tbn_matrix = pbr_build_tbn(attribs->normal, uniforms->normal_matrix,
                                      attribs->tangent, uniforms->model_matrix);

    varyings->position = vec3_from_vec4(world_pos);
    varyings->texcoord = attribs->texcoord;
    varyings->tbn_matrix = tbn_matrix;
    return clip_pos;
}

static vec4_t get_basecolor(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->basecolor_texture) {
        vec4_t sample = texture_sample(uniforms->basecolor_texture, texcoord);
        return vec4_modulate(uniforms->basecolor_factor, sample);
    } else {
        return uniforms->basecolor_factor;
    }
}

static float get_metallic(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->metallic_texture) {
        vec4_t sample = texture_sample(uniforms->metallic_texture, texcoord);
        return uniforms->metallic_factor * sample.x;
    } else {
        return uniforms->metallic_factor;
    }
}

static float get_roughness(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->roughness_texture) {
        vec4_t sample = texture_sample(uniforms->roughness_texture, texcoord);
        return uniforms->roughness_factor * sample.x;
    } else {
        return uniforms->roughness_factor;
    }
}

static float get_occlusion(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->occlusion_texture) {
        vec4_t sample = texture_sample(uniforms->occlusion_texture, texcoord);
        return sample.x;
    } else {
        return 1;
    }
}

static vec3_t get_emission(vec2_t texcoord, metalness_uniforms_t *uniforms) {
    if (uniforms->emissive_texture) {
        vec4_t sample = texture_sample(uniforms->emissive_texture, texcoord);
        return vec3_from_vec4(sample);
    } else {
        return vec3_new(0, 0, 0);
    }
}

vec4_t metalness_fragment_shader(void *varyings_, void *uniforms_) {
    metalness_varyings_t *varyings = (metalness_varyings_t*)varyings_;
    metalness_uniforms_t *uniforms = (metalness_uniforms_t*)uniforms_;

    vec4_t basecolor_ = get_basecolor(varyings->texcoord, uniforms);
    float metallic = get_metallic(varyings->texcoord, uniforms);
    float roughness = get_roughness(varyings->texcoord, uniforms);
    float occlusion = get_occlusion(varyings->texcoord, uniforms);
    vec3_t emission = get_emission(varyings->texcoord, uniforms);
    vec3_t normal = pbr_get_normal(varyings->tbn_matrix, varyings->texcoord,
                                   uniforms->normal_texture);

    vec3_t basecolor = vec3_from_vec4(basecolor_);
    float alpha = basecolor_.w;

    vec3_t dielectric_specular = vec3_new(0.04f, 0.04f, 0.04f);
    vec3_t diffuse_color = vec3_mul(basecolor, (1 - 0.04f) * (1 - metallic));
    vec3_t specular_color = vec3_lerp(dielectric_specular, basecolor, metallic);

    vec3_t world_pos = varyings->position;
    vec3_t camera_pos = uniforms->camera_pos;
    vec3_t light_dir = vec3_negate(uniforms->light_dir);
    vec3_t view_dir = vec3_normalize(vec3_sub(camera_pos, world_pos));

    vec3_t dir_shade = pbr_dir_shade(light_dir, roughness,
                                     normal, view_dir,
                                     diffuse_color, specular_color);
    vec3_t ibl_shade = pbr_ibl_shade(uniforms->shared_ibldata, roughness,
                                     normal, view_dir,
                                     diffuse_color, specular_color);
    vec3_t color = vec3_add(dir_shade, ibl_shade);

    color = vec3_mul(color, occlusion);
    color = vec3_add(color, emission);

    return vec4_from_vec3(pbr_tone_map(color), alpha);
}

/* high-level api */

static metalness_uniforms_t *get_uniforms(model_t *model) {
    return (metalness_uniforms_t*)program_get_uniforms(model->program);
}

static void release_model(model_t *model) {
    metalness_uniforms_t *uniforms = get_uniforms(model);
    if (uniforms->basecolor_texture) {
        cache_release_texture(uniforms->basecolor_texture);
    }
    if (uniforms->metallic_texture) {
        cache_release_texture(uniforms->metallic_texture);
    }
    if (uniforms->roughness_texture) {
        cache_release_texture(uniforms->roughness_texture);
    }
    if (uniforms->normal_texture) {
        cache_release_texture(uniforms->normal_texture);
    }
    if (uniforms->occlusion_texture) {
        cache_release_texture(uniforms->occlusion_texture);
    }
    if (uniforms->emissive_texture) {
        cache_release_texture(uniforms->emissive_texture);
    }
    pbr_release_ibldata(uniforms->shared_ibldata);
    program_release(model->program);
    mesh_release(model->mesh);
    free(model);
}

static void draw_model(model_t *model, framebuffer_t *framebuffer) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    metalness_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = mesh_get_vertex(mesh, i, j);
            attribs = (metalness_attribs_t*)program_get_attribs(program, j);
            attribs->position = vertex.position;
            attribs->texcoord = vertex.texcoord;
            attribs->normal = vertex.normal;
            attribs->tangent = vertex.tangent;
        }
        graphics_draw_triangle(framebuffer, program);
    }
}

model_t *metalness_create_model(
        const char *mesh, mat4_t transform,
        metalness_material_t material, const char *env_name) {
    int sizeof_attribs = sizeof(metalness_attribs_t);
    int sizeof_varyings = sizeof(metalness_varyings_t);
    int sizeof_uniforms = sizeof(metalness_uniforms_t);
    metalness_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    assert(material.metallic_factor >= 0 && material.metallic_factor <= 1);
    assert(material.roughness_factor >= 0 && material.roughness_factor <= 1);

    program = program_create(metalness_vertex_shader, metalness_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             material.double_sided, material.enable_blend);
    uniforms = (metalness_uniforms_t*)program_get_uniforms(program);
    uniforms->basecolor_factor = material.basecolor_factor;
    uniforms->metallic_factor = material.metallic_factor;
    uniforms->roughness_factor = material.roughness_factor;
    if (material.basecolor_texture) {
        const char *basecolor_name = material.basecolor_texture;
        uniforms->basecolor_texture = cache_acquire_texture(basecolor_name, 1);
    }
    if (material.metallic_texture) {
        const char *metallic_name = material.metallic_texture;
        uniforms->metallic_texture = cache_acquire_texture(metallic_name, 0);
    }
    if (material.roughness_texture) {
        const char *roughness_name = material.roughness_texture;
        uniforms->roughness_texture = cache_acquire_texture(roughness_name, 0);
    }
    if (material.normal_texture) {
        const char *normal_name = material.normal_texture;
        uniforms->normal_texture = cache_acquire_texture(normal_name, 0);
    }
    if (material.occlusion_texture) {
        const char *occlusion_name = material.occlusion_texture;
        uniforms->occlusion_texture = cache_acquire_texture(occlusion_name, 0);
    }
    if (material.emissive_texture) {
        const char *emissive_name = material.emissive_texture;
        uniforms->emissive_texture = cache_acquire_texture(emissive_name, 1);
    }
    uniforms->shared_ibldata = pbr_acquire_ibldata(env_name);

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh      = mesh_load(mesh);
    model->transform = transform;
    model->program   = program;
    model->draw      = draw_model;
    model->release   = release_model;
    model->opaque    = !material.enable_blend;

    return model;
}

void metalness_update_uniforms(
        model_t *model, vec3_t light_dir, vec3_t camera_pos,
        mat4_t model_matrix, mat3_t normal_matrix, mat4_t viewproj_matrix) {
    metalness_uniforms_t *uniforms = get_uniforms(model);
    uniforms->light_dir = light_dir;
    uniforms->camera_pos = camera_pos;
    uniforms->model_matrix = model_matrix;
    uniforms->normal_matrix = normal_matrix;
    uniforms->viewproj_matrix = viewproj_matrix;
}

#include <assert.h>
#include <stdlib.h>
#include "../core/api.h"
#include "shader_helper.h"
#include "specular_shader.h"

/* low-level api */

vec4_t specular_vertex_shader(void *attribs_, void *varyings_,
                              void *uniforms_) {
    specular_attribs_t *attribs = (specular_attribs_t*)attribs_;
    specular_varyings_t *varyings = (specular_varyings_t*)varyings_;
    specular_uniforms_t *uniforms = (specular_uniforms_t*)uniforms_;

    mat4_t model_matrix = uniforms->model_matrix;
    mat3_t normal_matrix = uniforms->normal_matrix;

    vec4_t local_pos = vec4_from_vec3(attribs->position, 1);
    vec4_t world_pos = mat4_mul_vec4(model_matrix, local_pos);
    vec4_t clip_pos = mat4_mul_vec4(uniforms->viewproj_matrix, world_pos);

    if (uniforms->normal_texture) {
        mat3_t tbn_matrix = normal_build_tbn(attribs->normal, normal_matrix,
                                             attribs->tangent, model_matrix);
        varyings->tbn_matrix = tbn_matrix;
    } else {
        vec3_t world_normal = mat3_mul_vec3(normal_matrix, attribs->normal);
        varyings->normal = vec3_normalize(world_normal);
    }
    varyings->position = vec3_from_vec4(world_pos);
    varyings->texcoord = attribs->texcoord;
    return clip_pos;
}

static vec4_t get_diffuse(vec2_t texcoord, specular_uniforms_t *uniforms) {
    if (uniforms->diffuse_texture) {
        vec4_t sample = texture_sample(uniforms->diffuse_texture, texcoord);
        return vec4_modulate(uniforms->diffuse_factor, sample);
    } else {
        return uniforms->diffuse_factor;
    }
}

static vec3_t get_specular(vec2_t texcoord, specular_uniforms_t *uniforms) {
    if (uniforms->specular_texture) {
        vec3_t factor = uniforms->specular_factor;
        vec4_t color = texture_sample(uniforms->specular_texture, texcoord);
        return vec3_modulate(factor, vec3_from_vec4(color));
    } else {
        return uniforms->specular_factor;
    }
}

static float get_glossiness(vec2_t texcoord, specular_uniforms_t *uniforms) {
    if (uniforms->glossiness_texture) {
        vec4_t sample = texture_sample(uniforms->glossiness_texture, texcoord);
        return uniforms->glossiness_factor * sample.x;
    } else {
        return uniforms->glossiness_factor;
    }
}

static float get_occlusion(vec2_t texcoord, specular_uniforms_t *uniforms) {
    if (uniforms->occlusion_texture) {
        vec4_t sample = texture_sample(uniforms->occlusion_texture, texcoord);
        return sample.x;
    } else {
        return 1;
    }
}

static vec3_t get_emission(vec2_t texcoord, specular_uniforms_t *uniforms) {
    if (uniforms->emissive_texture) {
        vec4_t sample = texture_sample(uniforms->emissive_texture, texcoord);
        return vec3_from_vec4(sample);
    } else {
        return vec3_new(0, 0, 0);
    }
}

static vec3_t get_normal(specular_varyings_t *varyings,
                         specular_uniforms_t *uniforms) {
    if (uniforms->normal_texture) {
        return normal_from_texture(varyings->tbn_matrix,
                                   varyings->texcoord,
                                   uniforms->normal_texture);
    } else {
        return vec3_normalize(varyings->normal);
    }
}

static float max_component(vec3_t v) {
    return v.x > v.y && v.x > v.z ? v.x : (v.y > v.z ? v.y : v.z);
}

vec4_t specular_fragment_shader(void *varyings_, void *uniforms_,
                                int *discard) {
    specular_varyings_t *varyings = (specular_varyings_t*)varyings_;
    specular_uniforms_t *uniforms = (specular_uniforms_t*)uniforms_;

    vec4_t diffuse_ = get_diffuse(varyings->texcoord, uniforms);
    vec3_t specular = get_specular(varyings->texcoord, uniforms);
    float glossiness = get_glossiness(varyings->texcoord, uniforms);
    float occlusion = get_occlusion(varyings->texcoord, uniforms);
    vec3_t emission = get_emission(varyings->texcoord, uniforms);
    vec3_t normal = get_normal(varyings, uniforms);

    vec3_t diffuse = vec3_from_vec4(diffuse_);
    float alpha = diffuse_.w;

    vec3_t diffuse_color = vec3_mul(diffuse, 1 - max_component(specular));
    vec3_t specular_color = specular;
    float roughness = 1 - glossiness;

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

    if (uniforms->alpha_cutoff > 0 && alpha < uniforms->alpha_cutoff) {
        *discard = 1;
        return vec4_new(0, 0, 0, 0);
    }

    return vec4_from_vec3(pbr_tone_map(color), alpha);
}

/* high-level api */

static void draw_model(model_t *model, framebuffer_t *framebuffer) {
    program_t *program = model->program;
    mesh_t *mesh = model->mesh;
    int num_faces = mesh_get_num_faces(mesh);
    specular_attribs_t *attribs;
    int i, j;

    for (i = 0; i < num_faces; i++) {
        for (j = 0; j < 3; j++) {
            vertex_t vertex = mesh_get_vertex(mesh, i, j);
            attribs = (specular_attribs_t*)program_get_attribs(program, j);
            attribs->position = vertex.position;
            attribs->texcoord = vertex.texcoord;
            attribs->normal = vertex.normal;
            attribs->tangent = vertex.tangent;
        }
        graphics_draw_triangle(framebuffer, program);
    }
}

static void release_model(model_t *model) {
    specular_uniforms_t *uniforms;
    uniforms = (specular_uniforms_t*)program_get_uniforms(model->program);
    cache_release_texture(uniforms->diffuse_texture);
    cache_release_texture(uniforms->specular_texture);
    cache_release_texture(uniforms->glossiness_texture);
    cache_release_texture(uniforms->normal_texture);
    cache_release_texture(uniforms->occlusion_texture);
    cache_release_texture(uniforms->emissive_texture);
    cache_release_ibldata(uniforms->shared_ibldata);
    program_release(model->program);
    cache_release_mesh(model->mesh);
    free(model);
}

model_t *specular_create_model(
        const char *mesh, mat4_t transform,
        specular_material_t material, const char *env_name) {
    int sizeof_attribs = sizeof(specular_attribs_t);
    int sizeof_varyings = sizeof(specular_varyings_t);
    int sizeof_uniforms = sizeof(specular_uniforms_t);
    specular_uniforms_t *uniforms;
    program_t *program;
    model_t *model;

    assert(material.glossiness_factor >= 0 && material.glossiness_factor <= 1);

    program = program_create(specular_vertex_shader, specular_fragment_shader,
                             sizeof_attribs, sizeof_varyings, sizeof_uniforms,
                             material.double_sided, material.enable_blend);

    uniforms = (specular_uniforms_t*)program_get_uniforms(program);
    uniforms->diffuse_factor = material.diffuse_factor;
    uniforms->specular_factor = material.specular_factor;
    uniforms->glossiness_factor = material.glossiness_factor;
    uniforms->alpha_cutoff = material.alpha_cutoff;
    uniforms->diffuse_texture = cache_acquire_texture(
            material.diffuse_texture, 1);
    uniforms->specular_texture = cache_acquire_texture(
            material.specular_texture, 1);
    uniforms->glossiness_texture = cache_acquire_texture(
            material.glossiness_texture, 0);
    uniforms->normal_texture = cache_acquire_texture(
            material.normal_texture, 0);
    uniforms->occlusion_texture = cache_acquire_texture(
            material.occlusion_texture, 0);
    uniforms->emissive_texture = cache_acquire_texture(
            material.emissive_texture, 1);
    uniforms->shared_ibldata = cache_acquire_ibldata(env_name);

    model = (model_t*)malloc(sizeof(model_t));
    model->mesh      = cache_acquire_mesh(mesh);
    model->transform = transform;
    model->program   = program;
    model->draw      = draw_model;
    model->release   = release_model;
    model->opaque    = !material.enable_blend;

    return model;
}

void specular_update_uniforms(
        model_t *model, vec3_t light_dir, vec3_t camera_pos,
        mat4_t model_matrix, mat3_t normal_matrix, mat4_t viewproj_matrix) {
    specular_uniforms_t *uniforms;
    uniforms = (specular_uniforms_t*)program_get_uniforms(model->program);
    uniforms->light_dir = light_dir;
    uniforms->camera_pos = camera_pos;
    uniforms->model_matrix = model_matrix;
    uniforms->normal_matrix = normal_matrix;
    uniforms->viewproj_matrix = viewproj_matrix;
}

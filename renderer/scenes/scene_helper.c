#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "../core/api.h"
#include "../shaders/blinn_shader.h"
#include "../shaders/pbr_shader.h"
#include "../shaders/skybox_shader.h"
#include "scene_helper.h"

typedef struct {
    vec3_t background;
    char skybox[128];
    char shadow[128];
    float ambient;
    float punctual;
    char environment[128];
} scene_light_t;

typedef struct {
    vec4_t basecolor;
    float shininess;
    char diffuse_map[128];
    char specular_map[128];
    char emission_map[128];
    int double_sided;
    int enable_blend;
    float alpha_cutoff;
} scene_blinn_t;

typedef struct {
    vec4_t basecolor_factor;
    float metalness_factor;
    float roughness_factor;
    char basecolor_map[128];
    char metalness_map[128];
    char roughness_map[128];
    char normal_map[128];
    char occlusion_map[128];
    char emission_map[128];
    int double_sided;
    int enable_blend;
    float alpha_cutoff;
} scene_pbrm_t;

typedef struct {
    vec4_t diffuse_factor;
    vec3_t specular_factor;
    float glossiness_factor;
    char diffuse_map[128];
    char specular_map[128];
    char glossiness_map[128];
    char normal_map[128];
    char occlusion_map[128];
    char emission_map[128];
    int double_sided;
    int enable_blend;
    float alpha_cutoff;
} scene_pbrs_t;

typedef struct {
    char mesh[128];
    char skeleton[128];
    int attached;
    int material;
    int transform;
} scene_model_t;

static void read_line(FILE *file, char line[LINE_LENGTH]) {
    if (fgets(line, LINE_LENGTH, file) == NULL) {
        assert(0);
    }
}

static int starts_with(const char *string, const char *prefix) {
    return strncmp(string, prefix, strlen(prefix)) == 0;
}

static int equals_to(const char *string1, const char *string2) {
    return strcmp(string1, string2) == 0;
}

static const char *wrap_string(const char *string) {
    if (equals_to(string, "off")) {
        return NULL;
    } else {
        return string;
    }
}

static void read_blinn_type(FILE *file) {
    char line[LINE_LENGTH];
    read_line(file, line);
    assert(starts_with(line, "type: blinn"));
}

static void read_pbrm_type(FILE *file) {
    char line[LINE_LENGTH];
    read_line(file, line);
    assert(starts_with(line, "type: pbrm"));
}

static void read_pbrs_type(FILE *file) {
    char line[LINE_LENGTH];
    read_line(file, line);
    assert(starts_with(line, "type: pbrs"));
}

static scene_light_t read_light(FILE *file) {
    char line[LINE_LENGTH];
    scene_light_t light;
    int items;

    read_line(file, line);
    assert(starts_with(line, "lighting:"));
    items = fscanf(file, " background: %f %f %f",
                   &light.background.x,
                   &light.background.y,
                   &light.background.z);
    assert(items == 3);
    items = fscanf(file, " skybox: %s", light.skybox);
    assert(items == 1);
    items = fscanf(file, " shadow: %s", light.shadow);
    assert(items == 1);
    items = fscanf(file, " ambient: %f", &light.ambient);
    assert(items == 1);
    items = fscanf(file, " punctual: %f", &light.punctual);
    assert(items == 1);
    items = fscanf(file, " environment: %s", light.environment);
    assert(items == 1);

    return light;
}

static scene_blinn_t read_blinn_material(FILE *file) {
    scene_blinn_t material;
    int items;
    int dummy;

    items = fscanf(file, " material %d:", &dummy);
    assert(items == 1);
    items = fscanf(file, " basecolor: %f %f %f %f",
                   &material.basecolor.x,
                   &material.basecolor.y,
                   &material.basecolor.z,
                   &material.basecolor.w);
    assert(items == 4);
    items = fscanf(file, " shininess: %f", &material.shininess);
    assert(items == 1);
    items = fscanf(file, " diffuse_map: %s", material.diffuse_map);
    assert(items == 1);
    items = fscanf(file, " specular_map: %s", material.specular_map);
    assert(items == 1);
    items = fscanf(file, " emission_map: %s", material.emission_map);
    assert(items == 1);
    items = fscanf(file, " double_sided: %d", &material.double_sided);
    assert(items == 1);
    items = fscanf(file, " enable_blend: %d", &material.enable_blend);
    assert(items == 1);
    items = fscanf(file, " alpha_cutoff: %f", &material.alpha_cutoff);
    assert(items == 1);

    return material;
}

static scene_blinn_t *read_blinn_materials(FILE *file) {
    scene_blinn_t *materials = NULL;
    int num_materials;
    int items;
    int i;

    items = fscanf(file, " materials %d:", &num_materials);
    assert(items == 1);
    for (i = 0; i < num_materials; i++) {
        scene_blinn_t material = read_blinn_material(file);
        darray_push(materials, material);
    }
    return materials;
}

static scene_pbrm_t read_pbrm_material(FILE *file) {
    scene_pbrm_t material;
    int items;
    int dummy;

    items = fscanf(file, " material %d:", &dummy);
    assert(items == 1);
    items = fscanf(file, " basecolor_factor: %f %f %f %f",
                   &material.basecolor_factor.x,
                   &material.basecolor_factor.y,
                   &material.basecolor_factor.z,
                   &material.basecolor_factor.w);
    assert(items == 4);
    items = fscanf(file, " metalness_factor: %f", &material.metalness_factor);
    assert(items == 1);
    items = fscanf(file, " roughness_factor: %f", &material.roughness_factor);
    assert(items == 1);
    items = fscanf(file, " basecolor_map: %s", material.basecolor_map);
    assert(items == 1);
    items = fscanf(file, " metalness_map: %s", material.metalness_map);
    assert(items == 1);
    items = fscanf(file, " roughness_map: %s", material.roughness_map);
    assert(items == 1);
    items = fscanf(file, " normal_map: %s", material.normal_map);
    assert(items == 1);
    items = fscanf(file, " occlusion_map: %s", material.occlusion_map);
    assert(items == 1);
    items = fscanf(file, " emission_map: %s", material.emission_map);
    assert(items == 1);
    items = fscanf(file, " double_sided: %d", &material.double_sided);
    assert(items == 1);
    items = fscanf(file, " enable_blend: %d", &material.enable_blend);
    assert(items == 1);
    items = fscanf(file, " alpha_cutoff: %f", &material.alpha_cutoff);
    assert(items == 1);

    return material;
}

static scene_pbrm_t *read_pbrm_materials(FILE *file) {
    scene_pbrm_t *materials = NULL;
    int num_materials;
    int items;
    int i;

    items = fscanf(file, " materials %d:", &num_materials);
    assert(items == 1);
    for (i = 0; i < num_materials; i++) {
        scene_pbrm_t material = read_pbrm_material(file);
        darray_push(materials, material);
    }
    return materials;
}

static scene_pbrs_t read_pbrs_material(FILE *file) {
    scene_pbrs_t material;
    int items;
    int dummy;

    items = fscanf(file, " material %d:", &dummy);
    assert(items == 1);
    items = fscanf(file, " diffuse_factor: %f %f %f %f",
                   &material.diffuse_factor.x,
                   &material.diffuse_factor.y,
                   &material.diffuse_factor.z,
                   &material.diffuse_factor.w);
    assert(items == 4);
    items = fscanf(file, " specular_factor: %f %f %f",
                   &material.specular_factor.x,
                   &material.specular_factor.y,
                   &material.specular_factor.z);
    assert(items == 3);
    items = fscanf(file, " glossiness_factor: %f", &material.glossiness_factor);
    assert(items == 1);
    items = fscanf(file, " diffuse_map: %s", material.diffuse_map);
    assert(items == 1);
    items = fscanf(file, " specular_map: %s", material.specular_map);
    assert(items == 1);
    items = fscanf(file, " glossiness_map: %s", material.glossiness_map);
    assert(items == 1);
    items = fscanf(file, " normal_map: %s", material.normal_map);
    assert(items == 1);
    items = fscanf(file, " occlusion_map: %s", material.occlusion_map);
    assert(items == 1);
    items = fscanf(file, " emission_map: %s", material.emission_map);
    assert(items == 1);
    items = fscanf(file, " double_sided: %d", &material.double_sided);
    assert(items == 1);
    items = fscanf(file, " enable_blend: %d", &material.enable_blend);
    assert(items == 1);
    items = fscanf(file, " alpha_cutoff: %f", &material.alpha_cutoff);
    assert(items == 1);

    return material;
}

static scene_pbrs_t *read_pbrs_materials(FILE *file) {
    scene_pbrs_t *materials = NULL;
    int num_materials;
    int items;
    int i;

    items = fscanf(file, " materials %d:", &num_materials);
    assert(items == 1);
    for (i = 0; i < num_materials; i++) {
        scene_pbrs_t material = read_pbrs_material(file);
        darray_push(materials, material);
    }
    return materials;
}

static mat4_t read_transform(FILE *file) {
    mat4_t transform;
    int items;
    int dummy;
    int i;

    items = fscanf(file, " transform %d:", &dummy);
    assert(items == 1);
    for (i = 0; i < 4; i++) {
        items = fscanf(file, " %f %f %f %f",
                      &transform.m[i][0],
                      &transform.m[i][1],
                      &transform.m[i][2],
                      &transform.m[i][3]);
        assert(items == 4);
    }

    return transform;
}

static mat4_t *read_transforms(FILE *file) {
    mat4_t *transforms = NULL;
    int num_transforms;
    int items;
    int i;

    items = fscanf(file, " transforms %d:", &num_transforms);
    assert(items == 1);
    for (i = 0; i < num_transforms; i++) {
        mat4_t transform = read_transform(file);
        darray_push(transforms, transform);
    }
    return transforms;
}

static scene_model_t read_model(FILE *file) {
    scene_model_t model;
    int items;
    int dummy;

    items = fscanf(file, " model %d:", &dummy);
    assert(items == 1);
    items = fscanf(file, " mesh: %s", model.mesh);
    assert(items == 1);
    items = fscanf(file, " skeleton: %s", model.skeleton);
    assert(items == 1);
    items = fscanf(file, " attached: %d", &model.attached);
    assert(items == 1);
    items = fscanf(file, " material: %d", &model.material);
    assert(items == 1);
    items = fscanf(file, " transform: %d", &model.transform);
    assert(items == 1);

    return model;
}

static scene_model_t *read_models(FILE *file) {
    scene_model_t *models = NULL;
    int num_models;
    int items;
    int i;

    items = fscanf(file, " models %d:", &num_models);
    assert(items == 1);
    for (i = 0; i < num_models; i++) {
        scene_model_t model = read_model(file);
        darray_push(models, model);
    }
    return models;
}

static scene_t *create_scene(scene_light_t light, model_t **models) {
    model_t *skybox;
    int with_shadow;
    if (equals_to(light.skybox, "off")) {
        skybox = NULL;
    } else {
        skybox = skybox_create_model(light.skybox);
    }
    if (equals_to(light.shadow, "off")) {
        with_shadow = 0;
    } else {
        assert(equals_to(light.shadow, "on"));
        with_shadow = 1;
    }
    return scene_create(light.background, skybox, models,
                        light.ambient, light.punctual, with_shadow);
}

scene_t *helper_load_blinn_scene(const char *filename, mat4_t root_transform) {
    char line[LINE_LENGTH];
    scene_light_t scene_light;
    scene_blinn_t *scene_materials;
    mat4_t *scene_transforms;
    scene_model_t *scene_models;
    int num_materials;
    int num_transforms;
    int num_models;
    model_t **models = NULL;
    scene_t *scene;
    FILE *file;
    int i;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_blinn_type(file);
    read_line(file, line);
    scene_light = read_light(file);
    read_line(file, line);
    scene_materials = read_blinn_materials(file);
    read_line(file, line);
    scene_transforms = read_transforms(file);
    read_line(file, line);
    scene_models = read_models(file);
    fclose(file);

    num_materials = darray_size(scene_materials);
    num_transforms = darray_size(scene_transforms);
    num_models = darray_size(scene_models);
    for (i = 0; i < num_models; i++) {
        scene_model_t scene_model = scene_models[i];
        scene_blinn_t scene_material;
        blinn_material_t material;
        const char *skeleton;
        mat4_t transform;
        model_t *model;
        int node_index;

        assert(scene_model.transform < num_transforms);
        transform = scene_transforms[scene_model.transform];
        transform = mat4_mul_mat4(root_transform, transform);

        assert(scene_model.material < num_materials);
        scene_material = scene_materials[scene_model.material];
        material.basecolor = scene_material.basecolor;
        material.shininess = scene_material.shininess;
        material.diffuse_map = wrap_string(scene_material.diffuse_map);
        material.specular_map = wrap_string(scene_material.specular_map);
        material.emission_map = wrap_string(scene_material.emission_map);
        material.double_sided = scene_material.double_sided;
        material.enable_blend = scene_material.enable_blend;
        material.alpha_cutoff = scene_material.alpha_cutoff;

        skeleton = wrap_string(scene_model.skeleton);
        node_index = scene_model.attached;

        model = blinn_create_model(scene_model.mesh, skeleton, node_index,
                                   transform, material);
        darray_push(models, model);
    }

    scene = create_scene(scene_light, models);
    darray_free(scene_materials);
    darray_free(scene_transforms);
    darray_free(scene_models);

    return scene;
}

scene_t *helper_load_pbrm_scene(const char *filename, mat4_t root_transform) {
    char line[LINE_LENGTH];
    scene_light_t scene_light;
    scene_pbrm_t *scene_materials;
    mat4_t *scene_transforms;
    scene_model_t *scene_models;
    int num_materials;
    int num_transforms;
    int num_models;
    const char *env_name;
    model_t **models = NULL;
    scene_t *scene;
    FILE *file;
    int i;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_pbrm_type(file);
    read_line(file, line);
    scene_light = read_light(file);
    read_line(file, line);
    scene_materials = read_pbrm_materials(file);
    read_line(file, line);
    scene_transforms = read_transforms(file);
    read_line(file, line);
    scene_models = read_models(file);
    fclose(file);

    env_name = wrap_string(scene_light.environment);
    num_materials = darray_size(scene_materials);
    num_transforms = darray_size(scene_transforms);
    num_models = darray_size(scene_models);
    for (i = 0; i < num_models; i++) {
        scene_model_t scene_model = scene_models[i];
        scene_pbrm_t scene_material;
        pbrm_material_t material;
        const char *skeleton;
        mat4_t transform;
        model_t *model;
        int node_index;

        assert(scene_model.transform < num_transforms);
        transform = scene_transforms[scene_model.transform];
        transform = mat4_mul_mat4(root_transform, transform);

        assert(scene_model.material < num_materials);
        scene_material = scene_materials[scene_model.material];
        material.basecolor_factor = scene_material.basecolor_factor;
        material.metalness_factor = scene_material.metalness_factor;
        material.roughness_factor = scene_material.roughness_factor;
        material.basecolor_map = wrap_string(scene_material.basecolor_map);
        material.metalness_map = wrap_string(scene_material.metalness_map);
        material.roughness_map = wrap_string(scene_material.roughness_map);
        material.normal_map = wrap_string(scene_material.normal_map);
        material.occlusion_map = wrap_string(scene_material.occlusion_map);
        material.emission_map = wrap_string(scene_material.emission_map);
        material.double_sided = scene_material.double_sided;
        material.enable_blend = scene_material.enable_blend;
        material.alpha_cutoff = scene_material.alpha_cutoff;

        skeleton = wrap_string(scene_model.skeleton);
        node_index = scene_model.attached;

        model = pbrm_create_model(scene_model.mesh, skeleton, node_index,
                                   transform, material, env_name);
        darray_push(models, model);
    }

    scene = create_scene(scene_light, models);
    darray_free(scene_materials);
    darray_free(scene_transforms);
    darray_free(scene_models);

    return scene;
}

scene_t *helper_load_pbrs_scene(const char *filename, mat4_t root_transform) {
    char line[LINE_LENGTH];
    scene_light_t scene_light;
    scene_pbrs_t *scene_materials;
    mat4_t *scene_transforms;
    scene_model_t *scene_models;
    int num_materials;
    int num_transforms;
    int num_models;
    const char *env_name;
    model_t **models = NULL;
    scene_t *scene;
    FILE *file;
    int i;

    file = fopen(filename, "rb");
    assert(file != NULL);
    read_pbrs_type(file);
    read_line(file, line);
    scene_light = read_light(file);
    read_line(file, line);
    scene_materials = read_pbrs_materials(file);
    read_line(file, line);
    scene_transforms = read_transforms(file);
    read_line(file, line);
    scene_models = read_models(file);
    fclose(file);

    env_name = wrap_string(scene_light.environment);
    num_materials = darray_size(scene_materials);
    num_transforms = darray_size(scene_transforms);
    num_models = darray_size(scene_models);
    for (i = 0; i < num_models; i++) {
        scene_model_t scene_model = scene_models[i];
        scene_pbrs_t scene_material;
        pbrs_material_t material;
        const char *skeleton;
        mat4_t transform;
        model_t *model;
        int node_index;

        assert(scene_model.transform < num_transforms);
        transform = scene_transforms[scene_model.transform];
        transform = mat4_mul_mat4(root_transform, transform);

        assert(scene_model.material < num_materials);
        scene_material = scene_materials[scene_model.material];
        material.diffuse_factor = scene_material.diffuse_factor;
        material.specular_factor = scene_material.specular_factor;
        material.glossiness_factor = scene_material.glossiness_factor;
        material.diffuse_map = wrap_string(scene_material.diffuse_map);
        material.specular_map = wrap_string(scene_material.specular_map);
        material.glossiness_map = wrap_string(scene_material.glossiness_map);
        material.normal_map = wrap_string(scene_material.normal_map);
        material.occlusion_map = wrap_string(scene_material.occlusion_map);
        material.emission_map = wrap_string(scene_material.emission_map);
        material.double_sided = scene_material.double_sided;
        material.enable_blend = scene_material.enable_blend;
        material.alpha_cutoff = scene_material.alpha_cutoff;

        skeleton = wrap_string(scene_model.skeleton);
        node_index = scene_model.attached;

        model = pbrs_create_model(scene_model.mesh, skeleton, node_index,
                                   transform, material, env_name);
        darray_push(models, model);
    }

    scene = create_scene(scene_light, models);
    darray_free(scene_materials);
    darray_free(scene_transforms);
    darray_free(scene_models);

    return scene;
}

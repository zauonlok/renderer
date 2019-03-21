#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../core/api.h"
#include "cache_helper.h"
#include "pbr_shader.h"

static char *duplicate_string(const char *source) {
    char *target = (char*)malloc(strlen(source) + 1);
    strcpy(target, source);
    return target;
}

/* mesh related functions */

typedef struct {
    char *filename;
    mesh_t *mesh;
    int references;
} cached_mesh_t;

static cached_mesh_t *g_meshes = NULL;

mesh_t *cache_acquire_mesh(const char *filename) {
    if (filename != NULL) {
        cached_mesh_t cached_mesh;
        int num_meshes = darray_size(g_meshes);
        int i;

        for (i = 0; i < num_meshes; i++) {
            if (strcmp(g_meshes[i].filename, filename) == 0) {
                if (g_meshes[i].references > 0) {
                    g_meshes[i].references += 1;
                } else {
                    assert(g_meshes[i].references == 0);
                    assert(g_meshes[i].mesh == NULL);
                    g_meshes[i].mesh = mesh_load(filename);
                    g_meshes[i].references = 1;
                }
                return g_meshes[i].mesh;
            }
        }

        cached_mesh.filename = duplicate_string(filename);
        cached_mesh.mesh = mesh_load(filename);
        cached_mesh.references = 1;
        darray_push(g_meshes, cached_mesh);
        return cached_mesh.mesh;
    } else {
        return NULL;
    }
}

void cache_release_mesh(mesh_t *mesh) {
    if (mesh != NULL) {
        int num_meshes = darray_size(g_meshes);
        int i;
        for (i = 0; i < num_meshes; i++) {
            if (g_meshes[i].mesh == mesh) {
                assert(g_meshes[i].references > 0);
                g_meshes[i].references -= 1;
                if (g_meshes[i].references == 0) {
                    mesh_release(g_meshes[i].mesh);
                    g_meshes[i].mesh = NULL;
                }
                return;
            }
        }
        assert(0);
    }
}

/* skeleton related functions */

typedef struct {
    char *filename;
    skeleton_t *skeleton;
    int references;
} cached_skeleton_t;

static cached_skeleton_t *g_skeletons = NULL;

skeleton_t *cache_acquire_skeleton(const char *filename) {
    if (filename != NULL) {
        cached_skeleton_t cached_skeleton;
        int num_skeletons = darray_size(g_skeletons);
        int i;

        for (i = 0; i < num_skeletons; i++) {
            if (strcmp(g_skeletons[i].filename, filename) == 0) {
                if (g_skeletons[i].references > 0) {
                    g_skeletons[i].references += 1;
                } else {
                    assert(g_skeletons[i].references == 0);
                    assert(g_skeletons[i].skeleton == NULL);
                    g_skeletons[i].skeleton = skeleton_load(filename);
                    g_skeletons[i].references = 1;
                }
                return g_skeletons[i].skeleton;
            }
        }

        cached_skeleton.filename = duplicate_string(filename);
        cached_skeleton.skeleton = skeleton_load(filename);
        cached_skeleton.references = 1;
        darray_push(g_skeletons, cached_skeleton);
        return cached_skeleton.skeleton;
    } else {
        return NULL;
    }
}

void cache_release_skeleton(skeleton_t *skeleton) {
    if (skeleton != NULL) {
        int num_skeletons = darray_size(g_skeletons);
        int i;
        for (i = 0; i < num_skeletons; i++) {
            if (g_skeletons[i].skeleton == skeleton) {
                assert(g_skeletons[i].references > 0);
                g_skeletons[i].references -= 1;
                if (g_skeletons[i].references == 0) {
                    skeleton_release(g_skeletons[i].skeleton);
                    g_skeletons[i].skeleton = NULL;
                }
                return;
            }
        }
        assert(0);
    }
}

/* texture related functions */

typedef struct {
    char *filename;
    int srgb2linear;
    texture_t *texture;
    int references;
} cached_texture_t;

static cached_texture_t *g_textures = NULL;

texture_t *cache_acquire_texture(const char *filename, int srgb2linear) {
    if (filename != NULL) {
        cached_texture_t cached_texture;
        int num_textures = darray_size(g_textures);
        int i;

        assert(srgb2linear == 0 || srgb2linear == 1);

        for (i = 0; i < num_textures; i++) {
            if (strcmp(g_textures[i].filename, filename) == 0) {
                if (g_textures[i].srgb2linear == srgb2linear) {
                    if (g_textures[i].references > 0) {
                        g_textures[i].references += 1;
                    } else {
                        assert(g_textures[i].references == 0);
                        assert(g_textures[i].texture == NULL);
                        g_textures[i].texture = texture_from_file(filename);
                        if (srgb2linear) {
                            texture_srgb2linear(g_textures[i].texture);
                        }
                        g_textures[i].references = 1;
                    }
                    return g_textures[i].texture;
                }
            }
        }

        cached_texture.filename = duplicate_string(filename);
        cached_texture.srgb2linear = srgb2linear;
        cached_texture.texture = texture_from_file(filename);
        if (srgb2linear) {
            texture_srgb2linear(cached_texture.texture);
        }
        cached_texture.references = 1;
        darray_push(g_textures, cached_texture);
        return cached_texture.texture;
    } else {
        return NULL;
    }
}

void cache_release_texture(texture_t *texture) {
    if (texture != NULL) {
        int num_textures = darray_size(g_textures);
        int i;

        for (i = 0; i < num_textures; i++) {
            if (g_textures[i].texture == texture) {
                assert(g_textures[i].references > 0);
                g_textures[i].references -= 1;
                if (g_textures[i].references == 0) {
                    texture_release(g_textures[i].texture);
                    g_textures[i].texture = NULL;
                }
                return;
            }
        }
        assert(0);
    }
}

/* skybox related functions */

typedef struct {
    const char *skybox_name;
    cubemap_t *skybox;
    int references;
} skybox_t;

static skybox_t g_skyboxes[] = {
    {"papermill", NULL, 0},
};

static cubemap_t *load_skybox(const char *skybox_name) {
    const char *faces[6] = {"right", "left", "top", "bottom", "front", "back"};
    const char *format = "assets/common/%s/skybox_%s.tga";
    char paths[6][128];
    cubemap_t *skybox;
    int i;

    for (i = 0; i < 6; i++) {
        sprintf(paths[i], format, skybox_name, faces[i]);
    }
    skybox = cubemap_from_files(paths[0], paths[1], paths[2],
                                paths[3], paths[4], paths[5]);

    return skybox;
}

static void free_skybox(cubemap_t *skybox) {
    cubemap_release(skybox);
}

cubemap_t *cache_acquire_skybox(const char *skybox_name) {
    if (skybox_name != NULL) {
        int num_skyboxes = ARRAY_SIZE(g_skyboxes);
        int i;
        for (i = 0; i < num_skyboxes; i++) {
            if (strcmp(g_skyboxes[i].skybox_name, skybox_name) == 0) {
                if (g_skyboxes[i].references > 0) {
                    g_skyboxes[i].references += 1;
                } else {
                    assert(g_skyboxes[i].skybox == NULL);
                    assert(g_skyboxes[i].references == 0);
                    g_skyboxes[i].skybox = load_skybox(skybox_name);
                    g_skyboxes[i].references = 1;
                }
                return g_skyboxes[i].skybox;
            }
        }
        assert(0);
        return NULL;
    } else {
        return NULL;
    }
}

void cache_release_skybox(cubemap_t *skybox) {
    if (skybox != NULL) {
        int num_skyboxes = ARRAY_SIZE(g_skyboxes);
        int i;
        for (i = 0; i < num_skyboxes; i++) {
            if (g_skyboxes[i].skybox == skybox) {
                assert(g_skyboxes[i].references > 0);
                g_skyboxes[i].references -= 1;
                if (g_skyboxes[i].references == 0) {
                    free_skybox(g_skyboxes[i].skybox);
                    g_skyboxes[i].skybox = NULL;
                }
                return;
            }
        }
        assert(0);
    }
}

/* ibldata related functions */

typedef struct {
    const char *env_name;
    int mip_level;
    ibldata_t *ibldata;
    int references;
} envinfo_t;

static envinfo_t g_envinfo[] = {
    {"papermill", 10, NULL, 0},
};

static ibldata_t *load_ibldata(const char *env_name, int mip_level) {
    const char *faces[6] = {"right", "left", "top", "bottom", "front", "back"};
    const char *format = "assets/common/%s/%s_%s_%d.tga";
    char paths[6][128];
    ibldata_t *ibldata;
    int i, j;

    ibldata = (ibldata_t*)malloc(sizeof(ibldata_t));
    ibldata->mip_level = mip_level;

    /* diffuse environment map */
    for (j = 0; j < 6; j++) {
        sprintf(paths[j], format, env_name, "diffuse", faces[j], 0);
    }
    ibldata->diffuse_map = cubemap_from_files(paths[0], paths[1], paths[2],
                                              paths[3], paths[4], paths[5]);
    cubemap_srgb2linear(ibldata->diffuse_map);

    /* specular environment maps */
    for (i = 0; i < mip_level; i++) {
        cubemap_t *specular_map;
        for (j = 0; j < 6; j++) {
            sprintf(paths[j], format, env_name, "specular", faces[j], i);
        }
        specular_map = cubemap_from_files(paths[0], paths[1], paths[2],
                                          paths[3], paths[4], paths[5]);
        cubemap_srgb2linear(specular_map);
        ibldata->specular_maps[i] = specular_map;
    }

    /* brdf lookup table */
    ibldata->brdf_lut = cache_acquire_texture("assets/common/brdf_lut.tga", 0);

    return ibldata;
}

static void free_ibldata(ibldata_t *ibldata) {
    int i;
    cubemap_release(ibldata->diffuse_map);
    for (i = 0; i < ibldata->mip_level; i++) {
        cubemap_release(ibldata->specular_maps[i]);
    }
    cache_release_texture(ibldata->brdf_lut);
    free(ibldata);
}

ibldata_t *cache_acquire_ibldata(const char *env_name) {
    if (env_name != NULL) {
        int num_envinfo = ARRAY_SIZE(g_envinfo);
        int i;
        for (i = 0; i < num_envinfo; i++) {
            if (strcmp(g_envinfo[i].env_name, env_name) == 0) {
                if (g_envinfo[i].references > 0) {
                    g_envinfo[i].references += 1;
                } else {
                    int mip_level = g_envinfo[i].mip_level;
                    assert(g_envinfo[i].ibldata == NULL);
                    assert(g_envinfo[i].references == 0);
                    g_envinfo[i].ibldata = load_ibldata(env_name, mip_level);
                    g_envinfo[i].references = 1;
                }
                return g_envinfo[i].ibldata;
            }
        }
        assert(0);
        return NULL;
    } else {
        return NULL;
    }
}

void cache_release_ibldata(ibldata_t *ibldata) {
    if (ibldata != NULL) {
        int num_envinfo = ARRAY_SIZE(g_envinfo);
        int i;
        for (i = 0; i < num_envinfo; i++) {
            if (g_envinfo[i].ibldata == ibldata) {
                assert(g_envinfo[i].references > 0);
                g_envinfo[i].references -= 1;
                if (g_envinfo[i].references == 0) {
                    free_ibldata(g_envinfo[i].ibldata);
                    g_envinfo[i].ibldata = NULL;
                }
                return;
            }
        }
        assert(0);
    }
}

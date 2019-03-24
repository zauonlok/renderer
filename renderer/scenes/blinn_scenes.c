#include <assert.h>
#include <stdio.h>
#include "../core/api.h"
#include "../shaders/blinn_shader.h"
#include "blinn_scenes.h"

scene_t *blinn_centaur_scene(void) {
    const char *meshes[] = {
        "assets/centaur/body.obj",
        "assets/centaur/flame.obj",
        "assets/centaur/gas.obj",
    };
    blinn_material_t materials[] = {
        {
            {1, 1, 1, 1}, 32,
            "assets/centaur/body_diffuse.tga",
            "assets/centaur/body_specular.tga",
            "assets/centaur/body_emission.tga",
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 32,
            "assets/centaur/flame_diffuse.tga",
            NULL,
            "assets/centaur/flame_emission.tga",
            0, 1, 0,
        },
        {
            {1, 1, 1, 1}, 32,
            "assets/centaur/gas_diffuse.tga",
            "assets/centaur/gas_specular.tga",
            NULL,
            0, 0, 0,
        },
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    model_t **models = NULL;
    model_t *model;
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    translation = mat4_translate(0.154f, -7.579f, -30.749f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    rotation = mat4_mul_mat4(mat4_rotate_y(TO_RADIANS(-90)), rotation);
    scale = mat4_scale(0.016f, 0.016f, 0.016f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        model = blinn_create_model(meshes[i], NULL, root, materials[i]);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 0.5f, 1, 0);
}

scene_t *blinn_craftsman_scene(void) {
    const char *meshes[] = {
        "assets/craftsman/anvil.obj",
        "assets/craftsman/floor.obj",
        "assets/craftsman/hammer.obj",
        "assets/craftsman/hotiron.obj",
        "assets/craftsman/shoulderpad0.obj",
        "assets/craftsman/shoulderpad1.obj",
        "assets/craftsman/smith.obj",
    };
    blinn_material_t materials[] = {
        {
            {1, 1, 1, 1}, 32,
            "assets/craftsman/anvil_diffuse.tga",
            NULL,
            NULL,
            0, 0, 0,
        },
        {
            {1, 1, 1, 1}, 32,
            "assets/craftsman/floor_diffuse.tga",
            NULL,
            NULL,
            1, 0, 1,
        },
        {
            {1, 1, 1, 1}, 32,
            "assets/craftsman/smith_diffuse.tga",
            NULL,
            "assets/craftsman/smith_emission.tga",
            0, 0, 1,
        },
    };
    mat4_t transforms[] = {
        {{
            {  0.936571f,   0.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   0.936571f,   0.000000f},
            {  0.000000f,  -0.936571f,   0.000000f,  23.366537f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  1.259828f,   0.000000f,   0.000000f,   1.668093f},
            {  0.000000f,   0.000000f,   0.885767f,   0.000000f},
            {  0.000000f,  -1.259828f,   0.000000f,  10.833580f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.324427f,   0.217420f,   0.920584f,  11.052060f},
            { -0.852411f,   0.489096f,   0.184888f,  78.562383f},
            { -0.410056f,  -0.844697f,   0.344006f,  -8.390521f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.840193f,  -0.540788f,  -0.040288f, -34.668739f},
            { -0.102606f,  -0.085585f,  -0.991035f,  17.130267f},
            {  0.532492f,   0.836793f,  -0.127395f,  56.477256f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  1.000000f,   0.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   1.000000f,   0.000000f},
            {  0.000000f,  -1.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.954568f,  -0.168932f,  -0.248769f,  10.987287f},
            {  0.171253f,  -0.392511f,   0.908502f,   8.632905f},
            { -0.243870f,  -0.936879f,  -0.335766f,  10.008259f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  1.000000f,   0.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   1.000000f,   0.000000f},
            {  0.000000f,  -1.000000f,   0.000000f,   0.000000f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
    };
    const char *spark_mesh = "assets/craftsman/spark.obj";
    blinn_material_t spark_material = {
        {1, 1, 1, 1}, 32,
        "assets/craftsman/spark_diffuse.tga",
        NULL,
        "assets/craftsman/spark_emission.tga",
        1, 1, 0,
    };
    mat4_t spark_transforms[] = {
        {{
            {  0.104984f,   0.009185f,   0.000000f,   9.940392f},
            { -0.009185f,   0.104984f,   0.000000f,  23.342182f},
            { -0.000000f,  -0.000000f,   0.105385f,  23.126503f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            { -0.058686f,  -0.101646f,  -0.000000f,  -9.783746f},
            {  0.101646f,  -0.058686f,  -0.000000f,  24.797363f},
            {  0.000000f,  -0.000000f,   0.117371f,  23.126503f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.020583f,   0.001801f,   0.117176f,   1.669557f},
            { -0.049384f,  -0.107757f,   0.010331f,  11.562137f},
            {  0.106276f,  -0.050421f,  -0.017893f,  36.131775f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            { -0.032629f,  -0.046599f,   0.098531f,  -5.532722f},
            {  0.093198f,  -0.065258f,  -0.000000f,  20.369579f},
            {  0.056515f,   0.080712f,   0.056887f,  30.495888f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.050042f,   0.004378f,   0.087006f,   3.001525f},
            { -0.008756f,   0.100084f,   0.000000f,  23.325027f},
            { -0.086675f,  -0.007583f,   0.050233f,  17.500957f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            { -0.081984f,  -0.007173f,   0.057625f,  -2.616677f},
            { -0.008756f,   0.100084f,   0.000000f,  22.141081f},
            { -0.057406f,  -0.005022f,  -0.082297f,  21.804188f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.068182f,  -0.039782f,  -0.089026f,   6.102993f},
            { -0.049384f,  -0.107757f,   0.010331f,  16.986937f},
            { -0.084080f,   0.031031f,  -0.078261f,  15.379788f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.064266f,   0.091782f,  -0.019757f,   6.415188f},
            {  0.093198f,  -0.065258f,  -0.000000f,  20.369579f},
            { -0.011332f,  -0.016184f,  -0.112045f,  24.519733f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.037722f,   0.065337f,   0.089912f,   3.502418f},
            {  0.101646f,  -0.058686f,  -0.000000f,  24.797363f},
            {  0.044956f,   0.077866f,  -0.075445f,  32.513157f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.008723f,   0.000763f,  -0.100084f,  -0.887759f},
            { -0.008756f,   0.100084f,   0.000000f,  21.874636f},
            {  0.099703f,   0.008723f,   0.008756f,  27.667028f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            { -0.098563f,  -0.008623f,  -0.017446f, -16.835070f},
            { -0.008756f,   0.100084f,   0.000000f,  28.851065f},
            {  0.017379f,   0.001521f,  -0.098940f,  26.335102f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            { -0.067482f,  -0.005904f,   0.080729f,  -7.468997f},
            { -0.009185f,   0.104984f,   0.000000f,  27.920433f},
            { -0.080422f,  -0.007036f,  -0.067740f,  19.437935f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            { -0.098563f,  -0.008623f,   0.017446f,   0.181899f},
            { -0.008756f,   0.100084f,   0.000000f,  21.874636f},
            { -0.017379f,  -0.001520f,  -0.098940f,  22.585428f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            { -0.005688f,  -0.008123f,   0.113340f,   1.477568f},
            {  0.093198f,  -0.065258f,  -0.000000f,  20.274914f},
            {  0.065009f,   0.092843f,   0.009916f,  26.414782f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            { -0.053187f,  -0.092123f,   0.049603f,  -5.980136f},
            {  0.101646f,  -0.058686f,  -0.000000f,  24.797363f},
            {  0.024802f,   0.042958f,   0.106374f,  24.368771f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.076669f,   0.006708f,   0.064578f,   4.792371f},
            { -0.008756f,   0.100084f,   0.000000f,  22.141081f},
            { -0.064333f,  -0.005628f,   0.076962f,  22.409786f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.070770f,   0.006192f,  -0.071040f,   8.650177f},
            { -0.008756f,   0.100084f,   0.000000f,  23.325027f},
            {  0.070770f,   0.006192f,   0.071040f,  27.770390f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.059841f,  -0.059590f,  -0.054420f,   0.180720f},
            {  0.065058f,   0.075709f,  -0.011363f,  27.998913f},
            {  0.047749f,  -0.028472f,   0.083683f,  22.997925f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.015977f,   0.096166f,  -0.024297f,  21.038574f},
            { -0.069497f,   0.028413f,   0.066756f,  23.325027f},
            {  0.070770f,   0.006192f,   0.071040f,  27.770390f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
        {{
            {  0.059841f,  -0.059590f,  -0.054420f,  -2.989837f},
            {  0.065058f,   0.075709f,  -0.011363f,  34.510429f},
            {  0.047749f,  -0.028472f,   0.083683f,  22.997925f},
            {  0.000000f,   0.000000f,   0.000000f,   1.000000f},
        }},
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    mat4_t scale, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    int num_sparks = ARRAY_SIZE(spark_transforms);
    model_t **models = NULL;
    model_t *model;
    int i;

    assert((int)ARRAY_SIZE(materials) <= num_meshes);
    assert(ARRAY_SIZE(transforms) == num_meshes);

    translation = mat4_translate(-1.668f, -27.061f, -10.834f);
    scale = mat4_scale(0.016f, 0.016f, 0.016f);
    root = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        mat4_t transform = mat4_mul_mat4(root, transforms[i]);
        blinn_material_t material;
        if (i < (int)ARRAY_SIZE(materials)) {
            material = materials[i];
        } else {
            material = materials[ARRAY_SIZE(materials) - 1];
        }
        model = blinn_create_model(meshes[i], NULL, transform, material);
        darray_push(models, model);
    }
    for (i = 0; i < num_sparks; i++) {
        mat4_t transform = mat4_mul_mat4(root, spark_transforms[i]);
        model = blinn_create_model(spark_mesh, NULL, transform, spark_material);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 0.5f, 1, 0);
}

scene_t *blinn_drone_scene(void) {
    const char *mesh = "assets/drone/drone.obj";
    const char *skeleton = "assets/drone/drone.ani";
    blinn_material_t material = {
        {1, 1, 1, 1}, 32,
        "assets/drone/drone_diffuse.tga",
        "assets/drone/drone_specular.tga",
        "assets/drone/drone_emission.tga",
        0, 0, 0,
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    model_t **models = NULL;

    mat4_t translation = mat4_translate(0, -79.181f, -4.447f);
    mat4_t rotation = mat4_rotate_y(TO_RADIANS(180));
    mat4_t scale = mat4_scale(0.0028f, 0.0028f, 0.0028f);
    mat4_t root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    model_t *model = blinn_create_model(mesh, skeleton, root, material);
    darray_push(models, model);

    return scene_create(background, NULL, models, 0.5f, 1, 0);
}

scene_t *blinn_elfgirl_scene(void) {
    const char *meshes[] = {
        "assets/elfgirl/base.obj",
        "assets/elfgirl/body0.obj",
        "assets/elfgirl/body1.obj",
        "assets/elfgirl/body2.obj",
        "assets/elfgirl/face0.obj",
        "assets/elfgirl/face1.obj",
        "assets/elfgirl/hair.obj",
    };
    const char *textures[] = {
        "assets/elfgirl/base_diffuse.tga",
        "assets/elfgirl/body_diffuse.tga",
        "assets/elfgirl/body_diffuse.tga",
        "assets/elfgirl/body_diffuse.tga",
        "assets/elfgirl/face_diffuse.tga",
        "assets/elfgirl/face_diffuse.tga",
        "assets/elfgirl/hair_diffuse.tga",
    };
    blinn_material_t material = {{1, 1, 1, 1}, 32, NULL, NULL, NULL, 0, 0, 0};
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    model_t **models = NULL;
    model_t *model;
    int i;

    assert(ARRAY_SIZE(textures) == num_meshes);

    translation = mat4_translate(2.449f, -2.472f, -20.907f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    scale = mat4_scale(0.023f, 0.023f, 0.023f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        material.diffuse_map = textures[i];
        model = blinn_create_model(meshes[i], NULL, root, material);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 0.5f, 0.75f, 0);
}

scene_t *blinn_kgirls_scene(void) {
    const char *meshes[] = {
        "assets/kgirls/body.obj",
        "assets/kgirls/face.obj",
        "assets/kgirls/hair.obj",
        "assets/kgirls/pupils.obj",
    };
    const char *skeleton = "assets/kgirls/kgirls.ani";
    blinn_material_t material = {
        {1, 1, 1, 1}, 32,
        "assets/kgirls/kgirls_diffuse.tga", NULL, NULL,
        0, 0, 0,
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    model_t **models = NULL;
    model_t *model;
    int i;

    translation = mat4_translate(0, -4.937f, -96.547f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    rotation = mat4_mul_mat4(mat4_rotate_y(TO_RADIANS(90)), rotation);
    scale = mat4_scale(0.005f, 0.005f, 0.005f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        model = blinn_create_model(meshes[i], skeleton, root, material);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 0.5f, 1, 0);
}

scene_t *blinn_mccree_scene(void) {
    mat4_t transforms[8] = {
        {{
            {+0.186340f, +0.000000f, +0.000000f, -0.006200f},
            {+0.000000f, +0.000000f, +0.186340f, +0.131450f},
            {+0.000000f, -0.186340f, +0.000000f, -0.010720f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {+0.239850f, +0.008740f, +0.004060f, -0.014310f},
            {-0.020360f, +0.034120f, +0.052410f, +2.016510f},
            {+0.008460f, -0.165570f, +0.011010f, -0.055260f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {+0.287440f, -0.019260f, +0.000000f, -0.008370f},
            {-0.007360f, -0.108460f, -0.010090f, +2.267050f},
            {-0.017800f, -0.266190f, +0.004110f, -0.083880f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.039080f, -0.003570f, +0.033020f, +0.279880f},
            {-0.009790f, -0.000520f, -0.132230f, +1.397640f},
            {-0.003590f, +0.040280f, +0.001210f, +0.006560f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.039130f, -0.006760f, +0.025830f, +0.252460f},
            {-0.006030f, -0.009210f, -0.131150f, +1.364990f},
            {-0.008250f, +0.038800f, -0.026640f, -0.108820f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.039130f, -0.006760f, +0.025830f, +0.182500f},
            {-0.006030f, -0.009210f, -0.131150f, +1.340350f},
            {-0.008250f, +0.038800f, -0.026640f, -0.178540f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.019630f, -0.037890f, +0.016320f, +0.498480f},
            {-0.013760f, -0.058690f, -0.014030f, +1.926990f},
            {-0.031050f, +0.049970f, -0.004100f, +0.389470f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
        {{
            {-0.000780f, +0.023700f, +0.000570f, -0.440940f},
            {-0.004270f, -0.000700f, +0.023330f, +0.955610f},
            {-0.023320f, -0.000670f, -0.004290f, +0.163730f},
            {+0.000000f, +0.000000f, +0.000000f, +1.000000f},
        }},
    };
    blinn_material_t materials[28] = {
        {{0.614f, 0.396f, 0.464f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.689f, 0.441f, 0.516f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.736f, 0.471f, 0.552f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.430f, 0.241f, 0.296f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.347f, 0.855f, 1.000f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.399f, 0.522f, 0.397f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.850f, 0.160f, 0.086f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{1.000f, 0.423f, 0.170f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.193f, 0.374f, 0.333f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{1.000f, 1.000f, 1.000f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.317f, 0.158f, 0.229f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.425f, 0.198f, 0.233f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.779f, 0.310f, 0.263f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.964f, 0.744f, 0.622f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.641f, 0.252f, 0.234f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.973f, 0.801f, 0.705f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.938f, 0.814f, 0.687f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.748f, 0.596f, 0.418f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.831f, 0.618f, 0.525f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.694f, 0.510f, 0.427f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.336f, 0.149f, 0.205f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.445f, 0.195f, 0.270f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.444f, 0.187f, 0.207f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.927f, 0.483f, 0.498f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.815f, 0.403f, 0.418f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.827f, 0.430f, 0.449f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{0.542f, 0.222f, 0.247f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
        {{1.000f, 0.808f, 0.571f, 1}, 32, NULL, NULL, NULL, 1, 0, 0},
    };
    int mesh2transform[46] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 1, 1, 2, 2, 3, 3, 3, 4, 4,
        4, 5, 5, 5, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7,
    };
    int mesh2material[46] = {
        17, 13, 18, 19, 20, 21,  3, 11, 22, 23, 24, 25,  0,  4, 26,  2,
         1,  6,  9, 27, 12, 13, 14, 15, 16,  3, 11, 10,  4,  1, 10,  4,
         1, 10,  4,  1,  5,  6,  7,  8,  9,  0,  1,  2,  3,  4,
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    mat4_t scale, translation, root;
    int num_meshes = ARRAY_SIZE(mesh2transform);
    model_t **models = NULL;
    model_t *model;
    int i;

    assert(ARRAY_SIZE(mesh2material) == num_meshes);

    translation = mat4_translate(0.108f, -1.479f, 0.034f);
    scale = mat4_scale(0.337f, 0.337f, 0.337f);
    root = mat4_mul_mat4(scale, translation);
    for (i = 0; i < num_meshes; i++) {
        int transform_index = mesh2transform[i];
        int material_index = mesh2material[i];
        mat4_t transform = mat4_mul_mat4(root, transforms[transform_index]);
        blinn_material_t material = materials[material_index];
        const char *obj_template = "assets/mccree/mccree%d.obj";
        char obj_filename[64];
        sprintf(obj_filename, obj_template, i);
        model = blinn_create_model(obj_filename, NULL, transform, material);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 0.75f, 0.25f, 0);
}

scene_t *blinn_phoenix_scene(void) {
    const char *meshes[] = {
        "assets/phoenix/body.obj",
        "assets/phoenix/wings.obj",
    };
    const char *skeleton = "assets/phoenix/phoenix.ani";
    blinn_material_t materials[] = {
        {
            {1, 1, 1, 1}, 32,
            "assets/phoenix/body_diffuse.tga",
            NULL,
            "assets/phoenix/body_emission.tga",
            0, 0, 0.5f,
        },
        {
            {1, 1, 1, 1}, 32,
            "assets/phoenix/wings_diffuse.tga",
            NULL,
            "assets/phoenix/wings_emission.tga",
            0, 0, 0.5f,
        },
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    model_t **models = NULL;
    model_t *model;
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    translation = mat4_translate(376.905f, -169.495f, 0);
    rotation = mat4_rotate_y(TO_RADIANS(180));
    scale = mat4_scale(0.001f, 0.001f, 0.001f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        model = blinn_create_model(meshes[i], skeleton, root, materials[i]);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 0.75f, 0.25f, 0);
}

scene_t *blinn_witch_scene(void) {
    const char *meshes[] = {
        "assets/witch/object.obj",
        "assets/witch/witch.obj",
    };
    blinn_material_t materials[] = {
        {
            {1, 1, 1, 1}, 32,
            "assets/witch/object_diffuse.tga", NULL, NULL,
            1, 0, 0,
        },
        {
            {1, 1, 1, 1}, 32,
            "assets/witch/witch_diffuse.tga", NULL, NULL,
            1, 1, 0,
        },
    };
    vec4_t background = vec4_new(0.196f, 0.196f, 0.196f, 1);
    mat4_t scale, rotation, translation, root;
    int num_meshes = ARRAY_SIZE(meshes);
    model_t **models = NULL;
    model_t *model;
    int i;

    assert(ARRAY_SIZE(materials) == num_meshes);

    translation = mat4_translate(-17.924f, -16.974f, -32.691f);
    rotation = mat4_rotate_x(TO_RADIANS(-90));
    scale = mat4_scale(0.02f, 0.02f, 0.02f);
    root = mat4_mul_mat4(scale, mat4_mul_mat4(rotation, translation));
    for (i = 0; i < num_meshes; i++) {
        model = blinn_create_model(meshes[i], NULL, root, materials[i]);
        darray_push(models, model);
    }

    return scene_create(background, NULL, models, 0.5f, 1, 0);
}

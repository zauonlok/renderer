#ifndef CONSTANT_MODELS_H
#define CONSTANT_MODELS_H

#include "../core/apis.h"

model_t **constant_mccree_models(void);
void constant_release_models(model_t **models);

#endif

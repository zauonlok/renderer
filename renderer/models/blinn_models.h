#ifndef BLINN_MODELS_H
#define BLINN_MODELS_H

#include "../core/apis.h"

model_t **blinn_craftsman_models(void);
void blinn_release_models(model_t **models);

#endif

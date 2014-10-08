/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#ifndef NH_CONTROLLER_H_
#define NH_CONTROLLER_H_


SSE_BEGIN_C_DECLS

typedef struct NHController_ NHController;

NHController * NHController_New(Moat in_moat);
void NHController_Delete(NHController *self);

sse_int NHController_Begin(NHController *self);
sse_int NHController_End(NHController *self);

SSE_END_C_DECLS

#endif /* NH_CONTROLLER_H_ */

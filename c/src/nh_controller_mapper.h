/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#ifndef NH_CONTROLLER_MAPPER_H_
#define NH_CONTROLLER_MAPPER_H_


SSE_BEGIN_C_DECLS

typedef struct NHControllerMapper_ NHControllerMapper;

NHControllerMapper * NHControllerMapper_New(sse_pointer in_context, sse_int (*PerformRemoteCallProc)(sse_pointer), NHRequestBuilder* in_builder, NHResponseParser *in_parser);
void NHControllerMapper_Delete(NHControllerMapper *self);
ModelMapper * NHControllerMapper_GetModelMapper(NHControllerMapper *self);

SSE_END_C_DECLS

#endif /* NH_CONTROLLER_MAPPER_H_ */

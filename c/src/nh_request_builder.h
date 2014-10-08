/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#ifndef NH_REQUEST_BUILDER_H_
#define NH_REQUEST_BUILDER_H_


SSE_BEGIN_C_DECLS

typedef struct NHRequestBuilder_ NHRequestBuilder;

NHRequestBuilder * NHRequestBuilder_New(void);
void NHRequestBuilder_Delete(NHRequestBuilder *self);

sse_int NHRequestBuilder_Build(NHRequestBuilder *self, sse_byte **out_payload, sse_size *out_len);
sse_int NHRequestBuilder_Buzzer(NHRequestBuilder *self, sse_char *in_arg);
sse_int NHRequestBuilder_Red(NHRequestBuilder *self, sse_char *in_arg);
sse_int NHRequestBuilder_Yellow(NHRequestBuilder *self, sse_char *in_arg);
sse_int NHRequestBuilder_Green(NHRequestBuilder *self, sse_char *in_arg);
sse_int NHRequestBuilder_White(NHRequestBuilder *self, sse_char *in_arg);
sse_int NHRequestBuilder_Blue(NHRequestBuilder *self, sse_char *in_arg);
sse_int NHRequestBuilder_GotIt(NHRequestBuilder *self);
sse_int NHRequestBuilder_TellMe(NHRequestBuilder *self);

SSE_END_C_DECLS

#endif /* NH_REQUEST_BUILDER_H_ */

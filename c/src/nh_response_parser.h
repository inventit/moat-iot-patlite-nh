/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#ifndef NH_RESPONSE_PARSER_H_
#define NH_RESPONSE_PARSER_H_


SSE_BEGIN_C_DECLS

struct NHResponseParser_ {
  sse_bool IsStatusOk;
  sse_byte *Raw;
  sse_size Length;
};
typedef struct NHResponseParser_ NHResponseParser;

NHResponseParser * NHResponseParser_New(void);
void NHResponseParser_Delete(NHResponseParser *self);
sse_int NHResponseParser_Parse(NHResponseParser *self, sse_byte *in_content, sse_size in_length);

SSE_END_C_DECLS

#endif /* NH_RESPONSE_PARSER_H_ */

/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#include <servicesync/moat.h>
#include "nh_response_parser.h"

#define NHRP_LOG_TAG "NHResponseParser"
#define NHRP_LOG_ERROR(format, ...)   MOAT_LOG_ERROR(NHRP_LOG_TAG, format, ##__VA_ARGS__)
#define NHRP_LOG_INFO(format, ...)    MOAT_LOG_INFO(NHRP_LOG_TAG, format, ##__VA_ARGS__)
#define NHRP_LOG_DEBUG(format, ...)   MOAT_LOG_DEBUG(NHRP_LOG_TAG, format, ##__VA_ARGS__)
#define NHRP_LOG_TRACE(format, ...)   MOAT_LOG_TRACE(NHRP_LOG_TAG, format, ##__VA_ARGS__)
#define NHRP_ENTER()                  NHRP_LOG_TRACE("enter")
#define NHRP_LEAVE()                  NHRP_LOG_TRACE("leave")

NHResponseParser *
NHResponseParser_New(void)
{
  NHResponseParser *instance = NULL;

  NHRP_ENTER();
  instance = sse_malloc(sizeof(NHResponseParser));
  if (instance == NULL) {
    NHRP_LOG_ERROR("No memory.");
    goto on_error;
  }
  instance->IsStatusOk = sse_false;
  instance->Raw = NULL;
  instance->Length = 0;
  goto finally;

on_error:
finally:
  NHRP_LEAVE();
  return instance;
}

void
NHResponseParser_Delete(NHResponseParser *self)
{
  if (self == NULL) {
    return;
  }
  if (self->Raw != NULL) {
    sse_free(self->Raw);
  }
  sse_free(self);
}

static sse_bool
is_status_ok(sse_byte val)
{
  if (val == 0x06) {
    return sse_true;
  }
  return sse_false;
}

sse_int
NHResponseParser_Parse(NHResponseParser *self, sse_byte *in_content, sse_size in_length)
{
  sse_int err = SSE_E_OK;
  sse_byte *raw = NULL;

  NHRP_ENTER();
  if ((in_content == NULL) || (in_length < 1)) {
    err = SSE_E_INVAL;
    goto on_error;
  }
  if (in_length == 1) {
    self->IsStatusOk = is_status_ok(*in_content);
  } else {
    self->IsStatusOk = (in_length == 6) ? sse_true : sse_false;
  }
  raw = sse_memdup(in_content, in_length);
  self->Length = in_length;
  self->Raw = raw;
  goto finally;

on_error:
finally:
  NHRP_LEAVE();
  return err;
}

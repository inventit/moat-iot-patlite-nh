/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#include <servicesync/moat.h>
#include "nh_request_builder.h"
#include "nh_response_parser.h"
#include "nh_controller_mapper.h"

#define NHCM_LOG_TAG "NHControllerMapper"
#define NHCM_LOG_ERROR(format, ...)   MOAT_LOG_ERROR(NHCM_LOG_TAG, format, ##__VA_ARGS__)
#define NHCM_LOG_INFO(format, ...)    MOAT_LOG_INFO(NHCM_LOG_TAG, format, ##__VA_ARGS__)
#define NHCM_LOG_DEBUG(format, ...)   MOAT_LOG_DEBUG(NHCM_LOG_TAG, format, ##__VA_ARGS__)
#define NHCM_LOG_TRACE(format, ...)   MOAT_LOG_TRACE(NHCM_LOG_TAG, format, ##__VA_ARGS__)
#define NHCM_ENTER()                  NHCM_LOG_TRACE("enter")
#define NHCM_LEAVE()                  NHCM_LOG_TRACE("leave")

struct NHControllerMapper_ {
  ModelMapper ModelMapper;
  Moat Moat;
  MoatObject *Repo;
  sse_pointer RemoteCallContext;
  sse_int (*RemoteCallProc)(sse_pointer, sse_char *, sse_int);
  NHRequestBuilder *RequestBuilder;
  NHResponseParser *ResponseParser;
};

static sse_int
NHControllerMapper_AddProc(Moat in_moat, sse_char *in_uid, MoatObject *in_object, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  sse_int err = SSE_E_OK;

  NHCM_ENTER();
  if ((in_uid == NULL) || (in_object == NULL)) {
    err = SSE_E_INVAL;
    goto on_error;
  }
  NHCM_LOG_DEBUG("uid=[%s]", in_uid);
  self = (NHControllerMapper *) in_model_context;
  err = moat_object_add_object_value(self->Repo, in_uid, in_object, sse_true, sse_false);
  if (err) {
    goto on_error;
  }
  err = SSE_E_OK;
  goto finally;

on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

static sse_int
NHControllerMapper_UpdateProc(Moat in_moat, sse_char *in_uid, MoatObject *in_object, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  sse_int err = SSE_E_OK;
  MoatObject *element = NULL;

  NHCM_ENTER();
  if ((in_uid == NULL) || (in_object == NULL)) {
    err = SSE_E_INVAL;
    goto on_error;
  }
  NHCM_LOG_DEBUG("uid=[%s]", in_uid);
  self = (NHControllerMapper *) in_model_context;
  err = moat_object_get_object_value(self->Repo, in_uid, &element);
  if (err) {
    goto on_error;
  }
  err = moat_object_add_object_value(self->Repo, in_uid, in_object, sse_true, sse_true);
  if (err) {
    goto on_error;
  }

  err = SSE_E_OK;
  goto finally;

on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

static void
deep_copy_fields(MoatObject *src, MoatObject *dest)
{
  MoatObjectIterator *iterator = NULL;
  sse_char *field_name = NULL;
  MoatValue *field_value = NULL;

  iterator = moat_object_create_iterator(src);
  while (moat_object_iterator_has_next(iterator)) {
    field_name = moat_object_iterator_get_next_key(iterator);
    if (field_name == NULL) {
      continue;
    }
    field_value = moat_object_get_value(src, field_name);
    if (field_value == NULL) {
      moat_object_remove_value(dest, field_name);
    } else {
      moat_object_add_value(dest, field_name, field_value, sse_true, sse_true);
    }
  }
}

static sse_int
NHControllerMapper_UpdateFieldProc(Moat in_moat, sse_char *in_uid, MoatObject *in_object, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  sse_int err = SSE_E_OK;
  MoatObject *element = NULL;
  sse_int count = 0;

  NHCM_ENTER();
  if ((in_uid == NULL) || (in_object == NULL)) {
    err = SSE_E_INVAL;
    goto on_error;
  }
  count = moat_object_get_length(in_object);
  if (count == 0) {
    err = SSE_E_OK;
    goto finally;
  }
  NHCM_LOG_DEBUG("uid=[%s]", in_uid);
  self = (NHControllerMapper *) in_model_context;
  err = moat_object_get_object_value(self->Repo, in_uid, &element);
  if (err) {
    goto on_error;
  }
  deep_copy_fields(in_object, element);
  err = moat_object_add_object_value(self->Repo, in_uid, in_object, sse_false, sse_true);
  if (err) {
    goto on_error;
  }

  err = SSE_E_OK;
  goto finally;

on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

static sse_int
NHControllerMapper_RemoveProc(Moat in_moat, sse_char *in_uid, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  sse_int err = SSE_E_OK;

  NHCM_ENTER();
  if (in_uid == NULL) {
    err = SSE_E_INVAL;
    goto on_error;
  }
  NHCM_LOG_DEBUG("uid=[%s]", in_uid);
  self = (NHControllerMapper *) in_model_context;
  moat_object_remove_value(self->Repo, in_uid);

  err = SSE_E_OK;
  goto finally;

on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

static sse_int
NHControllerMapper_FindByUidProc(Moat in_moat, sse_char *in_uid, MoatObject **out_object, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  sse_int err = SSE_E_OK;

  NHCM_ENTER();
  if (in_uid == NULL) {
    err = SSE_E_INVAL;
    goto on_error;
  }
  NHCM_LOG_DEBUG("uid=[%s]", in_uid);
  self = (NHControllerMapper *) in_model_context;
  err = moat_object_get_object_value(self->Repo, in_uid, out_object);
  if (err) {
    goto on_error;
  }

  err = SSE_E_OK;
  goto finally;

on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

static sse_int
NHControllerMapper_FindAllUidsProc(Moat in_moat, SSESList **out_uids, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  MoatObjectIterator *iterator = NULL;
  SSESList *uids = NULL;
  sse_int err = SSE_E_OK;
  sse_char *uid = NULL;
  
  NHCM_ENTER();
  self = (NHControllerMapper *) in_model_context;
  uids = sse_slist_new();
  if (uids == NULL) {
    err = SSE_E_NOMEM;
    goto on_error;
  }
  
  iterator = moat_object_create_iterator(self->Repo);
  while (moat_object_iterator_has_next(iterator)) {
    uid = moat_object_iterator_get_next_key(iterator);
    if (uid == NULL) {
      continue;
    }
    uid = sse_strdup(uid);
    if (uid == NULL) {
      err = SSE_E_NOMEM;
      goto on_error;
    }
    uids = sse_slist_add(uids, uid);
  }
  *out_uids = uids;

  err = SSE_E_OK;
  goto finally;

on_error:
  if (uids != NULL) {
    while (uids != NULL) {
      uid = (sse_char *)uids->data;
      uids = sse_slist_remove(uids, uids->data);
      sse_free(uid);
    }
    sse_slist_free(uids);
  }
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

static sse_int
NHControllerMapper_CountProc(Moat moat, sse_uint *out_count, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  sse_int err = SSE_E_OK;

  NHCM_ENTER();
  self = (NHControllerMapper *) in_model_context;
  *out_count = moat_object_get_length(self->Repo);
  
  err = SSE_E_OK;
  goto finally;

on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

NHControllerMapper *
NHControllerMapper_New(sse_pointer in_context, sse_int (*PerformRemoteCallProc)(sse_pointer, sse_char *, sse_int), NHRequestBuilder* in_builder, NHResponseParser *in_parser)
{
  NHControllerMapper *instance = NULL;

  NHCM_ENTER();
  instance = sse_malloc(sizeof(NHControllerMapper));
  if (instance == NULL) {
    NHCM_LOG_ERROR("No memory.");
    goto on_error;
  }
  sse_memset(&instance->ModelMapper, 0, sizeof(instance->ModelMapper));
  instance->ModelMapper.AddProc = NHControllerMapper_AddProc;
  instance->ModelMapper.RemoveProc = NHControllerMapper_RemoveProc;
  instance->ModelMapper.UpdateProc = NHControllerMapper_UpdateProc;
  instance->ModelMapper.UpdateFieldsProc = NHControllerMapper_UpdateFieldProc;
  instance->ModelMapper.FindAllUidsProc = NHControllerMapper_FindAllUidsProc;
  instance->ModelMapper.FindByUidProc = NHControllerMapper_FindByUidProc;
  instance->ModelMapper.CountProc = NHControllerMapper_CountProc;
  instance->RemoteCallContext = in_context;
  instance->RemoteCallProc = PerformRemoteCallProc;
  instance->RequestBuilder = in_builder;
  instance->ResponseParser = in_parser;
  instance->Repo = moat_object_new();
  if (instance->Repo == NULL) {
    NHCM_LOG_ERROR("No memory.");
    goto on_error;
  }

  goto finally;

on_error:
  if (instance != NULL) {
    NHControllerMapper_Delete(instance);
    instance = NULL;
  }
  
finally:
  NHCM_LEAVE();
  return instance;
}

void
NHControllerMapper_Delete(NHControllerMapper *self)
{
  NHCM_ENTER();

  if (self == NULL) {
    goto finally;
  }
  if (self->Repo != NULL) {
    moat_object_free(self->Repo);
  }
  sse_free(self);
  goto finally;

finally:
  NHCM_LEAVE();
}

ModelMapper *
NHControllerMapper_GetModelMapper(NHControllerMapper *self)
{
  if (self == NULL) {
    return NULL;
  }
  return &self->ModelMapper;
}

sse_int
NHController_play(Moat moat, sse_char *in_uid, sse_char *in_key, MoatValue *in_data, sse_pointer in_model_context)
{
  sse_int err;
  NHCM_ENTER();
  
  // TODO
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  
  err = SSE_E_OK;
  goto finally;
on_error:
finally:
  NHCM_LEAVE();
  return err;
}

sse_int
NHController_okGotIt(Moat moat, sse_char *in_uid, sse_char *in_key, MoatValue *in_data, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  NHRequestBuilder *builder = NULL;
  sse_int err = SSE_E_OK;
  MoatObject *object = NULL;
  sse_char *ipv4_address = NULL;
  sse_uint ipv4_address_len = 0;
  sse_int32 port = 0;

  NHCM_ENTER();
  self = (NHControllerMapper *) in_model_context;
  builder = self->RequestBuilder;
  err = NHControllerMapper_FindByUidProc(moat, in_uid, &object, in_model_context);
  if (err) {
    goto on_error;
  }
  err = moat_object_get_string_value(object, "ipv4Address", &ipv4_address, &ipv4_address_len);
  if (err) {
    goto on_error;
  }
  err = moat_object_get_int32_value(object, "port", &port);
  if (err) {
    goto on_error;
  }
  err = NHRequestBuilder_GotIt(builder);
  if (err) {
    goto on_error;
  }
  err = self->RemoteCallProc(self->RemoteCallContext, ipv4_address, (sse_int) port);
  if (err) {
    goto on_error;
  }
  
  err = SSE_E_OK;
  goto finally;
on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

sse_int
NHController_apply(Moat moat, sse_char *in_uid, sse_char *in_key, MoatValue *in_data, sse_pointer in_model_context)
{
  NHControllerMapper *self = NULL;
  NHRequestBuilder *builder = NULL;
  sse_int err = SSE_E_OK;
  MoatObject *object = NULL;
  sse_char *ipv4_address = NULL;
  sse_uint ipv4_address_len = 0;
  sse_int32 port = 0;
  sse_char *arg = NULL;
  sse_uint arg_len = 0;
  MoatValue *value;

  NHCM_ENTER();
  self = (NHControllerMapper *) in_model_context;
  builder = self->RequestBuilder;
  err = NHControllerMapper_FindByUidProc(moat, in_uid, &object, in_model_context);
  if (err) {
    goto on_error;
  }
  err = moat_object_get_string_value(object, "ipv4Address", &ipv4_address, &ipv4_address_len);
  if (err) {
    NHCM_LOG_ERROR("ipv4Address is missing.", err);
    err = SSE_E_NOTCONN;
    goto on_error;
  }
  value = moat_object_get_value(object, "port");
  if (value == NULL) {
    NHCM_LOG_ERROR("port is missing.", err);
    err = SSE_E_INVAL;
    goto on_error;
  }
  port = *((sse_int32 *) moat_value_peek_value(value));
  err = moat_object_get_string_value(object, "red", &arg, &arg_len);
  if (SSE_E_OK == err) {
    err = NHRequestBuilder_Red(builder, arg);
    if (err) {
      goto on_error;
    }
  }
  err = moat_object_get_string_value(object, "yellow", &arg, &arg_len);
  if (SSE_E_OK == err) {
    err = NHRequestBuilder_Yellow(builder, arg);
    if (err) {
      goto on_error;
    }
  }
  err = moat_object_get_string_value(object, "green", &arg, &arg_len);
  if (SSE_E_OK == err) {
    err = NHRequestBuilder_Green(builder, arg);
    if (err) {
      goto on_error;
    }
  }
  err = moat_object_get_string_value(object, "blue", &arg, &arg_len);
  if (SSE_E_OK == err) {
    err = NHRequestBuilder_Blue(builder, arg);
    if (err) {
      goto on_error;
    }
  }
  err = moat_object_get_string_value(object, "white", &arg, &arg_len);
  if (SSE_E_OK == err) {
    err = NHRequestBuilder_White(builder, arg);
    if (err) {
      goto on_error;
    }
  }
  err = moat_object_get_string_value(object, "buzzer", &arg, &arg_len);
  if (SSE_E_OK == err) {
    err = NHRequestBuilder_Buzzer(builder, arg);
    if (err) {
      goto on_error;
    }
  }
  err = self->RemoteCallProc(self->RemoteCallContext, ipv4_address, port);
  if (err) {
    goto on_error;
  }

  err = SSE_E_OK;
  goto finally;
on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

sse_int
NHController_load(Moat moat, sse_char *in_uid, sse_char *in_key, MoatValue *in_data, sse_pointer in_model_context)
{
  sse_int err;
  NHCM_ENTER();
  
  // TODO
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  NHCM_LOG_INFO("TODO");
  
  err = SSE_E_OK;
  goto finally;
on_error:
  NHCM_LOG_ERROR("err=[%d]", err);

finally:
  NHCM_LEAVE();
  return err;
}

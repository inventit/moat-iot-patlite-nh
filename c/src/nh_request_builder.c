/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#include <servicesync/moat.h>
#include "nh_request_builder.h"

#define NHRB_LOG_TAG "NHRequestBuilder"
#define NHRB_LOG_ERROR(format, ...)   MOAT_LOG_ERROR(NHRB_LOG_TAG, format, ##__VA_ARGS__)
#define NHRB_LOG_INFO(format, ...)    MOAT_LOG_INFO(NHRB_LOG_TAG, format, ##__VA_ARGS__)
#define NHRB_LOG_DEBUG(format, ...)   MOAT_LOG_DEBUG(NHRB_LOG_TAG, format, ##__VA_ARGS__)
#define NHRB_LOG_TRACE(format, ...)   MOAT_LOG_TRACE(NHRB_LOG_TAG, format, ##__VA_ARGS__)
#define NHRB_ENTER()                  NHRB_LOG_TRACE("enter")
#define NHRB_LEAVE()                  NHRB_LOG_TRACE("leave")

#define NHRB_HEADER(x) \
do { \
  x[0] = 0x58; \
  x[1] = 0x58; \
  x[2] = 0x53; \
  x[3] = 0x00; \
  x[4] = 0x00; \
  x[5] = 0x06; \
} while (0);

#define NHRB_HEADER_OK_GOT_IT(x) \
do { \
  x[0] = 0x58; \
  x[1] = 0x58; \
  x[2] = 0x43; \
  x[3] = 0x00; \
  x[4] = 0x00; \
  x[5] = 0x00; \
} while (0);

#define NHRB_HEADER_TELL_ME(x) \
do { \
  x[0] = 0x58; \
  x[1] = 0x58; \
  x[2] = 0x47; \
  x[3] = 0x00; \
  x[4] = 0x00; \
  x[5] = 0x00; \
} while (0);


#define NHRB_RESET_BOOL() do {\
  self->OkGotIt = sse_false; \
  self->TellMe  = sse_false; \
} while (0);


enum nhrb_light_type_ {
  RED_LIGHT,
  YELLOW_LIGHT,
  GREEN_LIGHT,
  BLUE_LIGHT,
  WHITE_LIGHT
};
typedef enum nhrb_light_type_ nhrb_light_type;

enum nhrb_light_ {
    LIGHT_OFF        = 0x00,
    LIGHT_ALWAYS_ON  = 0x01,
    LIGHT_BLINK1     = 0x02,
    LIGHT_BLINK2     = 0x03,
    LIGHT_NOT_CHANGE = 0x09
};
typedef enum nhrb_light_ nhrb_light;

enum nhrb_buzzer_ {
    BUZZER_OFF        = 0x00,
    BUZZER_SIREN1     = 0x01,
    BUZZER_SIREN2     = 0x02,
    BUZZER_SIREN3     = 0x03,
    BUZZER_SIREN4     = 0x04,
    BUZZER_NOT_CHANGE = 0x09
};
typedef enum nhrb_buzzer_ nhrb_buzzer;

struct NHRequestBuilder_ {
  nhrb_light Red;
  nhrb_light Yellow;
  nhrb_light Green;
  nhrb_light Blue;
  nhrb_light White;
  nhrb_buzzer Buzzer;
  sse_bool OkGotIt;
  sse_bool TellMe;
};

static void
reset(NHRequestBuilder *self)
{
  self->Red = LIGHT_NOT_CHANGE;
  self->Yellow = LIGHT_NOT_CHANGE;
  self->Green = LIGHT_NOT_CHANGE;
  self->Blue = LIGHT_NOT_CHANGE;
  self->White = LIGHT_NOT_CHANGE;
  self->Buzzer = BUZZER_NOT_CHANGE;
  self->OkGotIt = sse_false;
  self->TellMe = sse_false;
}

NHRequestBuilder *
NHRequestBuilder_New(void)
{
  NHRequestBuilder *instance = NULL;

  NHRB_ENTER();
  instance = sse_malloc(sizeof(NHRequestBuilder));
  if (instance == NULL) {
    NHRB_LOG_ERROR("No memory.");
    goto on_error;
  }
  reset(instance);
  goto finally;

on_error:
finally:
  NHRB_LEAVE();
  return instance;
}

void
NHRequestBuilder_Delete(NHRequestBuilder *self)
{
  if (self == NULL) {
    return;
  }
  sse_free(self);
}

static nhrb_light
to_nhrb_light(sse_char *arg)
{
  if (arg == NULL) {
    return LIGHT_NOT_CHANGE;
  }
  if (sse_strncmp("OFF", arg, 10) == 0) {
    return LIGHT_OFF;
  }
  if (sse_strncmp("ALWAYS_ON", arg, 10) == 0) {
    return LIGHT_ALWAYS_ON;
  }
  if (sse_strncmp("BLINK1", arg, 10) == 0) {
    return LIGHT_BLINK1;
  }
  if (sse_strncmp("BLINK2", arg, 10) == 0) {
    return LIGHT_BLINK2;
  }
  return LIGHT_NOT_CHANGE;
}

static sse_int
light(NHRequestBuilder *self, nhrb_light_type light, nhrb_light val)
{
  if (self == NULL) {
    return SSE_E_INVAL;
  }

  NHRB_RESET_BOOL();
  switch (light) {
    case RED_LIGHT:
      self->Red = val;
      break;
    case YELLOW_LIGHT:
      self->Yellow = val;
      break;
    case GREEN_LIGHT:
      self->Green = val;
      break;
    case BLUE_LIGHT:
      self->Blue = val;
      break;
    case WHITE_LIGHT:
      self->White = val;
      break;
    default:
      // do nothing
      ;
  }
  return SSE_E_OK;
}

sse_int
NHRequestBuilder_Build(NHRequestBuilder *self, sse_byte **out_payload, sse_size *out_len)
{
  sse_int err = SSE_E_OK;
  sse_byte *payload = NULL;
  sse_int i = 0;
  NHRB_ENTER();

  if (self->OkGotIt || self->TellMe) {
    *out_len = 6;
  } else {
    *out_len = 12;
  }
  payload = sse_malloc(*out_len);
  if (payload == NULL) {
    err = SSE_E_NOMEM;
    goto on_error;
  }
  if (self->OkGotIt) {
    NHRB_HEADER_OK_GOT_IT(payload);
  } else if (self->TellMe) {
    NHRB_HEADER_TELL_ME(payload);
  } else {
    NHRB_HEADER(payload);
    i = 6;
    payload[i++] = (sse_byte) self->Red;
    payload[i++] = (sse_byte) self->Yellow;
    payload[i++] = (sse_byte) self->Green;
    payload[i++] = (sse_byte) self->Blue;
    payload[i++] = (sse_byte) self->White;
    payload[i++] = (sse_byte) self->Buzzer;
  }

  *out_payload = payload;
  reset(self);
  NHRB_LOG_INFO("The payload has been generated. The internal state is cleared!");
  goto finally;
  
on_error:
  *out_len = -1;

finally:
  NHRB_LEAVE();
  return err;
}

static nhrb_buzzer
to_nhrb_buzzer(sse_char *arg)
{
  if (arg == NULL) {
    return BUZZER_NOT_CHANGE;
  }
  if (sse_strncmp("OFF", arg, 10) == 0) {
    return BUZZER_OFF;
  }
  if (sse_strncmp("SIREN1", arg, 10) == 0) {
    return BUZZER_SIREN1;
  }
  if (sse_strncmp("SIREN2", arg, 10) == 0) {
    return BUZZER_SIREN2;
  }
  if (sse_strncmp("SIREN3", arg, 10) == 0) {
    return BUZZER_SIREN3;
  }
  if (sse_strncmp("SIREN4", arg, 10) == 0) {
    return BUZZER_SIREN4;
  }
  return BUZZER_NOT_CHANGE;
}

sse_int
NHRequestBuilder_Buzzer(NHRequestBuilder *self, sse_char *arg)
{
  nhrb_buzzer val;
  NHRB_ENTER();
  val = to_nhrb_buzzer(arg);
  self->Buzzer = val;
  NHRB_RESET_BOOL();
  NHRB_LEAVE();
  return SSE_E_OK;
}

sse_int
NHRequestBuilder_Red(NHRequestBuilder *self, sse_char *arg)
{
  sse_int err = SSE_E_OK;
  nhrb_light val;
  NHRB_ENTER();
  val = to_nhrb_light(arg);
  err = light(self, RED_LIGHT, val);
  NHRB_LEAVE();
  return err;
}

sse_int
NHRequestBuilder_Yellow(NHRequestBuilder *self, sse_char *arg)
{
  sse_int err = SSE_E_OK;
  nhrb_light val;
  NHRB_ENTER();
  val = to_nhrb_light(arg);
  err = light(self, YELLOW_LIGHT, val);
  NHRB_LEAVE();
  return err;
}

sse_int
NHRequestBuilder_Green(NHRequestBuilder *self, sse_char *arg)
{
  sse_int err = SSE_E_OK;
  nhrb_light val;
  NHRB_ENTER();
  val = to_nhrb_light(arg);
  err = light(self, GREEN_LIGHT, val);
  NHRB_LEAVE();
  return err;
}

sse_int
NHRequestBuilder_White(NHRequestBuilder *self, sse_char *arg)
{
  sse_int err = SSE_E_OK;
  nhrb_light val;
  NHRB_ENTER();
  val = to_nhrb_light(arg);
  err = light(self, WHITE_LIGHT, val);
  NHRB_LEAVE();
  return err;
}

sse_int
NHRequestBuilder_Blue(NHRequestBuilder *self, sse_char *arg)
{
  sse_int err = SSE_E_OK;
  nhrb_light val;
  NHRB_ENTER();
  val = to_nhrb_light(arg);
  err = light(self, BLUE_LIGHT, val);
  NHRB_LEAVE();
  return err;
}

sse_int
NHRequestBuilder_GotIt(NHRequestBuilder *self)
{
  NHRB_ENTER();
  self->OkGotIt = sse_true;
  self->TellMe = sse_false;
  NHRB_LEAVE();
  return SSE_E_OK;
}

sse_int
NHRequestBuilder_TellMe(NHRequestBuilder *self)
{
  NHRB_ENTER();
  self->TellMe = sse_true;
  self->OkGotIt = sse_false;
  NHRB_LEAVE();
  return SSE_E_OK;
}

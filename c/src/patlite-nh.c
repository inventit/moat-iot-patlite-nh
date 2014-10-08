/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#include <servicesync/moat.h>
#include "nh_controller.h"

#define PLNH_LOG_TAG "PatliteNH"
#define PLNH_LOG_ERROR(format, ...)   MOAT_LOG_ERROR(PLNH_LOG_TAG, format, ##__VA_ARGS__)
#define PLNH_LOG_INFO(format, ...)    MOAT_LOG_INFO(PLNH_LOG_TAG, format, ##__VA_ARGS__)
#define PLNH_LOG_DEBUG(format, ...)   MOAT_LOG_DEBUG(PLNH_LOG_TAG, format, ##__VA_ARGS__)
#define PLNH_LOG_TRACE(format, ...)   MOAT_LOG_TRACE(PLNH_LOG_TAG, format, ##__VA_ARGS__)
#define PLNH_ENTER()                  PLNH_LOG_TRACE("enter")
#define PLNH_LEAVE()                  PLNH_LOG_TRACE("leave")

sse_int
moat_app_main(sse_int in_argc, sse_char *argv[])
{
  sse_char *pkg_urn = NULL;
  Moat moat = NULL;
  sse_int err = SSE_E_OK;
  NHController *nh_controller = NULL;

  PLNH_ENTER();
  pkg_urn = argv[0];
  err = moat_init(pkg_urn, &moat);
  if (err != SSE_E_OK) {
    goto on_error_before_run;
  }
  
  nh_controller = NHController_New(moat);
  if (nh_controller == NULL) {
    goto on_error_before_run;
  }

  err = NHController_Begin(nh_controller);
  if (err != SSE_E_OK) {
    goto on_error_before_run;
  }

  err = moat_run(moat);
  if (err != SSE_E_OK) {
    goto on_error;
  }
  
  err = NHController_End(nh_controller);
  if (err != SSE_E_OK) {
    goto on_error;
  }

  err = SSE_E_OK;
  goto finally;

on_error_before_run:
  PLNH_LOG_ERROR("Failed to start app! err=[%d]", err);
  goto finally;
on_error:
  PLNH_LOG_ERROR("Failed on the app exiting! err=[%d]", err);
  goto finally;
finally:
  if (nh_controller != NULL) {
    NHController_Delete(nh_controller);
  }
  if (moat != NULL) {
    moat_destroy(moat);
  }
  PLNH_LEAVE();
  return err;
}

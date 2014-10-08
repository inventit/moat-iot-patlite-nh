/*
 * Copyright (C) 2014 InventIt Inc.
 * 
 * See https://github.com/inventit/moat-iot-patlite-nh
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <servicesync/moat.h>
#include "nh_controller.h"
#include "nh_request_builder.h"
#include "nh_response_parser.h"
#include "nh_controller_mapper.h"

#define NHC_LOG_TAG "NHController"
#define NHC_LOG_ERROR(format, ...)   MOAT_LOG_ERROR(NHC_LOG_TAG, format, ##__VA_ARGS__)
#define NHC_LOG_INFO(format, ...)    MOAT_LOG_INFO(NHC_LOG_TAG, format, ##__VA_ARGS__)
#define NHC_LOG_DEBUG(format, ...)   MOAT_LOG_DEBUG(NHC_LOG_TAG, format, ##__VA_ARGS__)
#define NHC_LOG_TRACE(format, ...)   MOAT_LOG_TRACE(NHC_LOG_TAG, format, ##__VA_ARGS__)
#define NHC_ENTER()                  NHC_LOG_TRACE("enter")
#define NHC_LEAVE()                  NHC_LOG_TRACE("leave")

struct NHController_ {
  sse_int SocketFd;
  sse_char *IpV4Address;
  sse_int Port;
  NHRequestBuilder *RequestBuilder;
  NHResponseParser *ResponseParser;
  sse_int LastError;
  sse_byte RecvBuffer[64];
  NHControllerMapper *ControllerMapper;
	Moat Moat;
};

static sse_int
NHController_PerformRemoteCallProc(sse_pointer context)
{
  NHController *self = NULL;
  sse_int err = SSE_E_OK;
  sse_byte *payload = NULL;
  sse_size len = 0;
  
  NHC_ENTER();
  if (context == NULL) {
    err = SSE_E_INVAL;
    goto on_error;
  }
  self = (NHController *) context;
  err = NHRequestBuilder_Build(self->RequestBuilder, &payload, &len);
  if (err) {
    goto on_error;
  }
  err = send(self->SocketFd, payload, sizeof(payload), 0);
  if (err < 0) {
    NHC_LOG_ERROR("Failed to send packets. errno=[%d]", errno);
    err = SSE_E_TIMEDOUT;
    goto on_error;
  }
  len = recv(self->SocketFd, self->RecvBuffer, sizeof(self->RecvBuffer) - 1, 0);
  if (len < 0) {
    NHC_LOG_ERROR("Failed to receive packets. errno=[%d]", errno);
    err = SSE_E_TIMEDOUT;
    goto on_error;
  }
  err = NHResponseParser_Parse(self->ResponseParser, self->RecvBuffer, len);
  if (err) {
    goto on_error;
  }
  if (self->ResponseParser->IsStatusOk != sse_true) {
    err = SSE_E_GENERIC;
    goto on_error;
  }

  err = SSE_E_OK;
  goto finally;

on_error:
finally:
  NHC_LEAVE();
  return err;
}

NHController *
NHController_New(Moat in_moat)
{
  NHController *instance = NULL;
  sse_int err = SSE_E_OK;
  ModelMapper * mapper = NULL;

  NHC_ENTER();
  instance = sse_malloc(sizeof(NHController));
  if (instance == NULL) {
    NHC_LOG_ERROR("No memory.");
    goto on_error;
  }
  instance->SocketFd = -1;
  instance->IpV4Address = NULL;
  instance->Port = -1;
  instance->RequestBuilder = NHRequestBuilder_New();
  instance->ResponseParser = NHResponseParser_New();
  instance->LastError = 0;
  instance->ControllerMapper = NHControllerMapper_New(instance, NHController_PerformRemoteCallProc, instance->RequestBuilder, instance->ResponseParser);
  instance->Moat = in_moat;
  mapper = NHControllerMapper_GetModelMapper(instance->ControllerMapper);
  err = moat_register_model(in_moat, "NHController", mapper, instance);
  if (err) {
    goto on_error;
  }

  goto finally;

on_error:
  if (instance != NULL) {
    NHController_Delete(instance);
    instance = NULL;
  }
  
finally:
  NHC_LEAVE();
  return instance;
}

void
NHController_Delete(NHController *self)
{
  NHC_ENTER();

  if (self == NULL) {
    goto finally;
  }
  if (self->Moat != NULL) {
    moat_unregister_model(self->Moat, "NHController");
  }
  NHResponseParser_Delete(self->ResponseParser);
  NHRequestBuilder_Delete(self->RequestBuilder);
  NHControllerMapper_Delete(self->ControllerMapper);
  sse_free(self);
  goto finally;

on_error:
finally:
  NHC_LEAVE();
}

static sse_int
socket_open(sse_int *out_sockfd)
{
  sse_int err;
  sse_int yes = 1;
  struct timeval timeout;

  NHC_ENTER();
  *out_sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (*out_sockfd < 0) {
    err = SSE_E_INVAL;
    goto on_error;
  }
  
	err = setsockopt(*out_sockfd, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(yes));
  if (err < 0) {
    NHC_LOG_ERROR("Failed to set SO_REUSEADDR => errno=[%d]", errno);
    err = SSE_E_INVAL;
    goto on_error;
  }

	err = setsockopt(*out_sockfd, IPPROTO_TCP, TCP_NODELAY, (const char *)&yes, sizeof(yes));
  if (err < 0) {
    NHC_LOG_ERROR("Failed to set TCP_NODELAY => errno=[%d]", errno);
    err = SSE_E_INVAL;
    goto on_error;
  }

	timeout.tv_sec = 1;
	timeout.tv_usec = 0;
	err = setsockopt(*out_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
  if (err < 0) {
    NHC_LOG_ERROR("Failed to set SO_RCVTIMEO => errno=[%d]", errno);
    err = SSE_E_INVAL;
    goto on_error;
  }

	setsockopt(*out_sockfd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
  if (err < 0) {
    NHC_LOG_ERROR("Failed to set SO_SNDTIMEO => errno=[%d]", errno);
    err = SSE_E_INVAL;
    goto on_error;
  }

  err = SSE_E_OK;
  goto finally;
  
on_error:
  if (*out_sockfd >= 0) {
    close(*out_sockfd);
  }
  *out_sockfd = -1;
finally:
  NHC_LEAVE();
  return err;
}

static sse_int
socket_connect(sse_int in_sockfd, sse_char *in_ipv4_address, sse_int in_port)
{
  sse_int err = SSE_E_OK;
  struct sockaddr_in serv_addr;
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(in_port);

  NHC_ENTER();
  memset(&serv_addr, '0', sizeof(serv_addr)); 
  err = inet_pton(AF_INET, in_ipv4_address, &serv_addr.sin_addr);
  if (err <= 0) {
    NHC_LOG_ERROR("Failed to perform inet_pton() => err=[%d]", err);
    err = SSE_E_INVAL;
    goto on_error;
  }
  
  err = connect(in_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (err) {
    NHC_LOG_ERROR("Failed to perform connect() => errno=[%d]", errno);
    err = SSE_E_INVAL;
    goto on_error;
  }

  err = SSE_E_OK;
  goto finally;

on_error:
  close(in_sockfd);
finally:
  NHC_LEAVE();
  return err;
}

sse_int
NHController_Begin(NHController *self)
{
  sse_int err = SSE_E_OK;
  
  NHC_ENTER();
  if (self == NULL) {
    goto finally;
  }
  err = socket_open(&self->SocketFd);
  if (err) {
    goto on_error;
  }
  err = socket_connect(self->SocketFd, self->IpV4Address, self->Port);
  if (err) {
    goto on_error;
  }
  
  err = SSE_E_OK;
  goto finally;

on_error:
finally:
  NHC_LEAVE();
  return err;
}

sse_int
NHController_End(NHController *self)
{
  sse_int err = SSE_E_OK;
  
  NHC_ENTER();
  if (self == NULL) {
    goto finally;
  }
  if (self->SocketFd >= 0) {
    close(self->SocketFd);
  }
  err = SSE_E_OK;
  goto finally;

on_error:
finally:
  NHC_LEAVE();
  return err;
}

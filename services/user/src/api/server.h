#pragma once

#include "api/health_handler.h"
#include "api/request_logger.h"
#include "api/user_handlers.h"
#include "config/config.h"

namespace rockvn::user::api {

// Registers listeners, routes, and observers on the Drogon application.
// Called by the composition root (main) and by integration tests — the one
// definition of "the wired server" so tests exercise production wiring.
// Handlers are injected by reference and must outlive drogon::app().run().
void configure_app(const config::Config& cfg, HealthHandler& health, RequestLogger& request_logger,
                   UserHandlers& users);

}  // namespace rockvn::user::api

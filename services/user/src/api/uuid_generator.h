#pragma once

#include <drogon/utils/Utilities.h>

#include <string>

#include "domain/id_generator.h"

namespace rockvn::user::api {

// Production adapter for the domain's IdGenerator seam. Lives in api/ so
// the framework utility it wraps never touches domain; tests inject
// deterministic fakes instead.
class UuidGenerator final : public domain::IdGenerator {
 public:
  std::string generate() const override { return drogon::utils::getUuid(); }
};

}  // namespace rockvn::user::api

#pragma once

#include <string>

namespace rockvn::user::domain {

// ID generation is a seam: production wires a UUID adapter (in api/, where
// the existing framework utility lives); tests inject deterministic IDs.
class IdGenerator {
 public:
  virtual ~IdGenerator() = default;
  virtual std::string generate() const = 0;
};

}  // namespace rockvn::user::domain

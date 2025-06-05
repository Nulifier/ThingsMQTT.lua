#pragma once

struct cJSON;  // Forward declare cJSON

namespace json {
/**
 * Compare two cJSON objects and return a new cJSON object containing only
 * the differences. This also updates the cache with any new values from
 * newObj. If there are no differences, returns nullptr.
 */
cJSON* compareWithCache(cJSON* newObj, cJSON*& cache);
}  // namespace json
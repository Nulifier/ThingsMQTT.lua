#include "json-utils.hpp"
#include <cjson/cJSON.h>
#include <stdexcept>

cJSON* compareWithCache(cJSON* newObj, cJSON*& cache) {
	if (newObj == nullptr) {
		throw std::invalid_argument("newObj cannot be null");
	}

	// If there is no cache yet, initialize it with a deep copy of newObj
	// and return a deep copy as the diff (since everything is new)
	if (cache == nullptr) {
		cache = cJSON_Duplicate(newObj, 1);
		if (cache == nullptr) {
			throw std::runtime_error("Failed to allocate cache duplicate");
		}
		cJSON* fullDiff = cJSON_Duplicate(newObj, 1);
		if (fullDiff == nullptr) {
			// Cleanup cache if we fail to allocate fullDiff
			cJSON_Delete(cache);
			cache = nullptr;
			throw std::runtime_error("Failed to allocate full diff duplicate");
		}
		return fullDiff;
	}

	cJSON* diff = cJSON_CreateObject();
	if (diff == nullptr) {
		throw std::runtime_error("Failed to create cJSON object");
	}

	// Iterate over newObj and compare with cached. If changed, add to diff and
	// update cache
	cJSON* item = nullptr;
	cJSON_ArrayForEach(item, newObj) {
		const char* key = item->string;
		if (key == nullptr) {
			continue;  // Skip items without keys
		}

		cJSON* cachedItem = cJSON_GetObjectItemCaseSensitive(cache, key);

		// If item is new or differs from cachedItem, add to diff and update
		// cache
		if (cachedItem == nullptr || !cJSON_Compare(item, cachedItem, 1)) {
			// Duplicate item for diff
			cJSON* itemCopy = cJSON_Duplicate(item, 1);
			if (itemCopy == nullptr) {
				cJSON_Delete(diff);
				throw std::runtime_error("Failed to duplicate changed item");
			}
			cJSON_AddItemToObject(diff, key, itemCopy);

			// Duplicate item for cache
			cJSON* newCacheItem = cJSON_Duplicate(item, 1);
			if (newCacheItem == nullptr) {
				cJSON_Delete(diff);
				throw std::runtime_error("Failed to duplicate item for cache");
			}

			if (cachedItem != nullptr) {
				// Replace existing cached item
				cJSON_ReplaceItemInObject(cache, key, newCacheItem);
			} else {
				// Add new item to cache
				cJSON_AddItemToObject(cache, key, newCacheItem);
			}
		}
	}

	// If no differences found, clean up and return nullptr
	if (cJSON_GetArraySize(diff) == 0) {
		cJSON_Delete(diff);
		return nullptr;
	}

	return diff;
}

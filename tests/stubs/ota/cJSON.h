/** @brief Typed cJSON tree fixture, not a JSON parser. Syntax parsing belongs to the ESP32 component. */
#pragma once
#include <string>
#include <vector>
#include <utility>
struct cJSON {
	int type = 0;
	double valuedouble = 0;
	const char *valuestring = nullptr;
	std::vector<std::pair<std::string, cJSON *>> fields;
	std::vector<cJSON *> items;
};
inline cJSON *test_json_root = nullptr;
inline unsigned test_json_parses = 0;
inline cJSON *cJSON_ParseWithLengthOpts(const char *json, size_t length, const char **end, bool) {
	++test_json_parses; if (end) { *end = json + length; } return test_json_root;
}
inline void cJSON_Delete(cJSON *) {}
inline bool cJSON_IsObject(const cJSON *v) { return v && v->type == 1; }
inline bool cJSON_IsArray(const cJSON *v) { return v && v->type == 2; }
inline bool cJSON_IsNumber(const cJSON *v) { return v && v->type == 3; }
inline bool cJSON_IsString(const cJSON *v) { return v && v->type == 4; }
inline bool cJSON_IsTrue(const cJSON *v) { return v && v->type == 5; }
inline const cJSON *cJSON_GetObjectItemCaseSensitive(const cJSON *v, const char *name) {
	if (v) { for (const auto &entry : v->fields) { if (entry.first == name) { return entry.second; } } }
	return nullptr;
}
inline int cJSON_GetArraySize(const cJSON *v) { return v ? int(v->items.size()) : 0; }
inline const cJSON *cJSON_GetArrayItem(const cJSON *v, int i) { return v && i >= 0 && size_t(i) < v->items.size() ? v->items[i] : nullptr; }

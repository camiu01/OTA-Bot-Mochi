/**
 * @file network_weather_test.cpp
 * @brief Provider field validation on typed cJSON fixtures; not HTTP/TLS or JSON syntax validation.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/network/network_weather.h"
#include <cJSON.h>
#include <cassert>
#include <deque>
#include <limits>
#include <iostream>
#include <string>

static std::deque<cJSON> arena;
static cJSON *node(int type) { arena.emplace_back(); arena.back().type = type; return &arena.back(); }
static cJSON *number(double value) { auto *v = node(3); v->valuedouble = value; return v; }
static cJSON *text(const char *value) { auto *v = node(4); v->valuestring = value; return v; }
static cJSON *object(std::initializer_list<std::pair<std::string, cJSON *>> fields) {
	auto *v = node(1); v->fields = fields; return v;
}
static cJSON *array(std::initializer_list<cJSON *> items) { auto *v = node(2); v->items = items; return v; }
static void forecasts() {
	auto *temperature = number(21.6), *code = number(95), *high = number(25), *low = number(12);
	auto *dates = array({text("2026-07-15"),text("2026-07-16"),text("2026-07-17")});
	test_json_root = object({
		{"current",object({{"temperature_2m",temperature},{"weather_code",code},{"time",text("2026-07-15T14:15")}})},
		{"daily",object({{"time",dates},{"weather_code",array({number(0),number(45),number(85)})},
			{"temperature_2m_max",array({high,number(24),number(18)})},
			{"temperature_2m_min",array({low,number(13),number(9)})}})}
	});
	WeatherReply reply; reply.revision = 42;
	assert(network_weather_parse("{}", 2, reply) && reply.ok && reply.count == 3 && reply.revision == 42);
	assert(reply.days[0].temp == 22 && reply.days[0].icon == 3 && reply.days[0].day == 3);
	assert(reply.days[1].icon == 7 && reply.days[2].icon == 2 && std::string(reply.time) == "14:15");
	temperature->valuedouble = std::numeric_limits<double>::quiet_NaN(); assert(!network_weather_parse("{}", 2, reply));
	temperature->valuedouble = 100; assert(!network_weather_parse("{}", 2, reply));
	temperature->valuedouble = 20; code->valuedouble = 4; assert(!network_weather_parse("{}", 2, reply));
	code->valuedouble = 1.5; assert(!network_weather_parse("{}", 2, reply));
	code->valuedouble = 1; high->valuedouble = 5; assert(!network_weather_parse("{}", 2, reply));
	high->valuedouble = 25; dates->items[0]->valuestring = "2026-02-30"; assert(!network_weather_parse("{}", 2, reply));
	dates->items[0]->valuestring = "2026-07-15"; dates->items.pop_back(); assert(!network_weather_parse("{}", 2, reply));
}
static void locations() {
	auto *name = text("Berlin"), *latitude = number(52.52437);
	auto *location = object({{"name",name},{"admin1",text("Berlin")},{"country_code",text("DE")},
		{"latitude",latitude},{"longitude",number(13.41053)}});
	auto *items = array({location}); test_json_root = object({{"results",items}});
	WeatherReply reply; reply.search = true;
	assert(network_weather_parse("{}", 2, reply) && reply.count == 1);
	assert(reply.locations[0].latitude == 52524370 && reply.locations[0].longitude == 13410530);
	name->valuestring = "bad\nname"; assert(!network_weather_parse("{}", 2, reply));
	name->valuestring = "Berlin"; latitude->valuedouble = 91; assert(!network_weather_parse("{}", 2, reply));
	latitude->valuedouble = 52; items->items.assign(6, location); assert(!network_weather_parse("{}", 2, reply));
	test_json_root = object({}); assert(network_weather_parse("{}", 2, reply) && reply.count == 0);
	test_json_root = object({{"error",node(5)}}); assert(!network_weather_parse("{}", 2, reply));
}
int main() {
	forecasts(); locations();
	assert(network_weather_icon(0) == 1 && network_weather_icon(2) == 0 && network_weather_icon(3) == 4);
	assert(network_weather_icon(51) == 3 && network_weather_icon(71) == 2 && network_weather_icon(99) == 3);
	assert(network_weather_icon(-1) < 0 && network_weather_icon(4) < 0 && network_weather_icon(100) < 0);
	WeatherReply reply; unsigned parses = test_json_parses;
	std::string deep(17, '['); deep += std::string(17, ']');
	assert(!network_weather_parse(deep.c_str(), deep.size(), reply) && test_json_parses == parses);
	std::string large(8193, ' '); assert(!network_weather_parse(large.c_str(), large.size(), reply));
	assert(!network_weather_parse(nullptr, 0, reply));
	std::cout << "Provider typed fields, WMO mapping, bounds, impossible dates, nulls and nesting limits passed (not HTTP/TLS/JSON syntax)\n";
}

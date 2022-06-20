#include <Arduino.h>
#include "FastLED.h"
#include "BLEDevice.h"
#include "WiFi.h"
#include "blufi.h"
#include "RpcServer.h"
#include <atomic>
#include "mdns.h"

#include "util.h"

#define NUM_DIGITS 2
#define NUM_SEGMENTS_PER_DIGIT 7
#define NUM_LEDS_PER_SEGMENT 12
#define NUM_LEDS_PER_DIGIT NUM_SEGMENTS_PER_DIGIT *NUM_LEDS_PER_SEGMENT
#define NUM_LEDS NUM_DIGITS *NUM_SEGMENTS_PER_DIGIT *NUM_LEDS_PER_SEGMENT

#define NUM_HTTP_HANDLERS 9

const uint8_t ZERO[] = {0, 1, 2, 4, 5, 6};
const uint8_t ONE[] = {0, 4};
const uint8_t TWO[] = {0, 1, 3, 5, 6};
const uint8_t THREE[] = {0, 1, 3, 4, 5};
const uint8_t FOUR[] = {0, 2, 3, 4};
const uint8_t FIVE[] = {1, 2, 3, 4, 5};
const uint8_t SIX[] = {1, 2, 3, 4, 5, 6};
const uint8_t SEVEN[] = {0, 1, 4};
const uint8_t EIGHT[] = {0, 1, 2, 3, 4, 5, 6};
const uint8_t NINE[] = {0, 1, 2, 3, 4, 5};
const uint8_t DIGIT_LENGTHS[] = {
	sizeof(ZERO) / sizeof(ZERO[0]),
	sizeof(ONE) / sizeof(ONE[0]),
	sizeof(TWO) / sizeof(TWO[0]),
	sizeof(THREE) / sizeof(THREE[0]),
	sizeof(FOUR) / sizeof(FOUR[0]),
	sizeof(FIVE) / sizeof(FIVE[0]),
	sizeof(SIX) / sizeof(SIX[0]),
	sizeof(SEVEN) / sizeof(SEVEN[0]),
	sizeof(EIGHT) / sizeof(EIGHT[0]),
	sizeof(NINE) / sizeof(NINE[0])};
const uint8_t *DIGITS[] = {ZERO, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE};

CRGB leds[NUM_LEDS];
httpd_handle_t http_server;
RpcServer rpc;

std::atomic<uint> count;
CRGB color;
std::atomic<ushort> brightness;
std::atomic<bool> needUpdate;

extern const uint8_t index_start[] asm("_binary_index_htm_start");
extern const uint8_t index_end[] asm("_binary_index_htm_end");
const size_t index_size = (index_end - index_start);

void draw_number(int number);
esp_err_t InitWeb();
esp_err_t InitRpc();
esp_err_t InitMdns();

void setup()
{
	//See https://github.com/espressif/arduino-esp32/issues/4732
	#ifdef INADDR_NONE
	#undef INADDR_NONE
	#define INADDR_NONE ((u32_t)0x0UL)
	#endif
	WiFi.config(INADDR_NONE ,INADDR_NONE ,INADDR_NONE );

	Serial.begin(115200);
	BLEDevice::init("7 Segment Counter");
	WiFi.begin();
	InitWeb();
	InitRpc();
	InitMdns();
	Blufi::init();
	FastLED.addLeds<WS2812B, 2, GRB>(leds, NUM_LEDS);

	count.store(0);
	color = CRGB::Red;
	brightness = 127;
}

esp_err_t InitWeb(){
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.max_uri_handlers = NUM_HTTP_HANDLERS;
    
    if(auto res = httpd_start(&http_server, &config)){
		ESP_LOGE("InitWeb", "httpd_start failed with code %d\n", res);
		return res;
	}

	httpd_uri_t cfg = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = [](httpd_req_t *req) -> esp_err_t { 
			httpd_resp_send(req, (const char*) index_start, index_size);
			return ESP_OK;
			},
        .user_ctx = NULL
    };

    return httpd_register_uri_handler(http_server, &cfg);
}

esp_err_t InitRpc(){
	rpc = RpcServer("/rpc/", http_server);
	auto res = rpc.Start(5);
	if(res != ESP_OK){
		Serial.println("RPC Server failed to start");
	}else{
		//rpc handler that increments the displayed count by 1
		rpc.RegisterHandler("up", HTTP_POST, [](rpc_request *req, char *resp, size_t len) {
			snprintf(resp, len, "%02d", ++count);
			needUpdate.store(true);
			return ESP_OK;
		});
		//rpc handler that decrements the displayed count by 1
		rpc.RegisterHandler("down", HTTP_POST, [](rpc_request *req, char *resp, size_t len) {
			if(count != 0){
				count--;
				needUpdate.store(true);
			}
			snprintf(resp, len, "%02d", count.load());
			return ESP_OK;
		});
		rpc.RegisterHandler("count", HTTP_GET, [](rpc_request *req, char *resp, size_t len){
			snprintf(resp, len, "%02d", count.load());
			return ESP_OK;
		});
		rpc.RegisterHandler("reset", HTTP_POST, [](rpc_request *req, char *resp, size_t len) {
			count.store(0);
			snprintf(resp, len, "%02d", 0);
			needUpdate.store(true);
			return ESP_OK;
		});
		//rpc handler that allows setting led brightness
		rpc.RegisterPropertyHandler<ushort>("brightness", 
			[]() -> ushort{ return brightness.load(); }, 
			[](ushort value) {
			brightness.store(value);
			needUpdate.store(true);
			return ESP_OK;
		});

		rpc.RegisterPropertyHandler<long>("color", 
			[]() -> long { 
				return CrgbToLong(color);
			}, 
			[](long value) {
				color = CRGB(value);
				needUpdate.store(true);
				return ESP_OK;
			});
	}
	return res;
}
// A function that separates an integer into its digits and stores them
// in a heap allocated array from least to most significant
void separate_digits(int number, uint8_t *digits)
{
	for (int i = 0; i < NUM_DIGITS; i++)
	{
		digits[i] = number % 10;
		number /= 10;
	}
}

// A function that draws a series of digits to each of the LED digits
void draw_digits(uint8_t *dig)
{
	for (int i = 0; i < NUM_DIGITS; i++)
	{
		auto segments = DIGITS[dig[i]];
		auto offset = NUM_LEDS_PER_DIGIT * i;
		for (int seg = 0; seg < DIGIT_LENGTHS[dig[i]]; seg++)
		{
			for (int led = 0; led < NUM_LEDS_PER_SEGMENT; led++)
			{
				leds[offset + segments[seg] * NUM_LEDS_PER_SEGMENT + led] = color;
			}
		}
	}
}

void draw_number(int number)
{
	uint8_t digits[NUM_DIGITS];
	separate_digits(number, digits);
	draw_digits(digits);
}

esp_err_t InitMdns(){
    if(auto err = mdns_init()){
        log_e("MDNS Failed to initialize: %d", err);
        return err;
    }

    mdns_hostname_set("7 Segment Counter");
    mdns_instance_name_set("7 Segment Counter V1.0");

    mdns_service_add(NULL, "_7segrpc", "_tcp", 80, NULL, 0);

    return ESP_OK;
}

void loop()
{
	static int lastCount;
	static byte lastBrightness;

	if(needUpdate.load()){
		needUpdate.store(false);

		if(lastBrightness != brightness.load())
		{
			lastBrightness = brightness.load();
			ESP_LOGI("Brightness", "%d", lastBrightness);
			FastLED.setBrightness(lastBrightness);
		}

		if(lastCount != count.load())
		{
			FastLED.clear();
			lastCount = count.load();
		}

		draw_number(lastCount);
		FastLED.show();
	}
	delay(100);	
}
/** @brief Native callback harness; does not exercise the core's HTTP parser. */
#pragma once
#include <WiFi.h>
#include <functional>
#include <map>
enum HTTPUploadStatus { UPLOAD_FILE_START, UPLOAD_FILE_WRITE, UPLOAD_FILE_END, UPLOAD_FILE_ABORTED };
enum HTTPMethod { HTTP_GET, HTTP_POST };
struct HTTPUpload {
	HTTPUploadStatus status = UPLOAD_FILE_START;
	String filename, name;
	size_t currentSize = 0;
	uint8_t buf[1436] = {};
};
class WebServer {
public:
	inline static WebServer *instance = nullptr;
	std::function<void()> get, post, upload_callback, pending;
	std::map<std::string, String> headers;
	std::map<std::string, std::function<void()>> routes;
	HTTPUpload file;
	String host = "192.168.4.1", body;
	String request_body;
	std::map<std::string, String> arguments;
	int code = 0;
	bool listening = false, stopped = false;
	IPAddress local_address{192, 168, 4, 1};
	IPAddress localIP() { return local_address; }
	WebServer(int) { instance = this; }
	WebServer(IPAddress, int) { instance = this; }
	String hostHeader() { return host; }
	String header(const char *key) { return headers[key]; }
	String arg(const char *key) { return std::string(key) == "plain" ? request_body : arguments[key]; }
	bool hasArg(const char *key) { return arguments.count(key) != 0; }
	int args() { return int(arguments.size()); }
	HTTPUpload &upload() { return file; }
	WebServer &client() { return *this; }
	void stop() { listening = false; stopped = true; }
	void collectHeaders(const char **, size_t) {}
	void sendHeader(const char *, const char *) {}
	void send(int value, const char *, const String &text) { code = value; body = text; }
	void on(const char *path, HTTPMethod method, std::function<void()> fn) {
		routes[std::string(method == HTTP_GET ? "GET " : "POST ") + path] = fn;
		if (std::string(path) == "/" && method == HTTP_GET) { get = fn; }
		if (std::string(path) == "/update" && method == HTTP_POST) { post = fn; }
	}
	void on(const char *, HTTPMethod, std::function<void()> fn, std::function<void()> upload) {
		post = fn; upload_callback = upload;
	}
	void onNotFound(std::function<void()>) {}
	void begin() { listening = true; stopped = false; }
	void handleClient() { if (pending) { auto fn = pending; pending = nullptr; fn(); } }
};

#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <optional>

/**
* @namespace Zia
* Zia API
*/
namespace Zia {

/*! \mainpage 
 * 
 * ZIA ARCHITECTURE
 * Zia is a modular HTTP server.
 * This interface is meant to make the modules.
 * This description is meant to explain the relationship between module kinds.
 * 
 * I - ILogger
 * The server accepts an arbitrary amount of loggers in conf. Upon a logged line, all loggers will be
 * called with such data as parameter.
 * 
 * II - IConnectionWrapper
 * The client connects to the server. A IConnection marked as default is created. If a single
 * IConnectionWrapper module is activated by conf, a new connection will be created around the base
 * connection using this module. The resulting connection becomes the default connection for
 * reading / writing from the client.
 * 
 * III - IParser
 * As the client connects, a parser instance is created from the parser module selected by configuration.
 * The server will regularely call IParser::IInstance::parse() to parse available bytes and make the parser
 * pass requests to the handlers.
 * 
 * IV - IHandler
 * The server accepts an arbitrary amount of handlers in conf. Each handler has a priority that can be overriden in conf.
 * When a request is received, the server tries to resolve it by calling IHandler::gotRequest on each handler. On the first
 * non-std::nullopt response, the request is marked as resolved and is written on the default connection.
 * 
 * V - ISniffer
 * The server accepts an arbitrary amount of sniffers in conf. Each sniffer will be called at each received request with
 * the request. Each sniffer will be called at each resolved request with the response and the associated request.
 * 
 */

/**
* @interface IInput
* Non-blocking input stream.
*/
class IInput
{
public:
	virtual ~IInput(void) = default;

	/**
	* @fn read
	* Read available bytes on the stream
	* @param size_t buf_size: available bytes count at buf
	* @param char *buf: buffer to write stream available bytes to
	* @return size_t: the amount of bytes polled from the stream and written into buf
	*/
	virtual size_t read(size_t buf_size, char *buf) = 0;
};

/**
* @interface IOutput
* Non-blocking output stream.
*/
class IOutput
{
public:
	virtual ~IOutput(void) = default;

	/**
	* @fn write
	* Write bytes on the stream.
	* @param size_t buf_size: available bytes count at buf
	* @param char *buf: buffer with bytes that will be written on the stream
	* @return size_t: the amount of bytes actually written on the stream
	* @note This function can very well return 0 if no bytes are available on stream for writing,
	* even if buf_size is superior to 0. It is reasonnable to keep trying to write those bytes
	* until a strictly positive response is returned, as writing availability should come in at any time
	* in the near future. Also, assume that this availability might never actually come so you should
	* also take that into consideration.
	*/
	virtual size_t write(size_t buf_size, char *buf) = 0;	
};

/**
* @interface IInputOutput
* Non-blocking input/output stream.
*/
class IInputOutput : public IInput, public IOutput
{
public:
	virtual ~IInputOutput(void) override = default;
};

/**
* @interface ILogger
* Abstract logging system.
*/
class ILogger
{
public:
	virtual ~ILogger(void) = default;

	/**
	* @fn log
	* Log a string to the logging stream.
	* @param const std::string &str: the string to log
	*/
	virtual void log(const std::string &str) = 0;
};

/**
* @interface IConnection
* Represents a connection to the server.
* The connection has a logging stream associated with it.
*/
class IConnection : public IInputOutput, public ILogger
{
public:
	virtual ~IConnection(void) override = default;
};

/**
* @struct Request
* Represents a HTTP 1.1 request.
*/
struct Request
{
	/**
	 * @var data
	 * Request raw data.
	 */
	std::vector<char> data;

	/**
	 * @var lines
	 * The request, line by line.
	 */
	std::vector<std::string> lines;

	/**
	 * @var options
	 * Request options by key-value. One pair by line when a colon is detected.
	 * ex: `{{"Connection", "keep-alive"}}`
	 */
	std::map<std::string, std::string> options;

	/**
	* @enum Method
	* HTTP 1.1 methods.
	*/
	enum class Method {
		Options,
		Get,
		Head,
		Post,
		Put,
		Delete,
		Trace,
		Connect,
		Patch,
		Link,
		Unlink
	};

	/**
	 * @var method
	 * Request's HTTP method
	 */
	Method method;

	/**
	 * @var url
	 * Request's url.
	 * ex: `"/login.html?username=John&pasword=sample_pass"`
	 */
	std::string url;

	/**
	 * @var path
	 * Request's path, without arguments.
	 * ex: `"/login.html"`
	 */
	std::string path;

	/**
	 * @var arguments
	 * Request's arguments, decoded from the url.
	 * ex: `{{"username", "John"}, {"password", "sample_pass"}}`
	 */
	std::map<std::string, std::string> arguments;

	/**
	 * @var protocol
	 * Request's protocol.
	 * ex: `"HTTP/1.1"`
	 */
	std::string protocol;

	/**
	 * @var host
	 * Request's host.
	 * ex: `"localhost:5000"`
	 */
	std::string host;

	/**
	 * @var userAgent
	 * Request's User-Agent.
	 * ex: `"Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:84.0) Gecko/20100101 Firefox/84.0"`
	 */
	std::string userAgent;

	/**
	* @struct MediaRange
	* An accepted media kind.
	*/
	struct MediaRange
	{
		/**
		 * @var type
		 * For the following, consider [empty] as non-existent 7 characters.
		 * ex: `"*[empty]/[empty]*"`
		 */
		std::string type;

		/**
		 * @var quality
		 * `0.0` to `1.0`
		 */
		double quality;

		/**
		 * @var extension
		 * User-defined media-range data.
		 */
		std::map<std::string, std::string> extension;
	};

	/**
	 * @var accept
	 * Request's Accept.
	 */
	std::vector<MediaRange> accept;

	/**
	* @struct LanguageRange
	* An accepted language.
	*/
	struct LanguageRange
	{
		/**
		 * @var quality
		 * ex: `"en-US"`
		 */
		std::string language;

		/**
		 * @var quality
		 * `0.0` to `1.0`
		 */
		double quality;
	};

	/**
	 * @var acceptLanguage
	 * Request's Accept-Language.
	 */
	std::vector<LanguageRange> acceptLanguage;

	/**
	* @struct Codings
	* An accepted encoding.
	*/
	struct Codings
	{
		/**
		 * @var contentCoding
		 * ex: `"gzip"`
		 */
		std::string contentCoding;

		/**
		 * @var quality
		 * `0.0` to `1.0`
		 */
		double quality;
	};

	/**
	 * @var acceptEncoding
	 * Request's Accept-Encoding.
	 */
	std::vector<Codings> acceptEncoding;

	/**
	 * @var closeConnection
	 * Request's Connection.
	 * `true` if connection must be closed after response (`options["Connection"] == "close"`).
	 * `false` if connection must be kept alive.
	 */
	bool closeConnection;

	/**
	 * @var upgradeInsecureRequests
	 * Request's UpgradeInsecureRequests.
	 * @see https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Upgrade-Insecure-Requests
	 */
	bool upgradeInsecureRequests;

	/**
	* @interface IEmitter
	* Represents an incoming requests receiver.
	*/
	class IEmitter
	{
	public:
		virtual ~IEmitter(void) = default;

		/**
		* @fn emit
		* Emit a request.
		* @param const Request &request: the emitted request
		*/
		virtual void emit(const Request &request) = 0;
	};
};

/**
* @struct Response
* Represents a HTTP 1.1 response.
* No formatting is required, to allow maximum flexibility.
*/
struct Response
{
	std::vector<char> data;
};

/**
* @interface IConf
* Represents a configuration handler for a given entity.
* Configuration can be read (raw bytes) and written (raw bytes + format hint).
*/
class IConf
{
public:
	virtual ~IConf(void) = default;

	/**
	* @fn read
	* Read the configuration.
	* @return std::vector<char>: configuration data
	* @note If the configuration if read and there was no precedent write to it,
	* this method returns an empty vector.
	*/
	virtual std::vector<char> read(void) const = 0;

	/**
	* @enum Type
	* Represents a configuration format.
	*/
	enum class Type {
		/**
		 * @var Undefined
		 * Binary format, should be stored in a discrete file.
		 * This format is not recommended.
		 */
		Undefined,
		/**
		 * @var Json
		 * JavaScript Object Notation.
		 * This is the preferred configuration format, and all entities should use it for consistency.
		 */
		Json,
		/**
		 * @var Xml
		 * Extensible Markup language.
		 * C-tier format.
		 */
		Xml,
		/**
		 * @var Ini
		 * Windows 85 configuration format.
		 * You kidding right ? ω-tier format.
		 */
		Ini
	};

	/**
	* @fn write
	* Overwrite the configuration.
	* @param Type type: the type of the configuration data
	* @param const std::vector<char> &data: the data to write to the configuration
	* @note The type is an hint to the server about the configuration format. If the server
	* uses JSON to store its own configuration and this entity also stores its configuration
	* as JSON, the server can reasonably include this entity's configuration into its own
	* configuration file. This is recommended to make the configuration easier to modify.
	*/
	virtual void write(Type type, const std::vector<char> &data) const = 0;
};

/**
* @namespace Module
* All interfaces for modules.
* One interface = one module kind.
*/
namespace Module {

using ILogger = Zia::ILogger;

/**
* @interface IConnectionWrapper
* A module wrapping an existing connection into a new connection.
* Typically used to implement a SSL or TLS layer on top of HTTP.
*/
class IConnectionWrapper
{
public:
	virtual ~IConnectionWrapper(void) = default;

	/**
	* @fn create
	* Create a connection derivative.
	* @param IConnection &connection: the base connection
	* @return std::unique_ptr<IConnection>: the derived connection
	* @note Return value must be destroyed before connection.
	*/
	virtual std::unique_ptr<IConnection> create(IConnection &connection) = 0;
};

/**
* @interface IParser
* Abstract HTTP request parser.
* Will be called with an input stream, this module can.
* emit parsed requests using requestEmitter.
*/
class IParser
{
public:
	virtual ~IParser(void) = default;

	/**
	* @interface IInstance
	* Parser instance, storing parser state and stream / logger / request emitter.
	*/
	class IInstance
	{
	public:
		virtual ~IInstance(void) = default;

		/**
		* @fn parse
		* Parse incoming requests from available bytes on input stream.
		* @note The stream / logger / request emitter are implicitely referenced on construction.
		*/
		virtual void parse(void) = 0;
	};

	/**
	* @fn create
	* Create a parser instance with input, logger and request receiver.
	* @param IInput &input: the input stream
	* @param ILogger &logger: the logger associated with the input stream
	* @param Request::IEmitter &requestEmitter: the emitter where parsed requests should go
	* @return std::unique_ptr<IInstance>: the parser instance associated with such objects.
	*/
	virtual std::unique_ptr<IInstance> create(IInput &input, ILogger &logger, Request::IEmitter &requestEmitter) = 0;

};

/**
* @interface IHandler
* A module receiving all unresolved requests,
* the module responds by resolving it or do nothing.
*/
class IHandler
{
public:
	virtual ~IHandler(void) = default;

	/**
	* @fn getAccept
	* Get handler's managed media types.
	* Order in result vector has no incidence on scanning order.
	* Each value in vector is a media type (ex: `text/html`) and a priority.
	* A larger value represents a higher priority.
	* A smaller value represents a lower priority.
	* Priority will impact server's handlers scanning order when a request is received.
	* @return double: the priority of the handler
	*/
	virtual std::vector<std::pair<std::string, double>> getAccept(void) const = 0;

	/**
	* @fn handle
	* Actual handler function. Called when a request is received and is unresolved.
	* @param const Request &request: the incoming request
	* @param ILogger &connectionLogger: the logger object to log stuff
	* @return std::optional<Response>: the response if the request has been handled,
	* otherwise std::nullopt is returned.
	*/
	virtual std::optional<Response> handle(const Request &request, ILogger &connectionLogger) = 0;
};

/**
* @interface ISniffer
* Similar to IHandler, except modules of this kind cannot resolve requests.
* The module can only observe them as they come. This module will receive all
* incoming requests, regardless of whether they can be resolved or not.
* Responses will also be independently observed.
*/
class ISniffer
{
public:
	virtual ~ISniffer(void) = default;

	/**
	* @fn gotRequest
	* Called when a request is received.
	* @param const Request &request: the incoming request
	* @param ILogger &connectionLogger: the logger object to log stuff
	*/
	virtual void gotRequest(const Request &request, ILogger &connectionLogger) = 0;

	/**
	* @fn gotResponse
	* Called when a request is resolved.
	* @param const Request &request: the incoming request
	* @param const Response &response: the resolved response to request
	* @param ILogger &connectionLogger: the logger object to log stuff
	*/
	virtual void gotResponse(const Request &request, const Response &response, ILogger &connectionLogger) = 0;

	/**
	* @fn gotRequestMiss
	* Called when a request couldn't be handled.
	* @param const Request &request: the incoming request
	* @param ILogger &connectionLogger: the logger object to log stuff
	*/
	virtual void gotRequestMiss(const Request &request, ILogger &connectionLogger) = 0;
};

}

}
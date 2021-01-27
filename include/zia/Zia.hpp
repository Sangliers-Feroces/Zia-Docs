#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <optional>
#include <any>

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
 * The server accepts an arbitrary amount of handlers in conf.
 * When a request is received, the server calls all handlers in conf-order.
 * Each handler can modify the response header and response body. When an handler
 * sets a code to a non-2** value, this marks the last handler.
 * After last handler, the response is written to client default connection.
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
* @interface IRequest
* Abstract HTTP request.
*/
class IRequest
{
public:
	virtual ~IRequest(void) = default;

	/**
	* @fn getMethod
	* Get HTTP method for the request. Ex: `"GET"`, `"POST"` or `"DELETE"`.
	* @return const std::string&: the method in caps
	*/
	virtual const std::string& getMethod(void) const = 0;

	/**
	* @fn getFilename
	* Get request filename, without any argument. Ex: `"/login.html"`
	* @return const std::string&: the request filename
	*/
	virtual const std::string& getFilename(void) const = 0;

	/**
	* @fn getArgument
	* Query an argument (key-values trailing in URL). Returns non-null if found, null otherwise.
	* @param const std::string &name: the name of the argument to query
	* @return const std::string*: the optional argument
	*/
	virtual const std::string* getArgument(const std::string &name) const = 0;

	/**
	* @fn getHeader
	* Query a header parameter. Returns non-null if found, null otherwise.
	* @param const std::string &key: the key of the parameter to query. Ex: `"Connection"`
	* @return const std::string*: the optional parameter
	*/
	virtual const std::string* getHeader(const std::string &key) const = 0;

	/**
	* @fn getBody
	* Query request body. Returns non-null if present, null otherwise.
	* @return const std::vector<char>*: the optional buffer containing body data
	*/
	virtual const std::vector<char>* getBody(void) const = 0;

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
		virtual void emit(const IRequest &request) = 0;
	};
};

/**
* @interface IResponse
* Abstract HTTP response.
*/
class IResponse
{
public:
	virtual ~IResponse(void) = default;

	/**
	* @fn setCode
	* Set response status. On non-2**, no handlers will be called after current one.
	* Default code must be 200.
	* @param size_t code: value of the status code to set
	*/
	virtual void setCode(size_t code) = 0;

	/**
	* @fn getHeader
	* Query a header parameter. Returns non-null if found, null otherwise.
	* @param const std::string &key: the key of the parameter to query. Ex: `"Connection"`
	* @return const std::string*: the optional parameter
	*/
	virtual const std::string* getHeader(const std::string &key) const = 0;

	/**
	* @fn setHeader
	* Sets a header parameter.
	* @param const std::string &key: the key of the parameter to set. Ex: `"content-type"`
	* @param const std::string &value: the value of the parameter to set. Ex: `"application/json"`
	*/
	virtual void setHeader(const std::string &key, const std::string &value) const = 0;

	/**
	* @fn getBody
	* Query response body. Returns non-null if present, null otherwise.
	* @return const std::vector<char>*: the optional buffer containing body data
	*/
	virtual const std::vector<char>* getBody(void) const = 0;

	/**
	* @fn setBody
	* Set response body.
	* @param const std::vector<char> &body: the buffer to set for body data
	*/
	virtual void setBody(const std::vector<char> &body) const = 0;
};

/**
* @interface IContext
* Abstract context values. They are stored by std::string keys, value is a std::any.
*/
class IContext
{
public:
	virtual ~IContext(void) = default;

	/**
	* @fn get
	* Get context value by key. Returns non-null if present, null otherwise
	* @param const std::string &key: key of context parameter to retrieve
	* @return const std::any*: the optional context value
	*/
	virtual const std::any* get(const std::string &key) = 0;

	/**
	* @fn set
	* Set context value.
	* @param const std::string &key: key of context parameter to set
	* @param const const std::any &value: value of context parameter to set
	*/
	virtual void set(const std::string &key, const std::any &value) = 0;
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
	* @param ILogger &log: the logger associated with the input stream
	* @param IRequest::IEmitter &requestEmitter: the emitter where parsed requests should go
	* @return std::unique_ptr<IInstance>: the parser instance associated with such objects.
	*/
	virtual std::unique_ptr<IInstance> create(IInput &input, ILogger &log, IRequest::IEmitter &requestEmitter) = 0;
};

/**
* @interface IResponse
* Abstract HTTP handler.
*/
class IHandler
{
public:
	virtual ~IHandler(void) = default;

	/**
	* @fn handle
	* Handle request.
	* @param const IRequest &req: the original request
	* @param IResponse &res: the under-construction response
	* @param IContext &ctx: the request-associated context
	* @param ILogger &log: the client-associated logger
	*/
	virtual void handle(const IRequest &req, IResponse &res, IContext &ctx, ILogger &log) = 0;
};

}

}
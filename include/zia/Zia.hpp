#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <map>
#include <optional>
#include <any>
#include <type_traits>
#include <tuple>

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
 * calls abortPipeline on the request, this marks the last handler and no more handler
 * will be called in the handlers pipeline.
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
	* @param const char *buf: buffer with bytes that will be written on the stream
	* @return size_t: the amount of bytes actually written on the stream
	* @note This function can very well return 0 if no bytes are available on stream for writing,
	* even if buf_size is superior to 0. It is reasonnable to keep trying to write those bytes
	* until a strictly positive response is returned, as writing availability should come in at any time
	* in the near future. Also, assume that this availability might never actually come so you should
	* also take that into consideration.
	*/
	virtual size_t write(size_t buf_size, const char *buf) = 0;
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

	/**
	* @fn getRemoteIP
	* Get the IP of the other end of the connection. Ex: `"192.168.1.90"`.
	* @return const std::string&: the remote IP
	*/
	virtual const std::string& getRemoteIP(void) const = 0;
	
	/**
	* @fn getNativeSocket
	* Return the native socket file descriptor
	* @return int: the file descriptor
	*/
	virtual int getNativeSocket(void) const = 0;
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
	* @fn getClientIP
	* Get client IP. Ex: `"192.168.1.90"`.
	* @return const std::string&: the client IP
	*/
	virtual const std::string& getClientIP(void) const = 0;

	/**
	* @fn getMethod
	* Get HTTP method for the request. Ex: `"GET"`, `"POST"` or `"DELETE"`.
	* @return const std::string&: the method in caps
	*/
	virtual const std::string& getMethod(void) const = 0;

	/**
	* @fn getProtocol
	* Get HTTP protocol for the request. Ex: `"HTTP/1.1"`.
	* @return const std::string&: the protocol as a string
	*/
	virtual const std::string& getProtocol(void) const = 0;

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
	* @fn getArgumentsKeys
	* Get keys for all arguments. Those keys can be used in any call to `getArgument`.
	* @return std::vector<std::string>: the optional argument
	*/
	virtual std::vector<std::string> getArgumentsKeys(void) const = 0;

	/**
	* @fn getURL
	* Get full URL, with arguments. Ex: `"/login.html?username=John&password=12345678"`
	* @return const std::string&: the protocol as a string
	*/
	virtual const std::string& getURL(void) const = 0;

	/**
	* @fn getHeader
	* Query a header parameter. Returns non-null if found, null otherwise.
	* @param const std::string &key: the key of the parameter to query. Ex: `"Connection"`
	* @return const std::string*: the optional parameter
	*/
	virtual const std::string* getHeader(const std::string &key) const = 0;

	/**
	* @fn getHeaderKeys
	* Query header keys. Those keys can be used in any call to `getHeader`.
	* @return const std::string*: the header keys
	*/
	virtual std::vector<std::string> getHeaderKeys(void) const = 0;

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
	* @fn abortPipeline
	* Stop handlers traversal. After current handler, no more handler will be called for current response.
	*/
	virtual void abortPipeline(void) = 0;

	/**
	* @fn getCode
	* Get response status.
	* Default code must be 200.
	* @return size_t: current value of the status code
	*/
	virtual size_t getCode(void) const = 0;

	/**
	* @fn setCode
	* Set response status.
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
	virtual void setHeader(const std::string &key, const std::string &value) = 0;

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
	virtual void setBody(const std::vector<char> &body) = 0;
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
	virtual const std::any* get(const std::string &key) const = 0;

	/**
	* @fn set
	* Set context value.
	* @param const std::string &key: key of context parameter to set
	* @param const const std::any &value: value of context parameter to set
	*/
	virtual void set(const std::string &key, const std::any &value) = 0;
};

/**
* @namespace Json
* JSON-related types
*/
namespace Json {

/**
* @typedef Number
* JSON number
*/
using Number = double;

/**
* @typedef Integer
* JSON integer
* @note will be 64-bit wide on 64-bit platforms, 32-bit wide on 32-bit platforms and long bytes otherwise.
*/
using Integer = std::conditional_t<sizeof(void*) == 8, int64_t, std::conditional_t<sizeof(void*) == 4, int32_t, long>>;

/**
* @typedef String
* JSON string
*/
using String = std::string;

/**
* @typedef Bool
* JSON bool
*/
using Bool = bool;

class IObject;
class IArray;

/**
* @class IContainer
* Abstract container containing various JSON elements.
* @param typename ...Args: the keys used to address objects within the container.
* @note In case of a map, Args might be const std::string&,
* in case of an array it might be size_t, in case of a stack / queue it might be nothing.
* @note For all methods starting with `set`, any previous reference to the value at such key
* will be invalidated.
*/
template <typename ...Args>
class IContainer
{
public:
	virtual ~IContainer(void) = default;

	/**
	* @fn getNumber
	* Get a number for a certain key. Returns std::nullopt when not found or type not matching,
	* and the corresponding Json::Number otherwise.
	* @param Args ...args: the key to address the number in the container
	* @return std::optional<Json::Number>: the optional number
	*/
	virtual std::optional<Json::Number> getNumber(Args ...args) const = 0;

	/**
	* @fn getInteger
	* Get an integer for a certain key. Returns std::nullopt when not found or type not matching,
	* and the corresponding Json::Integer otherwise.
	* @param Args ...args: the key to address the integer in the container
	* @return std::optional<Json::Integer>: the optional integer
	*/
	virtual std::optional<Json::Integer> getInteger(Args ...args) const = 0;

	/**
	* @fn getString
	* Get a string for a certain key. Returns std::nullopt when not found or type not matching,
	* and the corresponding Json::String otherwise.
	* @param Args ...args: the key to address the string in the container
	* @return std::optional<Json::String>: the optional string
	*/
	virtual std::optional<Json::String> getString(Args ...args) const = 0;

	/**
	* @fn getBool
	* Get a bool for a certain key. Returns std::nullopt when not found or type not matching,
	* and the corresponding Json::Bool otherwise.
	* @param Args ...args: the key to address the bool in the container
	* @return std::optional<Json::Bool>: the optional bool
	*/
	virtual std::optional<Json::Bool> getBool(Args ...args) const = 0;

	/**
	* @fn isNull
	* Check whether an element in the container is null or not.
	* Returns true if the value is null and false otherwise.
	* @param Args ...args: the key referencing the checked value
	* @return bool: the test result
	*/
	virtual bool isNull(Args ...args) const = 0;

	/**
	* @fn isUndefined
	* Check whether an element in the container is undefined or not.
	* Returns true if the value is undefined and false otherwise.
	* @param Args ...args: the key referencing the checked value
	* @return bool: the test result
	*/
	virtual bool isUndefined(Args ...args) const = 0;

	/**
	* @fn getObject
	* Get a object for a certain key. Returns nullptr when not found or type not matching,
	* and the corresponding pointer to Json::IObject otherwise.
	* @param Args ...args: the key to address the object in the container
	* @return IObject*: the optional object
	*/
	virtual const IObject* getObject(Args ...args) const = 0;

	/**
	* @fn getArray
	* Get an array for a certain key. Returns nullptr when not found or type not matching,
	* and the corresponding pointer to Json::IArray otherwise.
	* @param Args ...args: the key to address the array in the container
	* @return IArray*: the optional array
	*/
	virtual const IArray* getArray(Args ...args) const = 0;

	/**
	* @fn setNumber
	* Set a number at a certain location in the container.
	* @param Args ...args: the key in which the number will be inserted
	* @param Number value: the value to insert
	*/
	virtual void setNumber(Args ...args, Number value) = 0;

	/**
	* @fn setInteger
	* Set an integer at a certain location in the container.
	* @param Args ...args: the key in which the integer will be inserted
	* @param Integer value: the value to insert
	*/
	virtual void setInteger(Args ...args, Integer value) = 0;

	/**
	* @fn setString
	* Set a string at a certain location in the container.
	* @param Args ...args: the key in which the string will be inserted
	* @param const String &value: the value to insert
	*/
	virtual void setString(Args ...args, const String &value) = 0;

	/**
	* @fn setBool
	* Set a bool at a certain location in the container.
	* @param Args ...args: the key in which the bool will be inserted
	* @param Bool value: the value to insert
	*/
	virtual void setBool(Args ...args, Bool value) = 0;

	/**
	* @fn setNull
	* Set the value at certain location in the container to null.
	* @param Args ...args: the key in which the null will be inserted
	*/
	virtual void setNull(Args ...args) = 0;

	/**
	* @fn setUndefined
	* Set the value at certain location in the container to undefined.
	* @param Args ...args: the key in which the undefined will be inserted
	*/
	virtual void setUndefined(Args ...args) = 0;

	/**
	* @fn getObject
	* Get a object for a certain key. Returns nullptr when not found or type not matching,
	* and the corresponding pointer to Json::IObject otherwise.
	* @param Args ...args: the key to address the object in the container
	* @return IObject*: the optional object
	*/
	virtual IObject* getObject(Args ...args) = 0;

	/**
	* @fn getArray
	* Get an array for a certain key. Returns nullptr when not found or type not matching,
	* and the corresponding pointer to Json::IArray otherwise.
	* @param Args ...args: the key to address the array in the container
	* @return IArray*: the optional array
	*/
	virtual IArray* getArray(Args ...args) = 0;

	/**
	* @fn createEmptyObject
	* Create an empty object at a certain location in the container.
	* @param Args ...args: the key in which the empty object will be inserted
	* @return IObject&: the reference to the created object
	*/
	virtual IObject& createEmptyObject(Args ...args) = 0;

	/**
	* @fn createEmptyArray
	* Create an empty array at a certain location in the container.
	* @param Args ...args: the key in which the empty array will be inserted
	* @return IArray&: the reference to the created array
	*/
	virtual IArray& createEmptyArray(Args ...args) = 0;

	/**
	* @fn clear
	* Remove all values in the container.
	* @note All references / pointers to values retrieved from this object
	* will be invalidated.
	*/
	virtual void clear(void) = 0;
};

/**
* @class IObject
* Abstract JSON object.
* @note The key type is `const std::string&`, which means the function to implement
* for `setString` is `void setString(const std::string &key, const String &value) override`.
*/
class IObject : public IContainer<const std::string&>
{
public:
	virtual ~IObject(void) = default;

	/**
	* @fn getKeys
	* Get all the keys referencing each value in the object.
	* @return std::vector<std::string>: the keys
	* @note A value is returned instead of a reference. This design assumes this method
	* will not be called frequently, and we can reasonably create from scratch the response.
	* It also lets the implementation not to maintain the same object with the keys.
	*/
	virtual std::vector<std::string> getKeys(void) const = 0;
};

/**
* @class IObject
* Abstract JSON array.
* @note The key type is `size_t`, which means the function to implement
* for `setString` is `void setString(size_t index, const String &value) override`.
* Set values will insert the value in the array so that the inserted value is located at the specified index.
* Setting undefined will not change the index of any element in the array.
*/
class IArray : public IContainer<size_t>
{
public:
	virtual ~IArray(void) = default;

	/**
	* @fn getSize
	* Get the size of the array.
	* @return size_t: the size of the array
	*/
	virtual size_t getSize(void) const = 0;

	/**
	* @fn remove
	* Remove a value at a certain index. This will decrement the index of all values
	* with an index superior to index.
	* @param size_t index: the index of the value to remove
	*/
	virtual void remove(size_t index) = 0;
};

}

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
	* @return std::unique_ptr<Json::IObject>: configuration data
	* @note If the configuration if read and there was no precedent write to it,
	* this method returns an empty object.
	*/
	virtual std::unique_ptr<Json::IObject> read(void) const = 0;

	/**
	* @fn write
	* Overwrite the configuration.
	* @param const Json::IObject &jsonObject: the object containing the data to use for conf
	*/
	virtual void write(const Json::IObject &jsonObject) = 0;
};

#if _MSC_VER && !__INTEL_COMPILER
#define ZIA_EXPORT_SYMBOL __declspec(dllexport)
#else
#define ZIA_EXPORT_SYMBOL
#endif

/**
* @namespace Module
* All interfaces for modules.
* One interface = one module kind.
*/
namespace Module {

using ILogger = Zia::ILogger;
using FN_createLogger = Zia::Module::ILogger* (Zia::IConf &conf);

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
using FN_createConnectionWrapper = Zia::Module::IConnectionWrapper* (Zia::IConf &conf);

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
using FN_createParser = Zia::Module::IParser* (Zia::IConf &conf);

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
using FN_createHandler = Zia::Module::IHandler* (Zia::IConf &conf);

}

}
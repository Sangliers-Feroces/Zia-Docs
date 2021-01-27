# Zia-Docs
### API proposal for EPITECH Zia project

This document describes our proposal for the Zia API.
Zia is a modular HTTP server. Each module can be plugged into the core to provide simple and flexible pipeline for HTTP request handling.
This API describes what should be present in each shared library describing each module. See `include/zia/module` folder for symbols of each kind of module.

# Four kinds of modules

## I - ILogger
The server accepts an arbitrary amount of loggers in conf.
Upon a logged line, all loggers will be
called with such data as parameter.

## II - IConnectionWrapper
Optional module in conf.
The client connects to the server. A IConnection marked as default is created. If a single
IConnectionWrapper module is activated by conf, a new connection will be created around the base
connection using this module. The resulting connection becomes the default connection for
reading / writing from the client. Usually used to provide SSL / TLS support.

## III - IParser
Required module in conf, only one can be used at any time.
As the client connects, a parser instance is created from the parser module selected by configuration.
The server will regularely call IParser::IInstance::parse() to parse available bytes and make the parser
pass requests to the handlers.

## IV - IHandler
The server accepts an arbitrary amount of handlers in conf.
When a request is received, the server calls all handlers in conf-order.
Each handler can modify the response header and response body. When an handler
sets a code to a non-2** value (being implicitely set to 200 before first handler), this marks the last handler.
After last handler, the response is written to client default connection.

## Configuration example
### This kind of configuration could be used by the core implementation to specify all used modules

Here, each module is an object containing a "path" string and an optional "conf" object, containing the configuration forwarded to the module on loading.

```json
{
  "loggers": [
    {
      "path": "mod/filelogger",
      "conf": {
        "output": "./log.txt"
      }
    }
  ],
  "connection_wrapper": {
    "path": "mod/ssl"
  },
  "parser": {
    "path": "mod/parser",
    "conf": {
    }
  },
  "handlers": [
    {
      "path": "mod/fileloader",
      "conf": {
        "load": [
          "txt",
          "html",
          "php",
          "js",
          "jpg",
          "png",
          "bmp",
          "wav",
          "mp3",
          "flac"
        ]
      }
    },
    {
      "path": "mod/php"
    },
    {
      "path": "mod/jsminifyr"
    }
  ]
}
```
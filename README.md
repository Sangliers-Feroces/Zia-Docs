# Zia-Docs
### API proposal for EPITECH Zia project

This document describes our proposal for the Zia API.
Zia is a modular HTTP server. Each module can be plugged into the core to provide simple and flexible pipeline for HTTP request handling.
This API describes what should be present in each shared library describing each module. See `include/zia/module` folder for symbols of each kind of module.

# Core flow

Capture of `docs/server_flow.drawio`. Describes context around each module and core implementation example.

![Core flow](https://github.com/Sangliers-Feroces/Zia-Docs/blob/master/docs/server_flow.png)

# Four kinds of modules

## I - `ILogger`
The server accepts an arbitrary amount of loggers in conf.
Upon a logged line, all loggers will be
called with such data as parameter.

## II - `IConnectionWrapper`
Optional module in conf.
The client connects to the server. A IConnection marked as default is created. If a single
IConnectionWrapper module is activated by conf, a new connection will be created around the base
connection using this module. The resulting connection becomes the default connection for
reading / writing from the client. Usually used to provide SSL / TLS support.

## III - `IParser`
Required module in conf, only one can be used at any time.
As the client connects, a parser instance is created from the parser module selected by configuration.
The server will regularely call IParser::IInstance::parse() to parse available bytes and make the parser
pass requests to the handlers.

## IV - `IHandler`
The server accepts an arbitrary amount of handlers in conf.
When a request is received, the server calls all handlers in conf-order.
Each handler can modify the response header and response body. When an handler
calls abortPipeline on the request, this marks the last handler and no more handler
will be called in the handlers pipeline.
After last handler, the response is written to client default connection.

## Configuration example
### This kind of configuration could be used by the core implementation to specify all used modules

Here, each module is an object containing a `"path"` string and an optional `"conf"` object, containing the configuration forwarded to the module on loading.

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

# Class diagram

Capture of `docs/server_module_api.drawio`. Describes classes in `include/zia/Zia.hpp`.

![Class diagram](https://github.com/Sangliers-Feroces/Zia-Docs/blob/master/docs/server_module_api.png)

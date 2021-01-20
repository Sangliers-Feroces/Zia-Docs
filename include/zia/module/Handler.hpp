#pragma once

#include "../Zia.hpp"

/** @file
 * Include that in your Zia::Module::IHandler implementation.
 * Implement createHandler. Put that symbol in a shared lib.
 * Congratulations ! You've got a module.
*/

extern "C" {

/**
* @fn createHandler
* Create a module instance.
* @param Zia::IConf &conf: module file unique configuration entity
* @return std::unique_ptr<Zia::Module::IHandler>: the module instance
* @note The module should use the given configuration object to store its configuration.
* It's not recommanded to write the configuration in a file on the filesystem, as this
* may conflict with other modules and server files.
*/
std::unique_ptr<Zia::Module::IHandler> createHandler(Zia::IConf &conf);

}
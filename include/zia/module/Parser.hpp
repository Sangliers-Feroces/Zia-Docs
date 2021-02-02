#pragma once

#include "../Zia.hpp"

/** @file
 * Include that in your Zia::Module::IParser implementation.
 * Implement createParser. Put that symbol in a shared lib.
 * Congratulations ! You've got a module.
*/

extern "C" {

/**
* @fn createParser
* Create a module instance.
* @param Zia::IConf &conf: module file unique configuration entity
* @return Zia::Module::IParser*: the module instance, created with new. This
* object lifetime is managed by the caller and should be deleted before program termination.
* @note The module should use the given configuration object to store its configuration.
* It's not recommanded to write the configuration in a file on the filesystem, as this
* may conflict with other modules and server files.
*/
ZIA_EXPORT_SYMBOL Zia::Module::IParser* createParser(Zia::IConf &conf);

}
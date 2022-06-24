#pragma once

/*
 * Wrapper header for asio which makes compiler warnings go away
 */

#ifdef _WIN32
#define _WIN32_WINNT 0x0A00
#endif
#define ASIO_STANDALONE
#include <asio.hpp>

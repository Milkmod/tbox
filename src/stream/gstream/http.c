/*!The Treasure Box Library
 * 
 * TBox is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 * 
 * TBox is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with TBox; 
 * If not, see <a href="http://www.gnu.org/licenses/"> http://www.gnu.org/licenses/</a>
 * 
 * Copyright (C) 2009 - 2012, ruki All rights reserved.
 *
 * @author		ruki
 * \http		http.c
 *
 */

/* ///////////////////////////////////////////////////////////////////////
 * includes
 */
#include "prefix.h"
#include "../../asio/asio.h"
#include "../../string/string.h"
#include "../../network/network.h"
#include "../../platform/platform.h"

/* ///////////////////////////////////////////////////////////////////////
 * types
 */

// the http stream type
typedef struct __tb_gstream_http_t
{
	// the base
	tb_gstream_t 		base;

	// the http 
	tb_handle_t 		http;

}tb_gstream_http_t;

/* ///////////////////////////////////////////////////////////////////////
 * implementation
 */
static __tb_inline__ tb_gstream_http_t* tb_gstream_http_cast(tb_gstream_t* gstream)
{
	tb_assert_and_check_return_val(gstream && gstream->type == TB_GSTREAM_TYPE_HTTP, tb_null);
	return (tb_gstream_http_t*)gstream;
}
static tb_long_t tb_gstream_http_open(tb_gstream_t* gstream)
{
	// check
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	tb_assert_and_check_return_val(hstream && hstream->http, -1);

	// the http status
	tb_http_status_t const*	status = tb_http_status(hstream->http);
	tb_assert_and_check_return_val(status, -1);

	// open it
	tb_long_t ok = tb_http_aopen(hstream->http);

	// save state
	gstream->state = ok >= 0? TB_GSTREAM_STATE_OK : status->state;

	// ok?
	return ok;
}
static tb_long_t tb_gstream_http_clos(tb_gstream_t* gstream)
{
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	tb_assert_and_check_return_val(hstream && hstream->http, -1);

	// close it
	return tb_http_aclos(hstream->http);
}
static tb_void_t tb_gstream_http_exit(tb_gstream_t* gstream)
{
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	if (hstream && hstream->http) tb_http_exit(hstream->http);
}
static tb_long_t tb_gstream_http_read(tb_gstream_t* gstream, tb_byte_t* data, tb_size_t size, tb_bool_t sync)
{
	// check
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	tb_assert_and_check_return_val(hstream && hstream->http, -1);

	// the http status
	tb_http_status_t const*	status = tb_http_status(hstream->http);
	tb_assert_and_check_return_val(status, -1);

	// check
	tb_check_return_val(data, -1);
	tb_check_return_val(size, 0);

	// read data
	tb_long_t ok = tb_http_aread(hstream->http, data, size);

	// save state
	gstream->state = ok >= 0? TB_GSTREAM_STATE_OK : status->state;

	// ok?
	return ok;
}
static tb_long_t tb_gstream_http_writ(tb_gstream_t* gstream, tb_byte_t const* data, tb_size_t size, tb_bool_t sync)
{
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	tb_assert_and_check_return_val(hstream && hstream->http, -1);

	// writ data or afwrit data
	return sync? tb_http_afwrit(hstream->http, data, size) : tb_http_awrit(hstream->http, data, size);
}
static tb_hize_t tb_gstream_http_size(tb_gstream_t* gstream)
{
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	tb_assert_and_check_return_val(hstream && hstream->http, 0);

	// status
	tb_http_status_t const* status = tb_http_status(hstream->http);
	tb_assert_and_check_return_val(status, 0);

	// document_size
	return (!status->bgzip && !status->bdeflate)? status->document_size : 0;
}
static tb_long_t tb_gstream_http_seek(tb_gstream_t* gstream, tb_hize_t offset)
{
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	tb_assert_and_check_return_val(hstream && hstream->http, -1);

	// status
	tb_http_status_t const* status = tb_http_status(hstream->http);
	tb_assert_and_check_return_val(status, -1);

	// be able to seek?
	tb_check_return_val(status->bseeked, -1);

	// seek
	return tb_http_aseek(hstream->http, offset);
}
static tb_long_t tb_gstream_http_wait(tb_gstream_t* gstream, tb_size_t wait, tb_long_t timeout)
{
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	tb_assert_and_check_return_val(hstream && hstream->http, -1);

	// the http status
	tb_http_status_t const*	status = tb_http_status(hstream->http);
	tb_assert_and_check_return_val(status, -1);

	// wait
	tb_long_t ok = tb_http_wait(hstream->http, wait, timeout);

	// save state
	gstream->state = ok >= 0? TB_GSTREAM_STATE_OK : status->state;

	// ok?
	return ok;
}
static tb_bool_t tb_gstream_http_ctrl(tb_gstream_t* gstream, tb_size_t ctrl, tb_va_list_t args)
{
	// check
	tb_gstream_http_t* hstream = tb_gstream_http_cast(gstream);
	tb_assert_and_check_return_val(hstream && hstream->http, tb_false);

	// done
	switch (ctrl)
	{
	case TB_GSTREAM_CTRL_SET_URL:
		{
			// url
			tb_char_t const* url = (tb_char_t const*)tb_va_arg(args, tb_char_t const*);
			tb_assert_and_check_return_val(url, tb_false);
		
			// set url
			return tb_http_option(hstream->http, TB_HTTP_OPTION_SET_URL, url);
		}
		break;
	case TB_GSTREAM_CTRL_GET_URL:
		{
			// purl
			tb_char_t const** purl = (tb_char_t const**)tb_va_arg(args, tb_char_t const**);
			tb_assert_and_check_return_val(purl, tb_false);
	
			// get url
			return tb_http_option(hstream->http, TB_HTTP_OPTION_GET_URL, purl);
		}
		break;
	case TB_GSTREAM_CTRL_SET_HOST:
		{
			// host
			tb_char_t const* host = (tb_char_t const*)tb_va_arg(args, tb_char_t const*);
			tb_assert_and_check_return_val(host, tb_false);
	
			// set host
			return tb_http_option(hstream->http, TB_HTTP_OPTION_SET_HOST, host);
		}
		break;
	case TB_GSTREAM_CTRL_GET_HOST:
		{
			// phost
			tb_char_t const** phost = (tb_char_t const**)tb_va_arg(args, tb_char_t const**);
			tb_assert_and_check_return_val(phost, tb_false); 

			// get host
			return tb_http_option(hstream->http, TB_HTTP_OPTION_GET_HOST, phost);
		}
		break;
	case TB_GSTREAM_CTRL_SET_PORT:
		{
			// port
			tb_size_t port = (tb_size_t)tb_va_arg(args, tb_size_t);
			tb_assert_and_check_return_val(port, tb_false);
	
			// set port
			return tb_http_option(hstream->http, TB_HTTP_OPTION_SET_PORT, port);
		}
		break;
	case TB_GSTREAM_CTRL_GET_PORT:
		{
			// pport
			tb_size_t* pport = (tb_size_t*)tb_va_arg(args, tb_size_t*);
			tb_assert_and_check_return_val(pport, tb_false);
	
			// get port
			return tb_http_option(hstream->http, TB_HTTP_OPTION_GET_PORT, pport);
		}
		break;
	case TB_GSTREAM_CTRL_SET_PATH:
		{
			// path
			tb_char_t const* path = (tb_char_t const*)tb_va_arg(args, tb_char_t const*);
			tb_assert_and_check_return_val(path, tb_false);
	
			// set path
			return tb_http_option(hstream->http, TB_HTTP_OPTION_SET_PATH, path);
		}
		break;
	case TB_GSTREAM_CTRL_GET_PATH:
		{
			// ppath
			tb_char_t const** ppath = (tb_char_t const**)tb_va_arg(args, tb_char_t const**);
			tb_assert_and_check_return_val(ppath, tb_false);
	
			// get path
			return tb_http_option(hstream->http, TB_HTTP_OPTION_GET_PATH, ppath);
		}
		break;
	case TB_GSTREAM_CTRL_SET_SSL:
		{
			// bssl
			tb_bool_t bssl = (tb_bool_t)tb_va_arg(args, tb_bool_t);
	
			// set ssl
			return tb_http_option(hstream->http, TB_HTTP_OPTION_SET_SSL, bssl);
		}
		break;
	case TB_GSTREAM_CTRL_GET_SSL:
		{
			// pssl
			tb_bool_t* pssl = (tb_bool_t*)tb_va_arg(args, tb_bool_t*);
			tb_assert_and_check_return_val(pssl, tb_false);
	
			// get ssl
			return tb_http_option(hstream->http, TB_HTTP_OPTION_GET_SSL, pssl);
		}
		break;
	case TB_GSTREAM_CTRL_SET_TIMEOUT:
		{
			// timeout
			tb_size_t timeout = (tb_size_t)tb_va_arg(args, tb_size_t);
			tb_assert_and_check_return_val(timeout, tb_false);
	
			// set timeout
			return tb_http_option(hstream->http, TB_HTTP_OPTION_SET_TIMEOUT, timeout);
		}
		break;
	case TB_GSTREAM_CTRL_GET_TIMEOUT:
		{
			// ptimeout
			tb_size_t* ptimeout = (tb_size_t*)tb_va_arg(args, tb_size_t*);
			tb_assert_and_check_return_val(ptimeout, tb_false);
	
			// get timeout
			return tb_http_option(hstream->http, TB_HTTP_OPTION_GET_TIMEOUT, ptimeout);
		}
		break;
	default:
		break;
	}
	return tb_false;
}

/* ///////////////////////////////////////////////////////////////////////
 * interfaces
 */

tb_gstream_t* tb_gstream_init_http()
{
	// make stream
	tb_gstream_http_t* gstream = (tb_gstream_http_t*)tb_malloc0(sizeof(tb_gstream_http_t));
	tb_assert_and_check_return_val(gstream, tb_null);

	// init stream
	if (!tb_gstream_init((tb_gstream_t*)gstream, TB_GSTREAM_TYPE_HTTP)) goto fail;
	gstream->base.open 	= tb_gstream_http_open;
	gstream->base.clos 	= tb_gstream_http_clos;
	gstream->base.read 	= tb_gstream_http_read;
	gstream->base.writ 	= tb_gstream_http_writ;
	gstream->base.seek 	= tb_gstream_http_seek;
	gstream->base.size 	= tb_gstream_http_size;
	gstream->base.wait 	= tb_gstream_http_wait;
	gstream->base.ctrl 	= tb_gstream_http_ctrl;
	gstream->base.exit 	= tb_gstream_http_exit;
	gstream->http 		= tb_http_init();
	tb_assert_and_check_goto(gstream->http, fail);

	// ok
	return (tb_gstream_t*)gstream;

fail:
	if (gstream) tb_gstream_exit((tb_gstream_t*)gstream);
	return tb_null;
}

tb_gstream_t* tb_gstream_init_from_http(tb_char_t const* host, tb_size_t port, tb_char_t const* path, tb_bool_t bssl)
{
	tb_assert_and_check_return_val(host && port && path, tb_null);

	// init http stream
	tb_gstream_t* gstream = tb_gstream_init_http();
	tb_assert_and_check_return_val(gstream, tb_null);

	// ioctl
	if (!tb_gstream_ctrl(gstream, TB_GSTREAM_CTRL_SET_HOST, host)) goto fail;
	if (!tb_gstream_ctrl(gstream, TB_GSTREAM_CTRL_SET_PORT, port)) goto fail;
	if (!tb_gstream_ctrl(gstream, TB_GSTREAM_CTRL_SET_PATH, path)) goto fail;
	if (!tb_gstream_ctrl(gstream, TB_GSTREAM_CTRL_SET_SSL, bssl)) goto fail;
	
	// ok
	return gstream;

fail:
	if (gstream) tb_gstream_exit(gstream);
	return tb_null;
}

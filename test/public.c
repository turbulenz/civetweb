/* Copyright (c) 2015 the Civetweb developers
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "public.h"
#include <civetweb.h>

#if defined(_WIN32)
#include <Windows.h>
#define mg_Sleep(x) (Sleep(x * 1000))
#else
#include <unistd.h>
#define mg_Sleep(x) (sleep(x))
#endif

/* This unit test file uses the excellent Check unit testing library.
 * The API documentation is available here:
 * http://check.sourceforge.net/doc/check_html/index.html
 */

START_TEST(test_mg_version)
{
	const char *ver = mg_version();
	unsigned major = 0, minor = 0;
	int ret;

	ck_assert(ver != NULL);
	ck_assert_str_eq(ver, CIVETWEB_VERSION);

	/* check structure of version string */
	ret = sscanf(ver, "%u.%u", &major, &minor);
	ck_assert_int_eq(ret, 2);
	ck_assert_uint_ge(major, 1);
}
END_TEST


START_TEST(test_mg_get_valid_options)
{
	int i;
	const struct mg_option *default_options = mg_get_valid_options();

	ck_assert(default_options != NULL);

	for (i = 0; default_options[i].name != NULL; i++) {
		ck_assert(default_options[i].name != NULL);
		ck_assert(strlen(default_options[i].name) > 0);
		ck_assert(((int)default_options[i].type) > 0);
	}

	ck_assert(i > 0);
}
END_TEST


START_TEST(test_mg_get_builtin_mime_type)
{
	ck_assert_str_eq(mg_get_builtin_mime_type("x.txt"), "text/plain");
	ck_assert_str_eq(mg_get_builtin_mime_type("x.html"), "text/html");
	ck_assert_str_eq(mg_get_builtin_mime_type("x.HTML"), "text/html");
	ck_assert_str_eq(mg_get_builtin_mime_type("x.hTmL"), "text/html");
	ck_assert_str_eq(mg_get_builtin_mime_type("/abc/def/ghi.htm"), "text/html");
	ck_assert_str_eq(mg_get_builtin_mime_type("x.unknown_extention_xyz"),
	                 "text/plain");
}
END_TEST


START_TEST(test_mg_strncasecmp)
{
	ck_assert(mg_strncasecmp("abc", "abc", 3) == 0);
	ck_assert(mg_strncasecmp("abc", "abcd", 3) == 0);
	ck_assert(mg_strncasecmp("abc", "abcd", 4) != 0);
	ck_assert(mg_strncasecmp("a", "A", 1) == 0);

	ck_assert(mg_strncasecmp("A", "B", 1) < 0);
	ck_assert(mg_strncasecmp("A", "b", 1) < 0);
	ck_assert(mg_strncasecmp("a", "B", 1) < 0);
	ck_assert(mg_strncasecmp("a", "b", 1) < 0);
	ck_assert(mg_strncasecmp("b", "A", 1) > 0);
	ck_assert(mg_strncasecmp("B", "A", 1) > 0);
	ck_assert(mg_strncasecmp("b", "a", 1) > 0);
	ck_assert(mg_strncasecmp("B", "a", 1) > 0);

	ck_assert(mg_strncasecmp("xAx", "xBx", 3) < 0);
	ck_assert(mg_strncasecmp("xAx", "xbx", 3) < 0);
	ck_assert(mg_strncasecmp("xax", "xBx", 3) < 0);
	ck_assert(mg_strncasecmp("xax", "xbx", 3) < 0);
	ck_assert(mg_strncasecmp("xbx", "xAx", 3) > 0);
	ck_assert(mg_strncasecmp("xBx", "xAx", 3) > 0);
	ck_assert(mg_strncasecmp("xbx", "xax", 3) > 0);
	ck_assert(mg_strncasecmp("xBx", "xax", 3) > 0);
}
END_TEST


START_TEST(test_mg_get_cookie)
{
	char buf[32];
	int ret;
	const char *longcookie = "key1=1; key2=2; key3; key4=4; key5=; key6; "
	                         "key7=this+is+it; key8=8; key9";

	/* invalid result buffer */
	ret = mg_get_cookie("", "notfound", NULL, 999);
	ck_assert_int_eq(ret, -2);

	/* zero size result buffer */
	ret = mg_get_cookie("", "notfound", buf, 0);
	ck_assert_int_eq(ret, -2);

	/* too small result buffer */
	ret = mg_get_cookie("key=toooooooooolong", "key", buf, 4);
	ck_assert_int_eq(ret, -3);

	/* key not found in string */
	ret = mg_get_cookie("", "notfound", buf, sizeof(buf));
	ck_assert_int_eq(ret, -1);

	ret = mg_get_cookie(longcookie, "notfound", buf, sizeof(buf));
	ck_assert_int_eq(ret, -1);

	/* key not found in string */
	ret = mg_get_cookie("key1=1; key2=2; key3=3", "notfound", buf, sizeof(buf));
	ck_assert_int_eq(ret, -1);

	/* keys are found as first, middle and last key */
	memset(buf, 77, sizeof(buf));
	ret = mg_get_cookie("key1=1; key2=2; key3=3", "key1", buf, sizeof(buf));
	ck_assert_int_eq(ret, 1);
	ck_assert_str_eq("1", buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_get_cookie("key1=1; key2=2; key3=3", "key2", buf, sizeof(buf));
	ck_assert_int_eq(ret, 1);
	ck_assert_str_eq("2", buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_get_cookie("key1=1; key2=2; key3=3", "key3", buf, sizeof(buf));
	ck_assert_int_eq(ret, 1);
	ck_assert_str_eq("3", buf);

	/* longer value in the middle of a longer string */
	memset(buf, 77, sizeof(buf));
	ret = mg_get_cookie(longcookie, "key7", buf, sizeof(buf));
	ck_assert_int_eq(ret, 10);
	ck_assert_str_eq("this+is+it", buf);

	/* key with = but without value in the middle of a longer string */
	memset(buf, 77, sizeof(buf));
	ret = mg_get_cookie(longcookie, "key5", buf, sizeof(buf));
	ck_assert_int_eq(ret, 0);
	ck_assert_str_eq("", buf);

	/* key without = and without value in the middle of a longer string */
	memset(buf, 77, sizeof(buf));
	ret = mg_get_cookie(longcookie, "key6", buf, sizeof(buf));
	ck_assert_int_eq(ret, -1);
	/* TODO: mg_get_cookie and mg_get_var(2) should have the same behavior */
}
END_TEST


START_TEST(test_mg_get_var)
{
	char buf[32];
	int ret;
	const char *shortquery = "key1=1&key2=2&key3=3";
	const char *longquery = "key1=1&key2=2&key3&key4=4&key5=&key6&"
	                        "key7=this+is+it&key8=8&key9&&key10=&&"
	                        "key7=that+is+it&key12=12";

	/* invalid result buffer */
	ret = mg_get_var2("", 0, "notfound", NULL, 999, 0);
	ck_assert_int_eq(ret, -2);

	/* zero size result buffer */
	ret = mg_get_var2("", 0, "notfound", buf, 0, 0);
	ck_assert_int_eq(ret, -2);

	/* too small result buffer */
	ret = mg_get_var2("key=toooooooooolong", 19, "key", buf, 4, 0);
	/* ck_assert_int_eq(ret, -3);
	   --> TODO: mg_get_cookie returns -3, mg_get_var -2. This should be
	   unified. */
	ck_assert(ret < 0);

	/* key not found in string */
	ret = mg_get_var2("", 0, "notfound", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, -1);

	ret = mg_get_var2(
	    longquery, strlen(longquery), "notfound", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, -1);

	/* key not found in string */
	ret = mg_get_var2(
	    shortquery, strlen(shortquery), "notfound", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, -1);

	/* key not found in string */
	ret = mg_get_var2("key1=1&key2=2&key3=3&notfound=here",
	                  strlen(shortquery),
	                  "notfound",
	                  buf,
	                  sizeof(buf),
	                  0);
	ck_assert_int_eq(ret, -1);

	/* key not found in string */
	ret = mg_get_var2(
	    shortquery, strlen(shortquery), "key1", buf, sizeof(buf), 1);
	ck_assert_int_eq(ret, -1);

	/* keys are found as first, middle and last key */
	memset(buf, 77, sizeof(buf));
	ret = mg_get_var2(
	    shortquery, strlen(shortquery), "key1", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, 1);
	ck_assert_str_eq("1", buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_get_var2(
	    shortquery, strlen(shortquery), "key2", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, 1);
	ck_assert_str_eq("2", buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_get_var2(
	    shortquery, strlen(shortquery), "key3", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, 1);
	ck_assert_str_eq("3", buf);

	/* longer value in the middle of a longer string */
	memset(buf, 77, sizeof(buf));
	ret =
	    mg_get_var2(longquery, strlen(longquery), "key7", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, 10);
	ck_assert_str_eq("this is it", buf);

	/* longer value in the middle of a longer string - seccond occurance of key
	 */
	memset(buf, 77, sizeof(buf));
	ret =
	    mg_get_var2(longquery, strlen(longquery), "key7", buf, sizeof(buf), 1);
	ck_assert_int_eq(ret, 10);
	ck_assert_str_eq("that is it", buf);

	/* key with = but without value in the middle of a longer string */
	memset(buf, 77, sizeof(buf));
	ret =
	    mg_get_var2(longquery, strlen(longquery), "key5", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, 0);
	ck_assert_str_eq(buf, "");

	/* key without = and without value in the middle of a longer string */
	memset(buf, 77, sizeof(buf));
	ret =
	    mg_get_var2(longquery, strlen(longquery), "key6", buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, -1);
	ck_assert_str_eq(buf, "");
	/* TODO: this is the same situation as with mg_get_value */
}
END_TEST


START_TEST(test_mg_md5)
{
	char buf[33];
	char *ret;
	const char *long_str =
	    "_123456789A123456789B123456789C123456789D123456789E123456789F123456789"
	    "G123456789H123456789I123456789J123456789K123456789L123456789M123456789"
	    "N123456789O123456789P123456789Q123456789R123456789S123456789T123456789"
	    "U123456789V123456789W123456789X123456789Y123456789Z";

	memset(buf, 77, sizeof(buf));
	ret = mg_md5(buf, NULL);
	ck_assert_str_eq(buf, "d41d8cd98f00b204e9800998ecf8427e");
	ck_assert_str_eq(ret, "d41d8cd98f00b204e9800998ecf8427e");
	ck_assert_ptr_eq(ret, buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_md5(buf, "The quick brown fox jumps over the lazy dog.", NULL);
	ck_assert_str_eq(buf, "e4d909c290d0fb1ca068ffaddf22cbd0");
	ck_assert_str_eq(ret, "e4d909c290d0fb1ca068ffaddf22cbd0");
	ck_assert_ptr_eq(ret, buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_md5(buf,
	             "",
	             "The qu",
	             "ick bro",
	             "",
	             "wn fox ju",
	             "m",
	             "ps over the la",
	             "",
	             "",
	             "zy dog.",
	             "",
	             NULL);
	ck_assert_str_eq(buf, "e4d909c290d0fb1ca068ffaddf22cbd0");
	ck_assert_str_eq(ret, "e4d909c290d0fb1ca068ffaddf22cbd0");
	ck_assert_ptr_eq(ret, buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_md5(buf, long_str, NULL);
	ck_assert_str_eq(buf, "1cb13cf9f16427807f081b2138241f08");
	ck_assert_str_eq(ret, "1cb13cf9f16427807f081b2138241f08");
	ck_assert_ptr_eq(ret, buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_md5(buf, long_str + 1, NULL);
	ck_assert_str_eq(buf, "cf62d3264334154f5779d3694cc5093f");
	ck_assert_str_eq(ret, "cf62d3264334154f5779d3694cc5093f");
	ck_assert_ptr_eq(ret, buf);
}
END_TEST


START_TEST(test_mg_url_encode)
{
	char buf[20];
	int ret;

	memset(buf, 77, sizeof(buf));
	ret = mg_url_encode("abc", buf, sizeof(buf));
	ck_assert_int_eq(3, ret);
	ck_assert_str_eq("abc", buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_url_encode("a%b/c&d.e", buf, sizeof(buf));
	ck_assert_int_eq(15, ret);
	ck_assert_str_eq("a%25b%2fc%26d.e", buf);

	memset(buf, 77, sizeof(buf));
	ret = mg_url_encode("%%%", buf, 4);
	ck_assert_int_eq(-1, ret);
	ck_assert_str_eq("%25", buf);
}
END_TEST


START_TEST(test_mg_url_decode)
{
	char buf[20];
	int ret;

	ret = mg_url_decode("abc", 3, buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, 3);
	ck_assert_str_eq(buf, "abc");

	ret = mg_url_decode("abcdef", 3, buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, 3);
	ck_assert_str_eq(buf, "abc");

	ret = mg_url_decode("x+y", 3, buf, sizeof(buf), 0);
	ck_assert_int_eq(ret, 3);
	ck_assert_str_eq(buf, "x+y");

	ret = mg_url_decode("x+y", 3, buf, sizeof(buf), 1);
	ck_assert_int_eq(ret, 3);
	ck_assert_str_eq(buf, "x y");

	ret = mg_url_decode("%25", 3, buf, sizeof(buf), 1);
	ck_assert_int_eq(ret, 1);
	ck_assert_str_eq(buf, "%");
}
END_TEST


START_TEST(test_mg_start_stop_http_server)
{
	struct mg_context *ctx;
	const char *OPTIONS[] = {
	    "document_root", ".", "listening_ports", "8080", NULL,
	};
	size_t ports_cnt;
	int ports[16];
	int ssl[16];

	memset(ports, 0, sizeof(ports));
	memset(ssl, 0, sizeof(ssl));

	ctx = mg_start(NULL, NULL, OPTIONS);
	ck_assert(ctx != NULL);

	ports_cnt = mg_get_ports(ctx, 16, ports, ssl);
	ck_assert_uint_eq(ports_cnt, 1);
	ck_assert_int_eq(ports[0], 8080);
	ck_assert_int_eq(ssl[0], 0);
	ck_assert_int_eq(ports[1], 0);
	ck_assert_int_eq(ssl[1], 0);

	mg_Sleep(1);
	mg_stop(ctx);
}
END_TEST


START_TEST(test_mg_start_stop_https_server)
{
	struct mg_context *ctx;
	const char *OPTIONS[] = {
	    "document_root",
	    ".",
	    "listening_ports",
	    "8080,8443s",
	    "ssl_certificate",
	    "../resources/ssl_cert.pem", // TODO: check working path of CI test
	                                 // system
	    NULL,
	};
	size_t ports_cnt;
	int ports[16];
	int ssl[16];

	memset(ports, 0, sizeof(ports));
	memset(ssl, 0, sizeof(ssl));

	ctx = mg_start(NULL, NULL, OPTIONS);
	if (!ctx) {
		/* TODO (high): The continuous integration test environment seems to
		 * not have SSL libraries available. */
		return;
	}

	ck_assert(ctx != NULL);

	ports_cnt = mg_get_ports(ctx, 16, ports, ssl);
	ck_assert_uint_eq(ports_cnt, 2);
	ck_assert_int_eq(ports[0], 8080);
	ck_assert_int_eq(ssl[0], 0);
	ck_assert_int_eq(ports[1], 8443);
	ck_assert_int_eq(ssl[1], 1);
	ck_assert_int_eq(ports[2], 0);
	ck_assert_int_eq(ssl[2], 0);

	mg_Sleep(1);
	mg_stop(ctx);
}
END_TEST

static struct mg_context *g_ctx;

static int request_test_handler(struct mg_connection *conn, void *cbdata)
{
	int i;
	char chunk_data[32];
	const struct mg_request_info *ri;
	struct mg_context *ctx;
	void *ud, *cud;

	ctx = mg_get_context(conn);
	ud = mg_get_user_data(ctx);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert(ctx == g_ctx);
	ck_assert(ud == &g_ctx);

	mg_set_user_connection_data(conn, (void *)6543);
	cud = mg_get_user_connection_data(conn);
	ck_assert(cud == (void *)6543);

	ck_assert(cbdata == (void *)7);
	strcpy(chunk_data, "123456789A123456789B123456789C");

	mg_printf(conn,
	          "HTTP/1.1 200 OK\r\n"
	          "Transfer-Encoding: chunked\r\n"
	          "Content-Type: text/plain\r\n\r\n");

	for (i = 1; i <= 10; i++) {
		mg_printf(conn, "%x\r\n", i);
		mg_write(conn, chunk_data, (unsigned)i);
		mg_printf(conn, "\r\n");
	}

	mg_printf(conn, "0\r\n\r\n");

	return 1;
}


START_TEST(test_request_handlers)
{
	char ebuf[100];
	struct mg_context *ctx;
	struct mg_connection *conn;
	const struct mg_request_info *ri;
	char uri[64];
	char buf[1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10 + 8];
	int i;
	const char *request = "GET /U7 HTTP/1.0\r\n\r\n";
	const char *HTTP_PORT = "8087";
	const char *OPTIONS[8]; /* initializer list here is rejected by CI test */
	const char *opt;
	FILE *f;

	memset((void *)OPTIONS, 0, sizeof(OPTIONS));
	OPTIONS[0] = "listening_ports";
	OPTIONS[1] = HTTP_PORT;
	OPTIONS[2] = "document_root";
	OPTIONS[3] = ".";
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 1] == NULL);
	ck_assert(OPTIONS[sizeof(OPTIONS) / sizeof(OPTIONS[0]) - 2] == NULL);

	ctx = mg_start(NULL, &g_ctx, OPTIONS);
	ck_assert(ctx != NULL);
	g_ctx = ctx;

	opt = mg_get_option(ctx, "listening_ports");
	ck_assert_str_eq(opt, HTTP_PORT);
	opt = mg_get_option(ctx, "cgi_environment");
	ck_assert_str_eq(opt, "");
	opt = mg_get_option(ctx, "unknown_option_name");
	ck_assert(opt == NULL);

	for (i = 0; i < 1000; i++) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx, uri, request_test_handler, NULL);
	}
	for (i = 500; i < 800; i++) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx, uri, NULL, (void *)1);
	}
	for (i = 600; i >= 0; i--) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx, uri, NULL, (void *)2);
	}
	for (i = 750; i <= 1000; i++) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(ctx, uri, NULL, (void *)3);
	}
	for (i = 5; i < 9; i++) {
		sprintf(uri, "/U%u", i);
		mg_set_request_handler(
		    ctx, uri, request_test_handler, (void *)(ptrdiff_t)i);
	}


	/* Try to load non existing file */
	conn = mg_download("localhost",
	                   atoi(HTTP_PORT),
	                   0,
	                   ebuf,
	                   sizeof(ebuf),
	                   "%s",
	                   "GET /file/not/found HTTP/1.0\r\n\r\n");
	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert_str_eq(ri->uri, "404");
	mg_close_connection(conn);


	/* Get data from callback */
	conn = mg_download(
	    "localhost", atoi(HTTP_PORT), 0, ebuf, sizeof(ebuf), "%s", request);
	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert_str_eq(ri->uri, "200");
	i = mg_read(conn, buf, sizeof(buf));
	ck_assert_int_eq(i, 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9 + 10);
	mg_close_connection(conn);


/* It seems to be impossible to find out what the actual working
 * directory of the CI test environment is. Before breaking another
 * dozen of builds by trying blindly with different paths, just
 * create the file here */
#ifdef _WIN32
	f = fopen("test.txt", "wb");
#else
	f = fopen("test.txt", "w");
#endif
	fwrite("simple text file\n", 17, 1, f);
	fclose(f);

	/* Get static data */
	conn = mg_download("localhost",
	                   atoi(HTTP_PORT),
	                   0,
	                   ebuf,
	                   sizeof(ebuf),
	                   "%s",
	                   "GET /test.txt HTTP/1.0\r\n\r\n");
	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert_str_eq(ri->uri, "200");
	i = mg_read(conn, buf, sizeof(buf));
	ck_assert_int_eq(i, 17);
	if ((i >= 0) && (i < (int)sizeof(buf))) {
		buf[i] = 0;
	}
	ck_assert_str_eq(buf, "simple text file\n");
	mg_close_connection(conn);


	/* Get directory listing */
	conn = mg_download("localhost",
	                   atoi(HTTP_PORT),
	                   0,
	                   ebuf,
	                   sizeof(ebuf),
	                   "%s",
	                   "GET / HTTP/1.0\r\n\r\n");
	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert_str_eq(ri->uri, "200");
	i = mg_read(conn, buf, sizeof(buf));
	ck_assert(i > 6);
	buf[6] = 0;
	ck_assert_str_eq(buf, "<html>");
	mg_close_connection(conn);


	/* POST to static file (will not work) */
	conn = mg_download("localhost",
	                   atoi(HTTP_PORT),
	                   0,
	                   ebuf,
	                   sizeof(ebuf),
	                   "%s",
	                   "POST /test.txt HTTP/1.0\r\n\r\n");
	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert_str_eq(ri->uri, "405");
	i = mg_read(conn, buf, sizeof(buf));
	ck_assert(i >= 29);
	buf[29] = 0;
	ck_assert_str_eq(buf, "Error 405: Method Not Allowed");
	mg_close_connection(conn);


	/* PUT to static file (will not work) */
	conn = mg_download("localhost",
	                   atoi(HTTP_PORT),
	                   0,
	                   ebuf,
	                   sizeof(ebuf),
	                   "%s",
	                   "PUT /test.txt HTTP/1.0\r\n\r\n");
	ck_assert(conn != NULL);
	ri = mg_get_request_info(conn);

	ck_assert(ri != NULL);
	ck_assert_str_eq(ri->uri, "401"); /* not authorized */
	mg_close_connection(conn);


	/* Close the server */
	g_ctx = NULL;
	mg_stop(ctx);
	mg_Sleep(1);
}
END_TEST

Suite *make_public_suite(void)
{

	Suite *const suite = suite_create("Public");

	TCase *const version = tcase_create("Version");
	TCase *const get_valid_options = tcase_create("Options");
	TCase *const get_builtin_mime_type = tcase_create("MIME types");
	TCase *const tstrncasecmp = tcase_create("strcasecmp");
	TCase *const urlencodingdecoding = tcase_create("URL encoding decoding");
	TCase *const cookies = tcase_create("Cookies and variables");
	TCase *const md5 = tcase_create("MD5");
	TCase *const startstophttp = tcase_create("Start Stop HTTP Server");
	TCase *const startstophttps = tcase_create("Start Stop HTTPS Server");

	tcase_add_test(version, test_mg_version);
	suite_add_tcase(suite, version);

	tcase_add_test(get_valid_options, test_mg_get_valid_options);
	suite_add_tcase(suite, get_valid_options);

	tcase_add_test(get_builtin_mime_type, test_mg_get_builtin_mime_type);
	suite_add_tcase(suite, get_builtin_mime_type);

	tcase_add_test(tstrncasecmp, test_mg_strncasecmp);
	suite_add_tcase(suite, tstrncasecmp);

	tcase_add_test(urlencodingdecoding, test_mg_url_encode);
	tcase_add_test(urlencodingdecoding, test_mg_url_decode);
	suite_add_tcase(suite, urlencodingdecoding);

	tcase_add_test(cookies, test_mg_get_cookie);
	tcase_add_test(cookies, test_mg_get_var);
	suite_add_tcase(suite, cookies);

	tcase_add_test(md5, test_mg_md5);
	suite_add_tcase(suite, md5);

	tcase_add_test(startstophttp, test_mg_start_stop_http_server);
	tcase_add_test(startstophttp, test_request_handlers);
	suite_add_tcase(suite, startstophttp);

	tcase_add_test(startstophttps, test_mg_start_stop_https_server);
	suite_add_tcase(suite, startstophttps);

	return suite;
}

#if 0
/* Used to debug test cases without using the check framework */
void main(void)
{
	test_mg_start_stop_http_server(0);
	test_mg_start_stop_https_server(0);
	test_request_handlers(0);
}

void _ck_assert_failed(const char *file, int line, const char *expr, ...)
{
	fprintf(stderr, "error!\n");
}

void _mark_point(const char *file, int line) {}
void tcase_fn_start(const char *fname, const char *file, int line) {}
void suite_add_tcase(Suite *s, TCase *tc){};
void _tcase_add_test(TCase *tc,
                     TFun tf,
                     const char *fname,
                     int _signal,
                     int allowed_exit_value,
                     int start,
                     int end){};
TCase *tcase_create(const char *name) { return NULL; };
Suite *suite_create(const char *name) { return NULL; };
#endif

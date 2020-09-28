
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <curl/curl.h>

#define VERSION_STR	"V1.0"
//size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);

/* send RTSP OPTIONS request */
bool rtsp_options(CURL *curl)
{
	//CURLcode res = CURLE_OK;
	//printf("\nRTSP: OPTIONS %s\n", uri);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_OPTIONS) != CURLE_OK )
		return false;
	if( curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL) != CURLE_OK )
		return false;
	return (curl_easy_perform(curl) == CURLE_OK);
}


/* send RTSP DESCRIBE request and write sdp response to a file */
bool rtsp_describe(CURL *curl) {
	//CURLcode res = CURLE_OK;
	//printf("\nRTSP: DESCRIBE %s\n", uri);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_DESCRIBE) != CURLE_OK )
		return false;
	if( curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL) != CURLE_OK )
		return false;
	return (curl_easy_perform(curl) == CURLE_OK);
}

/* send RTSP SETUP request */
bool rtsp_setup(CURL *curl, const char *transport)
{
	//CURLcode res = CURLE_OK;
	//printf("\nRTSP: SETUP %s\n", uri);
	//printf("		TRANSPORT %s\n", transport);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_TRANSPORT, transport) != CURLE_OK )
		return false;
	if( curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_SETUP) != CURLE_OK )
		return false;
	return (curl_easy_perform(curl) == CURLE_OK);
}


/* send RTSP PLAY request */
bool rtsp_play(CURL *curl, const char *range)
{
	//CURLcode res = CURLE_OK;
	//printf("\nRTSP: PLAY %s\n", uri);
	if( curl_easy_setopt(curl, CURLOPT_RANGE, range) != CURLE_OK )
		return false;
	if( curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_PLAY) != CURLE_OK )
		return false;
	return (curl_easy_perform(curl) == CURLE_OK);

	/* switch off using range again */
	//curl_easy_setopt(curl, CURLOPT_RANGE, NULL);
}


/* send RTSP TEARDOWN request */
bool rtsp_teardown(CURL *curl)
{
	//CURLcode res = CURLE_OK;
	//printf("\nRTSP: TEARDOWN %s\n", uri);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_TEARDOWN) != CURLE_OK )
		return false;
	return (curl_easy_perform(curl) == CURLE_OK);
}

bool rtsp_set(CURL *curl, long length, const char* str)
{
	//CURLcode res = CURLE_OK;
	//printf("\nRTSP: TEARDOWN %s\n", uri);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_SET_PARAMETER) != CURLE_OK )
		return false;
	if( curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, length) != CURLE_OK )
		return false;
	if( curl_easy_setopt(curl, CURLOPT_POSTFIELDS, str) != CURLE_OK )
		return false;
	return (curl_easy_perform(curl) == CURLE_OK);
}


/* main app */
int main(int argc, char * const argv[])
{
	//const char *transport = "RTP/AVP;unicast;client_port=1234-1235";	// UDP
	const char *transport1 = "RTP/AVP/UDP;unicast;client_port=1234-1235";
	const char *transport2 = "RTP/AVP/UDP;unicast;client_port=1236-1237";
	//const char *url = "rtsp://10.224.88.1:554/onvif1"
	const char *url = "rtsp://admin:123@10.224.88.1:554/onvif1";
	const char *range = "npt=0.000-";

	char *uri = malloc(strlen(url) + 32);
	char *sdp_filename = malloc(strlen(url) + 32);
	char *control = malloc(strlen(url) + 32);
	CURLcode res;

	/* initialize curl */
	res = curl_global_init(CURL_GLOBAL_ALL);
	if(res != CURLE_OK)
		exit(1);
	curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
	CURL *curl;
	fprintf(stderr, "	curl V%s loaded\n", data->version);

	/* initialize this curl session */
	curl = curl_easy_init();
	if(curl == NULL)
		exit(1);
	curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, stdout);
	if( curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK )
		exit(1);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, url) != CURLE_OK )
		exit(1);
	//curl_easy_setopt(curl, CURLOPT_RTSP_TRANSPORT, transport);
	//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, NULL );

	/* request server options */
	//snprintf(uri, strlen(url) + 32, "%s", url);
	//curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, uri);

	if( !rtsp_options(curl) )
		exit(1);

	/* request session description and write response to sdp file */
	if( !rtsp_describe(curl) )
		exit(1);

	/* setup media stream */
	snprintf(uri, strlen(url) + 32, "%s/%s", url, "track1");
	if( curl_easy_setopt(curl, CURLOPT_URL, uri) != CURLE_OK )
		exit(1);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, uri) != CURLE_OK )
		exit(1);
	//printf("setup uri=%s\n", uri);
	rtsp_setup(curl, transport1);
	snprintf(uri, strlen(url) + 32, "%s/%s", url, "track2");
	if( curl_easy_setopt(curl, CURLOPT_URL, uri) != CURLE_OK )
		exit(1);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, uri) != CURLE_OK )
		exit(1);
	//printf("setup uri=%s\n", uri);
	if( !rtsp_setup(curl, transport2) )
		exit(1);


	if( curl_easy_setopt(curl, CURLOPT_URL, url) != CURLE_OK )
		exit(1);
	if( curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, url) != CURLE_OK )
		exit(1);
	const char* cmd = "ptzCmd:xRIGHT";
	//snprintf(uri, strlen(url) + 32, "%s/", url);
	if( !rtsp_set(curl, strlen(cmd), cmd) )
		exit(1);

	/* start playing media stream */
//	snprintf(uri, strlen(url) + 32, "%s/", url);
//	if( !rtsp_play(curl, url, range) )
//		exit(1);
//	printf("Playing video, press any key to stop ...");
//	fgetc(stdin);
//	printf("\n");

	/* teardown session */
	rtsp_teardown(curl);

	/* cleanup */
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	free(control);
	free(sdp_filename);
	free(uri);
	exit(0);
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	return nmemb;
}


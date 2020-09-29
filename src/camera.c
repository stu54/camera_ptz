
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <termios.h>
#include <unistd.h>
#include <curl/curl.h>


char* outputStr;

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
//size_t header_callback(char *ptr, size_t size, size_t nmemb, void *userdata);
char errStr[80];
bool headerDataSet;

/* main app */
int main(int argc, char * const argv[])
{
	const char *transport1 = "RTP/AVP/UDP;unicast;client_port=1234-1235";
	const char *transport2 = "RTP/AVP/UDP;unicast;client_port=1236-1237";
	const char *url = "rtsp://10.172.180.1:554/onvif1";
	//const char *url = "rtsp://admin:123@10.224.88.1:554/onvif1";
	//const char *url = "rtsp://admin:stuarta1@192.168.1.147:554/onvif1";
	//const char *url = "rtsp://192.168.1.147:554/onvif1";

	char *uri = malloc(strlen(url) + 32);
	CURLcode res;
	long resp;

	/* initialize curl */
	if( (res = curl_global_init(CURL_GLOBAL_ALL)) != CURLE_OK )
		exit(1);
	curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);
	CURL *curl;
	printf("curl V%s loaded\n", data->version);

	/* initialize this curl session */
	curl = curl_easy_init();
	if(curl == NULL)
		exit(1);
	res = curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
	res = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L);
	//curl_easy_setopt(curl, CURLOPT_HEADERDATA, stdout);
	res = curl_easy_setopt(curl, CURLOPT_URL, url);
	res = curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, url);
	res = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	//res = curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);
	//res = curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
	//curl_easy_setopt(curl, CURLOPT_RTSP_TRANSPORT, transport);
	//curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback );
	headerDataSet = false;

	/* request server options */
	//snprintf(uri, strlen(url) + 32, "%s", url);
	//curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, uri);

	curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_OPTIONS);
	if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
		printf("err=%s\n", curl_easy_strerror(res));
		exit(1);
	}

	outputStr = NULL;
	/* request session description and write response to sdp file */
	// First try with Basic authorization
	curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_DESCRIBE);
	if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
		printf("err=%s\n", curl_easy_strerror(res));
		exit(1);
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp );
	printf("resp=%ld\n", resp);
	if( resp == 401 ) {
		// Try with Digest authorization
		res = curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_DIGEST);
		headerDataSet = false;
		if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
			printf("err=%s\n", curl_easy_strerror(res));
			exit(1);
		}
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp );
		if( resp != 200 ) {
			printf("resp=%ld\n", resp);
			exit(1);
		}
	} else if( resp != 200 ) {
		printf("err=%s\n", errStr);
		exit(1);
	}
	printf("*************Output Begin************\n");
	printf("%s", outputStr);
	printf("*************Output End************\n");
	free(outputStr);

	/* setup media stream */
	headerDataSet = false;
	snprintf(uri, strlen(url) + 32, "%s/%s", url, "track1");
	res = curl_easy_setopt(curl, CURLOPT_URL, uri);
	res = curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, uri);
	res = curl_easy_setopt(curl, CURLOPT_RTSP_TRANSPORT, transport1);
	res = curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_SETUP);
	//printf("setup uri=%s\n", uri);
	if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
		printf("err=%s\n", curl_easy_strerror(res));
		exit(1);
	}

	headerDataSet = false;
	snprintf(uri, strlen(url) + 32, "%s/%s", url, "track2");
	curl_easy_setopt(curl, CURLOPT_URL, uri);
	curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, uri) ;
	curl_easy_setopt(curl, CURLOPT_RTSP_TRANSPORT, transport2);
	//printf("setup uri=%s\n", uri);
	if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
		printf("err=%s\n", curl_easy_strerror(res));
		exit(1);
	}

#ifdef NEVER
	/* start playing media stream */
	//snprintf(uri, strlen(url) + 32, "%s/", url);
	const char *range = "npt=0.000-";
	headerDataSet = false;
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, url);
	curl_easy_setopt(curl, CURLOPT_RANGE, range);
	curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_PLAY);
	if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
		printf("exiting\n");
		printf("err=%s\n", errStr);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp );
		printf("resp=%ld\n", resp);
		exit(1);
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp );
	printf("resp=%ld\n", resp);
	printf("Playing video, press any key to stop ...");
	//fgetc(stdin);
	//printf("\n");
#endif

	headerDataSet = false;
	res = curl_easy_setopt(curl, CURLOPT_URL, url);
	res = curl_easy_setopt(curl, CURLOPT_RTSP_STREAM_URI, url);
	struct curl_slist* list = NULL;
	list = curl_slist_append(list, "Content-length: strlen(Content-type)");
	list = curl_slist_append(list, "Content-type: ptzCmd:RIGHT");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_SET_PARAMETER);
	for( int i = 0; i < 20; i++ ) {
		if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
			printf("exiting\n");
			printf("err=%s\n", curl_easy_strerror(res));
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp );
			printf("resp=%ld\n", resp);
			exit(1);
		}
		//sleep(1);
	}
	curl_slist_free_all(list);

	sleep(1);
	list = NULL;
	list = curl_slist_append(list, "Content-length: strlen(Content-type)");
	list = curl_slist_append(list, "Content-type: ptzCmd:LEFT");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
	for( int i = 0; i < 20; i++ ) {
		if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
			printf("exiting\n");
			printf("err=%s\n", errStr);
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp );
			printf("resp=%ld\n", resp);
			exit(1);
		}
		//sleep(1);
	}
	curl_slist_free_all(list);
#ifdef NEVER
	/* teardown session */
	curl_easy_setopt(curl, CURLOPT_RTSP_REQUEST, (long)CURL_RTSPREQ_TEARDOWN);
	if( (res = curl_easy_perform(curl)) != CURLE_OK ) {
		printf("exiting\n");
		printf("err=%s\n", errStr);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp );
		printf("resp=%ld\n", resp);
		exit(1);
	}
	curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &resp );
	printf("resp=%ld\n", resp);
#endif
	/* cleanup */
	curl_easy_cleanup(curl);
	curl_global_cleanup();
	free(uri);
	printf("Done\n");
	exit(0);
}

size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
	outputStr = malloc((nmemb+1) * size);
	bzero( outputStr, (nmemb+1)*size );
	strncpy( outputStr, ptr, nmemb );
	return nmemb;
}

#ifdef NEVER
size_t header_callback(char *ptr, size_t size, size_t nmemb, void *userdata) {
/*
	if( !headerDataSet ) {
		bzero( errStr, sizeof(errStr) );
		size_t len = nmemb;
		if( len > sizeof(errStr) )
			len = sizeof(errStr);
		strncpy( errStr, ptr, len-1 );
		headerDataSet = true;
	}
*/
	return nmemb;
}
#endif

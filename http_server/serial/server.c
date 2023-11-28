#include <stdio.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include "main_http.h"

// La fonction d'affichage de messages

int print(char * fmt, ...)
{
        int print_count;
        va_list myargs;
        va_start(myargs,fmt);
        print_count = vfprintf(stderr,fmt,myargs);
        va_end(myargs);
        return print_count;
}

// Des fonctions utilitaires

int check_file_exists(const char * path)
{
	FILE * fp;
	fp=fopen(path,"r");
	if (fp)
	{
		fclose(fp);
		return 1;
	}      
	return 0;
}
 
int file_size(const char *path)
{
	if(!check_file_exists(path)) exit(0);

	struct stat st;
	stat(path,&st);
	return st.st_size;
}

int check_folder_exists(const char *path)
{
	struct stat st;
	if(lstat(path,&st)<0)
	{	print("ROOT DIRECTORY DOES NOT EXISTS");
		return 0;
	}
	if(S_ISDIR(st.st_mode))
		return 1;	
	return 0;
}

int set_index(char *path)
{
	struct stat st;
	if(lstat(path,&st)<0)
	{	perror("");
		return -1;
	}
	if(S_ISDIR(st.st_mode))  strcat(path,"/index.html");
	return 1;	
}

void trim_resource(char * resource_location)
{
	if(strstr(resource_location,"#")) strcpy(strpbrk(resource_location,"#"),"");
	if(strstr(resource_location,"?")) strcpy(strpbrk(resource_location,"?"),"");
}

// Les variables globales

static  char  	      path_root[PATH_MAX] = CURRENT_DIRECTORY;
static  int   	      port_number         = DEFAULT_PORT_NUMBER;
typedef void 	      (*strategy_t)(int);
static  int   	      buffer_max          = DEFAULT_BUFFER_SIZE;
static  int   	      worker_max          = DEFAULT_WORKER_SIZE;
static  sig_atomic_t  status_on           = True;
char  	      strategy_name[STRATEGY_MAX];
pthread_mutex_t buffer_lock_mtx          = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  buffer_not_full_cond      = PTHREAD_COND_INITIALIZER;
pthread_cond_t  buffer_not_empty_cond     = PTHREAD_COND_INITIALIZER;

const char* get_extension(const char *path)
{
	if(strstr(path,".HTML") || strstr(path,".html")) return "text/html";
	if(strstr(path,".JPEG") || strstr(path,".jpeg")) return "image/jpeg";
	if(strstr(path,".PNG" ) || strstr(path,".png" )) return "image/png";
	if(strstr(path,".TXT" ) || strstr(path,".txt" )) return "text";
	if(strstr(path,".JPG" ) || strstr(path,".jpg" )) return "image/jpg";
	if(strstr(path,".CSS" ) || strstr(path,".css" )) return "text/css";
	if(strstr(path,".JS"  ) || strstr(path,".js"  )) return "application/javascript";
	if(strstr(path,".XML" ) || strstr(path,".xml" )) return "application/xml";
	if(strstr(path,".MP3" ) || strstr(path,".mp3" )) return "audio/mpeg";
	if(strstr(path,".MPEG") || strstr(path,".mpeg")) return "video/mpeg";
	if(strstr(path,".MPG" ) || strstr(path,".mpg" )) return "video/mpeg";
	if(strstr(path,".MP4" ) || strstr(path,".mp4" )) return "video/mp4";
	if(strstr(path,".MOV" ) || strstr(path,".mov" )) return "video/quicktime";
	return "text/html";
}

static int next_request(int fd,http_request_t *request)
{
	char   command_line  [MAX_HEADER_LINE_LENGTH];
	char   method        [MAX_METHODS];
	char   payload_source[PATH_MAX];
	int    minor_version;
	int    major_version;
	char   head_name     [MAX_HEADER_LINE_LENGTH];
	char   head_value    [MAX_HEADER_VALUE_LENGTH];
	int    head_count = 0;
	FILE * fr;
	fr = fdopen(dup(fd),"r");	

puts("next request");

	if(fr)
	{
		fgets(command_line,MAX_HEADER_LINE_LENGTH,fr);
printf("command_line=[%s]\n",command_line);
		sscanf(command_line, "%s %s HTTP/%d.%d%*s",method,payload_source,&major_version,&minor_version);

		if(strcmp(method,"GET")==0)
		{
			request->method        = HTTP_METHOD_GET;
			trim_resource(payload_source);
			strcpy(request->uri,payload_source);
			request->major_version = major_version;
			request->minor_version = minor_version;
		}
		else
		{
			request->method        = HTTP_STATUS_NOT_IMPLEMENTED;
		}
	}
	while(head_count < MAX_HEADERS)
	{
		fgets(command_line,MAX_HEADER_LINE_LENGTH,fr);
printf("command_line=[%s]\n",command_line);

		if(strstr(command_line,":"))
		{
			sscanf(command_line,"%s: %s",head_name,head_value);

			strcpy(request->headers[head_count].field_name,head_name);
			strcpy(request->headers[head_count++].field_value,head_value);    
		}
		else 
			break;
	}

	request->header_count=head_count;
	fclose(fr);
	return 1;
}

static int set_response_field_name_and_value(http_response_t *response,const char *name,const char *value)
{
	strcpy(response->headers[response->header_count].field_name,name);
	strcpy(response->headers[response->header_count++].field_value,value);
	return 1;
}

static int handle_error(http_status_t status,char * error_resource)
{
	if(status.code == HTTP_STATUS_LOOKUP[HTTP_STATUS_OK].code)	return 1;
	if(status.code == HTTP_STATUS_LOOKUP[HTTP_STATUS_NOT_FOUND].code)
	{
		strcpy(error_resource,path_root);
		strcat(error_resource,ERROR_NOT_FOUND_404);

		if(!check_file_exists(error_resource))
		{
			strcpy(error_resource,path_root);
			strcat(error_resource,ERROR_BAD_REQUEST_400);
		}
		if(!check_file_exists(error_resource))
		{
			strcpy(error_resource,DEFUALT_ERROR_NOT_FOUND_404);
		}
	}
	return 0;
}

static http_status_t check_response_status(const int status,const char * path)
{
	if(status == HTTP_STATUS_NOT_IMPLEMENTED) return HTTP_STATUS_LOOKUP[status];
	if(!check_file_exists(path))              return HTTP_STATUS_LOOKUP[HTTP_STATUS_NOT_FOUND];
	return HTTP_STATUS_LOOKUP[HTTP_STATUS_OK];
}

static int build_response(const http_request_t *request,http_response_t *response)
{
	char buffer[MAX_HEADER_VALUE_LENGTH];
	int head_count=0;
	time_t now=0;
	struct tm *t;
	strcat(response->resource_path,request->uri);
	set_index(response->resource_path);	

	response->status        = check_response_status(request->method,response->resource_path);
	handle_error(response->status, response->resource_path);

	response->major_version = request->major_version;
	response->minor_version = request->minor_version;

	now = time(NULL);
	t = gmtime(&now);
	strftime(buffer,30,"%a, %d %b %Y %H:%M:%S %Z",t);

	set_response_field_name_and_value(response,"Date",buffer);
	set_response_field_name_and_value(response,"Server","MAIN HTTP");
	set_response_field_name_and_value(response,"Content-Type",get_extension(response->resource_path));
	printf("response=%s\n",response->resource_path);
	sprintf(buffer,"%d",file_size(response->resource_path));
	set_response_field_name_and_value(response,"Content-Length",buffer);

	return 1;
}

static int send_response(int fd,const http_response_t *response)
{
	time_t now;
	struct tm t;
	FILE   *fr;
	char   buf[MAX_HEADER_VALUE_LENGTH];
	FILE   *fp = fdopen(dup(fd),"w");
	size_t size;
	int    head_no;
	int    ch;	
	fprintf(fp,"HTTP/%d.%d %d %s\r\n",response->major_version,response->minor_version,response->status.code,response->status.reason);
	for(head_no = 0; head_no<response->header_count; head_no++)
	{
		fprintf(fp,"%s: %s\r\n",response->headers[head_no].field_name,response->headers[head_no].field_value);
	}
	fprintf(fp,"\n");
	fr=fopen(response->resource_path,"r");  //print payload 
	if(fr)
	{
		while((ch=getc(fr))!=EOF)  fprintf(fp,"%c",ch);
		fclose(fr);
	}
	fclose(fp);
	return 1;
}    

static int clear_responses(http_response_t *response)//clearing responses avoids possiblity of duplicate headers err
{
	int head_no;
	for(head_no =0; head_no<response->header_count; head_no++)
	{
		strcpy(response->headers[head_no].field_name,"");
		strcpy(response->headers[head_no].field_value,"");
	}
	response->header_count = 0;
	return 1;
}

static void manage_single_request(int peer_sfd)
{
	http_request_t  *request  = (http_request_t*)malloc(sizeof(http_request_t));	
	http_response_t *response = (http_response_t*)malloc(sizeof(http_response_t));	
	strcpy(response->resource_path,path_root);
	response->header_count=0;

	next_request(peer_sfd, request);
	build_response(request, response);
	send_response(peer_sfd, response);

	clear_responses(response);
	free(request);
	free(response);	
}

static void perform_serially(int sfd)
{
	print("\nPERFORMING SERIALLY");
	while(status_on)
	{
		int peer_sfd = accept(sfd,NULL,NULL);                           

		if(peer_sfd == -1)
		{	
			print("\nACCEPT FAILED");
			continue;
		}

		manage_single_request(peer_sfd);
		close(peer_sfd);
	}
}

int initialize_server()
{
	struct sockaddr_in myaddr;
	int    sfd; 
	int    optval = 1;

	sfd = socket(AF_INET,SOCK_STREAM,0);    //creating socket

	if(sfd == -1)
	{
		print("socket");
		exit(0);
	}

	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
		print("\nsetsockopt");

	memset(&myaddr,0,sizeof(myaddr));
	myaddr.sin_family      = AF_INET;
	myaddr.sin_port        = htons(port_number);
	myaddr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sfd, (struct sockaddr*) &myaddr, sizeof(myaddr)) == -1)  
	{	
		print("PORT NO NOT FOUND GLOBAL ERROR STATUS IS:");
		exit(0);
	}
	if(listen(sfd,BACKLOG)==-1)                                     
		print("\nLISTEN FAILED");             

	return sfd;
}

void configure_server(int argc,char *argv[])
{
	int option,option_count=0;

	int cpt=0;

	while((option = getopt(argc,argv,"p:d:w:q:ft"))!=-1)
	{
		switch (option)
		{
			case 'p':
				port_number = atoi(optarg);
				break;
			case 'd':
				strcpy(path_root,optarg);
				break;
			case 'q':
				buffer_max = atoi(optarg);
				break;
			default:
				print("Please enter the right arguments\n");
				break;
		}
	}

	printf("|%s|\n",path_root);

	if(option_count > 1)
	{
		print("\nDon't pass arguments to use more than one strategy.\n");
		exit(0);
	}
	
	if(!check_folder_exists(path_root)) exit(0);
	
	if(option_count ==0) 
	{	
		strcpy(strategy_name,"Serial Operation");
	}
}

int main(int argc,char *argv[])
{	
	strategy_t server_operation;
	int sfd;
	configure_server(argc,argv);		
	sfd              = initialize_server();
	perform_serially(sfd);  				//start server
	if(close(sfd)==-1)					//close server
		print("\nError while closing");

	return 0;
}

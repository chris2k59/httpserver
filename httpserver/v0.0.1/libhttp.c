#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include "libhttp.h"

#define LIBHTTP_REQUEST_MAX_SIZE 8192

void http_fatal_error(char *message) {
  fprintf(stderr, "%s\n", message);
  exit(ENOBUFS);
}

struct http_request *http_request_parse(int fd) {
  struct http_request *request = malloc(sizeof(struct http_request));
  if (!request) http_fatal_error("Malloc failed");

  char *read_buffer = malloc(LIBHTTP_REQUEST_MAX_SIZE + 1);
  if (!read_buffer) http_fatal_error("Malloc failed");

  int bytes_read = read(fd, read_buffer, LIBHTTP_REQUEST_MAX_SIZE);
  read_buffer[bytes_read] = '\0'; /* Always null-terminate. */

  char *read_start, *read_end;
  size_t read_size;

  do {
    /* Read in the HTTP method: "[A-Z]*" */
    read_start = read_end = read_buffer;
    while (*read_end >= 'A' && *read_end <= 'Z') read_end++;
    read_size = read_end - read_start;
    if (read_size == 0) break;
    request->method = malloc(read_size + 1);
    memcpy(request->method, read_start, read_size);
    request->method[read_size] = '\0';

    /* Read in a space character. */
    read_start = read_end;
    if (*read_end != ' ') break;
    read_end++;

    /* Read in the path: "[^ \n]*" */
    read_start = read_end;
    while (*read_end != '\0' && *read_end != ' ' && *read_end != '\n') read_end++;
    read_size = read_end - read_start;
    if (read_size == 0) break;
    request->path = malloc(read_size + 1);
    memcpy(request->path, read_start, read_size);
    request->path[read_size] = '\0';

    /* Read in HTTP version and rest of request line: ".*" */
    read_start = read_end;
    while (*read_end != '\0' && *read_end != '\n') read_end++;
    if (*read_end != '\n') break;
    read_end++;

    free(read_buffer);
    return request;
  } while (0);

  /* An error occurred. */
  free(request);
  free(read_buffer);
  return NULL;

}

char* http_get_response_message(int status_code) {
  switch (status_code) {
    case 100:
      return "Continue";
    case 200:
      return "OK";
    case 301:
      return "Moved Permanently";
    case 302:
      return "Found";
    case 304:
      return "Not Modified";
    case 400:
      return "Bad Request";
    case 401:
      return "Unauthorized";
    case 403:
      return "Forbidden";
    case 404:
      return "Not Found";
    case 405:
      return "Method Not Allowed";
    default:
      return "Internal Server Error";
  }
}

void http_start_response(int fd, int status_code) {
  dprintf(fd, "HTTP/1.0 %d %s\r\n", status_code,
      http_get_response_message(status_code));
}

void http_send_header(int fd, char *key, char *value) {
  dprintf(fd, "%s: %s\r\n", key, value);
}

void http_end_headers(int fd) {
  dprintf(fd, "\r\n");
}

void http_send_string(int fd, char *data) {
  http_send_data(fd, data, strlen(data));
}

void http_send_data(int fd, char *data, size_t size) {
  ssize_t bytes_sent;
  while (size > 0) {
    bytes_sent = write(fd, data, size);
    if (bytes_sent < 0)
      return;
    size -= bytes_sent;
    data += bytes_sent;
  }
}

char *http_get_mime_type(char *file_name) {
  char *file_extension = strrchr(file_name, '.');
  if (file_extension == NULL) {
    return "text/plain";
  }

  if (strcmp(file_extension, ".html") == 0 || strcmp(file_extension, ".htm") == 0) {
    return "text/html";
  } else if (strcmp(file_extension, ".jpg") == 0 || strcmp(file_extension, ".jpeg") == 0) {
    return "image/jpeg";
  } else if (strcmp(file_extension, ".png") == 0) {
    return "image/png";
  } else if (strcmp(file_extension, ".css") == 0) {
    return "text/css";
  } else if (strcmp(file_extension, ".js") == 0) {
    return "application/javascript";
  } else if (strcmp(file_extension, ".pdf") == 0) {
    return "application/pdf";
  } else {
    return "text/plain";
  }
}

// Load files into a char *

char *name_fix(char *file_name){
  char *start = "./files";
  char *path = (char *) malloc(1 + strlen(start) + strlen(file_name));
  strcpy(path, start);
  strcat(path, file_name);
  return path;
}

char *append(char *file_name, char *end){
  
  char *path = (char *) malloc(1 + strlen(end) + strlen(file_name));
  strcpy(path, file_name);
  strcat(path, end);
  return path;
}



char *buffer(char *file_name, long unsigned *bSize){
  
  FILE *fp;
  
  char *buffer;
  
  
  fp = fopen(file_name,"rb");
  if(!fp) perror(file_name),exit(1);

  fseek(fp, 0l, SEEK_END);
  *bSize = ftell(fp);
  rewind(fp);

  buffer = calloc(1, *bSize+1);
  if(!buffer) fclose(fp),fputs("mem alloc fail", stderr),exit(1);

  if( fread(buffer, *bSize, 1, fp) != 1)
    fclose(fp),free(buffer),exit(1);
  
  fclose(fp);
 
  return buffer;
}

void find(char *path){
  char * token;
  token = strtok(path,"/");
  int i = 0;
  while(token){
    printf("%d:%s",i , token);
    token = strtok(NULL, "/");
    i++;
  }


}

char *itoa(long unsigned bSize){
  char *temp = "";
  size_t size = snprintf(temp,0,"%lu",bSize);
  char result[size+1];

  snprintf(result, size+1, "%lu", bSize);
  
  return result;
}

char *Dirlist(char *dir){
  char *path= name_fix(dir);
  int size =0;  

  DIR *dp;
  struct dirent *ep;
  char *result = "<h1>LINKS</h1>";
  
 
 
  dp = opendir(path);
  if(dp != NULL)
  {
    while( (ep = readdir(dp)) )
    { 
      size++;
    }
  }else 
    perror("Could not open directory");
  char *dirlist[size];
  char *index ="index.html"; 
  rewinddir(dp);
  if(dp != NULL)
  {
    int i = 0;    
    while((ep = readdir(dp)))
    {
        
      dirlist[i] = ep->d_name;
      i++;
    }
   (void) closedir(dp);
  }else
    perror("ERROR with directory pointer");
  
  for(int i=0; i!=size;i++)
  {
    if(strcmp(dirlist[i],index)==0){
      result = index;
      break;
    }
    if(strcmp(dirlist[i],".")==0)
    { 
      continue;
    } 
  
  size_t size = snprintf(result,0,"%s<p><a href='%s%s'>%s</a></p>",result,dir,dirlist[i],dirlist[i]);
  char *rst=malloc(size+1);

  snprintf(rst, size+1,"%s<p><a href='%s%s'>%s</a></p>",result,dir,dirlist[i],dirlist[i]);
  
  result = rst;

  } 

  return result;    
}


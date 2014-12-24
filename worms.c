#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "telehash.h"
#include "net_udp4.h"

// load local keys from a file, or generate and save them there
lob_t idload(char *fname)
{
  lob_t secrets, keys, json;
  hashname_t id;
  FILE *fdout;

  if((json = util_fjson(fname))) return json;

  LOG("generating new id at %s",fname);
  secrets = e3x_generate();
  keys = lob_linked(secrets);
  if(!keys) return LOG("keygen failed: %s",e3x_err());
  id = hashname_keys(keys);
  json = lob_new();
  lob_set(json,"hashname",id->hashname);
  lob_set_raw(json,"keys",0,(char*)keys->head,keys->head_len);
  lob_set_raw(json,"secrets",0,(char*)secrets->head,secrets->head_len);
  hashname_free(id);
  lob_free(secrets);
  
  if(!(fdout = fopen(fname,"wb"))) return LOG("Error opening file: %s",strerror(errno));
  fwrite(json->head,1,json->head_len,fdout);
  fclose(fdout);

  return json;
}

// create a locally listening dgram port that will proxy
int proxyfd(int port)
{
  int sock;
  struct sockaddr_in sa;
  socklen_t size = sizeof(struct sockaddr_in);

  // create a udp socket
  if((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) ) < 0 ) return LOG("failed to create socket %s",strerror(errno))&&0;

  memset(&sa,0,sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  sa.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind (sock, (struct sockaddr*)&sa, size) < 0) return LOG("bind failed %s",strerror(errno))&&0;

  return sock;
}

int main(int argc, char *argv[])
{
  lob_t id, options;
  mesh_t mesh;
  net_udp4_t udp4;
  char *paths, *to;
  int len, proxy;

  if(argc != 2)
  {
    LOG("usage: `wormd 'host'` for the server and `worms 'worm://...'` to create the proxy");
    exit(1);
  }
  to = argv[1];
  LOG("proxying to %s",to);
  
  if(!(mesh = mesh_new(0))) exit(1);

  if(!(id = idload("id.json")))
  {
    LOG("failed to load/create id.json");
    exit(1);
  }

  mesh_load(mesh,lob_get_json(id,"secrets"),lob_get_json(id,"keys"));
  mesh_on_discover(mesh,"auto",mesh_add); // auto-link anyone

  udp4 = net_udp4_new(mesh, NULL);
  util_sock_timeout(udp4->server,100);

  // decide if we're in proxy client or server mode
  if(strncmp(to,"worm:",5) == 0)
  {
    proxy = proxyfd(623);
  }else{
    proxy = 0;
    LOG("worms '%s'",mesh_uri(mesh, "worm"));
  }

  while(net_udp4_receive(udp4));

  perror("exiting");
  return 0;
}

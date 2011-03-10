/***********************************************************
 * libetherflow
 *
 * content: a collection of routines to interface neuFlow.
 *          can also be used to do simple ethernet I/O in
 *          Lua.
 *
 * date: July  3, 2010, 1:29PM
 *
 * author: P.Akselrod, C.Farabet
 **********************************************************/
#include "etherflow.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/time.h>



#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <pcap.h>

#ifdef _LINUX_
 
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <linux/filter.h>
#include <asm/types.h>

#endif

#ifndef _LINUX_
/*
 *    IEEE 802.3 Ethernet magic constants.  The frame sizes omit the preamble
 *    and FCS/CRC (frame check sequence).
 */

#define ETH_ALEN    6        /* Octets in one ethernet addr     */
#define ETH_HLEN    14        /* Total octets in header.     */
#define ETH_ZLEN    60        /* Min. octets in frame sans FCS */
#define ETH_DATA_LEN    1500        /* Max. octets in payload     */
#define ETH_FRAME_LEN    1514        /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN    4        /* Octets in the FCS         */


#include <dnet.h>

#endif


#include <netinet/in.h>

#ifndef __NO_LUA__
#include <luaT.h>
#include <TH.h>
#endif

/***********************************************************
 * Global Parameters
 **********************************************************/
static const unsigned char neuflow_mac[6] = {0x01,0x02,0x03,0x04,0x05,0x06};
static unsigned char host_mac[6] = {0x01,0x02,0x03,0x04,0x05,0x06};
static const int neuflow_one_encoding = 1<<8;
static int neuflow_first_call = 1;

// socket descriptors
#ifdef _LINUX_
static int socketw;
static struct ifreq ifr;
static struct sockaddr_ll socket_address;
static int ifindex;
static socklen_t socklen;
#else 
static eth_t * socketw;
#endif
static pcap_t *socketr;



/***********************************************************
 * open_socket()
 * what: opens an ethernet socket
 * params:
 *    none
 * returns:
 *    socket - a socket descriptor
 **********************************************************/
int open_socket_C(const char *dev) {

#ifdef _LINUX_
  // Open socket for WRITE
  socketw = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
  if (socketw == -1) {
    perror("socket():");
    exit(1);
  }

  // retrieve ethernet interface index
  strncpy(ifr.ifr_name, dev, IFNAMSIZ);
  if (ioctl(socketw, SIOCGIFINDEX, &ifr) == -1) {
    perror(dev);
    exit(1);
  }
  ifindex = ifr.ifr_ifindex;

  // retrieve corresponding MAC
  if (ioctl(socketw, SIOCGIFHWADDR, &ifr) == -1) {
    perror("GET_HWADDR");
    exit(1);
  }
  if (strcmp(dev,"lo") != 0) {
    int i;
    for (i=0; i<ETH_ALEN; i++) host_mac[i] = ifr.ifr_hwaddr.sa_data[i];
  }

  // prepare sockaddr_ll
  socket_address.sll_family   = AF_PACKET;
  socket_address.sll_protocol = htons(ETH_P_ALL);
  socket_address.sll_ifindex  = ifindex;
  socket_address.sll_hatype   = 0;//ARPHRD_ETHER;
  socket_address.sll_pkttype  = 0;//PACKET_OTHERHOST;
  socket_address.sll_halen    = ETH_ALEN;
  socket_address.sll_addr[0]  = neuflow_mac[0];
  socket_address.sll_addr[1]  = neuflow_mac[1];
  socket_address.sll_addr[2]  = neuflow_mac[2];
  socket_address.sll_addr[3]  = neuflow_mac[3];
  socket_address.sll_addr[4]  = neuflow_mac[4];
  socket_address.sll_addr[5]  = neuflow_mac[5];
  socket_address.sll_addr[6]  = 0x00; 
  socket_address.sll_addr[7]  = 0x00;

  // size of socket
  socklen = sizeof(socket_address);
#else
  dev = "en0";
 socketw = eth_open(dev);
  if (socketw == NULL){
    perror("socketw():");
    printf("Couldn't open device: %s\n", dev);
    exit(1);
  }

#endif



  // open socket for RD (pcap), with a filter
  char errbuf[20];

  int wait_time = 0;
#ifdef _LINUX_
  wait_time = -1;
#else 
  wait_time = 1000;
#endif

  socketr = pcap_open_live(dev, 3000, 1, wait_time, errbuf);
  if (socketr == NULL) {
    fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
    exit(2);
  }

  // set up a filter
  char filter_exp[] = "ether src 01:02:03:04:05:06";
  struct bpf_program fp;
  if (pcap_compile(socketr, &fp, filter_exp, 0, 0) == -1) {
    fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(socketr));
    exit(2);
  }
  if (pcap_setfilter(socketr, &fp) == -1) {
    fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(socketr));
    exit(2);
  }

  // Message
  printf("# libetherflow: using dev %s\n", dev);
#ifdef _LINUX_
  // set buffer sizes
  int sockbufsize = 64*1024*1024;
  unsigned int size = sizeof(int);
  int realbufsize = 0;
  int set_res = setsockopt(socketw, SOL_SOCKET, SO_RCVBUFFORCE, (int *)&sockbufsize, sizeof(int));
  int get_res = getsockopt(socketw, SOL_SOCKET, SO_RCVBUF, &realbufsize, &size);
  if ((set_res < 0)||(get_res < 0)) {
    perror("set/get sockopt");
    close(socketw);
    exit(1);
  }
  printf("# libetherflow: set rx buffer size to %dMB\n", realbufsize/(1024*1024));
  set_res = setsockopt(socketw, SOL_SOCKET, SO_SNDBUFFORCE, (int *)&sockbufsize, sizeof(int));
  get_res = getsockopt(socketw, SOL_SOCKET, SO_SNDBUF, &realbufsize, &size);
  if ((set_res < 0)||(get_res < 0)) {
    perror("set/get sockopt");
    close(socketw);
    exit(1);
  }
  printf("# libetherflow: set tx buffer size to %dMB\n", realbufsize/(1024*1024));
#endif
  return 0;
}


/***********************************************************
 * close_socket()
 * what: closes an ethernet socket
 * params:
 *    socket
 * returns:
 *    none
 **********************************************************/
int close_socket_C() {
#ifdef _LINUX_
  close(socketw);
#else
  eth_close(socketw);
#endif
  pcap_close(socketr);
  return 0;
}


/***********************************************************
 * receive_frame_C()
 * what: receives an ethernet frame
 * params:
 *    socket - socket descriptor. 
 *    buffer - to receive the data
 * returns:
 *    length - nb of bytes read/received
 **********************************************************/
unsigned char * receive_frame_C(int *lengthp) {
  /*length of the received frame*/
  struct pcap_pkthdr header;
  unsigned char *buffer;
  
  while (1) {
    buffer = (unsigned char *)pcap_next(socketr, &header);
    if (buffer != NULL) break;
  }

  if (lengthp != NULL) (*lengthp) = header.len;
  
  //DEBUG
  //printf("recieved: %d bytes\n", header.len);
  //if(header.len == 78) 
  //  printf("%s\n", buffer+14);

  return buffer;
} 


/***********************************************************
 * send_frame_C()
 * what: sends an ethernet frame
 * params:
 *    socket - socket descriptor. 
 *    length - length of data to send
 *    data_p - data pointer
 * returns:
 *    error code 
 **********************************************************/
int send_frame_C(short int length, const unsigned char * data_p) {
  

  // buffer to send:
  unsigned char send_buffer[ETH_FRAME_LEN];
  
  // prepare send_buffer with DEST and SRC addresses 
  memcpy((void*)send_buffer, (void*)neuflow_mac, ETH_ALEN);
#ifdef _LINUX_  
  memcpy((void*)(send_buffer+ETH_ALEN), (void*)host_mac, ETH_ALEN);
#else
  unsigned char enet_src[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
  memcpy((void*)(send_buffer+ETH_ALEN), (void*)enet_src, ETH_ALEN);
#endif
  // copy length to send_buffer
  unsigned char* length_str_reversed = (unsigned char*)&length;
  send_buffer[ETH_ALEN*2] = length_str_reversed[1];
  send_buffer[ETH_ALEN*2+1] = length_str_reversed[0];

  // copy user data to send_buffer
  memcpy((void*)(send_buffer+ETH_HLEN), (void*)data_p, length);

#ifdef _LINUX_
  // send packet
  int sent = sendto(socketw, send_buffer, length+ETH_HLEN, 0,
                    (struct sockaddr*)&socket_address, socklen);
  if (sent == -1) {
    perror("sendto():");
    exit(1);
  }
  /* printf("sending: %d bytes\n", length); */
/*   if (length == 64) */
/*     printf("%s\n", send_buffer); */
#else
  eth_send(socketw, send_buffer, length+ETH_HLEN);
  /* printf("sending: %d bytes\n", length); */
/*   if (length == 64) */
/*     printf("%s\n", send_buffer); */
#endif
  return 0;
}


/***********************************************************
 * send_tensor()
 * what: sends a torch tensor by breaking it down into
 *       ethernet packets of maximum size
 *       a tensor of doubles is converted to Q8.8
 * params:
 *    socket - socket descriptor.
 *    tensor - tensor to send
 * returns:
 *    void
 **********************************************************/
int send_tensor_double_C(double * data, int size) {
  /* get the arguments */
  short int packet_size;
  int elements_pointer = 0;
  unsigned char packet[ETH_FRAME_LEN];
  int i;
  
  // this is the tensor descriptor header
  if (!neuflow_first_call) receive_frame_C(NULL);
  neuflow_first_call = 0;

  //DEBUG - do not remove!
  /* if (!neuflow_first_call){ */
/*     int s_str = 64; */
/*     unsigned char* str = receive_frame_C(&s_str); */
/*     printf("\n%s\n", (str+14)); */
/*   } */

  while(elements_pointer != size){
    // convert double -> Q8.8
    packet_size = 0;
    for (i = 0; i < ETH_DATA_LEN; i+=2){
      if (elements_pointer < size){
	double val = data[elements_pointer];
	short fixed_point = (short)(val * neuflow_one_encoding + 0.5);
	unsigned char* point_to_short = (unsigned char*)&fixed_point;
	packet[i] = point_to_short[0];
	packet[i+1] = point_to_short[1];
	
	elements_pointer++;
	packet_size+=2;
      }
      else break;
    }
    
    // only the last packet could be not dividable by 4
    while(packet_size%4 != 0){
      packet[packet_size] = 0;
      packet_size++;
    }
    
    // smaller than min packet size - pad
    if (packet_size < ETH_ZLEN+4) {
      for(i = packet_size; i < ETH_ZLEN+4; i++) {packet[i] = 0;}
      packet_size = ETH_ZLEN+4;
    }
#ifndef _LINUX_
    usleep(5);
    printf("..........\r"); 
    printf("..........\r"); 
    printf("..........\r"); 
    printf("..........\r"); 
    printf("..........\r"); 
    printf("..........\r"); 
#endif
    // send
    send_frame_C(packet_size, packet);
  }
  return 0;
}

int send_tensor_double_C_ack(double * data, int size) {
  /* get the arguments */
  short int packet_size;
  int elements_pointer = 0;
  unsigned char packet[ETH_FRAME_LEN];
  int i;
  
  // this is the tensor descriptor header
  if (!neuflow_first_call) receive_frame_C(NULL);
  neuflow_first_call = 0;

  //DEBUG - do not remove!
  //if (!neuflow_first_call){
  //  int s_str = 64;
  //  unsigned char* str = receive_frame_C(&s_str);
  //  printf("\n%s\n", (str+14));
  //}

  while(elements_pointer != size){
    // convert double -> Q8.8
    packet_size = 0;
    for (i = 0; i < ETH_DATA_LEN; i+=2){
      if (elements_pointer < size){
	
	double val = data[elements_pointer];
	short fixed_point = (short)(val * neuflow_one_encoding + 0.5);
	unsigned char* point_to_short = (unsigned char*)&fixed_point;
	packet[i] = point_to_short[0];
	packet[i+1] = point_to_short[1];
	
	elements_pointer++;
	packet_size+=2;
      }
      else break;
    }
    
    // only the last packet could be not dividable by 4
    while(packet_size%4 != 0){
      packet[packet_size] = 0;
      packet_size++;
    }
    
    // smaller than min packet size - pad
    if (packet_size < ETH_ZLEN+4) {
      for(i = packet_size; i < ETH_ZLEN+4; i++) {packet[i] = 0;}
      packet_size = ETH_ZLEN+4;
    }
    
#ifndef _LINUX_
    /* usleep(5); */
/*     printf("..........\r");  */
/*     printf("..........\r");  */
/*     printf("..........\r");  */
/*     printf("..........\r");  */
/*     printf("..........\r");  */
/*     printf("..........\r");  */
#endif

    int s_str = 64;
    unsigned char* str = receive_frame_C(&s_str); 
    //printf("\n%s\n", (str+14)); 

    // send
    send_frame_C(packet_size, packet);

    
  }

  return 0;
}





int send_tensor_float_C(float * data, int size) {
  /* get the arguments */
  short int packet_size;
  int elements_pointer = 0;
  unsigned char packet[ETH_FRAME_LEN];
  int i;

  // this is the tensor descriptor header
  if (!neuflow_first_call) receive_frame_C(NULL);
  neuflow_first_call = 0;

  while(elements_pointer != size){
    // convert float -> Q8.8
    packet_size = 0;
    for (i = 0; i < ETH_DATA_LEN; i+=2){
      if (elements_pointer < size){
	float val = data[elements_pointer];
	short fixed_point = (short)(val * neuflow_one_encoding + 0.5);
	unsigned char* point_to_short = (unsigned char*)&fixed_point;
	packet[i] = point_to_short[0];
	packet[i+1] = point_to_short[1];
	
	elements_pointer++;
	packet_size+=2;
      }
      else break;
    }
    
    // only the last packet could be not dividable by 4
    while(packet_size%4 != 0){
      packet[packet_size] = 0;
      packet_size++;
    }
    
    // smaller than min packet size - pad
    if (packet_size < ETH_ZLEN+4) {
      for(i = packet_size; i < ETH_ZLEN+4; i++) {packet[i] = 0;}
      packet_size = ETH_ZLEN+4;
    }
    
    // send
    send_frame_C(packet_size, packet);
  }
  return 0;
}

int send_tensor_byte_C(unsigned char * data, int size) {
  short int packet_size;
  unsigned char packet[ETH_FRAME_LEN];
  int elements_pointer = 0;
  int i;

  // this is the tensor descriptor header
  if (!neuflow_first_call) receive_frame_C(NULL);
  neuflow_first_call = 0;
  

  while(elements_pointer != size) {
    // send raw bytes
    packet_size = 0;
    for (i = 0; i < ETH_DATA_LEN; i++){
      if (elements_pointer < size){
	unsigned char val = data[elements_pointer];
	packet[i] = val;
	elements_pointer++;
	packet_size++;
      }
      else break;
    }
    
    // only the last packet could be not dividable by 4
    while(packet_size%4 != 0){
      packet[packet_size] = 0;
      packet_size++;
    }
    
    // smaller than min packet size - pad
    if (packet_size < ETH_ZLEN+4){ // smaller than min packet size - pad
      for(i = packet_size; i < ETH_ZLEN+4; i++){packet[i] = 0;}
      packet_size = ETH_ZLEN+4;
    }

    // this print is here to give time to the OS to flush the ETH buffers...
    printf("........................................\r"); 

#ifndef _LINUX_
    usleep(100);
    printf("..........\r"); 
    printf("..........\r"); 
    printf("..........\r"); 
    printf("..........\r"); 
    printf("..........\r"); 
    printf("..........\r"); 
#endif

    // send
    send_frame_C(packet_size, packet);
  }
  
 
  /* return the number of results */
  return 0;
}


/***********************************************************
 * receive_tensor_TYPE()
 * what: receives a torch tensor by concatenating eth packs
 *       a tensor of TYPE is created from Q8.8
 * params:
 *    socket - socket descriptor.
 *    tensor - tensor to fill
 * returns:
 *    void 
 **********************************************************/
int receive_tensor_double_C(double *data, int size, int height) {
  int length = 0;
  int currentlength = 0;
  unsigned char *buffer;
  int num_of_bytes = size*2; // each value is 2 bytes
  int tensor_pointer = 0;
  int i;
  int num_of_frames = 0;
  
  
  // this is the tensor descriptor header
  if (!neuflow_first_call) receive_frame_C(NULL);
    //DEBUG - do not remove!
  /* if (!neuflow_first_call){ */
/*     int s_str = 64; */
/*     unsigned char* str = receive_frame_C(&s_str); */
/*     printf("\n%s\n", (str+14)); */
/*   } */

  

  // if not a multiple of 4 the streamToHost function
  //will add an extra line to the stream
  //we want to make sure to receive it here
  if(num_of_bytes%4 != 0){
    num_of_bytes += (size/height)*2;
  }

  while (length < num_of_bytes){
    /* Grab a packet */
    buffer = receive_frame_C(&currentlength);
    length += currentlength-ETH_HLEN;
    num_of_frames++;
    
    /* Save data to tensor*/
    for (i = ETH_HLEN; tensor_pointer < size && i < currentlength; i+=2){ 
      short* val_short = (short*)&buffer[i]; 
      double val = (double)*val_short; 
      val /= neuflow_one_encoding; 
      data[tensor_pointer] = val; 
      tensor_pointer++; 
    } 

    //printf("num_of_frame = %d, num_of_bytes = %d, length = %d, current_length = %d, tensor_pointer = %d, i = %d\n", num_of_frames, num_of_bytes, length, currentlength, tensor_pointer, i); 
  }
  //DEBUG - do not remove!
  //printf("num_of_frame = %d, num_of_bytes = %d, length = %d, current_length = %d, tensor_pointer = %d, i = %d\n", num_of_frames, num_of_bytes, length, currentlength, tensor_pointer, i); 
  // send ack after each tensor
  send_frame_C(64,
               (unsigned char *)"1234567812345678123456781234567812345678123456781234567812345678");
  return 0;
}


int receive_tensor_double_C_ack(double *data, int size, int height) {
  int length = 0;
  int currentlength = 0;
  unsigned char *buffer;
  int num_of_bytes = size*2; // each value is 2 bytes
  int tensor_pointer = 0;
  int i;
  int num_of_frames = 0;
  
  
  // this is the tensor descriptor header
  //if (!neuflow_first_call) receive_frame_C(NULL);
  //DEBUG - do not remove!
  if (neuflow_first_call){
    int s_str = 64;
    unsigned char* str = receive_frame_C(&s_str);
    //printf("\n%s\n", (str+14));
    
    s_str = 64;
    str = receive_frame_C(&s_str);
    //printf("\n%s\n", (str+14));
  }
  else {
    int s_str = 64;
    unsigned char* str = receive_frame_C(&s_str);
    //printf("\n%s\n", (str+14));
  }
  

  // if not a multiple of 4 the streamToHost function
  //will add an extra line to the stream
  //we want to make sure to receive it here
  if(num_of_bytes%4 != 0){
    num_of_bytes += (size/height)*2;
  }

  while (length < num_of_bytes){
    /* Grab a packet */
    buffer = receive_frame_C(&currentlength);
    length += currentlength-ETH_HLEN;
    num_of_frames++;

    // send ack after each frame
     send_frame_C(64, (unsigned char *)"1234567812345678123456781234567812345678123456781234567812345678");
   
    
    /* Save data to tensor*/
    for (i = ETH_HLEN; tensor_pointer < size && i < currentlength; i+=2){ 
      short* val_short = (short*)&buffer[i]; 
      double val = (double)*val_short; 
      val /= neuflow_one_encoding; 
      data[tensor_pointer] = val; 
      tensor_pointer++; 
    } 

    
  //DEBUG - do not remove!
    //printf("num_of_frame = %d, num_of_bytes = %d, length = %d, current_length = %d, tensor_pointer = %d, i = %d\n", num_of_frames, num_of_bytes, length, currentlength, tensor_pointer, i);

  }
  
  // send ack after each tensor
  // send_frame_C(64,
  //(unsigned char *)"1234567812345678123456781234567812345678123456781234567812345678");
  return 0;
}




int receive_tensor_float_C(float *data, int size, int height) {
  int length = 0;
  int currentlength = 0;
  unsigned char *buffer;
  int num_of_bytes = size*2; // each value is 2 bytes
  int tensor_pointer = 0;
  int i;
  int num_of_frames = 0;
  
  // this is the tensor descriptor header
  if (!neuflow_first_call) receive_frame_C(NULL);
  
  // if not a multiple of 4 the streamToHost function
  //will add an extra line to the stream
  //we want to make sure to receive it here
  if(num_of_bytes%4 != 0){
    num_of_bytes += (size/height)*2;
  }
  
  while (length < num_of_bytes){
    /* Grab a packet */
    buffer = receive_frame_C(&currentlength);
    length += currentlength-ETH_HLEN;
    num_of_frames++;
    
    /* Save data to tensor*/
    for (i = ETH_HLEN; tensor_pointer < size && i < currentlength; i+=2){ 
      short* val_short = (short*)&buffer[i];
      float val = (float)*val_short;
      val /= neuflow_one_encoding;
      data[tensor_pointer] = val; 
      tensor_pointer++; 
    } 
  }

  // send ack after each tensor
  send_frame_C(64, 
               (unsigned char *)"1234567812345678123456781234567812345678123456781234567812345678");
  return 0;
}


#ifndef __NO_LUA__
/***********************************************************
 * Lua wrappers
 **********************************************************/
static int receive_tensor_lua(lua_State *L){
  /* get the arguments */
  THTensor * result = luaT_toudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  double * data = result->storage->data+result->storageOffset;
  int size = result->size[0]*result->size[1];
  receive_tensor_double_C(data, size, result->size[1]);
  return 0;
}

static int receive_tensor_lua_ack(lua_State *L){
  /* get the arguments */
  THTensor * result = luaT_toudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  double * data = result->storage->data+result->storageOffset;
  int size = result->size[0]*result->size[1];
  receive_tensor_double_C_ack(data, size, result->size[1]);
  return 0;
}


static int receive_tensor_float_lua(lua_State *L){
  /* get the arguments */
  THFloatTensor * result = luaT_toudata(L, 1, luaT_checktypename2id(L, "torch.FloatTensor"));
  float * data = result->storage->data+result->storageOffset;
  int size = result->size[0]*result->size[1];
  receive_tensor_float_C(data, size, result->size[1]);
  return 0;
}

static int send_tensor_lua(lua_State *L) {
  /* get the arguments */
  THTensor * tensorToSend = luaT_toudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  int size = tensorToSend->size[0] * tensorToSend->size[1];
  double *data = tensorToSend->storage->data+tensorToSend->storageOffset;
  send_tensor_double_C(data, size);
  return 0;
}

static int send_tensor_float_lua(lua_State *L) {
  /* get the arguments */
  THFloatTensor * tensorToSend = luaT_toudata(L, 1, luaT_checktypename2id(L, "torch.FloatTensor"));
  int size = tensorToSend->size[0] * tensorToSend->size[1];
  float *data = tensorToSend->storage->data+tensorToSend->storageOffset;
  send_tensor_float_C(data, size);
  return 0;
}

static int send_tensor_byte_lua(lua_State *L) {
  // get params
  THByteTensor * tensorToSend = luaT_toudata(L, 1, luaT_checktypename2id(L, "torch.ByteTensor"));
  int size = tensorToSend->size[0];
  unsigned char * data = tensorToSend->storage->data+tensorToSend->storageOffset;
  send_tensor_byte_C(data, size);
  return 0;
}

static int open_socket_lua(lua_State *L) {
  // Get optional arg
  char default_dev[] = "eth0";
  const char *dev;
  if (lua_isstring(L,1)) {
    dev = lua_tostring(L,1);
  } else {
    dev = default_dev;
  }
  open_socket_C(dev);
  return 0;
}

static int close_socket_lua(lua_State *L) {
  close_socket_C();
  return 0;
}

static int send_frame_lua(lua_State *L) {
  /* get the arguments */
  const char * data_p = lua_tostring(L, 1);
  int length = strlen(data_p);
  return send_frame_C(length, (unsigned char *)data_p);
}

static int send_tensor_lua_ack(lua_State *L) {
  /* get the arguments */
  THTensor * tensorToSend = luaT_toudata(L, 1, luaT_checktypename2id(L, "torch.Tensor"));
  int size = tensorToSend->size[0] * tensorToSend->size[1];
  double *data = tensorToSend->storage->data+tensorToSend->storageOffset;
  send_tensor_double_C_ack(data, size);
  return 0;
}


static int receive_string_lua(lua_State *L) {
  // receive frame
  int length;
  unsigned char *buffer = receive_frame_C(&length);
  
  // Protection: insert a 0 in case
  buffer[length] = 0;
  
  // Push string
  lua_pushstring(L, (char *)(buffer+ETH_HLEN));
  return 1;
}

static int receive_frame_lua(lua_State *L) {
  /* get the arguments */
  int length;
  unsigned char *buffer = receive_frame_C(&length);

  lua_pushnumber(L, length-ETH_HLEN);
  lua_newtable(L);
  
  int i;
  for(i=ETH_HLEN; i<length; i++){
    lua_pushnumber(L, i);
    lua_pushnumber(L, buffer[i]);
    lua_settable(L, -3);
  }
  return 2;
}

static int set_first_call(lua_State *L) {
  /* get the arguments */
  int val = lua_tointeger(L, 1);
  neuflow_first_call = val;
  //DEBUG
  //printf("set first call to %d\n", neuflow_first_call);
  return 0;
}


/***********************************************************
 * register functions for Lua
 **********************************************************/
static const struct luaL_reg etherflow [] = {
  {"open_socket", open_socket_lua},
  {"receive_frame",receive_frame_lua},
  {"receive_string",receive_string_lua},
  {"send_frame", send_frame_lua},
  {"send_tensor", send_tensor_lua},
  {"send_tensor_ack", send_tensor_lua_ack},
  {"send_tensor_float", send_tensor_float_lua},
  {"send_bytetensor", send_tensor_byte_lua},
  {"receive_tensor", receive_tensor_lua},
  {"receive_tensor_ack", receive_tensor_lua_ack},
  {"receive_tensor_float", receive_tensor_float_lua},
  {"close_socket", close_socket_lua},
  {"set_first_call", set_first_call},
  {NULL, NULL}  /* sentinel */
};

int luaopen_libetherflow(lua_State *L) {
  luaL_openlib(L, "libetherflow", etherflow, 0);
  return 1; 
}
#endif

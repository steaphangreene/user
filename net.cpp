#include <string.h>
#include <user.h>

#ifdef X_WINDOWS
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#ifdef DOS
#include <dpmi.h>
#endif

extern int errno;
extern Network *__Da_Network;

Network::Network()  {
  max_others = 16;
  qhead = 0;
  type = NETWORK_NONE;
  max_packet = 1024;
  last_source = 0;
  TCPIPbackup = NULL;
  max_others = 16;
  others = new Other[max_others];
  bzero(others, max_others * sizeof(Other));
  __Da_Network = this;
  }

void Network::IPX_ChangeSocket(unsigned short sock)  {
#ifdef DOS
  int ctr;
  CloseSocket(FlipEnd(IPXsock));
  OpenSocket(sock);
  for(ctr=0; ctr<NET_QUEUE_SIZE; ctr++)  {
    recb[ctr].socket = IPXsock;
    dosmemput(&recb[ctr], sizeof(ECB), (recb_seg[ctr]<<4));
    }

  IPX_Header tmphead;
  bzero(&tmphead, sizeof(IPX_Header));
  tmphead.packet_type = 4;
  tmphead.destination.socket = (unsigned short)IPXsock;
  tmphead.destination.node_address[0] = 0xFF;
  tmphead.destination.node_address[1] = 0xFF;
  tmphead.destination.node_address[2] = 0xFF;
  tmphead.destination.node_address[3] = 0xFF;
  tmphead.destination.node_address[4] = 0xFF;
  tmphead.destination.node_address[5] = 0xFF;
  dosmemput(&tmphead, sizeof(IPX_Header), (sblock_seg<<4));

  secb.socket = IPXsock;
  dosmemput(&secb, sizeof(ECB), (secb_seg<<4));

  for(ctr=0; ctr<NET_QUEUE_SIZE; ctr++)  {
    recb[ctr].socket = IPXsock;
    dosmemput(&recb[ctr], sizeof(ECB), (recb_seg[ctr]<<4));
    }
  for(ctr=NET_QUEUE_SIZE-1; ctr>=0; ctr--)
    ReceiveWith(recb_seg[ctr]);
  qhead = 0;
#endif
  }

void Network::SetTCPIP(char *addr, unsigned short port)  {
#ifdef X_WINDOWS
  if((!TCPIP_ConnectRemote(addr, port)) && (!TCPIP_ServeLocal(port)))
    return;
  TCPIP_NewOther(TCPIPsock);
  type |= NETWORK_TCPIP;
  if(write(TCPIPsock, "\r\n\0\0", 4) < 0)  {
    if(errno != EPIPE)  Exit(0, "Unable to write to socket\r\n");
    else return;
    }
#endif
  }

#ifdef X_WINDOWS
int Network::TCPIP_ServeLocal(unsigned short sock)  {
  socklen_t add_size = sizeof(sockaddr_in);
  sockaddr_in saddr;
  int Ssock;
  Ssock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (Ssock < 0)  {
    return (1==2);
    }
  saddr.sin_family=AF_INET;
  saddr.sin_addr.s_addr=htonl(INADDR_ANY);
  saddr.sin_port=htons(sock);
  if (bind(Ssock, (sockaddr *)&saddr, add_size) < 0)  {
    return (1==2);
    }
  if (listen(Ssock, 1) < 0)  {
    return (1==2);
    }
  TCPIPsock = accept(Ssock, (sockaddr *)&saddr, &add_size);
  if (TCPIPsock < 0)  {
    return (1==2);
    }
  termios attrib;
  fcntl(TCPIPsock, F_SETFL, O_NONBLOCK);
  tcgetattr(TCPIPsock, &attrib);
  cfmakeraw(&attrib);
  tcsetattr(TCPIPsock, TCSANOW, &attrib);
  return (1==1);
  }

int Network::TCPIP_ConnectRemote(char *addr, unsigned short sock)  {
  TCPIPsock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (TCPIPsock < 0)  {
    return (1==2);
    }
  sockaddr_in saddr;
  saddr.sin_family=AF_INET;
  saddr.sin_addr.s_addr=inet_addr(addr);
  saddr.sin_port=htons(sock);
  if ((connect(TCPIPsock, (sockaddr *)&saddr, sizeof(sockaddr_in))) < 0)  {
    close(TCPIPsock);
    return (1==2);
    }
  termios attrib;
  fcntl(TCPIPsock, F_SETFL, O_NONBLOCK);
  tcgetattr(TCPIPsock, &attrib);
  cfmakeraw(&attrib);
  tcsetattr(TCPIPsock, TCSANOW, &attrib);
//  setsockopt(TCPIPsock, TCP_NODELAY, 1, NULL, 1);
//  setsockopt(TCPIPsock, TCP_MAXSEG, 1, NULL, 1);
  return (1==1);
  }
#endif

void Network::SetIPX(char *head, unsigned short sock)  {
#ifdef DOS
  int ctr;
  int tmp_handle;
  header_len = strlen(head);
  header = new char[header_len+1];
  strcpy(header, head);

  __dpmi_regs dregs;
  dregs.x.ax = 0x7A00;
  __dpmi_int(0x2F, &dregs);
  if(dregs.h.al != 0xFF)  {
    return;
    }
  dregs.x.bx = 0x001A;
  __dpmi_int(0x7A, &dregs);
  max_packet = (long)(short)dregs.x.ax;
  if(max_packet < 576)  {
    return;
    }
  if(!OpenSocket(sock))  return;
  SetNetworkAddress();
  type |= NETWORK_IPX;

  IPX_Header tmphead;
  sblock_seg = __dpmi_allocate_dos_memory((max_packet+15)>>4, &tmp_handle);
  sblock_offset = (sblock_seg<<4);
  handles += tmp_handle;
  for(ctr=0; ctr<NET_QUEUE_SIZE; ctr++)  {
    rblock_seg[ctr] = __dpmi_allocate_dos_memory((max_packet+15)>>4, &tmp_handle);
    handles += tmp_handle;
    bzero(&recb[ctr], sizeof(ECB));
    }
  bzero(&secb, sizeof(ECB));
  bzero(&tmphead, sizeof(IPX_Header));

  tmphead.packet_type = 4;
  tmphead.destination.socket = (unsigned short)IPXsock;
  tmphead.destination.node_address[0] = 0xFF;
  tmphead.destination.node_address[1] = 0xFF;
  tmphead.destination.node_address[2] = 0xFF;
  tmphead.destination.node_address[3] = 0xFF;
  tmphead.destination.node_address[4] = 0xFF;
  tmphead.destination.node_address[5] = 0xFF;
  dosmemput(&tmphead, sizeof(IPX_Header), sblock_offset);
  sblock_offset+=sizeof(IPX_Header);
  dosmemput(head, header_len, sblock_offset);
  sblock_offset+=header_len;
  head_size = sblock_offset-(sblock_seg<<4);
  for(ctr=0; ctr<NET_QUEUE_SIZE; ctr++)
    rblock_offset[ctr] = (rblock_seg[ctr]<<4)+head_size;

  secb.socket = IPXsock;
  secb.fragment_count = 1;
  secb.fragment_data = ((unsigned long)sblock_seg)<<16;
  secb.fragment_size = head_size;

  for(ctr=0; ctr<NET_QUEUE_SIZE; ctr++)  {
    recb[ctr].socket = IPXsock;
    recb[ctr].fragment_count = 1;
    recb[ctr].fragment_data = ((unsigned long)rblock_seg[ctr])<<16;
    recb[ctr].fragment_size = max_packet;

    recb_seg[ctr] = __dpmi_allocate_dos_memory((sizeof(ECB)+15)/16, &tmp_handle);
    handles += tmp_handle;
    }
  for(ctr=0; ctr<NET_QUEUE_SIZE; ctr++)
    dosmemput(&recb[ctr], sizeof(ECB), (recb_seg[ctr]<<4));

  secb_seg = __dpmi_allocate_dos_memory((sizeof(ECB)+15)/16, &tmp_handle);
  handles += tmp_handle;
  dosmemput(&secb, sizeof(ECB), (secb_seg<<4));
  for(ctr=NET_QUEUE_SIZE-1; ctr>=0; ctr--)
    ReceiveWith(recb_seg[ctr]);
#endif
#ifdef X_WINDOWS
#ifdef AF_IPX
  printf("IPX supported!!!\r\n");
#endif
#endif

  }

Network::~Network()  {
  __Da_Network = NULL;
  if(type & NETWORK_IPX)  {

#ifdef DOS
    for(ctr=0; ctr<handles.Size(); ctr++)  {
      __dpmi_free_dos_memory(handles[ctr]);
      }
#endif

    }
  if(type & NETWORK_TCPIP)  {

#ifdef X_WINDOWS
    close(TCPIPsock);
#endif

    }
  }

void Network::SetNetworkAddress()  {
#ifdef DOS
  unsigned char buf[10];
  unsigned long seg;
  int handle;
  __dpmi_regs dregs;

  seg = __dpmi_allocate_dos_memory(1, &handle);

  dregs.x.bx = 0x0009;
  dregs.x.es = seg;
  dregs.x.si = 0x0000;

  __dpmi_int(0x7A, &dregs);
  dosmemget((seg<<4), 10,  buf);
  __dpmi_free_dos_memory(handle);
  local_address[0] = buf[4];
  local_address[1] = buf[5];
  local_address[2] = buf[6];
  local_address[3] = buf[7];
  local_address[4] = buf[8];
  local_address[5] = buf[9];
  printf("Local address = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\r\n",
	(int)local_address[0],  (int)local_address[1],  (int)local_address[2],
	(int)local_address[3],  (int)local_address[4],  (int)local_address[5]);
#endif
  }

int Network::OpenSocket(unsigned short sock)  {
#ifdef DOS
  IPXsock = FlipEnd(sock);
  __dpmi_regs dregs;
  dregs.h.al = 0x00;
  dregs.x.bx = 0x0000;
  dregs.x.dx = IPXsock;
  __dpmi_int(0x7A, &dregs);

  return(dregs.h.al == 0x00);
#endif

#ifdef X_WINDOWS
  return(1==2);
#endif
  }

void Network::CloseSocket(unsigned short sock)  {
#ifdef DOS
  __dpmi_regs dregs;
  dregs.x.bx = 0x0001;
  dregs.x.dx = FlipEnd(sock);
  __dpmi_int(0x7A, &dregs);
#endif
  }

void Network::Send(void *mes, int len)  {
  if(len>(10*GetMaxPacketSize()))  Exit(0, "Packet Too Big!!\n");
  int clen = len <? GetMaxPacketSize();
  char sb[4+clen];
  sb[0] = (len & 255);
  sb[1] = ((len>>8) & 255);
  sb[2] = ((len>>16) & 255);
  sb[3] = ((len>>24) & 255);
  memcpy(&sb[4], mes, clen);
  if(type & NETWORK_IPX)  {
#ifdef DOS
  int ctr;
  secb.fragment_size = head_size+clen+4;
  for(ctr=0; ctr < 6; ctr++)  {
    secb.immediate_address[ctr] = 0xFF;
    }

  dosmemput(&secb, sizeof(ECB), (secb_seg<<4));
  dosmemput(sb, clen+4, sblock_offset);

  __dpmi_regs dregs;
  dregs.x.bx = 0x0003;
  dregs.x.es = secb_seg;
  dregs.x.si = 0x0000;
  __dpmi_int(0x7A, &dregs);

  if(len>clen)  Send(&((char*)mes)[clen], len-clen);
#endif

    }
  if(type & NETWORK_TCPIP)  {

#ifdef X_WINDOWS
    if(write(TCPIPsock, sb, len+4) < 0)  {
      if(errno != EPIPE)  Exit(0, "Unable to write to socket\r\n");
      else return;
      }
    if(write(TCPIPsock, "\r\n\0\0", 4) < 0)  {
      if(errno != EPIPE)  Exit(0, "Unable to write to socket\r\n");
      else return;
      }
#endif

    }
  }

void Network::Send(void *mes, int len, int who)  {
  char sb[4+len];
  sb[0] = (len & 255);
  sb[1] = ((len>>8) & 255);
  sb[2] = ((len>>16) & 255);
  sb[3] = ((len>>24) & 255);
  memcpy(&sb[4], mes, len);

  if(type & NETWORK_IPX)  {
#ifdef DOS
    int ctr;
    secb.fragment_size = head_size+len+4;

//    memcpy(secb.immediate_address, others[who].ipx.addr, 6);
  for(ctr=0; ctr < 6; ctr++)  {
    secb.immediate_address[ctr] = 0xFF;
    }

    dosmemput(&secb, sizeof(ECB), (secb_seg<<4));
    dosmemput(sb, len+4, sblock_offset);

    __dpmi_regs dregs;
    dregs.x.bx = 0x0003;
    dregs.x.es = secb_seg;
    dregs.x.si = 0x0000;
    __dpmi_int(0x7A, &dregs);
#endif

    }
  if(type & NETWORK_TCPIP)  {

#ifdef X_WINDOWS
    if(write(TCPIPsock, sb, len+4) < 0)  {
      if(errno != EPIPE)  Exit(0, "Unable to write to socket\r\n");
      else return;
      }
    if(write(TCPIPsock, "\r\n\0\0", 4) < 0)  {
      if(errno != EPIPE)  Exit(0, "Unable to write to socket\r\n");
      else return;
      }
#endif

    }
  }

void Network::ReceiveWith(unsigned long in_seg)  {
#ifdef DOS
    __dpmi_regs dregs;
    dregs.x.bx = 0x0004;
    dregs.x.es = in_seg;
    dregs.x.si = 0x0000;
    __dpmi_int(0x7A, &dregs);
#endif
  }

int Network::PacketReceived()  {
#ifdef DOS
  int ctr;
  Update();
  dosmemget((recb_seg[qhead]<<4), sizeof(ECB), &recb[qhead]);

  if(recb[qhead].in_use != 0)  return(1==2);

  char headbuf[header_len+2];
  bzero(headbuf, header_len+2);
  dosmemget(rblock_offset[qhead]-header_len, header_len, headbuf);
  if(!strncmp(header, headbuf, header_len))  return (1==1);

  if((qhead+1) < NET_QUEUE_SIZE)  {
    dosmemget((recb_seg[qhead+1]<<4), sizeof(ECB), &recb[qhead+1]);
    }
  if(((qhead+1) >= NET_QUEUE_SIZE) || (recb[qhead+1].in_use != 0))  {
    for(ctr=qhead; ctr >= 0; ctr--)  {
      ReceiveWith(recb_seg[ctr]);
      }
    qhead = 0;
    return(1==2);
    }

  qhead++;
  return  PacketReceived();
#endif

#ifdef X_WINDOWS
  if(TCPIPbackup != NULL)  return (1==1);
  TCPIPbackup = (unsigned char *)Receive();
  if(TCPIPbackup != NULL)  return (1==1);
  return (1==2);
#endif
  }

int Network::LastSource()  {
  return last_source;
  }

void *Network::Receive()  {
  if(type & NETWORK_IPX)  {

#ifdef DOS
  int ctr, rsize;
  dosmemget((recb_seg[qhead]<<4), sizeof(ECB), &recb[qhead]);
  if(recb[qhead].in_use != 0)  {
    Update();
    dosmemget((recb_seg[qhead]<<4), sizeof(ECB), &recb[qhead]);
    }
  if(recb[qhead].in_use != 0)  return NULL;
  rsize = (recb[qhead].fragment_size-head_size)-4;
  if(rsize > 0)  {
    int size;
    unsigned char addbuf[12];
    char headbuf[header_len+2];
    bzero(headbuf, header_len+2);
    dosmemget(rblock_offset[qhead]-header_len, header_len, headbuf);
    dosmemget(rblock_offset[qhead], 4, &size);
//    if(rsize > GetMaxPacketSize() && rsize > size)
//	Exit(1, "Network Error: rsize(%d) > size(%d) ?!?!\n", rsize, size);
    if(rsize > size) rsize = size;
    char *retbuf = new char[size];
    dosmemget(rblock_offset[qhead]+4, rsize, retbuf);

    dosmemget((rblock_offset[qhead]-header_len)-12, 12, addbuf);

    if((qhead+1) < NET_QUEUE_SIZE)  {
      dosmemget((recb_seg[qhead+1]<<4), sizeof(ECB), &recb[qhead+1]);
      }
    if(((qhead+1) >= NET_QUEUE_SIZE) || (recb[qhead+1].in_use != 0))  {
      for(ctr=qhead; ctr >= 0; ctr--)  {
	ReceiveWith(recb_seg[ctr]);
	}
      qhead = 0;
      }
    else  {
      qhead++;
      }

    if(addbuf[4] == local_address[0] && addbuf[5] == local_address[1]
	&& addbuf[6] == local_address[2] && addbuf[7] == local_address[3]
	&& addbuf[8] == local_address[4] && addbuf[9] == local_address[5])  {
      delete retbuf;
      if(PacketReceived())
	return Receive();
      else
	return NULL;
      }

    for(ctr=0; ctr<max_others && (others[ctr].type != OTHER_IPX
	|| others[ctr].ipx.addr[0] != addbuf[4]
	|| others[ctr].ipx.addr[1] != addbuf[5]
	|| others[ctr].ipx.addr[2] != addbuf[6]
	|| others[ctr].ipx.addr[3] != addbuf[7]
	|| others[ctr].ipx.addr[4] != addbuf[8]
	|| others[ctr].ipx.addr[5] != addbuf[9]); ctr++);
    if(ctr >= max_others)
      last_source = IPX_NewOther((unsigned char *)&addbuf[4]);
    else last_source = ctr;
    if(!strncmp(header, headbuf, header_len))  {
//      printf("Incoming address = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\r\n",
//		(int)others[last_source].ipx.addr[0],
//		(int)others[last_source].ipx.addr[1],
//		(int)others[last_source].ipx.addr[2],
//		(int)others[last_source].ipx.addr[3],
//		(int)others[last_source].ipx.addr[4],
//		(int)others[last_source].ipx.addr[5]);
      if(rsize == size)  {
	return retbuf;
	}
      else  {
	char *tmpc=NULL;
	while(tmpc == NULL) tmpc = (char*)Receive();
	memcpy(&retbuf[rsize], tmpc, size-rsize);
	return retbuf;
	}
      }
    else  {
      delete retbuf;
      if(PacketReceived())
	return Receive();
      else
	return NULL;
      }
    }
  else  return NULL;
#endif

    }
  if(type & NETWORK_TCPIP)  {

#ifdef X_WINDOWS
    int len, ctr;
    static int gotfirst = 0;
    unsigned char *buf = TCPIPbackup;
    TCPIPbackup = NULL;
    if(buf != NULL)  return buf;
    buf = new unsigned char[PACKET_SIZE];
    int sz = 0;
    sz = read(TCPIPsock, &buf[gotfirst], 8-gotfirst);
//    if (sz != -1)  printf("sz = %d\r\n", sz);
    if(sz == 0 || (sz < 0 && (errno == EAGAIN || errno != ENOTCONN
	|| errno != EPIPE)))  {
      delete buf;
      return NULL;
      }
    if(sz < 0)  {
      perror("Net-1");
      Exit(1, "Read error on socket (%d)!\r\n", errno);
      }
    sz += gotfirst;
//    if(sz < 8)  printf("Bad\r\n");
    if(sz < 5)  {
      gotfirst = sz;
      delete buf;
      return NULL;
      }
    while(sz < 8)  {
      if(sz < 0) sz = 0;
      gotfirst += sz;
      sz = read(TCPIPsock, &buf[gotfirst], 8-gotfirst);
      if(sz < 0 && errno != EAGAIN)  {
        perror("Net-2");
	Exit(1, "Read error on socket (%d)!\r\n", errno);
        }
      if(sz >= 0) sz += gotfirst;
      }
    gotfirst = 0;
//    printf("Resolved\r\n");
    len = buf[4] | (buf[5]<<8) | (buf[6]<<16) | (buf[7]<<24);
//    printf("len = %d\r\n", len);
    buf[0] = 0;
    sz = read(TCPIPsock, buf, len);
    int gotten = 0;
//    printf("sz2 = %d\r\n", sz);
    while(sz < len || (sz < 0 && errno == EAGAIN))  {
//      printf("Bad2\r\n");
      if(sz < 0) sz = 0;
      gotten += sz;
      sz = read(TCPIPsock, &buf[gotten], len-gotten);
      if(sz >= 0) sz += gotten;
      }
    if(sz < 0)  {
      perror("Net-3");
      Exit(1, "Read error on socket!\r\n");
      }
    for(ctr=0; ctr<max_others && (others[ctr].type != OTHER_TCPIP
	|| others[ctr].tcpip.sock != TCPIPsock); ctr++);
    if(ctr >= max_others)  Exit(0, "TCP/IP Packet from unknown address!!!\r\n");
    last_source = ctr;
//    printf("Done!!\r\n");
    return buf;
#endif
    }
  return NULL;
  }

int Network::TCPIP_NewOther(int sock)  {
  int ctr;
  for(ctr=0; ctr<max_others && others[ctr].type != OTHER_NONE; ctr++);
  if(ctr >= max_others)  Exit(0, "Too many others on Network!!!\r\n");
  others[ctr].type = OTHER_TCPIP;
  others[ctr].tcpip.sock = sock;
  return ctr;
  }

int Network::IPX_NewOther(unsigned char *addr)  {
  int ctr;
  for(ctr=0; ctr<max_others && others[ctr].type != OTHER_NONE; ctr++);
  if(ctr >= max_others)  Exit(0, "Too many others on Network!!!\r\n");
  others[ctr].type = OTHER_IPX;
  others[ctr].ipx.addr[0] = addr[0];
  others[ctr].ipx.addr[1] = addr[1];
  others[ctr].ipx.addr[2] = addr[2];
  others[ctr].ipx.addr[3] = addr[3];
  others[ctr].ipx.addr[4] = addr[4];
  others[ctr].ipx.addr[5] = addr[5];
  printf("Added address = %.2X:%.2X:%.2X:%.2X:%.2X:%.2X\r\n",
	addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
  return ctr;
  }

void Network::Update()  {
#ifdef DOS
  __dpmi_regs dregs;
  dregs.x.bx = 0x000A;
  dregs.x.es = recb_seg[qhead];
  dregs.x.si = 0x0000;
  __dpmi_int(0x7A, &dregs);
#endif
  }

void *Network::Receive(unsigned long who)  {
  return Receive();
  }

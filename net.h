#ifndef INSOMNIA_USER_NET_H
#define INSOMNIA_USER_NET_H

#include "list.h"

#define NETWORK_NONE	0
#define NETWORK_IPX	1
#define NETWORK_TCPIP	2
#define NETWORK_FULL	3

#define NET_QUEUE_SIZE	16

#pragma pack(1)
#define PACKET_SIZE     1048576

struct net_address  {                                                        
  unsigned char net_address[4]   __attribute__((packed));  /* Network address */
  unsigned char node_address[6]  __attribute__((packed));  /* Node address */
  unsigned short socket __attribute__((packed));  /* Big endian socket number */
  };

#define OTHER_NONE      0
#define OTHER_IPX       1
#define OTHER_TCPIP     2

struct IPX_Other  {
  int type;
  unsigned char addr[6]  __attribute__((packed));
  };

struct TCPIP_Other  {
  int type;
  int sock;
  };

union Other  {
  int type;
  IPX_Other ipx;
  TCPIP_Other tcpip;
  };
   
struct ECB  {
  unsigned long     link              __attribute__((packed)); 
	/* Pointer to next ECB */
  unsigned long     ESR               __attribute__((packed)); 
	/* Event service routine 00000000h if none */
  unsigned char     in_use            __attribute__((packed)); 
	/* In use flag */
  unsigned char     complete          __attribute__((packed)); 
	/* Completing flag */
  unsigned short    socket            __attribute__((packed)); 
	/* Big endian socket number */
  unsigned char     IPX_work[4]       __attribute__((packed)); 
	/* IPX work space */
  unsigned char     D_work[12]        __attribute__((packed)); 
	/* Driver work space */
  unsigned char immediate_address[6]           __attribute__((packed)); 
	/* Immediate local node address */
  unsigned short    fragment_count    __attribute__((packed)); 
	/* Fragment count */
  unsigned long     fragment_data     __attribute__((packed)); 
	/* Pointer to data fragment */
  unsigned short    fragment_size     __attribute__((packed)); 
	/* Size of data fragment */
  };

struct IPX_Header  {
     unsigned short   checksum          __attribute__((packed));   
	/* Big endian checksum */
     unsigned short   length            __attribute__((packed));   
	/* Big endian length in bytes */
     unsigned char    transport_control __attribute__((packed));   
	/* Transport control */
     unsigned char    packet_type       __attribute__((packed));   
	/* Packet type */
     net_address destination       __attribute__((packed));   
	/* Destination network address */
     net_address source            __attribute__((packed));   
	/* Source network address */
    };
#pragma pack()


class Network  {
  public:
  Network();
  void SetTCPIP(char *, unsigned short);
  void SetIPX(char *, unsigned short);
  ~Network();
  void Send(void *, int);
  unsigned long WhoIsThere();
  void ReceiveWith(unsigned long);
  void *Receive();
  int PacketReceived();
  void Send(void *, int, int);
  void *Receive(unsigned long);
  int GetMaxPacketSize()  {return max_packet-40;};
  char Type()  {return type;};
  unsigned char *GetNetworkAddress() { return local_address; };
  int LastSource();
  void Update();
  void IPX_ChangeSocket(unsigned short);
  unsigned short FlipEnd(unsigned short in)
    {return (unsigned short)(((unsigned short)(in >> 8)) | ((unsigned short)(in << 8)));};

  private:
  Other *others;
  int max_others;

  void SetNetworkAddress();
  int IPX_NewOther(unsigned char *);
  ECB secb, recb[NET_QUEUE_SIZE];
  unsigned short IPXsock;
  unsigned long sblock_seg, sblock_offset;
  unsigned long rblock_seg[NET_QUEUE_SIZE], rblock_offset[NET_QUEUE_SIZE];
  unsigned long secb_seg;
  unsigned long recb_seg[NET_QUEUE_SIZE];
  int OpenSocket(unsigned short);
  void CloseSocket(unsigned short);
  char type;
  int max_packet;
  IntList handles;
  unsigned char local_address[6];
  int last_source;
  int head_size;
  int qhead;
  int header_len;
  char *header;

  int TCPIP_NewOther(int);
  int TCPIPsock;
  unsigned char *TCPIPbackup;

#ifdef X_WINDOWS
  int TCPIP_ConnectRemote(char *, unsigned short);
  int TCPIP_ServeLocal(unsigned short);
#endif
  };

#endif

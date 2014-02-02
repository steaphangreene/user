#ifndef INSOMNIA_USER_VESA_H
#define INSOMNIA_USER_VESA_H

#define PACKED __attribute__ ((packed))
#pragma pack(1)

struct VESAInfo {
  char    VESASignature[4] PACKED;      /* 'VESA' 4 byte signature          */
  short   VESAVersion PACKED;           /* VESA version number               */
  char    *OEMStringPtr PACKED;         /* Pointer to OEM string            */
  int     Capabilities PACKED;          /* Capabilities of video card       */
  short   *VideoModePtr PACKED;         /* Pointer to supported modes       */
  short   TotalMemory PACKED;           /* Number of 64kb memory blocks     */
  char    reserved[236] PACKED;         /* Pad to 256 byte block size       */
  };

struct  VESAModeInfo {
  short   ModeAttributes PACKED;        /* Mode attributes                  */
  char    WinAAttributes PACKED;        /* Window A attributes              */
  char    WinBAttributes PACKED;        /* Window B attributes              */
  short   WinGranularity PACKED;        /* Window granularity in k          */
  short   WinSize PACKED;               /* Window size in k                 */
  short   WinASegment PACKED;           /* Window A segment                 */
  short   WinBSegment PACKED;           /* Window B segment                 */
  void    *WinFuncPtr PACKED;           /* Pointer to window function       */
  short   BytesPerScanLine PACKED;      /* Bytes per scanline               */
  short   XResolution PACKED;           /* Horizontal resolution            */
  short   YResolution PACKED;           /* Vertical resolution              */
  char    XCharSize PACKED;             /* Character cell width             */
  char    YCharSize PACKED;             /* Character cell height            */
  char    NumberOfPlanes PACKED;        /* Number of memory planes          */
  char    BitsPerPixel PACKED;          /* Bits per pixel                   */
  char    NumberOfBanks PACKED;         /* Number of CGA style banks        */
  char    MemoryModel PACKED;           /* Memory model type                */
  char    BankSize PACKED;              /* Size of CGA style banks          */
  char    NumberOfImagePages PACKED;    /* Number of images pages           */
  char    res1 PACKED;                  /* Reserved                         */
  char    RedMaskSize PACKED;           /* Size of direct color red mask    */
  char    RedFieldPosition PACKED;      /* Bit posn of lsb of red mask      */
  char    GreenMaskSize PACKED;         /* Size of direct color green mask  */
  char    GreenFieldPosition PACKED;    /* Bit posn of lsb of green mask    */
  char    BlueMaskSize PACKED;          /* Size of direct color blue mask   */
  char    BlueFieldPosition PACKED;     /* Bit posn of lsb of blue mask     */
  char    RsvdMaskSize PACKED;          /* Size of direct color res mask    */
  char    RsvdFieldPosition PACKED;     /* Bit posn of lsb of res mask      */
  char    DirectColorModeInfo PACKED;   /* Direct color mode attributes     */

  /* VESA 2.0 variables */
  int     PhysBasePtr;                  /* physical address for flat frame buffer */
  int     OffScreenMemOffset;           /* pointer to start of off screen memory */
  short   OffScreenMemSize;      	/* amount of off screen memory in 1k units */
  char    res2[206] PACKED;             /* Pad to 256 byte block size       */
  };

struct VBE2_PM_Info {
   unsigned short setWindow PACKED;
   unsigned short setDisplayStart PACKED;
   unsigned short setPalette PACKED;
   unsigned short IOPrivInfo PACKED;
};

#pragma pack()
#endif

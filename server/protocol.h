//bc protocol
//First word - type
//Second word num bytes sent
//third word client from num
//fourth word client to
//remaining text
//


#define BC_TRANSCRIPT 1 //fourth word will be 11 to indicate server
#define BC_IM 2
#define BC_DEEPSIX 3 //fourth word will be client voted for
#define BC_EDIT 4
#define BC_LURKING 5 //fourth word will be same as third
#define BC_STRNAME 6 // fourth word will be 11 for server
#define BC_GRDNUM  7 // fourth word will be 11
#define BC_TIMEUPDATE 8 //Third word will be 11 for server
#define BC_BYTESTOSERVER 9
#define BC_BYTESFROMSERVER 10

//11-20 reserverd for Josh
//21-30 reserved for Tyler

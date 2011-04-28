/* Copyright (C) 2010  BlackChat Group 
This file is part of BlackChat.

Ashes is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Ashes is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with BlackChat.  If not, see <http://www.gnu.org/licenses/>.
*/

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

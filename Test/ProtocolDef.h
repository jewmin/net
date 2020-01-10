#ifndef Benchmark_ProtocolDef_INCLUDED
#define Benchmark_ProtocolDef_INCLUDED

#include "Net.h"

#pragma pack(1)

typedef struct tagPackHeader {
	u8 pack_begin_flag;	// 0xBF
	u16 data_len;		// [PackHeader] [DATA] data_len == sizeof(DATA)
	u16 crc_data;		// [pack_begin_flag | pack_end_flag] | [data_len]
	u8 pack_end_flag;	// 0xEF
} PackHeader;

#define PACK_BEGIN_FLAG					0xBF
#define PACK_END_FLAG					0xEF
#define PRIVATE_MAKE_CRC_DATA(x, y, z)	(((x) << 8 | (y)) | (z))
#define MAKE_CRC_DATA(x, y, z)			PRIVATE_MAKE_CRC_DATA(x, y, z)
#define PACK_HEADER_LEN					sizeof(PackHeader)

#pragma pack()

#endif
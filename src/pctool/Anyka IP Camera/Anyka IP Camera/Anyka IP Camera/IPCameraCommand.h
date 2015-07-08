#pragma once

#pragma warning(disable:4067)

#if define(WIN32) && !define(__MINGW32__) && !define(CYGWIN__)
#		define CONFIG_WIN32
#endif

#if define(WIN32) && !define(__MINGW32__) && !define(CYGWIN__) && !define(EMULATE_INTTYPES)
#		define EMULATE_INTTYPES
#endif

#ifndef EMULATE_INTTYPES
	#include <inttypes.h>
#else
	typedef signed char			int8_t;
	typedef signed short		int16_t;
	typedef signed int			int32_t;
	typedef unsigned char		uint8_t;
	typedef unsigned short		uint16_t;
	typedef unsigned int		uint32_t;
#ifdef CONFIG_WIN32
	typedef signed __int64		int64_t;
	typedef unsigned __int64	uint64_t;
#else
	typedef signed long long	int64_t;
	typedef unsigned long long	uint64_t;
#endif
#endif

#define	DEVICE_ID_LEN				32
#define STREAM_ADDR_LEN				16
#define SYSTEM_TAG					"AK39"
#define MAKE_SYSTEM_TAG(x)			(uint32_t)((x[3] << 24) | (x[2] << 16) | (x[1] << 8) | x[0])
#define CON_SYSTEM_TAG(x, y)		memcpy(y, x, 4);

#define MOTION_DETECT_AREA_ROWS		8
#define MOTION_DETECT_AREA_LINES	8

//uint64_t aMDRowsBegin[8] = {0x01, 0x1 << 8, 0x01 << 16, 0x01 << 24, 0x01 << 32, 0x01 << 40, 
							//0x01 << 48, 0x01 << 56};

//uint64_t aMDLinesIndex[8] = {1, 2, 4, 8, 16, 32, 64, 128};

typedef enum VideoMode_en
{
	VIDEO_MODE_QVGA = 0,
	VIDEO_MODE_VGA,
	VIDEO_MODE_D1,
	VIDEO_MODE_720P,
	VIDEO_MODE_MAX
}VIDEO_MODE;

typedef enum CommandType_en
{
	COMM_TYPE_OPEN_SERVICE = 0,
	COMM_TYPE_SET_PARAMETER,
	COMM_TYPE_GET_INFO,
	COMM_TYPE_SERVER_PROCESS_RETURN,
	COMM_TYPE_HEART_BEAT,
	COMM_TYPE_CNT
}COMMAND_TYPE;

typedef enum OpenServiceCommandType_en
{
	OPEN_COMM_RECODE = 0,
	OPEN_COMM_TALK,
	OPEN_COMM_TAKE_PIC,
	OPEN_COMM_MOTION_DETECT,
	OPEN_COMM_RECODE_CLOSE,
	OPEN_COMM_MAX
}OPEN_COMM_TYPE;

typedef enum SetParamCommandType_en
{
	SET_COMM_IMAGE = 0,
	SET_COMM_ISP,
	SET_COMM_PRIVACY_AREA,
	SET_COMM_ZOOM,
	SET_COMM_VOLUME,
	SET_COMM_CAMERA_MOVEMENT,
	SET_COMM_MAX
}SET_COMM_TYPE;

typedef enum GetParamCommandType_en
{
	GET_COMM_RECODE_FILE = 0,
	GET_COMM_SERVER_INFO,
	GET_COMM_PRIVACY_AREA,
	GET_COMM_MOTION_DETECT_AREAS,
	GET_COMM_IMAGE_SET,
	GET_COMM_FILES_LIST,
	GET_COMM_ISP_INFO,
	GET_COMM_MAX
}GET_COMM_TYPE;

typedef struct SystemHeader_st
{
	uint32_t	nSystemTag;
	uint32_t	nCommandCnt;
	uint32_t	nLen;
}SYSTEM_HEADER;

typedef struct CommandHeader_st
{
	uint8_t		CommandType;
	uint8_t		subCommandType;
	uint16_t	nLen;
}COMMAND_HEADER;

typedef struct ImageSet_st
{
	uint8_t		nContrast;
	uint8_t		nSaturation;
	uint8_t		nBrightness;
	uint8_t		nReserve;
}IMAGE_SET;

typedef struct ServerInfo_t
{
	char		strDeviceID[DEVICE_ID_LEN];
	uint16_t	nRtspPort;
	uint16_t	nCommandPort;
	
	char		strMainAddr[STREAM_ADDR_LEN];
	char		strSubAddr[STREAM_ADDR_LEN];
	VIDEO_MODE	enMainStream;
	VIDEO_MODE	enSubStream;
	int			nMainFps;
	int			nSubFps;
	IMAGE_SET	stImageSet;
}SERVER_INFO;

typedef struct TimeInfo_t
{
	uint8_t		nYear;	//since 1970
	uint8_t		nMon;
	uint8_t		nDay;
	uint8_t		nHour;
	uint8_t		nMin;
	uint8_t		nSecond;
}TIME_INFO;

typedef struct PrivacyArea_st
{
	uint16_t	nNumber;
	uint16_t	nLeftTopX;
	uint16_t	nLeftTopY;
	uint16_t	nWidth;
	uint16_t	nHeight;
}PRIVACY_AREA;

typedef struct MotionDectect_st
{
	uint32_t	nSensitivity;
	uint32_t	nReserve;
	uint64_t	nMDAreas;
}MOTION_DETECT;

typedef enum CameraMovementType_en
{
	CMT_STEP_UP = 0,
	CMT_STEP_DOWN,
	CMT_STEP_LEFT,
	CMT_STEP_RIGHT,
	CMT_RUN_UP_DOWN,
	CMT_RUN_LEFT_RIGHT,
	CMT_RUN_REPOSITION,
	CMT_SET_REPOSITION,
	CMT_STEP_UP_CONTINUE,
	CMT_STEP_DOWN_CONTINUE,
	CMT_STEP_LEFT_CONTINUE,
	CMT_STEP_RIGHT_CONTINUE,
	CMT_STEP_UP_CONTINUE_STOP,
	CMT_STEP_DOWN_CONTINUE_STOP,
	CMT_STEP_LEFT_CONTINUE_STOP,
	CMT_STEP_RIGHT_CONTINUE_STOP,
	CMT_RUN_MAX
}CAMERA_MOVEMENT_TYPE;

typedef enum AudioEncodeType_en
{
	AUDIO_EN_AAC = 0,
	AUDIO_EN_MP2,
	AUDIO_EN_MP3,
	AUDIO_EN_AMR,
	AUDIO_EN_ADPCM,
	AUDIO_EN_PCM,
	AUDIO_EN_MAX
}AUDIO_ENCODE_TYPE;

typedef enum AudioSampleFromat_en
{
    AUDIO_SAMPLE_FMT_U8,          ///< unsigned 8 bits
    AUDIO_SAMPLE_FMT_S16,         ///< signed 16 bits
    AUDIO_SAMPLE_FMT_S32,         ///< signed 32 bits
    AUDIO_SAMPLE_FMT_FLT,         ///< float
    AUDIO_SAMPLE_FMT_DBL,         ///< double

    AUDIO_SAMPLE_FMT_U8P,         ///< unsigned 8 bits, planar
    AUDIO_SAMPLE_FMT_S16P,        ///< signed 16 bits, planar
    AUDIO_SAMPLE_FMT_S32P,        ///< signed 32 bits, planar
    AUDIO_SAMPLE_FMT_FLTP,        ///< float, planar
    AUDIO_SAMPLE_FMT_DBLP,        ///< double, planar

    AUDIO_SAMPLE_FMT_NB           ///< Number of sample formats. DO NOT USE if linking dynamically
}AUDIO_SAMPLE_FMT;

typedef enum AudioSampleRate_en
{
	AUDIO_SAMPLERATE_8K = 0,
	AUDIO_SAMPLERATE_11K,
	AUDIO_SAMPLERATE_16K,
	AUDIO_SAMPLERATE_32K,
	AUDIO_SAMPLERATE_44K,
	AUDIO_SAMPLERATE_48K,
	AUDIO_SAMPLERATE_96K,
	AUDIO_SAMPLERATE_NB			 ///< Number of sample rate type. DO NOT USE if linking dynamically
}SAMPLERATE;

typedef struct ClientAudioParam_st
{
	uint8_t nEncodeType;
	uint8_t nChannels;
	uint8_t nSampleRateType;
	uint8_t nSampleFmt;
}AUDIOPARAMETER;

enum ErrorCode_en
{
	ERROR_UNKNOWN = 0,		// 未知原因错误
	ERROR_NO_STORGE_DEVICE,	// 无存储设备
	ERROR_NO_CAPTURE_DEVICE,// 无采集设备
	ERROR_NO_OUTPUT_DEVICE,	// 无输出设备
	ERROR_SD_MOUNT,			// SD卡挂载失败
	ERROR_SD_READONLY,		// SD卡只读
	ERROR_FILE_OPEN,		// 文件打开错误
	ERROR_FILE_WRITE,		// 文件写错误
	ERROR_FILE_READ,		// 文件读错误
	ERROR_MAX
	//待补充....
};

typedef enum ReturnType_en
{
	RET_SUCCESS = 0,
	RET_ERROR_CODE,
	RET_WARN_CODE,
	RET_TYPE_MAX
}RETTYPE;

typedef struct ServerProcessReturn_st
{
	int nCommandType;
	int nSubCommandType;
	RETTYPE retType;
	int nCode;
}RETINFO;

typedef uint8_t			VOLUME;
typedef uint8_t			ZOOM;
typedef uint8_t			CAMERAMOVEMENT;

#define ZOOM_IN			1
#define ZOOM_OUT		0

#define VOLUME_MINUS	1
#define VOLUME_PLUS		2
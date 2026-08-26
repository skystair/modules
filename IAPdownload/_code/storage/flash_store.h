/**
 * @file    flash_store.h
 * @brief   内部Flash文件存储系统 (STM32F103ZET6)
 * @details 固定槽位存储方案: 索引区(16KB) + 数据区(5×84KB)
 *          最多存储5个文件，单文件最大50KB
 *          所有地址和大小均为4字节对齐
 */

#ifndef __FLASH_STORE_H
#define __FLASH_STORE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              Flash布局配置                                  */
/* ========================================================================== */

#define FLASH_PAGE_SIZE         2048        /* ZET6页大小2KB */

/* 程序区: 0x08000000 ~ 0x08011FFF (72KB) */
#define PROGRAM_SIZE            (72 * 1024)

/* 索引区: 0x08012000 ~ 0x08015FFF (16KB, 8页) */
#define INDEX_START_ADDR        0x08012000
#define INDEX_SIZE              (16 * 1024)

/* 数据区: 0x08016000 ~ 0x0807EFFF (420KB, 5×84KB) */
#define DATA_START_ADDR         0x08016000
#define SLOT_SIZE               (84 * 1024)     /* 单槽84KB */
#define SLOT_COUNT              5               /* 5个槽位 */

/* 文件限制 */
#define MAX_FILE_NAME_LEN       24              /* 文件名最大长度(含'\0') */
#define MAX_FILE_SIZE           (50 * 1024)     /* 单文件最大50KB */

/* ========================================================================== */
/*                              数据结构定义                                    */
/* ========================================================================== */

/* 文件状态 */
#define FILE_STATE_EMPTY    0xFFFFFFFF          /* 空槽位(Flash擦除后) */
#define FILE_STATE_VALID    0x56414C44          /* "VALD" - 有效文件 */
#define FILE_STATE_DELETED  0x00000000          /* 已删除 */

/* 索引区魔数 */
#define INDEX_MAGIC         0x49445848          /* "IDXH" */
#define INDEX_VERSION       0x00000001

/* 文件元数据 (32字节, 4字节对齐) */
typedef struct {
    uint32_t state;                             /* 文件状态 */
    uint32_t start_addr;                        /* 数据起始地址 */
    uint32_t size;                              /* 文件大小(字节) */
    uint32_t crc32;                             /* 文件CRC32校验值 */
    char     name[MAX_FILE_NAME_LEN];           /* 文件名 */
} file_entry_t;                                 /* 32字节 */

/* 索引头 (16字节, 4字节对齐) */
typedef struct {
    uint32_t magic;                             /* 魔数 0x49445848 */
    uint32_t version;                           /* 索引版本 */
    uint32_t file_count;                        /* 有效文件数量 */
    uint32_t reserved;                          /* 保留 */
} index_header_t;                               /* 16字节 */

/* 完整索引结构 (16 + 32×5 = 176字节) */
typedef struct {
    index_header_t header;
    file_entry_t   entries[SLOT_COUNT];
} file_index_t;

/* 文件信息 (返回给调用者, 不存储在Flash) */
typedef struct {
    uint8_t  index;                             /* 槽位索引 0~4 */
    uint32_t size;                              /* 文件大小 */
    uint32_t crc32;                             /* CRC32校验值 */
    char     name[MAX_FILE_NAME_LEN];           /* 文件名 */
} file_info_t;

/* 错误码 */
typedef enum {
    FLASH_STORE_OK          =  0,               /* 成功 */
    FLASH_STORE_ERR_PARAM   = -1,               /* 参数错误 */
    FLASH_STORE_ERR_EMPTY   = -2,               /* 槽位为空 */
    FLASH_STORE_ERR_FULL    = -3,               /* 槽位已满 */
    FLASH_STORE_ERR_SIZE    = -4,               /* 文件过大 */
    FLASH_STORE_ERR_FLASH   = -5,               /* Flash操作失败 */
    FLASH_STORE_ERR_CRC     = -6,               /* CRC校验失败 */
    FLASH_STORE_ERR_INDEX   = -7,               /* 索引损坏 */
} flash_store_err_t;

/* ========================================================================== */
/*                              API接口声明                                    */
/* ========================================================================== */

/**
 * @brief       初始化存储系统
 * @note        从Flash读取索引到RAM，如果索引损坏则格式化
 * @return      0=成功, 负数=错误码
 */
int flash_store_init(void);

/**
 * @brief       格式化存储系统
 * @note        擦除索引区和所有数据区，清空所有文件
 * @return      0=成功, 负数=错误码
 */
int flash_store_format(void);

/**
 * @brief       获取当前文件数量
 * @return      0~5
 */
int flash_store_get_count(void);

/**
 * @brief       获取指定槽位的文件信息
 * @param       idx: 槽位索引 0~4
 * @param       info: 文件信息输出指针
 * @return      0=成功, 负数=错误码
 */
int flash_store_get_info(uint8_t idx, file_info_t *info);

/**
 * @brief       写入文件到指定槽位
 * @param       slot: 槽位索引 0~4
 * @param       name: 文件名 (最长23字符)
 * @param       data: 文件数据指针
 * @param       size: 文件大小 (最大50KB)
 * @param       crc32: 文件CRC32校验值
 * @return      0=成功, 负数=错误码
 */
int flash_store_write(uint8_t slot, const char *name,
                      const uint8_t *data, uint32_t size, uint32_t crc32);

/**
 * @brief       从指定槽位读取文件数据
 * @param       idx: 槽位索引 0~4
 * @param       buf: 输出缓冲区
 * @param       max_size: 缓冲区大小
 * @return      文件大小(字节), 负数=错误码
 */
int flash_store_read(uint8_t idx, uint8_t *buf, uint32_t max_size);

/**
 * @brief       删除指定槽位的文件
 * @param       idx: 槽位索引 0~4
 * @return      0=成功, 负数=错误码
 */
int flash_store_delete(uint8_t idx);

/**
 * @brief       计算CRC32校验值
 * @param       data: 数据指针
 * @param       len: 数据长度
 * @return      CRC32值
 */
uint32_t flash_store_crc32(const uint8_t *data, uint32_t len);

/**
 * @brief       获取槽位起始地址
 * @param       slot: 槽位索引 0~4
 * @return      Flash地址
 */
uint32_t flash_store_get_slot_addr(uint8_t slot);

/**
 * @brief       流式写入开始 - 擦除槽位，准备接收数据
 * @param       slot: 槽位索引 0~4
 * @param       name: 文件名
 * @return      0=成功, 负数=错误码
 */
int flash_store_write_begin(uint8_t slot, const char *name);

/**
 * @brief       流式写入数据 - 将一块数据写入槽位指定偏移
 * @param       offset: 槽位内偏移(字节)
 * @param       data: 数据指针
 * @param       len: 数据长度
 * @return      0=成功, 负数=错误码
 */
int flash_store_write_data(uint32_t offset, const uint8_t *data, uint32_t len);

/**
 * @brief       流式写入结束 - 更新索引
 * @param       size: 文件总大小
 * @param       crc32: 文件CRC32校验值
 * @return      0=成功, 负数=错误码
 */
int flash_store_write_end(uint32_t size, uint32_t crc32);

/**
 * @brief       进入流式写入模式(批量写入时跳过每包Unlock/Lock)
 * @note        调用后Flash保持解锁状态,直到write_stream_end
 */
void flash_store_write_stream_begin(void);

/**
 * @brief       退出流式写入模式(重新锁定Flash)
 */
void flash_store_write_stream_end(void);

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_STORE_H */

/**
 * @file    flash_store.c
 * @brief   内部Flash文件存储系统实现
 * @note    STM32F103ZET6: 512KB Flash, 页大小2KB
 *          程序占用前72KB，索引区16KB，数据区420KB(5×84KB)
 */

#include "config.h"

/* ========================================================================== */
/*                              内部变量                                       */
/* ========================================================================== */

static file_index_t g_index;                /* RAM中的索引缓存 */
static uint8_t      g_index_dirty = 0;      /* 索引是否需要写回 */
static uint8_t      g_initialized = 0;      /* 初始化标志 */
static uint8_t      g_flash_streaming = 0;  /* 流式写入模式(跳过每包Unlock/Lock) */

/* ========================================================================== */
/*                              CRC32计算                                      */
/* ========================================================================== */

/* ========================================================================== */
/*                              Flash底层操作                                   */
/* ========================================================================== */

/**
 * @brief       擦除Flash页
 * @param       addr: 页内任意地址
 * @return      0=成功, -1=失败
 */
static int flash_erase_page(uint32_t addr)
{
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    if (FLASH_ErasePage(addr) != FLASH_COMPLETE) {
        FLASH_Lock();
        return -1;
    }
    FLASH_Lock();
    return 0;
}

/**
 * @brief       向Flash写入数据块(自动处理半字对齐)
 * @param       addr: 目标地址(必须2字节对齐)
 * @param       data: 数据指针
 * @param       len: 数据长度(字节)
 * @return      0=成功, -1=失败
 */
static int flash_write_buffer(uint32_t addr, const uint8_t *data, uint32_t len)
{
    uint32_t i;
    uint16_t halfword;

    /* 流式模式: 由调用者负责Unlock/Lock, 此处跳过 */
    if (!g_flash_streaming) {
        FLASH_Unlock();
    }

    /* 写入完整的半字 */
    for (i = 0; i + 1 < len; i += 2) {
        halfword = data[i] | (data[i + 1] << 8);
        if (FLASH_ProgramHalfWord(addr + i, halfword) != FLASH_COMPLETE) {
            if (!g_flash_streaming) {
                FLASH_Lock();
            }
            return -1;
        }
    }

    /* 处理奇数字节 */
    if (i < len) {
        halfword = data[i] | 0xFF00;  /* 高字节保持0xFF */
        if (FLASH_ProgramHalfWord(addr + i, halfword) != FLASH_COMPLETE) {
            if (!g_flash_streaming) {
                FLASH_Lock();
            }
            return -1;
        }
    }

    if (!g_flash_streaming) {
        FLASH_Lock();
    }
    return 0;
}

/**
 * @brief       从Flash读取数据
 * @param       addr: 源地址
 * @param       buf: 输出缓冲区
 * @param       len: 读取长度
 */
static void flash_read_buffer(uint32_t addr, uint8_t *buf, uint32_t len)
{
    const uint8_t *src = (const uint8_t *)addr;
    memcpy(buf, src, len);
}

/* ========================================================================== */
/*                              索引管理                                       */
/* ========================================================================== */

/**
 * @brief       从Flash加载索引到RAM
 */
static void index_load(void)
{
    flash_read_buffer(INDEX_START_ADDR, (uint8_t *)&g_index, sizeof(file_index_t));
}

/**
 * @brief       将RAM索引写回Flash
 * @note        需要先擦除索引区(8页)
 * @return      0=成功, -1=失败
 */
static int index_save(void)
{
    uint32_t i;

    /* 擦除索引区 */
    for (i = 0; i < INDEX_SIZE / FLASH_PAGE_SIZE; i++) {
        if (flash_erase_page(INDEX_START_ADDR + i * FLASH_PAGE_SIZE) != 0) {
            return -1;
        }
    }

    /* 写入索引 */
    if (flash_write_buffer(INDEX_START_ADDR, (const uint8_t *)&g_index,
                           sizeof(file_index_t)) != 0) {
        return -1;
    }

    g_index_dirty = 0;
    return 0;
}

/**
 * @brief       重新计算索引中的文件数量
 */
static void index_recount(void)
{
    uint8_t i;
    uint32_t count = 0;

    for (i = 0; i < SLOT_COUNT; i++) {
        if (g_index.entries[i].state == FILE_STATE_VALID) {
            count++;
        }
    }
    g_index.header.file_count = count;
}

/**
 * @brief       验证索引有效性
 * @return      0=有效, -1=无效
 */
static int index_validate(void)
{
    if (g_index.header.magic != INDEX_MAGIC) {
        return -1;
    }
    if (g_index.header.version != INDEX_VERSION) {
        return -1;
    }
    if (g_index.header.file_count > SLOT_COUNT) {
        return -1;
    }
    return 0;
}

/**
 * @brief       初始化默认索引
 */
static void index_init_default(void)
{
    uint8_t i;

    memset(&g_index, 0xFF, sizeof(file_index_t));
    g_index.header.magic = INDEX_MAGIC;
    g_index.header.version = INDEX_VERSION;
    g_index.header.file_count = 0;
    g_index.header.reserved = 0xFFFFFFFF;

    for (i = 0; i < SLOT_COUNT; i++) {
        g_index.entries[i].state = FILE_STATE_EMPTY;
        g_index.entries[i].start_addr = flash_store_get_slot_addr(i);
    }

    g_index_dirty = 1;
}

/* ========================================================================== */
/*                              CRC32计算                                      */
/* ========================================================================== */

uint32_t flash_store_crc32(const uint8_t *data, uint32_t len)
{
    uint32_t crc = 0xFFFFFFFF;
    uint32_t i, j;

    for (i = 0; i < len; i++) {
        crc ^= data[i];
        for (j = 0; j < 8; j++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320 : 0);
        }
    }

    return crc ^ 0xFFFFFFFF;
}

/* ========================================================================== */
/*                              公共API实现                                    */
/* ========================================================================== */

int flash_store_init(void)
{
    /* 加载索引 */
    index_load();

    /* 验证索引 */
    if (index_validate() != 0) {
        /* 索引无效，初始化默认索引 */
        index_init_default();
        if (index_save() != 0) {
            return FLASH_STORE_ERR_FLASH;
        }
    }

    g_initialized = 1;
    return FLASH_STORE_OK;
}

int flash_store_format(void)
{
    uint32_t i;
    uint32_t addr;

    /* 擦除索引区 */
    for (i = 0; i < INDEX_SIZE / FLASH_PAGE_SIZE; i++) {
        if (flash_erase_page(INDEX_START_ADDR + i * FLASH_PAGE_SIZE) != 0) {
            return FLASH_STORE_ERR_FLASH;
        }
    }

    /* 擦除数据区 */
    for (i = 0; i < SLOT_COUNT; i++) {
        addr = flash_store_get_slot_addr(i);
        if (flash_erase_page(addr) != 0) {
            return FLASH_STORE_ERR_FLASH;
        }
        /* 每个槽位擦除第一页即可(简化处理) */
    }

    /* 初始化默认索引 */
    index_init_default();
    if (index_save() != 0) {
        return FLASH_STORE_ERR_FLASH;
    }

    return FLASH_STORE_OK;
}

int flash_store_get_count(void)
{
    if (!g_initialized) return 0;
    return (int)g_index.header.file_count;
}

int flash_store_get_info(uint8_t idx, file_info_t *info)
{
    if (idx >= SLOT_COUNT || info == NULL) {
        return FLASH_STORE_ERR_PARAM;
    }

    if (g_index.entries[idx].state != FILE_STATE_VALID) {
        return FLASH_STORE_ERR_EMPTY;
    }

    info->index = idx;
    info->size = g_index.entries[idx].size;
    info->crc32 = g_index.entries[idx].crc32;
    strncpy(info->name, g_index.entries[idx].name, MAX_FILE_NAME_LEN - 1);
    info->name[MAX_FILE_NAME_LEN - 1] = '\0';

    return FLASH_STORE_OK;
}

int flash_store_write(uint8_t slot, const char *name,
                      const uint8_t *data, uint32_t size, uint32_t crc32)
{
    uint32_t addr;
    uint32_t i;

    /* 参数检查 */
    if (slot >= SLOT_COUNT || name == NULL || data == NULL) {
        return FLASH_STORE_ERR_PARAM;
    }
    if (size == 0 || size > MAX_FILE_SIZE) {
        return FLASH_STORE_ERR_SIZE;
    }

    /* 计算槽位地址 */
    addr = flash_store_get_slot_addr(slot);

    /* 擦除槽位(擦除前几页) */
    for (i = 0; i < (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE; i++) {
        if (flash_erase_page(addr + i * FLASH_PAGE_SIZE) != 0) {
            return FLASH_STORE_ERR_FLASH;
        }
    }

    /* 写入文件数据 */
    if (flash_write_buffer(addr, data, size) != 0) {
        return FLASH_STORE_ERR_FLASH;
    }

    /* 更新索引 */
    g_index.entries[slot].state = FILE_STATE_VALID;
    g_index.entries[slot].start_addr = addr;
    g_index.entries[slot].size = size;
    g_index.entries[slot].crc32 = crc32;
    memset(g_index.entries[slot].name, 0, MAX_FILE_NAME_LEN);
    strncpy(g_index.entries[slot].name, name, MAX_FILE_NAME_LEN - 1);

    index_recount();
    g_index_dirty = 1;

    /* 保存索引 */
    if (index_save() != 0) {
        return FLASH_STORE_ERR_FLASH;
    }

    return FLASH_STORE_OK;
}

int flash_store_read(uint8_t idx, uint8_t *buf, uint32_t max_size)
{
    uint32_t addr;
    uint32_t read_size;

    if (idx >= SLOT_COUNT || buf == NULL) {
        return FLASH_STORE_ERR_PARAM;
    }

    if (g_index.entries[idx].state != FILE_STATE_VALID) {
        return FLASH_STORE_ERR_EMPTY;
    }

    addr = g_index.entries[idx].start_addr;
    read_size = g_index.entries[idx].size;

    if (read_size > max_size) {
        read_size = max_size;
    }

    flash_read_buffer(addr, buf, read_size);

    return (int)read_size;
}

int flash_store_delete(uint8_t idx)
{
    if (idx >= SLOT_COUNT) {
        return FLASH_STORE_ERR_PARAM;
    }

    if (g_index.entries[idx].state != FILE_STATE_VALID) {
        return FLASH_STORE_ERR_EMPTY;
    }

    /* 标记为已删除 */
    g_index.entries[idx].state = FILE_STATE_DELETED;
    g_index.entries[idx].size = 0;
    g_index.entries[idx].crc32 = 0;
    memset(g_index.entries[idx].name, 0, MAX_FILE_NAME_LEN);

    index_recount();
    g_index_dirty = 1;

    /* 保存索引 */
    if (index_save() != 0) {
        return FLASH_STORE_ERR_FLASH;
    }

    return FLASH_STORE_OK;
}

uint32_t flash_store_get_slot_addr(uint8_t slot)
{
    if (slot >= SLOT_COUNT) {
        return 0;
    }
    return DATA_START_ADDR + slot * SLOT_SIZE;
}

/* ========================================================================== */
/*                              流式写入实现                                    */
/* ========================================================================== */

static uint8_t  g_stream_slot = 0;          /* 当前流式写入的槽位 */
static uint32_t g_stream_addr = 0;          /* 当前槽位起始地址 */

int flash_store_write_begin(uint8_t slot, const char *name)
{
    uint32_t addr;
    uint32_t i;

    if (slot >= SLOT_COUNT || name == NULL) {
        return FLASH_STORE_ERR_PARAM;
    }

    addr = flash_store_get_slot_addr(slot);

    /* 擦除槽位所有页 */
    for (i = 0; i < SLOT_SIZE / FLASH_PAGE_SIZE; i++) {
        if (flash_erase_page(addr + i * FLASH_PAGE_SIZE) != 0) {
            return FLASH_STORE_ERR_FLASH;
        }
    }

    g_stream_slot = slot;
    g_stream_addr = addr;

    /* 预写入文件名到索引(状态暂不标记为VALID) */
    memset(g_index.entries[slot].name, 0, MAX_FILE_NAME_LEN);
    strncpy(g_index.entries[slot].name, name, MAX_FILE_NAME_LEN - 1);
    g_index.entries[slot].start_addr = addr;

    return FLASH_STORE_OK;
}

int flash_store_write_data(uint32_t offset, const uint8_t *data, uint32_t len)
{
    if (data == NULL || len == 0) {
        return FLASH_STORE_ERR_PARAM;
    }

    if (offset + len > SLOT_SIZE) {
        return FLASH_STORE_ERR_SIZE;
    }

    return flash_write_buffer(g_stream_addr + offset, data, len);
}

int flash_store_write_end(uint32_t size, uint32_t crc32)
{
    if (size == 0 || size > MAX_FILE_SIZE) {
        return FLASH_STORE_ERR_SIZE;
    }

    /* 更新索引 */
    g_index.entries[g_stream_slot].state = FILE_STATE_VALID;
    g_index.entries[g_stream_slot].size = size;
    g_index.entries[g_stream_slot].crc32 = crc32;

    index_recount();
    g_index_dirty = 1;

    return index_save();
}

/**
 * @brief       进入流式写入模式(批量写入时跳过每包Unlock/Lock)
 * @note        调用后Flash保持解锁状态,直到write_stream_end
 */
void flash_store_write_stream_begin(void)
{
    FLASH_Unlock();
    g_flash_streaming = 1;
}

/**
 * @brief       退出流式写入模式(重新锁定Flash)
 */
void flash_store_write_stream_end(void)
{
    g_flash_streaming = 0;
    FLASH_Lock();
}

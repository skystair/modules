/**
 * @file    transfer.h
 * @brief   USB传输控制模块
 * @details 管理PC到MCU的文件传输流程
 *          协议: USB CDC ASCII命令 + XMODEM数据传输
 */

#ifndef __TRANSFER_H
#define __TRANSFER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                              协议定义                                       */
/* ========================================================================== */

/* 命令ID */
#define CMD_QUERY           'Q'             /* 查询文件列表 */
#define CMD_SELECT          'S'             /* 选择槽位 */
#define CMD_ERASE           'E'             /* 擦除槽位 */
#define CMD_TRANSFER        'T'             /* 开始传输 */
#define CMD_VERIFY          'V'             /* 验证文件 */
#define CMD_DELETE          'D'             /* 删除文件 */
#define CMD_INFO            'I'             /* 获取MCU信息 */

/* 响应码 */
#define RESP_OK             'K'             /* 成功 */
#define RESP_ERR            'E'             /* 错误 */
#define RESP_READY          'R'             /* 就绪 */
#define RESP_BUSY           'B'             /* 忙碌 */

/* ========================================================================== */
/*                              传输状态                                       */
/* ========================================================================== */

typedef enum {
    TRANSFER_IDLE,                          /* 空闲 */
    TRANSFER_WAIT_CMD,                      /* 等待命令 */
    TRANSFER_SELECTED,                      /* 已选槽位 */
    TRANSFER_RECEIVING,                     /* 接收数据中 */
    TRANSFER_DONE,                          /* 传输完成 */
    TRANSFER_ERROR,                         /* 错误 */
    TRANSFER_DELETED,                       /* 文件已删除(UI需刷新) */
} transfer_state_t;

/* ========================================================================== */
/*                              回调定义                                        */
/* ========================================================================== */

/**
 * @brief       状态变化回调
 * @param       state: 新状态
 * @param       progress: 进度(0-100), -1=无效
 */
typedef void (*transfer_state_fn)(transfer_state_t state, int progress);

/* ========================================================================== */
/*                              API接口                                        */
/* ========================================================================== */

/**
 * @brief       初始化传输模块
 */
void transfer_init(void);

/**
 * @brief       注册状态回调
 * @param       cb: 状态回调函数
 */
void transfer_set_callback(transfer_state_fn cb);

/**
 * @brief       处理接收到的USB数据
 * @param       data: 数据指针
 * @param       len: 数据长度
 * @note        在USB CDC接收回调中调用
 */
void transfer_process(const uint8_t *data, uint32_t len);

/* (USB数据读取已移至 modefunc_Lastfunc，transfer_process 由外部直接调用) */

/**
 * @brief       获取当前状态
 * @return      当前传输状态
 */
transfer_state_t transfer_get_state(void);

/**
 * @brief       获取当前选中的槽位
 * @return      0-4, -1=未选择
 */
int transfer_get_slot(void);

/**
 * @brief       发送响应到PC
 * @param       resp: 响应码
 * @param       msg: 消息(可选)
 */
/**
 * @brief       发送响应到PC
 * @param       resp: 响应码
 * @param       msg: 消息(可NULL)
 */
void transfer_send_resp(uint8_t resp, const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* __TRANSFER_H */

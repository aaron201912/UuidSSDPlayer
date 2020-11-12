#ifndef _COAPI_GW_H_
#define _COAPI_GW_H_

#ifdef __cplusplus
extern "C" {
#endif

/*
 *  gw模块针对子设备接入的情况,子设备可以通过有线或者无线与网关进行连接
 *  常见的网关设备: 独立网关设备，智能AI音箱, 控制面板...
 *  常见的连接方式: BT-SPP/BT-BLE/2.4G/Zigbee/UART/SPI/...
 *
 *  网关使用逻辑与流程:
 *
 *　定义设备产品规格
 *        |
 *  申请Appkey并获得酷AI云登录帐号
 *        |
 *  定义设备的数据点
 *        |
 *  登录酷AI云配置数据点: 技能中心->设备能力; 参考: 文档中心->PLK/技能->设备能力数据点使用文档
 *        |
 *  配置完数据点后获得每个数据点ID, 把数据点ID在设备端定义使用; 参考: 文档中心->PLK/技能->设备能力数据点使用文档
 *        |
 *  设备功能开发
 *        |
 *  设备端、客户端、AI交互联调验证
 *        |
 *     量产出货
 */


/*
 * 一. 网关&子设备 数据传输协议
 *
 *  协议头+命令数据： gw_data_head_t + gw_subdev_xxx_t(根据：gw_cmd_id)
 *  协议头：gw_data_head_t, 长度：sizeof(gw_data_head_t)
 *  命令数据： 有四种情况：
 *             1. 请求注册(GW_DP_REQ_REG): gw_subdev_reg_t
 *             2. 请求数据点能力(GW_DP_REQ_ABI): gw_subdev_abi_t
 *             3. 数据传输(GW_DP_REQ_DATA): gw_subdev_data_t
 *             4. 反馈数据(GW_DP_RESP_XXX),是否需要反馈，取决于接收到对方的gw_data_head_t的qos字段信息
 *
 * 二. 网关应用开发流程
 *
 *    1. 网关初始化： coapi_gw_init
 *            |
 *    2. 子设备上线: coapi_gw_data_unpack(发现接收到来自子设备的注册信息)->coapi_gw_reg_unpack(解包注册数据)->coapi_subdev_online(子设备上线)
 *            |
 *    3. 子设备上报数据点能力(告知网关子设备支持哪些数据点)：coapi_gw_data_unpack(发现接收到来自子设备的数据点能力)->coapi_gw_abi_unpack(解包子设备数据点能力数据)->coapi_devctrl_insert(注册数据点回调用于处理来自云端的下发)
 *            |
 *    4. 接收到子设备上报的数据点)：coapi_gw_data_unpack(发现接收到来自子设备的数据点)->coapi_gw_dp_unpack(解包子设备的数据点数据)->coapi_devsta_xxx(根据数据点dptype调用对应的接口上报)
 *            |
 *    5. 云端下发数据点给子设备：第3步注册的回调函数会回调收到数据点->coapi_gw_data_pack(打包数据)->发送打包好的数据给子设备
 *            |
 *    6. 子设备断开/下线：coapi_subdev_offline
 *
 * 三. 子设备应用开发流程
 *
 *    1. 定义数据点能力(根据酷AI云->技能中心->设备能力配置),用于第3步，发送数据点能力
 *      static const gw_dp_t dps[] = {
 *          {201000001,3,0},
 *          {201000002,3,0},
 *          {201000003,2,0},
 *          {201000004,4,0},
 *          {201000005,3,1},
 *          {201000006,1,1},
 *      }
 *             |
 *      2. 连接网关注册上线(需要反馈qos = 1)： coapi_gw_reg_pack->发送打包好的数据
 *             |
 *      3. 发送子设备数据点能力(必须要在注册成功之后): coapi_gw_abi_pack(依赖第一步的dps)->发送打包好的数据点能力数据
 *             |
 *      4. 上报数据给云端: coapi_gw_data_pack->发送打包好的数据点数据
 *             |
 *      5. 处理云端下发的数据点：coapi_gw_data_unpack(发现收到数据点数据)->coapi_gw_dp_unpack(解包数据点)
 */


/*
 * 描述一个设备的能力(数据点)
 * @dpid: 数据点ID
 * @dptype: 数据点类型,参考@coapi_common.h
 * @cs: 上下行；0: 上行; 1: 下行
 */
typedef struct gw_dp {
    int  dpid;
    int  dptype;
    int  cs;
} gw_dp_t;


/*
 * 描述一个设备（子设备）
 * @appkey: 设备appkey
 * @secretkey: 设备secretkey
 * @devid 设备ID
 * @status: 状态，0: onffline; 1: online
 * @dps 数据点列表
 * @link_fd 连接描述符/句柄,包括不限于：BLE/Zigbee/2.4G/UART/SPI）
 */
typedef struct gw_dev {
    char *appkey;
    char *secretkey;
    char *devid;
    int  status;
    void *dps;
    char *link_fd;
} gw_dev_t;

/**
 * @brief 网关与子设备的命令ID,一共三类: 子设备注册/子设备上报能力/数据传输(基于数据点)
 * req_reg:注册: 子设备->网关
 * resp_reg:注册反馈: 网关->子设备
 * req_abi:告知网关子设备支持哪些数据点: 子设备->网关
 * resp_abi:告知网关数据点反馈: 网关->子设备
 * req_data:数据传输 子设备<->网关
 * resp_data:数据传输反馈: 网关<->子设备
 */
typedef enum {
    GW_DP_REQ_REG = 101,
    GW_DP_RESP_REG = 10001,
    GW_DP_REQ_ABI = 102,
    GW_DP_RESP_ABI = 10002,
    GW_DP_REQ_DATA = 103,
    GW_DP_RESP_DATA = 10003,
} gw_cmd_id;


/**
 * @brief 数据头
 * @packlen 数据总长度
 * @headlen 数据头长度
 * @cmdid 命令ID，参考gw_cmd_id
 * @qos  0: 无需反馈；1: 需要反馈
 */
typedef struct gw_data_head {
    unsigned int packlen;
    unsigned int headlen;
    unsigned int cmdid;
    unsigned int qos;
} gw_data_head_t;


/**
 * @brief 注册请求数据: 子设备->网关
 * @appkey_len: appkey长度n
 * @appkey: 子设备appkey
 * @secretkey_len: sccretkey长度
 * @secretkey: 子设备sccretkey
 * @devid_len: 设备ID长度
 * @devid: 设备ID
 * @version: 子设备版本号： 格式xx.xx
 */
typedef struct gw_subdev_reg {
    unsigned int appkey_len;
    char *appkey;
    unsigned int secretkey_len;
    char *secretkey;
    unsigned int devid_len;
    char *devid;
    char version[8];
} gw_subdev_reg_t;


/**
 * @brief 子设备能力（支持哪些数据点）: 子设备->网关
 * @appkey_len: appkey长度n
 * @appkey: 子设备appkey
 * @devid_len: 设备ID长度
 * @devid: 设备ID
 * @dpcnt: 数据点个数
 * @dpt: 子设备支持的数据点,参考@gw_dp_t
 */
typedef struct gw_subdev_abi {
    unsigned int appkey_len;
    char *appkey;
    unsigned int devid_len;
    char *devid;
    unsigned int dpcnt;
    gw_dp_t **dp;
} gw_subdev_abi_t;

/**
 * @brief 数据传输: 子设备<->网关
 * @appkey_len: appkey长度n
 * @appkey: 子设备appkey
 * @devid_len: 设备ID长度
 * @devid: 设备ID
 * @dpid: 数据点ID
 * @dptype: 数据点数据类型
 * @len: 数据点数据长度
 * @data: 数据点数据
 */
typedef struct gw_subdev_data {
    unsigned int appkey_len;
    char *appkey;
    unsigned int devid_len;
    char *devid;
    unsigned int dpid;
    unsigned int dptype;
    unsigned int len;
    char *data;
} gw_subdev_data_t;

/*
 * @brief  解包接收到的注册数据
 * @pbuf 接收到的数据
 * @len 接收到的数据的长度
 * @reg 注册数据
 */
void coapi_gw_reg_unpack(void *pbuf, int len, gw_subdev_reg_t *reg);

/*
 * @brief  解包接收到的数据点能力数据
 * @pbuf 接收到的数据
 * @len 接收到的数据的长度
 * @reg 支持的数据点能力数据(表示子设备支持哪些数据点)
 */
void coapi_gw_abi_unpack(void *pbuf, int len, gw_subdev_abi_t *abi);

/*
 * @brief  解包接收到的数据点数据
 * @pbuf 接收到的数据
 * @len 接收到的数据的长度
 * @d 数据点数据
 */
void coapi_gw_dp_unpack(void *pbuf, int len, gw_subdev_data_t *d);

/*
 * @brief  解包看接收到什么数据
 * @pbuf 接收到的数据
 * @len 接收到的数据的长度
 * @cmdid 解包出来接收到的命令ID
 * @qos 解包出来接收到的数据是否需要反馈
 */
void *coapi_gw_data_unpack(const void *pbuf, unsigned int len, int *cmdid, int *qos);


/*
 * @brief  打包反馈数据
 * @pbuf  打包输出数据
 * @len 打包输出数据长度
 * @cmdid  cmdid,参考@gw_cmd_id
 * @data 要反馈的数据信息
 */
void coapi_gw_resp_pack(const void *pbuf, unsigned int *len, int cmdid, char *data);


/*
 * @brief  打包注册数据 (主要子设备开发使用)
 * @pbuf  打包输出数据
 * @len 打包输出数据长度
 * @qos  是否需要对方接收到数据后反馈
 * @appkey 设备产品key
 * @secretkey  设备产品secretkey
 * @devid: 设备的唯一ID
 * @version: 子设备的版本号
 */
void coapi_gw_reg_pack(const void *pbuf, unsigned int *len, int qos,
        char *appkey, char *secretkey, char *devid, char *version);

/*
 * @brief  打包子设备数据点能力数据 (主要子设备开发使用)
 * @pbuf  打包输出数据
 * @len 打包输出数据长度
 * @qos  是否需要对方接收到数据后反馈
 * @appkey 设备产品key
 * @devid: 设备的唯一ID
 * @dpcnt: 数据点个数
 * @abi: 子设备数据点数组（支持的数据点，包含三个信息，参考@gw_dp_t）
 */
 void coapi_gw_abi_pack(const void *pbuf, unsigned int *len, int qos,
        char *appkey, char *devid, int dpcnt, gw_dp_t *abi);



/*
 * @brief  打包数据点
 * @pbuf  打包输出数据
 * @plen 打包输出数据长度
 * @qos  是否需要对方接收到数据后反馈
 * @appkey 设备产品key
 * @devid: 设备的唯一ID
 * @dpid 数据点ID
 * @dptype 数据点的数据类型
 * @data: 数据点数据
 * @len: 数据点数据长度
 */
void coapi_gw_data_pack(const void *pbuf, unsigned int *plen, int qos,
        char *appkey, char *devid, int dpid, int dptype, int len, char *data);

/* 网关初始化 */
int coapi_gw_init(void);

/* 网关反初始化 */
int coapi_gw_deinit(void);


#ifdef __cplusplus
} /* "C" */
#endif

#endif

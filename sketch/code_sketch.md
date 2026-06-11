---

excalidraw-plugin: parsed
tags: [excalidraw]

---
==⚠  Switch to EXCALIDRAW VIEW in the MORE OPTIONS menu of this document. ⚠== You can decompress Drawing data with the command palette: 'Decompress current Excalidraw file'. For more info check in plugin settings under 'Saving'


# Excalidraw Data

## Text Elements
/**
 * @brief Cấu trúc quản lý tin nhắn trong hệ thống CIEDPC
 * @param next: Con trỏ đến tin nhắn tiếp theo trong danh sách liên kết
 * @param src_task_id: ID của tác vụ nguồn gửi tin nhắn
 * @param des_task_id: ID của tác vụ đích nhận tin nhắn
 * @param sig: Tín hiệu của tin nhắn, dùng để xác định loại tin nhắn và hành động cần thực hiện
 * @param type: Loại Pool tin nhắn, dùng để xác định cách quản lý bộ nhớ cho tin nhắn
 * @param ref_count: Số lượng tham chiếu đến tin nhắn, dùng để quản lý việc giải phóng bộ nhớ khi tin nhắn được gửi đến nhiều tác vụ
 * @param data: Con trỏ đến vùng dữ liệu chứa payload của tin nhắn, kích thước và cách quản lý phụ thuộc vào loại Pool tin nhắn
 * @param interface: Metadata hỗ trợ cho việc quản lý tin nhắn từ các interface, bao gồm thông tin về nguồn và tín hiệu của tin nhắn
 */
typedef struct ciedpc_msg_t {
    /* Quản lý danh sách (Linker) */
    struct ciedpc_msg_t* next;
    
    /* Thông tin điều hướng */
    ui8 src_task_id;                 /* ID Nguồn */
    ui8 des_task_id;                 /* ID Đích */
    ui8 sig; /* Tín hiệu của tin nhắn */
    
    /* Quản lý bộ nhớ & Pool */
    ui8  type;          /* ciedpc_msg_type_t */
    ui8  ref_count;     /* Số lượng tham chiếu (dùng cho broadcast) */
    
    /* Payload dữ liệu */
    ui8* data;          /* Con trỏ đến vùng dữ liệu */
    
    /* Metadata hỗ trợ interface */
    struct {
            ui8 if_src_type;
            ui8 if_sig;
    } interface;
    
    /* Tùy chọn debug */
    #if defined(CIEDPC_DEBUG_FLAG) && (CIEDPC_DEBUG_FLAG & 0x01u)
            ui32 timestamp;   /* Thời điểm tạo tin nhắn */
    #endif
} ciedpc_msg_t; ^l9LE2JhO

/**
 * @brief Cấu trúc quản lý tin nhắn từ ngữ cảnh ISR
 * @param des_task_id: ID của tác vụ đích nhận tin nhắn từ ISR
 * @param sig: Tín hiệu của tin nhắn từ ISR
 */
typedef struct ciedpc_msg_isr_t {
        ui8 des_task_id;         
        ui8 sig;                                         
} ciedpc_msg_isr_t; ^COqJzU2y

ciedpc_msg_pool_header_t g_blank_pool_ctrl;
static ciedpc_msg_t blank_pool[CIEDPC_MSG_BLANK_QUEUE_SIZE];
#define CIEDPC_MSG_BLANK_QUEUE_SIZE  (8u) ^Z38RMV3R

ciedpc_msg_pool_header_t g_norm_pool_ctrl;
static ciedpc_msg_t norm_pool[CIEDPC_MSG_NORM_QUEUE_SIZE];
static ui8 norm_pool_data[CIEDPC_MSG_NORM_QUEUE_SIZE][CIEDPC_MSG_NORM_DATA_MAX];
#define CIEDPC_MSG_NORM_QUEUE_SIZE   (8u) // units
#define CIEDPC_MSG_NORM_DATA_MAX    (8u) // bytes ^sTT2xQp8

ciedpc_msg_pool_header_t g_alloc_pool_ctrl;
static ciedpc_msg_t alloc_pool[CIEDPC_MSG_ALLOC_QUEUE_SIZE];
static ui8 alloc_pool_data[CIEDPC_MSG_ALLOC_QUEUE_SIZE][CIEDPC_MSG_ALLOC_DATA_MAX];
#define CIEDPC_MSG_ALLOC_QUEUE_SIZE  (12u)  // units
#define CIEDPC_MSG_ALLOC_DATA_MAX   (8u) // bytes ^n4sQf8Lc

ciedpc_msg_pool_header_t g_extal_pool_ctrl;
static ciedpc_msg_t extal_pool[CIEDPC_MSG_EXTAL_QUEUE_SIZE];
static ui8 extal_pool_data[CIEDPC_MSG_EXTAL_QUEUE_SIZE][CIEDPC_MSG_EXTAL_DATA_MAX];
#define CIEDPC_MSG_EXTAL_QUEUE_SIZE  (16u)         // units
#define CIEDPC_MSG_EXTAL_DATA_MAX   (8u)                 // bytes ^Wi7FLqkx

static fifo_t isr_pool;
static ciedpc_msg_isr_t isr_pool_buffer[CIEDPC_MSG_ISR_QUEUE_SIZE];
#define CIEDPC_MSG_ISR_QUEUE_SIZE   (16u)         // units ^Ar9enanB

typedef struct ciedpc_msg_pool_header_t {
        ciedpc_msg_t* free_list;         
        ui8       used_count; 
        ui8       max_used;         
} ciedpc_msg_pool_header_t; ^XXUdTcu9

typedef enum ciedpc_msg_type_t {
    CIEDPC_MSG_TYPE_BLANK = 0,   
    CIEDPC_MSG_TYPE_NORM,                      
    CIEDPC_MSG_TYPE_ALLOC,                     
    CIEDPC_MSG_TYPE_EXTAL                            
} ciedpc_msg_type_t; ^zC0haUmt

sta task_norm_t g_current_task_norm; // Cấu trúc quản lý thông tin của tác vụ hiện tại đang được thực thi
sta task_id_t g_active_task_norm_id = CIEDPC_TASK_IDLE_ID; // ID của tác vụ hiện tại đang được thực thi
sta ciedpc_msg_t* g_current_msg; // Con trỏ đến tin nhắn hiện tại đang được xử lý bởi tác vụ hiện tại
sta task_pri_t g_task_norm_ready; // Biến bitmap quản lý trạng thái sẵn sàng của các tác vụ, mỗi bit tương ứng với một mức độ ưu tiên của tác vụ
sta task_norm_t* g_task_norm_table = NULL; // Bảng thông tin của các tác vụ bình thường
sta task_polling_t* g_task_polling_table = NULL; // Bảng thông tin của các tác vụ polling
sta ui8 g_task_norm_count = 0; // Số lượng tác vụ bình thường được đăng ký trong hệ thống
sta ui8 g_task_polling_count = 0; // Số lượng tác vụ polling được đăng ký trong hệ thống ^lNRlKPcb

#define CIEDPC_TASK_NORM_MAX_SIZE                          (16u) // 16 tác vụ, từ 0 đến 15
#define CIEDPC_TASK_NORM_TIM_ID                                (0xDF0) // Tác vụ timer
#define CIEDPC_TASK_NORM_IF_ID                      (0xDF1) // Tác vụ giao tiếp
#define CIEDPC_TASK_SYS_ID                                (0xDF2) // Tác vụ hệ thống (info + memrp)
#define CIEDPC_TASK_DBG_ID                                (0xDF3) // Tác vụ debug
#define CIEDPC_TASK_USR_ID                                (0xDF4) // Tác vụ người dùng
#define CIEDPC_TASK_IDLE_ID                                (0xDFE) // Tác vụ trống
#define CIEDPC_TASK_EOT_ID                                (0xDFF) // Kết thúc danh sách tác vụ
#define CIEDPC_TASK_MIN_ID                                 (0xDF0) // ID đầu tiên
#define CIEDPC_TASK_MAX_ID                                (0xDFF) // ID cuối cùng ^LYbLQyaL

typedef struct task_norm_t {
        task_id_t id;
        task_pri_t pri;
        ciedpc_fsm_t fsm;
        ciedpc_tsm_t tsm;
        pf_task_norm task_norm;
        fifo_t msg_queue;
        ciedpc_msg_t** msg_queue_buffer; 
} task_norm_t; ^p4OfjNOU

typedef struct task_polling_t {
task_id_t id;                                                                        
ui8 ability;                                                                  
pf_task_polling task_polling;        
} task_polling_t; ^QWd1YU5v

%%
## Drawing
```compressed-json
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
```
%%
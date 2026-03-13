#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>

// 角色宏定义
#define ROLE_ADMIN 0
#define ROLE_MANAGER 1
#define ROLE_EMPLOYEE 2

// --- 1. 数据结构定义 (融合图片创新点需求) ---

// 员工基本信息
typedef struct
{
    char id[20];        // 工号 (主键)
    char name[50];      // 姓名
    char password[64];  // 密码 (存储加密后的哈希值)
    int role;           // 角色 (0-管理员, 1-部门经理, 2-普通职员)
    char dept_id[20];   // 所属部门编号
    char join_date[15]; // 入职日期 YYYY-MM-DD (创新点：计算入职第N天)
    double base_salary; // 基础薪资 (创新点：预测薪资基础)
    int annual_leave;   // 剩余年假总时长(天)
} Employee;

typedef struct EmpNode
{
    Employee data;
    struct EmpNode *next;
} EmpNode;

// 打卡记录
typedef struct
{
    char record_id[30]; // 记录ID
    char emp_id[20];    // 员工工号
    char date[15];      // 日期 YYYY-MM-DD
    char clock_in[10];  // 上班打卡时间 HH:MM
    char clock_out[10]; // 下班打卡时间 HH:MM
    int status;         // 状态: 0-正常, 1-迟到, 2-早退, 3-缺勤
} AttendRecord;

typedef struct RecordNode
{
    AttendRecord data;
    struct RecordNode *next;
} RecordNode;

// 请假申请
typedef struct
{
    char leave_id[30];   // 请假单号
    char emp_id[20];     // 申请人工号
    int leave_type;      // 假期类型: 1-事假, 2-年假, 3-病假
    int days;            // 请假天数
    char start_date[15]; // 开始日期 YYYY-MM-DD
    char reason[100];    // 请假/取消原因备注 (创新点)
    int status;          // 状态: 0-待审批, 1-已通过, 2-已驳回, 3-已取消
} LeaveRequest;

typedef struct LeaveNode
{
    LeaveRequest data;
    struct LeaveNode *next;
} LeaveNode;

// --- 2. 全局工具函数声明 (utils.c) ---
void encrypt_password(const char *input, char *output);
void write_log(const char *operator_id, const char *action);
void get_current_date(char *date_str);
void get_current_time(char *time_str);
int calculate_days_from(const char *start_date);
void get_masked_password(char *pwd, int max_len);

// --- 3. 底层数据操作声明 (data_list.c) ---
EmpNode *load_emp_list();
void save_emp_list(EmpNode *head);
RecordNode *load_record_list();
void save_record_list(RecordNode *head);
LeaveNode *load_leave_list();
void save_leave_list(LeaveNode *head);

// (其他高级操作与业务菜单声明后续补充)
void insert_emp(EmpNode **head, Employee data);
void free_emp_list(EmpNode *head);
void insert_record(RecordNode **head, AttendRecord data);
void free_record_list(RecordNode *head);
void insert_leave(LeaveNode **head, LeaveRequest data);
void free_leave_list(LeaveNode *head);

int fuzzy_search_employee(EmpNode *head, const char *keyword);
void predict_monthly_salary(Employee *emp, RecordNode *rec_head, const char *month_prefix);
void sort_and_stat_attendance(EmpNode *emp_head, RecordNode *rec_head, const char *dept_id, const char *month_prefix, int sort_type);
#endif
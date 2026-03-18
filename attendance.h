#pragma once
#define _CRT_SECURE_NO_WARNINGS

#ifndef ATTENDANCE_H
#define ATTENDANCE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>
#include <ctype.h>

#define ROLE_ADMIN 0
#define ROLE_MANAGER 1
#define ROLE_EMPLOYEE 2

// --- 1. 数据结构定义 ---
typedef struct
{
    double late_penalty;
    double absent_penalty;
    double full_attendance_bonus;
} SalaryRule;

typedef struct
{
    char id[20];
    char name[50];
    char password[64];
    int role;
    char dept_name[32];
    char join_date[15];
    double base_salary;
    int annual_leave;
    int security_q_id;
    char security_answer[50];
    int failed_attempts;
    time_t lock_until;
} Employee;

typedef struct EmpNode
{
    Employee data;
    struct EmpNode *next;
} EmpNode;

typedef struct
{
    char record_id[30];
    char emp_id[20];
    char date[15];
    char clock_in[10];
    char clock_out[10];
    int status;
} AttendRecord;

typedef struct RecordNode
{
    AttendRecord data;
    struct RecordNode *next;
} RecordNode;

typedef struct
{
    char leave_id[30];
    char emp_id[20];
    int leave_type;
    int days;
    char start_date[15];
    char reason[100];
    int status;
} LeaveRequest;

typedef struct LeaveNode
{
    LeaveRequest data;
    struct LeaveNode *next;
} LeaveNode;

// --- 2. 全局工具与安全输入函数声明 ---
void encrypt_password(const char *input, char *output);
void write_log(const char *operator_id, const char *action);
void get_current_date(char *date_str);
void get_current_time(char *time_str);
int calculate_days_from(const char *start_date);
void get_masked_password(char *pwd, int max_len);
void get_valid_password(char *pwd);
void clear_input_buffer();
int get_int_input(const char *prompt, int min, int max);
double get_double_input(const char *prompt, double min, double max);
void get_string_input(const char *prompt, char *buffer, int max_len);
void get_date_input(const char *prompt, char *buffer);
void get_month_input(const char *prompt, char *buffer);
void get_line_input(const char *prompt, char *buffer, int max_len);
void get_non_empty_string(const char *prompt, char *buffer, int max_bytes);
SalaryRule load_salary_rule();
void save_salary_rule(SalaryRule rule);
void change_password(Employee *emp, EmpNode *emp_list);
Employee *find_employee_global(EmpNode *head, const char *id);
int run_captcha_verification();
void forgot_password_flow(Employee *emp, EmpNode *emp_list);
void init_security_question(Employee *emp, EmpNode *emp_list);

// --- 3. 底层数据操作声明 ---
EmpNode *load_emp_list();
void save_emp_list(EmpNode *head);
RecordNode *load_record_list();
void save_record_list(RecordNode *head);
LeaveNode *load_leave_list();
void save_leave_list(LeaveNode *head);
void insert_emp(EmpNode **head, Employee data);
void free_emp_list(EmpNode *head);
void insert_record(RecordNode **head, AttendRecord data);
void free_record_list(RecordNode *head);
void insert_leave(LeaveNode **head, LeaveRequest data);
void free_leave_list(LeaveNode *head);

// --- 4. 高级业务与各模块菜单声明 ---
int fuzzy_search_employee(EmpNode *head, const char *keyword);
void predict_monthly_salary(Employee *emp, RecordNode *rec_head, const char *month_prefix);
void sort_and_stat_attendance(EmpNode *emp_head, RecordNode *rec_head, const char *target_dept, const char *month_prefix, int sort_type);

// 【同步更新的新增/修改的签名】
void view_my_leaves(Employee *emp, LeaveNode *leave_list);
void admin_delete_user(Employee *admin, EmpNode **emp_head, RecordNode **rec_head, LeaveNode **leave_head);

void admin_menu(Employee *current_user, EmpNode **emp_list, RecordNode **rec_list, LeaveNode **leave_list);
void manager_menu(Employee *current_user, EmpNode **emp_list, RecordNode **rec_list, LeaveNode **leave_list);
void employee_menu(Employee *current_user, EmpNode **emp_list, RecordNode **rec_list, LeaveNode **leave_list);

#endif
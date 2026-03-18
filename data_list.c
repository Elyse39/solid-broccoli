#include "attendance.h"

// ==========================================
// 第一部分：Employee (员工信息) 链表与文件操作
// ==========================================

// 头插法插入员工节点
void insert_emp(EmpNode **head, Employee data)
{
    EmpNode *new_node = (EmpNode *)malloc(sizeof(EmpNode));
    if (!new_node)
    {
        printf("内存分配失败！\n");
        return;
    }
    new_node->data = data;
    new_node->next = *head;
    *head = new_node;
}

// 释放员工链表内存
void free_emp_list(EmpNode *head)
{
    EmpNode *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// 从二进制文件加载员工数据建立链表
EmpNode *load_emp_list()
{
    FILE *fp = fopen("employees.dat", "rb");
    EmpNode *head = NULL;
    if (!fp)
        return NULL; // 文件不存在说明是首次运行

    Employee temp;
    while (fread(&temp, sizeof(Employee), 1, fp) == 1)
    {
        insert_emp(&head, temp);
    }
    fclose(fp);
    return head;
}

// 将员工链表全量覆写到二进制文件
void save_emp_list(EmpNode *head)
{
    FILE *fp = fopen("employees.dat", "wb");
    if (!fp)
    {
        printf("无法打开文件保存员工数据！\n");
        return;
    }
    EmpNode *curr = head;
    while (curr != NULL)
    {
        fwrite(&(curr->data), sizeof(Employee), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}

// ==========================================
// 第二部分：AttendRecord (打卡记录) 链表与文件操作
// ==========================================

void insert_record(RecordNode **head, AttendRecord data)
{
    RecordNode *new_node = (RecordNode *)malloc(sizeof(RecordNode));
    if (!new_node)
        return;
    new_node->data = data;
    new_node->next = *head;
    *head = new_node;
}

void free_record_list(RecordNode *head)
{
    RecordNode *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

RecordNode *load_record_list()
{
    FILE *fp = fopen("records.dat", "rb");
    RecordNode *head = NULL;
    if (!fp)
        return NULL;

    AttendRecord temp;
    while (fread(&temp, sizeof(AttendRecord), 1, fp) == 1)
    {
        insert_record(&head, temp);
    }
    fclose(fp);
    return head;
}

void save_record_list(RecordNode *head)
{
    FILE *fp = fopen("records.dat", "wb");
    if (!fp)
        return;
    RecordNode *curr = head;
    while (curr != NULL)
    {
        fwrite(&(curr->data), sizeof(AttendRecord), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}

// ==========================================
// 第三部分：LeaveRequest (请假申请) 链表与文件操作
// ==========================================

void insert_leave(LeaveNode **head, LeaveRequest data)
{
    LeaveNode *new_node = (LeaveNode *)malloc(sizeof(LeaveNode));
    if (!new_node)
        return;
    new_node->data = data;
    new_node->next = *head;
    *head = new_node;
}

void free_leave_list(LeaveNode *head)
{
    LeaveNode *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

LeaveNode *load_leave_list()
{
    FILE *fp = fopen("leaves.dat", "rb");
    LeaveNode *head = NULL;
    if (!fp)
        return NULL;

    LeaveRequest temp;
    while (fread(&temp, sizeof(LeaveRequest), 1, fp) == 1)
    {
        insert_leave(&head, temp);
    }
    fclose(fp);
    return head;
}

void save_leave_list(LeaveNode *head)
{
    FILE *fp = fopen("leaves.dat", "wb");
    if (!fp)
        return;
    LeaveNode *curr = head;
    while (curr != NULL)
    {
        fwrite(&(curr->data), sizeof(LeaveRequest), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}
#include "attendance.h"

// ==========================================
// 第一部分：Employee (员工信息) 链表与文件操作
// ==========================================

// 头插法插入员工节点
void insert_emp(EmpNode **head, Employee data)
{
    EmpNode *new_node = (EmpNode *)malloc(sizeof(EmpNode));
    if (!new_node)
    {
        printf("内存分配失败！\n");
        return;
    }
    new_node->data = data;
    new_node->next = *head;
    *head = new_node;
}

// 释放员工链表内存
void free_emp_list(EmpNode *head)
{
    EmpNode *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

// 从二进制文件加载员工数据建立链表
EmpNode *load_emp_list()
{
    FILE *fp = fopen("employees.dat", "rb");
    EmpNode *head = NULL;
    if (!fp)
        return NULL; // 文件不存在说明是首次运行

    Employee temp;
    while (fread(&temp, sizeof(Employee), 1, fp) == 1)
    {
        insert_emp(&head, temp);
    }
    fclose(fp);
    return head;
}

// 将员工链表全量覆写到二进制文件
void save_emp_list(EmpNode *head)
{
    FILE *fp = fopen("employees.dat", "wb");
    if (!fp)
    {
        printf("无法打开文件保存员工数据！\n");
        return;
    }
    EmpNode *curr = head;
    while (curr != NULL)
    {
        fwrite(&(curr->data), sizeof(Employee), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}

// ==========================================
// 第二部分：AttendRecord (打卡记录) 链表与文件操作
// ==========================================

void insert_record(RecordNode **head, AttendRecord data)
{
    RecordNode *new_node = (RecordNode *)malloc(sizeof(RecordNode));
    if (!new_node)
        return;
    new_node->data = data;
    new_node->next = *head;
    *head = new_node;
}

void free_record_list(RecordNode *head)
{
    RecordNode *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

RecordNode *load_record_list()
{
    FILE *fp = fopen("records.dat", "rb");
    RecordNode *head = NULL;
    if (!fp)
        return NULL;

    AttendRecord temp;
    while (fread(&temp, sizeof(AttendRecord), 1, fp) == 1)
    {
        insert_record(&head, temp);
    }
    fclose(fp);
    return head;
}

void save_record_list(RecordNode *head)
{
    FILE *fp = fopen("records.dat", "wb");
    if (!fp)
        return;
    RecordNode *curr = head;
    while (curr != NULL)
    {
        fwrite(&(curr->data), sizeof(AttendRecord), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}

// ==========================================
// 第三部分：LeaveRequest (请假申请) 链表与文件操作
// ==========================================

void insert_leave(LeaveNode **head, LeaveRequest data)
{
    LeaveNode *new_node = (LeaveNode *)malloc(sizeof(LeaveNode));
    if (!new_node)
        return;
    new_node->data = data;
    new_node->next = *head;
    *head = new_node;
}

void free_leave_list(LeaveNode *head)
{
    LeaveNode *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

LeaveNode *load_leave_list()
{
    FILE *fp = fopen("leaves.dat", "rb");
    LeaveNode *head = NULL;
    if (!fp)
        return NULL;

    LeaveRequest temp;
    while (fread(&temp, sizeof(LeaveRequest), 1, fp) == 1)
    {
        insert_leave(&head, temp);
    }
    fclose(fp);
    return head;
}

void save_leave_list(LeaveNode *head)
{
    FILE *fp = fopen("leaves.dat", "wb");
    if (!fp)
        return;
    LeaveNode *curr = head;
    while (curr != NULL)
    {
        fwrite(&(curr->data), sizeof(LeaveRequest), 1, fp);
        curr = curr->next;
    }
    fclose(fp);
}
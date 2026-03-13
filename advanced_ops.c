#include "attendance.h"

// ==========================================
// 创新点 1：模糊查询处理重名情况
// ==========================================
// 返回匹配的员工数量，并在控制台打印列表供用户二次确认
int fuzzy_search_employee(EmpNode *head, const char *keyword)
{
    int count = 0;
    printf("\n--- 模糊查询结果 (关键词: '%s') ---\n", keyword);
    printf("%-15s %-15s %-15s %-10s\n", "工号", "姓名", "部门编号", "角色(0管1经2员)");
    printf("------------------------------------------------------------\n");

    while (head != NULL)
    {
        // strstr 用于在字符串中查找子串，实现模糊匹配
        if (strstr(head->data.name, keyword) != NULL)
        {
            printf("%-15s %-15s %-15s %d\n",
                   head->data.id, head->data.name, head->data.dept_id, head->data.role);
            count++;
        }
        head = head->next;
    }

    if (count == 0)
    {
        printf("未找到姓名包含 '%s' 的员工记录。\n", keyword);
    }
    else
    {
        printf("------------------------------------------------------------\n");
        printf("共查找到 %d 名相关员工。如遇重名，请在后续操作中核对【工号】。\n", count);
    }
    return count;
}

// ==========================================
// 创新点 2：根据考勤记录智能预测本月薪资
// ==========================================
// 假设规则：迟到早退每次扣50，缺卡(旷工)扣100，全勤无异常奖励200
void predict_monthly_salary(Employee *emp, RecordNode *rec_head, const char *month_prefix)
{
    int normal_days = 0;
    int late_early_days = 0;
    int absent_days = 0;

    RecordNode *curr = rec_head;
    while (curr != NULL)
    {
        // 匹配工号，并且通过 strncmp 匹配前7位字符 (例如 "2026-05")
        if (strcmp(curr->data.emp_id, emp->id) == 0 &&
            strncmp(curr->data.date, month_prefix, 7) == 0)
        {

            if (curr->data.status == 0)
                normal_days++;
            else if (curr->data.status == 1 || curr->data.status == 2)
                late_early_days++;
            else if (curr->data.status == 3)
                absent_days++;
        }
        curr = curr->next;
    }

    double penalty = (late_early_days * 50.0) + (absent_days * 100.0);
    // 只有当有正常出勤记录且没有任何异常时，才给全勤奖
    double bonus = (late_early_days == 0 && absent_days == 0 && normal_days > 0) ? 200.0 : 0.0;
    double predicted_salary = emp->base_salary - penalty + bonus;

    printf("\n>>> [%s] 月度薪资预测清单 (%s) <<<\n", month_prefix, emp->name);
    printf("  [基础薪资]: %.2f 元\n", emp->base_salary);
    printf("  [考勤明细]: 正常打卡 %d 次 | 迟到早退 %d 次 | 缺卡旷工 %d 次\n", normal_days, late_early_days, absent_days);
    printf("  [奖惩明细]: 满勤奖金 +%.2f 元 | 考勤违规扣款 -%.2f 元\n", bonus, penalty);
    printf("--------------------------------------------------\n");
    printf("  [预测实发]: %.2f 元\n\n", predicted_salary);
}

// ==========================================
// 创新点 3：复杂多属性排序与统计引擎
// ==========================================

// 为统计排榜量身定制的临时结构体
typedef struct
{
    char emp_id[20];
    char emp_name[50];
    int attendance_count; // 正常打卡次数
    int exception_count;  // 缺勤/异常次数
} EmpStat;

// qsort 比较函数 1：按缺勤/异常次数降序 (找出考勤最差的)
int cmp_exception_desc(const void *a, const void *b)
{
    EmpStat *statA = (EmpStat *)a;
    EmpStat *statB = (EmpStat *)b;
    return statB->exception_count - statA->exception_count;
}

// qsort 比较函数 2：按打卡次数升序
int cmp_attendance_asc(const void *a, const void *b)
{
    EmpStat *statA = (EmpStat *)a;
    EmpStat *statB = (EmpStat *)b;
    return statA->attendance_count - statB->attendance_count;
}

// 部门经理与管理员复用的核心统计模块：查月度打卡缺勤次数并排序
// dept_id 传NULL代表全公司(管理员特权)，否则仅统计指定部门(经理权限)
void sort_and_stat_attendance(EmpNode *emp_head, RecordNode *rec_head, const char *dept_id, const char *month_prefix, int sort_type)
{
    int count = 0;
    EmpNode *e_curr = emp_head;

    // 1. 第一遍遍历：计算符合条件的员工人数，用于动态分配内存
    while (e_curr != NULL)
    {
        if (dept_id == NULL || strcmp(e_curr->data.dept_id, dept_id) == 0)
        {
            count++;
        }
        e_curr = e_curr->next;
    }

    if (count == 0)
    {
        printf("无匹配的员工数据可供统计。\n");
        return;
    }

    // 2. 申请动态数组，提取并计算每个人的统计指标
    EmpStat *stats = (EmpStat *)malloc(count * sizeof(EmpStat));
    if (!stats)
        return;
    int idx = 0;

    e_curr = emp_head;
    while (e_curr != NULL)
    {
        if (dept_id == NULL || strcmp(e_curr->data.dept_id, dept_id) == 0)
        {
            strcpy(stats[idx].emp_id, e_curr->data.id);
            strcpy(stats[idx].emp_name, e_curr->data.name);
            stats[idx].attendance_count = 0;
            stats[idx].exception_count = 0;

            // 嵌套遍历记录表 (此算法时间复杂度为 O(N*M)，因课设数据量不大可接受)
            RecordNode *r_curr = rec_head;
            while (r_curr != NULL)
            {
                if (strcmp(r_curr->data.emp_id, stats[idx].emp_id) == 0 &&
                    strncmp(r_curr->data.date, month_prefix, 7) == 0)
                {
                    if (r_curr->data.status == 0)
                        stats[idx].attendance_count++;
                    else
                        stats[idx].exception_count++;
                }
                r_curr = r_curr->next;
            }
            idx++;
        }
        e_curr = e_curr->next;
    }

    // 3. 调用C标准库 qsort 进行极速泛型排序
    if (sort_type == 1)
    {
        qsort(stats, count, sizeof(EmpStat), cmp_exception_desc);
        printf("\n=== [%s] 月度异常/缺勤次数榜单 (降序排雷) ===\n", dept_id ? dept_id : "全公司");
    }
    else
    {
        qsort(stats, count, sizeof(EmpStat), cmp_attendance_asc);
        printf("\n=== [%s] 月度正常打卡次数榜单 (升序) ===\n", dept_id ? dept_id : "全公司");
    }

    // 4. 打印排榜结果
    printf("%-15s %-15s %-12s %-12s\n", "工号", "姓名", "正常出勤", "异常缺勤");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < count; i++)
    {
        printf("%-15s %-15s %-12d %-12d\n",
               stats[i].emp_id, stats[i].emp_name, stats[i].attendance_count, stats[i].exception_count);
    }
    printf("------------------------------------------------------\n");

    free(stats); // 防止内存泄漏
}
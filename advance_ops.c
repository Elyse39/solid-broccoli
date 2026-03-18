#include "attendance.h"

// ==========================================
// 模糊查询处理重名情况
// ==========================================
int fuzzy_search_employee(EmpNode *head, const char *keyword)
{
    int count = 0;
    printf("\n--- 模糊查询结果 (关键词: '%s') ---\n", keyword);
    printf("%-15s %-15s %-20s %-10s\n", "工号", "姓名", "部门名称", "角色(0管1经2员)");
    printf("----------------------------------------------------------------------\n");

    while (head != NULL)
    {
        if (strstr(head->data.name, keyword) != NULL)
        {
            printf("%-15s %-15s %-20s %d\n",
                   head->data.id, head->data.name, head->data.dept_name, head->data.role);
            count++;
        }
        head = head->next;
    }

    if (count == 0)
        printf("未找到姓名包含 '%s' 的员工记录。\n", keyword);
    else
    {
        printf("----------------------------------------------------------------------\n");
        printf("共查找到 %d 名相关员工。如遇重名，请在后续操作中核对【工号】。\n", count);
    }
    return count;
}

// ==========================================
// 根据考勤记录智能预测本月薪资 (动态公式)
// ==========================================
void predict_monthly_salary(Employee *emp, RecordNode *rec_head, const char *month_prefix)
{
    if (strncmp(month_prefix, emp->join_date, 7) < 0)
    {
        printf("\n[业务警报] 该员工 (%s) 在 %s 尚未入职！\n", emp->name, month_prefix);
        printf("入职日期为：%s，无法生成未入职期间的考勤与薪资明细。\n\n", emp->join_date);
        return;
    }

    int normal_days = 0, late_early_days = 0, absent_days = 0;
    RecordNode *curr = rec_head;
    while (curr != NULL)
    {
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

    SalaryRule rule = load_salary_rule();
    double penalty = (late_early_days * rule.late_penalty) + (absent_days * rule.absent_penalty);
    double bonus = (late_early_days == 0 && absent_days == 0 && normal_days > 0) ? rule.full_attendance_bonus : 0.0;
    double predicted_salary = emp->base_salary - penalty + bonus;

    printf("\n>>> [%s] 月度薪资预测清单 (%s - %s) <<<\n", month_prefix, emp->name, emp->dept_name);
    printf("  [基础薪资]: %.2f 元\n", emp->base_salary);
    printf("  [考勤明细]: 正常打卡 %d 次 | 迟到早退 %d 次 | 缺卡旷工 %d 次\n", normal_days, late_early_days, absent_days);
    printf("  [计算公式]: 迟到扣 %.2f/次 | 缺卡扣 %.2f/次 | 全勤奖 %.2f\n", rule.late_penalty, rule.absent_penalty, rule.full_attendance_bonus);
    printf("  [奖惩明细]: 满勤奖金 +%.2f 元 | 考勤违规扣款 -%.2f 元\n", bonus, penalty);
    printf("--------------------------------------------------\n");
    printf("  [预测实发]: %.2f 元\n\n", predicted_salary);
}

// ==========================================
// 全局通用：修改个人密码模块 (接入强密码校验)
// ==========================================
void change_password(Employee *emp, EmpNode *emp_list)
{
    char old_pwd[64], new_pwd1[64], new_pwd2[64], hashed_old[64], hashed_new[64];
    printf("\n--- 修改个人密码 ---\n");
    printf("请输入原密码: ");
    get_masked_password(old_pwd, 64);
    encrypt_password(old_pwd, hashed_old);

    if (strcmp(emp->password, hashed_old) != 0)
    {
        printf("[系统警告] 原密码错误，修改失败！\n");
        return;
    }

    printf("请输入新密码: ");
    get_valid_password(new_pwd1);
    printf("请再次确认新密码: ");
    get_masked_password(new_pwd2, 64);

    if (strcmp(new_pwd1, new_pwd2) != 0)
    {
        printf("[系统警告] 两次输入的新密码不一致，修改失败！\n");
        return;
    }

    encrypt_password(new_pwd1, hashed_new);
    strcpy(emp->password, hashed_new);
    save_emp_list(emp_list);

    printf("\n>>> 密码修改成功，请牢记新密码！ <<<\n");
    write_log(emp->id, "修改了个人密码");
}

// ==========================================
// 复杂多属性排序与统计引擎
// ==========================================
typedef struct
{
    char emp_id[20];
    char emp_name[50];
    int attendance_count;
    int exception_count;
} EmpStat;

// 【核心修复】引入次级排序条件(按工号)，保证相同数据的排序稳定性
int cmp_exception_desc(const void *a, const void *b)
{
    EmpStat *statA = (EmpStat *)a;
    EmpStat *statB = (EmpStat *)b;
    if (statB->exception_count != statA->exception_count)
    {
        return statB->exception_count - statA->exception_count;
    }
    return strcmp(statA->emp_id, statB->emp_id);
}

int cmp_attendance_asc(const void *a, const void *b)
{
    EmpStat *statA = (EmpStat *)a;
    EmpStat *statB = (EmpStat *)b;
    if (statA->attendance_count != statB->attendance_count)
    {
        return statA->attendance_count - statB->attendance_count;
    }
    return strcmp(statA->emp_id, statB->emp_id);
}

void sort_and_stat_attendance(EmpNode *emp_head, RecordNode *rec_head, const char *target_dept, const char *month_prefix, int sort_type)
{
    int count = 0;
    EmpNode *e_curr = emp_head;
    while (e_curr != NULL)
    {
        if (target_dept == NULL || strcmp(e_curr->data.dept_name, target_dept) == 0)
            count++;
        e_curr = e_curr->next;
    }

    if (count == 0)
    {
        printf("无匹配的员工数据可供统计。\n");
        return;
    }

    EmpStat *stats = (EmpStat *)malloc(count * sizeof(EmpStat));
    if (!stats)
        return;
    int idx = 0;

    e_curr = emp_head;
    while (e_curr != NULL)
    {
        if (target_dept == NULL || strcmp(e_curr->data.dept_name, target_dept) == 0)
        {
            strcpy(stats[idx].emp_id, e_curr->data.id);
            strcpy(stats[idx].emp_name, e_curr->data.name);
            stats[idx].attendance_count = 0;
            stats[idx].exception_count = 0;

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

    if (sort_type == 1)
    {
        qsort(stats, count, sizeof(EmpStat), cmp_exception_desc);
        printf("\n=== [%s] 月度异常/缺勤次数榜单 (降序排雷) ===\n", target_dept ? target_dept : "全公司");
    }
    else
    {
        qsort(stats, count, sizeof(EmpStat), cmp_attendance_asc);
        printf("\n=== [%s] 月度正常打卡次数榜单 (升序) ===\n", target_dept ? target_dept : "全公司");
    }

    printf("%-15s %-15s %-12s %-12s\n", "工号", "姓名", "正常出勤", "异常缺勤");
    printf("------------------------------------------------------\n");
    for (int i = 0; i < count; i++)
    {
        printf("%-15s %-15s %-12d %-12d\n",
               stats[i].emp_id, stats[i].emp_name, stats[i].attendance_count, stats[i].exception_count);
    }
    printf("------------------------------------------------------\n");
    free(stats);
}
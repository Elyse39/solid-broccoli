#include "attendance.h"

// ==========================================
// 辅助函数
// ==========================================
EmpNode *find_emp_prev(EmpNode *head, const char *id)
{
    EmpNode *prev = NULL;
    EmpNode *curr = head;
    while (curr != NULL)
    {
        if (strcmp(curr->data.id, id) == 0)
            return prev;
        prev = curr;
        curr = curr->next;
    }
    return NULL;
}

Employee *find_employee(EmpNode *head, const char *id)
{
    while (head != NULL)
    {
        if (strcmp(head->data.id, id) == 0)
            return &(head->data);
        head = head->next;
    }
    return NULL;
}

// ==========================================
// 管理员：添加新用户 (接入全套严格输入校验)
// ==========================================
void admin_add_user(Employee *admin, EmpNode **head)
{
    Employee new_emp;
    printf("\n--- 添加新用户 ---\n");

    get_string_input("请输入工号: ", new_emp.id, 20);

    if (find_employee(*head, new_emp.id) != NULL)
    {
        printf("该工号已存在！\n");
        return;
    }

    get_string_input("请输入姓名: ", new_emp.name, 50);
    new_emp.role = get_int_input("请输入角色 (1-部门经理, 2-普通职员): ", 1, 2);

    // 强制非空且限制 10 个中文字符的输入函数
    get_non_empty_string("请输入所属部门名称 (中文/英文，最多10个字符): ", new_emp.dept_name, 32);

    // 绝对白名单日期校验
    get_date_input("请输入入职日期 (YYYY-MM-DD): ", new_emp.join_date);

    // 防溢出金额限制
    new_emp.base_salary = get_double_input("请输入基础薪资: ", 0.0, 1000000.0);
    new_emp.annual_leave = 5;

    // 安全字段初始化
    new_emp.security_q_id = 0;
    new_emp.failed_attempts = 0;
    new_emp.lock_until = 0;

    // 默认初始密码为 123456 (完美符合 6-14 位数字要求)
    encrypt_password("123456", new_emp.password);
    insert_emp(head, new_emp);
    save_emp_list(*head);

    printf("用户 %s 添加成功，默认密码为 123456。\n", new_emp.name);

    char log_msg[100];
    sprintf(log_msg, "添加了新用户: %s (工号: %s)", new_emp.name, new_emp.id);
    write_log(admin->id, log_msg);
}

// ==========================================
// 管理员：删除用户 (【核心修复】引入级联删除防崩溃)
// ==========================================
void admin_delete_user(Employee *admin, EmpNode **emp_head, RecordNode **rec_head, LeaveNode **leave_head)
{
    char target_id[20];
    printf("\n--- 删除用户 ---\n");
    get_string_input("请输入要删除的员工工号: ", target_id, 20);

    if (strcmp(admin->id, target_id) == 0)
    {
        printf("警告：不能删除超级管理员自身！\n");
        return;
    }

    // 1. 从员工链表中删除
    EmpNode *prev = find_emp_prev(*emp_head, target_id);
    EmpNode *curr = NULL;

    if (prev == NULL)
    {
        if (*emp_head != NULL && strcmp((*emp_head)->data.id, target_id) == 0)
        {
            curr = *emp_head;
            *emp_head = curr->next;
        }
    }
    else
    {
        curr = prev->next;
        if (curr != NULL)
            prev->next = curr->next;
    }

    if (curr != NULL)
    {
        printf("已成功删除员工: %s 的基础档案。\n", curr->data.name);
        char log_msg[100];
        sprintf(log_msg, "删除了用户: %s (工号: %s)", curr->data.name, curr->data.id);
        write_log(admin->id, log_msg);
        free(curr);
        save_emp_list(*emp_head);

        // 2. 【级联删除】清理考勤记录中的冗余数据
        RecordNode *r_curr = *rec_head;
        RecordNode *r_prev = NULL;
        int rec_del_count = 0;
        while (r_curr != NULL)
        {
            if (strcmp(r_curr->data.emp_id, target_id) == 0)
            {
                RecordNode *temp = r_curr;
                if (r_prev == NULL)
                {
                    *rec_head = r_curr->next;
                    r_curr = *rec_head;
                }
                else
                {
                    r_prev->next = r_curr->next;
                    r_curr = r_prev->next;
                }
                free(temp);
                rec_del_count++;
            }
            else
            {
                r_prev = r_curr;
                r_curr = r_curr->next;
            }
        }
        if (rec_del_count > 0)
        {
            save_record_list(*rec_head);
            printf(">> 同步清理了该员工的 %d 条历史打卡记录。\n", rec_del_count);
        }

        // 3. 【级联删除】清理请假记录中的冗余数据
        LeaveNode *l_curr = *leave_head;
        LeaveNode *l_prev = NULL;
        int leave_del_count = 0;
        while (l_curr != NULL)
        {
            if (strcmp(l_curr->data.emp_id, target_id) == 0)
            {
                LeaveNode *temp = l_curr;
                if (l_prev == NULL)
                {
                    *leave_head = l_curr->next;
                    l_curr = *leave_head;
                }
                else
                {
                    l_prev->next = l_curr->next;
                    l_curr = l_prev->next;
                }
                free(temp);
                leave_del_count++;
            }
            else
            {
                l_prev = l_curr;
                l_curr = l_curr->next;
            }
        }
        if (leave_del_count > 0)
        {
            save_leave_list(*leave_head);
            printf(">> 同步清理了该员工的 %d 条历史请假记录。\n", leave_del_count);
        }
        printf("数据清理彻底完成！\n");
    }
    else
    {
        printf("未找到该工号的员工。\n");
    }
}

// ==========================================
// 管理员：重置用户密码
// ==========================================
void admin_reset_password(Employee *admin, EmpNode *head)
{
    char target_id[20];
    printf("\n--- 重置用户密码 ---\n");
    get_string_input("请输入要重置密码的员工工号: ", target_id, 20);

    Employee *emp = find_employee(head, target_id);
    if (emp)
    {
        encrypt_password("123456", emp->password);
        emp->failed_attempts = 0;
        emp->lock_until = 0;

        save_emp_list(head);
        printf("员工 %s 的密码已重置为 123456，且账户已解锁。\n", emp->name);

        char log_msg[100];
        sprintf(log_msg, "重置了用户密码并解锁 (工号: %s)", emp->id);
        write_log(admin->id, log_msg);
    }
    else
    {
        printf("未找到该员工！\n");
    }
}

// ==========================================
// 管理员：动态设置薪资预测计算公式
// ==========================================
void admin_set_salary_rule(Employee *admin)
{
    printf("\n--- 设置全局薪资预测公式 ---\n");
    SalaryRule rule = load_salary_rule();
    printf("【当前规则】迟到扣款: %.2f | 缺勤扣款: %.2f | 全勤奖励: %.2f\n",
           rule.late_penalty, rule.absent_penalty, rule.full_attendance_bonus);

    rule.late_penalty = get_double_input("请输入新的迟到/早退扣款金额: ", 0.0, 5000.0);
    rule.absent_penalty = get_double_input("请输入新的缺勤扣款金额: ", 0.0, 5000.0);
    rule.full_attendance_bonus = get_double_input("请输入新的全勤奖励金额: ", 0.0, 10000.0);

    save_salary_rule(rule);
    printf(">> 薪资公式规则更新成功！并已全局生效。\n");
    write_log(admin->id, "修改了系统薪资预测公式规则");
}

// ==========================================
// 全局账户安全审计大盘
// ==========================================
void admin_view_all_accounts(EmpNode *head)
{
    printf("\n=================================================================================\n");
    printf("                              【全局账户安全审计大盘】                              \n");
    printf("=================================================================================\n");
    printf("%-15s %-15s %-15s %-15s %-10s %-15s\n", "工号", "姓名", "部门", "角色", "连续错登", "账户状态");
    printf("---------------------------------------------------------------------------------\n");

    int total_users = 0;
    int locked_users = 0;
    time_t now = time(NULL);

    while (head != NULL)
    {
        char status_str[30];
        char role_str[20];

        if (head->data.role == ROLE_ADMIN)
            strcpy(role_str, "超级管理员");
        else if (head->data.role == ROLE_MANAGER)
            strcpy(role_str, "部门经理");
        else
            strcpy(role_str, "普通职员");

        if (head->data.lock_until > now)
        {
            int wait_mins = (int)((head->data.lock_until - now) / 60) + 1;
            sprintf(status_str, "已锁定(%d分)", wait_mins);
            locked_users++;
        }
        else
        {
            strcpy(status_str, "正常");
        }

        printf("%-15s %-15s %-15s %-15s %-10d %-15s\n",
               head->data.id,
               head->data.name,
               head->data.dept_name,
               role_str,
               head->data.failed_attempts,
               status_str);

        total_users++;
        head = head->next;
    }
    printf("---------------------------------------------------------------------------------\n");
    printf("统计信息：共计 %d 名用户，当前处于锁定状态的异常账户 %d 个。\n", total_users, locked_users);
    printf("=================================================================================\n");
}

// ==========================================
// 管理员主菜单
// ==========================================
void admin_menu(Employee *current_user, EmpNode **emp_list, RecordNode **rec_list, LeaveNode **leave_list)
{
    int choice;
    while (1)
    {
        printf("\n=============================\n");
        printf("    [超级管理员控制台]    \n");
        printf("=============================\n");
        printf("1. 添加新用户\n");
        printf("2. 删除用户\n");
        printf("3. 重置用户密码\n");
        printf("4. 全局考勤异常排行榜\n");
        printf("5. 修改薪资预测计算公式\n");
        printf("6. 修改个人登录密码\n");
        printf("7. 全局账户安全审计大盘\n");
        printf("8. 退出登录\n");

        choice = get_int_input("请选择操作: ", 1, 8);

        char month_prefix[10];
        switch (choice)
        {
        case 1:
            admin_add_user(current_user, emp_list);
            break;
        case 2:
            admin_delete_user(current_user, emp_list, rec_list, leave_list);
            break; // 传入全局链表进行级联删除
        case 3:
            admin_reset_password(current_user, *emp_list);
            break;
        case 4:
            get_month_input("请输入要统计的月份 (如 2026-03): ", month_prefix);
            sort_and_stat_attendance(*emp_list, *rec_list, NULL, month_prefix, 1);
            break;
        case 5:
            admin_set_salary_rule(current_user);
            break;
        case 6:
            change_password(current_user, *emp_list);
            break;
        case 7:
            admin_view_all_accounts(*emp_list);
            break;
        case 8:
            return;
        }
    }
}
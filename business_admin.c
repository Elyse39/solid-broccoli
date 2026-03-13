#include "attendance.h"

// ==========================================
// 辅助函数：根据工号查找员工节点的前驱，用于删除操作
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

// 辅助函数：根据工号查找员工本身
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
// 管理员：添加新用户 (包含密码加密初始化)
// ==========================================
void admin_add_user(Employee *admin, EmpNode **head)
{
    Employee new_emp;
    printf("\n--- 添加新用户 ---\n");
    printf("请输入工号: ");
    scanf("%s", new_emp.id);

    if (find_employee(*head, new_emp.id) != NULL)
    {
        printf("该工号已存在！\n");
        return;
    }

    printf("请输入姓名: ");
    scanf("%s", new_emp.name);
    printf("请输入角色 (1-部门经理, 2-普通职员): ");
    scanf("%d", &new_emp.role);
    printf("请输入所属部门编号: ");
    scanf("%s", new_emp.dept_id);
    printf("请输入入职日期 (YYYY-MM-DD): ");
    scanf("%s", new_emp.join_date);
    printf("请输入基础薪资: ");
    scanf("%lf", &new_emp.base_salary);
    new_emp.annual_leave = 5; // 默认给5天年假

    // 默认密码加密存储 (如设为 "123456")
    encrypt_password("123456", new_emp.password);

    insert_emp(head, new_emp);
    save_emp_list(*head); // 立即持久化

    printf("用户 %s 添加成功，默认密码为 123456。\n", new_emp.name);

    // 写入日志
    char log_msg[100];
    sprintf(log_msg, "添加了新用户: %s (工号: %s)", new_emp.name, new_emp.id);
    write_log(admin->id, log_msg);
}

// ==========================================
// 管理员：删除用户
// ==========================================
void admin_delete_user(Employee *admin, EmpNode **head)
{
    char target_id[20];
    printf("\n--- 删除用户 ---\n");
    printf("请输入要删除的员工工号: ");
    scanf("%s", target_id);

    if (strcmp(admin->id, target_id) == 0)
    {
        printf("警告：不能删除超级管理员自身！\n");
        return;
    }

    EmpNode *prev = find_emp_prev(*head, target_id);
    EmpNode *curr = NULL;

    if (prev == NULL)
    {
        if (*head != NULL && strcmp((*head)->data.id, target_id) == 0)
        {
            curr = *head;
            *head = curr->next;
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
        printf("已成功删除员工: %s\n", curr->data.name);

        // 写入日志
        char log_msg[100];
        sprintf(log_msg, "删除了用户: %s (工号: %s)", curr->data.name, curr->data.id);
        write_log(admin->id, log_msg);

        free(curr);
        save_emp_list(*head);
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
    printf("请输入要重置密码的员工工号: ");
    scanf("%s", target_id);

    Employee *emp = find_employee(head, target_id);
    if (emp)
    {
        encrypt_password("123456", emp->password);
        save_emp_list(head);
        printf("员工 %s 的密码已重置为 123456。\n", emp->name);

        char log_msg[100];
        sprintf(log_msg, "重置了用户密码 (工号: %s)", emp->id);
        write_log(admin->id, log_msg);
    }
    else
    {
        printf("未找到该员工！\n");
    }
}

// ==========================================
// 管理员：主菜单调度
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
        printf("4. 全局考勤异常排行榜 (降序)\n");
        printf("5. 退出登录\n");
        printf("请选择操作: ");
        scanf("%d", &choice);

        char month_prefix[10];
        switch (choice)
        {
        case 1:
            admin_add_user(current_user, emp_list);
            break;
        case 2:
            admin_delete_user(current_user, emp_list);
            break;
        case 3:
            admin_reset_password(current_user, *emp_list);
            break;
        case 4:
            printf("请输入要统计的月份 (如 2026-03): ");
            scanf("%s", month_prefix);
            // 传NULL代表全公司，传1代表降序排雷
            sort_and_stat_attendance(*emp_list, *rec_list, NULL, month_prefix, 1);
            break;
        case 5:
            return;
        default:
            printf("无效选择，请重试。\n");
        }
    }
}
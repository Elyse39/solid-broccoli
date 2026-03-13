#include "attendance.h"

// 登录验证核心逻辑
Employee *login_verify(EmpNode *emp_list, const char *id, const char *input_pwd)
{
    char hashed_pwd[64];
    encrypt_password(input_pwd, hashed_pwd); // 先将输入明文转哈希

    EmpNode *curr = emp_list;
    while (curr != NULL)
    {
        // 校验工号与加密后的哈希密码
        if (strcmp(curr->data.id, id) == 0 && strcmp(curr->data.password, hashed_pwd) == 0)
        {
            return &(curr->data);
        }
        curr = curr->next;
    }
    return NULL;
}

int main()
{
    // 1. 系统启动，从硬盘加载三大核心数据链表
    EmpNode *emp_list = load_emp_list();
    RecordNode *rec_list = load_record_list();
    LeaveNode *leave_list = load_leave_list();

    // 2. 初始状态检测：如果没用户，自动初始化一个Admin
    if (emp_list == NULL)
    {
        printf("[系统初始化] 检测到空数据库，正在创建默认超级管理员...\n");
        Employee admin;
        strcpy(admin.id, "admin");
        strcpy(admin.name, "系统管理员");
        admin.role = ROLE_ADMIN;
        strcpy(admin.dept_id, "000");
        strcpy(admin.join_date, "2026-01-01");
        admin.base_salary = 0.0;
        admin.annual_leave = 999;
        encrypt_password("admin123", admin.password); // 初始密码 admin123

        insert_emp(&emp_list, admin);
        save_emp_list(emp_list);
        printf("[系统初始化] 创建成功！账号: admin，初始密码: admin123\n\n");
    }

    // 3. 登录交互循环
    char input_id[20];
    char input_pwd[64];

    while (1)
    {
        printf("======================================\n");
        printf("      企业级考勤管理系统 (v2.0)     \n");
        printf("======================================\n");
        printf("请输入工号 (输入 'exit' 退出程序): ");
        scanf("%s", input_id);

        if (strcmp(input_id, "exit") == 0)
        {
            break; // 退出总循环
        }

        printf("请输入密码: ");
        get_masked_password(input_pwd, 64); // 调用之前写好的掩码工具

        // 验证身份
        Employee *current_user = login_verify(emp_list, input_id, input_pwd);

        if (current_user != NULL)
        {
            printf("\n>>> 登录成功！欢迎您, %s <<<\n", current_user->name);

            // 记录登录日志
            char log_msg[50];
            sprintf(log_msg, "系统登录成功 (Role: %d)", current_user->role);
            write_log(current_user->id, log_msg);

            // 根据角色派发系统菜单
            if (current_user->role == ROLE_ADMIN)
            {
                admin_menu(current_user, &emp_list, &rec_list, &leave_list);
            }
            else if (current_user->role == ROLE_MANAGER)
            {
                manager_menu(current_user, &emp_list, &rec_list, &leave_list);
            }
            else if (current_user->role == ROLE_EMPLOYEE)
            {
                employee_menu(current_user, &emp_list, &rec_list, &leave_list);
            }

            // 用户退出自己的菜单后，统一执行一次全量数据保存策底防丢
            save_emp_list(emp_list);
            save_record_list(rec_list);
            save_leave_list(leave_list);
            printf("\n[系统提示] 数据已同步保存，您已安全登出。\n\n");
        }
        else
        {
            printf("\n[错误] 工号或密码不匹配，请重试！\n\n");
            write_log(input_id, "尝试登录失败：密码错误");
        }
    }

    // 4. 程序彻底结束前，释放所有链表内存，严防内存泄漏
    free_emp_list(emp_list);
    free_record_list(rec_list);
    free_leave_list(leave_list);

    printf("\n感谢使用考勤管理系统，再见！\n");
    return 0;
}
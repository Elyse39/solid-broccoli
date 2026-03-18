#include "attendance.h"

int main()
{
    // 【系统级修复】全局只初始化一次随机数种子，防止高频调用时验证码重复
    srand((unsigned int)time(NULL));

    // 1. 系统启动加载数据
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
        strcpy(admin.dept_name, "总经办");
        strcpy(admin.join_date, "2026-01-01");
        admin.base_salary = 0.0;
        admin.annual_leave = 999;
        admin.security_q_id = 0;
        admin.failed_attempts = 0;
        admin.lock_until = 0;
        // 默认密码必须符合 6-14 位要求
        encrypt_password("admin123", admin.password);

        insert_emp(&emp_list, admin);
        save_emp_list(emp_list);
        printf("[系统初始化] 创建成功！账号: admin，初始密码: admin123\n\n");
    }

    char input_id[20];
    char input_pwd[64];

    // 3. 登录交互循环
    while (1)
    {
        printf("======================================\n");
        printf("      企业级考勤管理系统 (安全增强版)  \n");
        printf("======================================\n");

        get_string_input("请输入工号 (输入 'exit' 退出程序): ", input_id, 20);

        if (strcmp(input_id, "exit") == 0)
            break;

        // 【步骤 1：账号存在性校验】
        Employee *current_user = find_employee_global(emp_list, input_id);
        if (current_user == NULL)
        {
            printf("\n[错误] 该工号是未注册工号，请联系超级管理员进行注册！\n\n");
            continue;
        }

        // 【步骤 2：账户锁定校验】
        time_t now = time(NULL);
        if (current_user->lock_until > now)
        {
            int wait_mins = (int)((current_user->lock_until - now) / 60) + 1;
            printf("\n[安全拦截] 账户因多次输入错误已被锁定！请等待 %d 分钟后再试。\n\n", wait_mins);
            continue;
        }

        // 【步骤 2.5：登录系统环境混合验证码拦截】
        if (!run_captcha_verification())
        {
            printf("\n[系统拦截] 验证码输入错误，防暴恐登录机制已触发，请重新操作！\n\n");
            continue;
        }

        // 【步骤 3：密码校验】
        printf("请输入密码: ");
        get_masked_password(input_pwd, 64);

        char hashed_pwd[64];
        encrypt_password(input_pwd, hashed_pwd);

        if (strcmp(current_user->password, hashed_pwd) == 0)
        {
            // 登录成功逻辑
            current_user->failed_attempts = 0;
            current_user->lock_until = 0;

            if (current_user->security_q_id == 0 && strcmp(current_user->id, "admin") != 0)
            {
                printf("\n[系统安全提醒] 监测到您首次登录或未绑定密保，为保证账户安全，请强制绑定：\n");
                init_security_question(current_user, emp_list);
            }

            printf("\n>>> 登录成功！欢迎您, %s <<<\n", current_user->name);
            char log_msg[50];
            sprintf(log_msg, "系统登录成功 (Role: %d)", current_user->role);
            write_log(current_user->id, log_msg);

            // 派发菜单
            if (current_user->role == ROLE_ADMIN)
                admin_menu(current_user, &emp_list, &rec_list, &leave_list);
            else if (current_user->role == ROLE_MANAGER)
                manager_menu(current_user, &emp_list, &rec_list, &leave_list);
            else if (current_user->role == ROLE_EMPLOYEE)
                employee_menu(current_user, &emp_list, &rec_list, &leave_list);

            // 退出后全量保存
            save_emp_list(emp_list);
            save_record_list(rec_list);
            save_leave_list(leave_list);
            printf("\n[系统提示] 数据已同步保存，您已安全登出。\n\n");
        }
        else
        {
            // 密码错误逻辑
            current_user->failed_attempts++;
            save_emp_list(emp_list);

            write_log(input_id, "尝试登录失败：密码错误");

            if (current_user->failed_attempts >= 3)
            {
                current_user->lock_until = now + 5 * 60; // 锁定 5 分钟
                save_emp_list(emp_list);
                printf("\n[严重警告] 密码连续错误 3 次，为保护数据安全，您的账户已锁定 5 分钟！\n\n");

                char lock_msg[100];
                sprintf(lock_msg, "触发安全防护：连续3次密码错误，账户被锁定5分钟");
                write_log(input_id, lock_msg);
            }
            else
            {
                printf("\n[错误] 密码错误！您还有 %d 次尝试机会。\n", 3 - current_user->failed_attempts);

                char choice[10];
                get_string_input("是否忘记密码？(输入 Y 找回，任意键重试): ", choice, 10);
                if (strcmp(choice, "Y") == 0 || strcmp(choice, "y") == 0)
                {
                    forgot_password_flow(current_user, emp_list);
                }
            }
        }
    }

    // 4. 程序彻底结束前，释放内存
    free_emp_list(emp_list);
    free_record_list(rec_list);
    free_leave_list(leave_list);
    printf("\n感谢使用考勤管理系统，再见！\n");
    return 0;
}
package com.chinasoft.backend.service;

import com.baomidou.mybatisplus.extension.service.IService;
import com.chinasoft.backend.model.entity.Employee;

import javax.servlet.http.HttpServletRequest;

/**
 * @author 86178
 * @description 针对表【employee】的数据库操作Service
 * @createDate 2024-04-12 17:21:37
 */
public interface EmployeeService extends IService<Employee> {

    Employee adminLogin(String phone, String password, HttpServletRequest request);
    
    Employee getLoginEmployee(HttpServletRequest request);
}

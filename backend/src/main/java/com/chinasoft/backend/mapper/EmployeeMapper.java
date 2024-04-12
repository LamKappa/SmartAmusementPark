package com.chinasoft.backend.mapper;

import com.baomidou.mybatisplus.core.mapper.BaseMapper;
import com.chinasoft.backend.model.entity.Employee;
import org.apache.ibatis.annotations.Mapper;

/**
 * @author 86178
 * @description 针对表【employee】的数据库操作Mapper
 * @createDate 2024-04-12 17:21:37
 * @Entity com.chinasoft.backend.model.entity.Employee
 */
@Mapper
public interface EmployeeMapper extends BaseMapper<Employee> {

}





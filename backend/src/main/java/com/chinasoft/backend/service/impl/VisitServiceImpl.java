package com.chinasoft.backend.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.baomidou.mybatisplus.core.conditions.update.UpdateWrapper;
import com.baomidou.mybatisplus.core.toolkit.Wrappers;
import com.baomidou.mybatisplus.extension.service.impl.ServiceImpl;
import com.chinasoft.backend.common.ErrorCode;
import com.chinasoft.backend.constant.FacilityTypeConstant;
import com.chinasoft.backend.exception.BusinessException;
import com.chinasoft.backend.mapper.*;
import com.chinasoft.backend.model.entity.*;
import com.chinasoft.backend.model.request.VisitAndSubscribeAddRequest;
import com.chinasoft.backend.model.request.VisitAndSubscribeDeleteRequest;
import com.chinasoft.backend.model.vo.AmusementVandSVO;
import com.chinasoft.backend.service.VisitService;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

@Service
public class VisitServiceImpl extends ServiceImpl<VisitMapper, Visit>
    implements VisitService{

    @Autowired
    private UserMapper userMapper;

    @Autowired
    private AmusementFacilityMapper amusementFacilityMapper;

    @Autowired
    private RestaurantFacilityMapper restaurantFacilityMapper;

    @Autowired
    private BaseFacilityMapper baseFacilityMapper;



    @Override
    public Visit addVisit(VisitAndSubscribeAddRequest visitAndSubscribeAddRequest) {
        Long userId = Long.valueOf(visitAndSubscribeAddRequest.getUserId());
        Long facilityId = Long.valueOf(visitAndSubscribeAddRequest.getFacility().getFacilityId());
        Integer facilityType = visitAndSubscribeAddRequest.getFacility().getFacilityType();

        // 检查是否已存在相同数据的记录
        QueryWrapper<Visit> queryWrapper = Wrappers.<Visit>query()
                .eq("user_id", userId)
                .eq("facility_id", facilityId)
                .eq("facility_type", facilityType);

        Visit existingVisit = this.baseMapper.selectOne(queryWrapper);
        if (existingVisit != null) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR, "打卡重复");
        }

        // 检查用户是否存在
        QueryWrapper<User> queryWrapper2 = Wrappers.<User>query().eq("id", userId);
        User existingUser = userMapper.selectOne(queryWrapper2);
        if (existingUser == null) {
            throw new BusinessException(ErrorCode.PARAMS_ERROR, "用户不存在");
        }

        // 检查设施是否存在
        if(facilityType == FacilityTypeConstant.AMUSEMENT_TYPE){
            QueryWrapper<AmusementFacility> queryWrapper3 = Wrappers.<AmusementFacility>query().eq("id", facilityId);
            AmusementFacility existingAmusement = amusementFacilityMapper.selectOne(queryWrapper3);
            if (existingAmusement == null) {
                throw new BusinessException(ErrorCode.PARAMS_ERROR, "设施不存在");
            }
        }else if(facilityType == FacilityTypeConstant.RESTAURANT_TYPE){
            QueryWrapper<RestaurantFacility> queryWrapper3 = Wrappers.<RestaurantFacility>query().eq("id", facilityId);
            RestaurantFacility existingRestaurant = restaurantFacilityMapper.selectOne(queryWrapper3);
            if (existingRestaurant == null) {
                throw new BusinessException(ErrorCode.PARAMS_ERROR, "设施不存在");
            }
        }else if(facilityType == FacilityTypeConstant.BASE_TYPE){
            QueryWrapper<BaseFacility> queryWrapper3 = Wrappers.<BaseFacility>query().eq("id", facilityId);
            BaseFacility existingBase = baseFacilityMapper.selectOne(queryWrapper3);
            if (existingBase == null) {
                throw new BusinessException(ErrorCode.PARAMS_ERROR, "设施不存在");
            }
        }


        // 不无异常则正常插入
        Visit visit = new Visit();
        visit.setUserId(userId);
        visit.setFacilityId(facilityId);
        visit.setFacilityType(facilityType);

        this.baseMapper.insert(visit);

        return this.baseMapper.selectOne(Wrappers.<Visit>query().eq("id", visit.getId()));
    }

    @Override
    public Boolean deleteVisit(VisitAndSubscribeDeleteRequest visitAndSubscribeDeleteRequest) {
        Long id = visitAndSubscribeDeleteRequest.getId();

        // 设置is_deleted字段为1
        UpdateWrapper<Visit> updateWrapper = new UpdateWrapper<>();
        updateWrapper.eq("id", id).set("is_deleted", 1);

        // 执行更新操作
        int result = this.baseMapper.update(null, updateWrapper);

        // 判断是否更新成功
        return result > 0;

    }
}




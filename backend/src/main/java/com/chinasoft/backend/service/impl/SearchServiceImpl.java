package com.chinasoft.backend.service.impl;

import com.baomidou.mybatisplus.core.conditions.query.QueryWrapper;
import com.chinasoft.backend.mapper.AmusementFacilityMapper;
import com.chinasoft.backend.mapper.BaseFacilityMapper;
import com.chinasoft.backend.mapper.RestaurantFacilityMapper;
import com.chinasoft.backend.model.entity.facility.AmusementFacility;
import com.chinasoft.backend.model.entity.facility.BaseFacility;
import com.chinasoft.backend.model.entity.facility.RestaurantFacility;
import com.chinasoft.backend.model.request.facility.AmusementFilterRequest;
import com.chinasoft.backend.model.request.facility.BaseFilterRequest;
import com.chinasoft.backend.model.request.facility.RestaurantFilterRequest;
import com.chinasoft.backend.service.SearchService;
import com.chinasoft.backend.service.visitsubscribe.VisitAndSubscribeService;
import org.springframework.beans.BeanUtils;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.List;

/**
 * @author 姜堂蕴之
 * @description 针对搜索的数据库操作Service实现
 * @createDate 2024-04-05 17:00:00
 */
@Service
public class SearchServiceImpl implements SearchService {

    @Autowired
    private AmusementFacilityMapper amusementFacilityMapper;

    @Autowired
    private RestaurantFacilityMapper restaurantFacilityMapper;

    @Autowired
    private BaseFacilityMapper baseFacilityMapper;

    @Autowired
    private VisitAndSubscribeService visitAndSubscribeService;

    @Override
    public List<Object> search(String keyword) {
        List<Object> resultList = new ArrayList<>();

        // 如果关键字为空，则分别查询每个表的所有数据
        if (keyword == null || keyword.isEmpty()) {
            List<AmusementFacility> amusementFacilities = amusementFacilityMapper.selectList(null);
            for (AmusementFacility amusementFacility : amusementFacilities) {
                AmusementFilterRequest amusementFilterRequest = new AmusementFilterRequest();
                BeanUtils.copyProperties(amusementFacility, amusementFilterRequest);
                resultList.addAll(visitAndSubscribeService.getAmusementVisitSubscribe(amusementFilterRequest));
            }
            List<RestaurantFacility> restaurantFacilities = restaurantFacilityMapper.selectList(null);
            for (RestaurantFacility restaurantFacility : restaurantFacilities) {
                RestaurantFilterRequest restaurantFilterRequest = new RestaurantFilterRequest();
                BeanUtils.copyProperties(restaurantFacility, restaurantFilterRequest);
                resultList.addAll(visitAndSubscribeService.getRestaurantVisitSubscribe(restaurantFilterRequest));
            }
            List<BaseFacility> baseFacilities = baseFacilityMapper.selectList(null);
            for (BaseFacility baseFacility : baseFacilities) {
                BaseFilterRequest baseFilterRequest = new BaseFilterRequest();
                BeanUtils.copyProperties(baseFacility, baseFilterRequest);
                resultList.addAll(visitAndSubscribeService.getBaseVisitSubscribe(baseFilterRequest));
            }
        } else {
            // 创建QueryWrapper对象用于构建查询条件
            QueryWrapper<AmusementFacility> amusementQueryWrapper = new QueryWrapper<>();
            QueryWrapper<RestaurantFacility> restaurantQueryWrapper = new QueryWrapper<>();
            QueryWrapper<BaseFacility> baseQueryWrapper = new QueryWrapper<>();

            // 对关键字进行模糊查询
            amusementQueryWrapper.like("name", keyword).or().like("type", keyword);
            restaurantQueryWrapper.like("name", keyword).or().like("type", keyword);
            baseQueryWrapper.like("name", keyword);

            // 执行查询并将结果添加到结果列表中
            List<AmusementFacility> amusementFacilities = amusementFacilityMapper.selectList(amusementQueryWrapper);
            for (AmusementFacility amusementFacility : amusementFacilities) {
                AmusementFilterRequest amusementFilterRequest = new AmusementFilterRequest();
                BeanUtils.copyProperties(amusementFacility, amusementFilterRequest);
                resultList.addAll(visitAndSubscribeService.getAmusementVisitSubscribe(amusementFilterRequest));
            }

            List<RestaurantFacility> restaurantFacilities = restaurantFacilityMapper.selectList(restaurantQueryWrapper);
            for (RestaurantFacility restaurantFacility : restaurantFacilities) {
                RestaurantFilterRequest restaurantFilterRequest = new RestaurantFilterRequest();
                BeanUtils.copyProperties(restaurantFacility, restaurantFilterRequest);
                resultList.addAll(visitAndSubscribeService.getRestaurantVisitSubscribe(restaurantFilterRequest));
            }

            List<BaseFacility> baseFacilities = baseFacilityMapper.selectList(baseQueryWrapper);
            for (BaseFacility baseFacility : baseFacilities) {
                BaseFilterRequest baseFilterRequest = new BaseFilterRequest();
                BeanUtils.copyProperties(baseFacility, baseFilterRequest);
                resultList.addAll(visitAndSubscribeService.getBaseVisitSubscribe(baseFilterRequest));
            }

        }

        return resultList;
    }
}





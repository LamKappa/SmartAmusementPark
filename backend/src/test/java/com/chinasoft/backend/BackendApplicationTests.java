package com.chinasoft.backend;

import com.chinasoft.backend.model.entity.AmusementFacility;
import com.chinasoft.backend.service.AmusementFacilityService;
import com.chinasoft.backend.service.UserService;
import org.junit.jupiter.api.Test;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.test.context.SpringBootTest;

import java.util.List;

@SpringBootTest
class BackendApplicationTests {

    @Autowired
    UserService userService;

    @Autowired
    AmusementFacilityService amusementFacilityService;

    @Test
    void contextLoads() {

    }

    @Test
    void testUser() {
        System.out.println(userService.list());
    }

    @Test
    void testAmusementFacility() {
        List<AmusementFacility> list = amusementFacilityService.list();
        System.out.println(list);

    }

}

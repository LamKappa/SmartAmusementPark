package com.chinasoft.backend.task;

import com.chinasoft.backend.service.MqttService;
import lombok.extern.slf4j.Slf4j;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import java.util.Date;

@Slf4j
@Component
public class HandleIoTDataTask {

    @Autowired
    MqttService mqttService;

    @Scheduled(cron = "0 0/5 * * * ? ")
    // @Scheduled(cron = "0/10 * * * * ? ")
    public void handleIoTDataTask() {
        log.info("定时任务开始执行：{}", new Date());

        mqttService.handleIoTData();


    }

    @Scheduled(cron = "0 34 14 * * ?")
    public void handleTotalHeadTask(){
        log.info("定时任务2开始执行：{}", new Date());
        mqttService.handleTotalHeadCount();
    }

    @Scheduled(cron = "0 34 14 * * ?")
    public void handleFacilityHeadTask(){
        log.info("定时任务3开始执行：{}", new Date());
        mqttService.handleFacilityHeadCount();
    }


}

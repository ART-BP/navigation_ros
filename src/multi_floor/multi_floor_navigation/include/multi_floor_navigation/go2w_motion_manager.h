#ifndef GO2W_MOTION_MANAGER_H
#define GO2W_MOTION_MANAGER_H

#include <memory>
#include <string>
#include <mutex>

#include <ros/ros.h>

#include <unitree/robot/channel/channel_factory.hpp>
#include <unitree/robot/go2/sport/sport_client.hpp>

namespace multi_floor_navigation
{

class Go2WMotionManager
{
public:
    enum class MotionMode
    {
        NORMAL  = 0,   // 正常模式
        TERRAIN = 1,   // 地形模式
        CLIMB   = 2    // 攀爬模式
    };

public:
    Go2WMotionManager();

    /**
     * @brief 初始化 Unitree SDK
     * @param network_interface 机器人通信网卡，例如 eth0
     */
    bool init(const std::string& network_interface);

    /**
     * @brief 切换运动模式
     */
    bool setMode(MotionMode mode);

    bool setNormalMode();
    bool setTerrainMode();
    bool setClimbMode();

    /**
     * @brief 停止运动
     */
    bool stop();

    /**
     * @brief 获取当前记录的模式
     *
     * 注意：这里是本类最后一次成功切换的模式，
     * 不是从机器人实时读取的模式。
     */
    MotionMode getCurrentMode() const;

    bool isInitialized() const;

private:
    const char* modeToString(MotionMode mode) const;

private:
    // SportClient 必须在 ChannelFactory::Init() 之后构造。
    std::unique_ptr<unitree::robot::go2::SportClient> sport_client_;

    bool initialized_;

    // -1 表示尚未通过本类完成过模式切换
    int current_mode_;

    mutable std::mutex mutex_;
};

}  // namespace multi_floor_navigation

#endif

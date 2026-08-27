#include "multi_floor_navigation/go2w_motion_manager.h"

#include <exception>

namespace multi_floor_navigation
{

Go2WMotionManager::Go2WMotionManager()
    : sport_client_(nullptr),
      initialized_(false),
      current_mode_(-1)
{
}

bool Go2WMotionManager::init(const std::string& network_interface)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (initialized_)
    {
        ROS_WARN("Go2WMotionManager already initialized.");
        return true;
    }

    ROS_INFO(
        "Initializing Go2W MotionManager, interface: %s",
        network_interface.c_str()
    );

    try
    {
        // 初始化 Unitree DDS
        unitree::robot::ChannelFactory::Instance()->Init(
            0,
            network_interface
        );
        ROS_INFO("Unitree ChannelFactory initialized.");

        // SportClient 的构造依赖已经初始化的 ChannelFactory。
        sport_client_.reset(new unitree::robot::go2::SportClient());
        ROS_INFO("Unitree SportClient constructed.");
        sport_client_->SetTimeout(10.0f);
        sport_client_->Init();
        ROS_INFO("Unitree SportClient initialized.");
    }
    catch (const std::exception& error)
    {
        sport_client_.reset();
        ROS_ERROR("Failed to initialize Go2W MotionManager: %s", error.what());
        return false;
    }

    initialized_ = true;

    ROS_INFO("Go2W MotionManager initialized.");

    return true;
}

bool Go2WMotionManager::setMode(MotionMode mode)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_ || !sport_client_)
    {
        ROS_ERROR("Go2WMotionManager is not initialized.");
        return false;
    }

    const int target_mode = static_cast<int>(mode);

    // 已经处于该模式，不重复发送
    if (current_mode_ == target_mode)
    {
        ROS_DEBUG(
            "Go2W already in %s mode.",
            modeToString(mode)
        );

        return true;
    }

    ROS_INFO(
        "Switching Go2W motion mode: %s",
        modeToString(mode)
    );

    // 切模式前先停止
    int ret = sport_client_->StopMove();

    if (ret != 0)
    {
        ROS_WARN(
            "StopMove failed before mode switch, ret = %d",
            ret
        );
    }

    // 给机器人一点停止/模式切换缓冲时间
    ros::WallDuration(0.3).sleep();

    ret = sport_client_->SwitchGait(target_mode);

    if (ret != 0)
    {
        ROS_ERROR(
            "Failed to switch Go2W mode to %s, ret = %d",
            modeToString(mode),
            ret
        );

        return false;
    }

    current_mode_ = target_mode;

    ROS_INFO(
        "Go2W motion mode switched to %s.",
        modeToString(mode)
    );

    return true;
}

bool Go2WMotionManager::setNormalMode()
{
    return setMode(MotionMode::NORMAL);
}

bool Go2WMotionManager::setTerrainMode()
{
    return setMode(MotionMode::TERRAIN);
}

bool Go2WMotionManager::setClimbMode()
{
    return setMode(MotionMode::CLIMB);
}

bool Go2WMotionManager::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!initialized_ || !sport_client_)
    {
        ROS_ERROR("Go2WMotionManager is not initialized.");
        return false;
    }

    int ret = sport_client_->StopMove();

    if (ret != 0)
    {
        ROS_ERROR(
            "Go2W StopMove failed, ret = %d",
            ret
        );

        return false;
    }

    return true;
}

Go2WMotionManager::MotionMode
Go2WMotionManager::getCurrentMode() const
{
    std::lock_guard<std::mutex> lock(mutex_);

    // 如果还没有记录，默认返回 NORMAL
    // 但注意这不代表机器人实际一定处于 NORMAL
    if (current_mode_ < 0)
    {
        return MotionMode::NORMAL;
    }

    return static_cast<MotionMode>(current_mode_);
}

bool Go2WMotionManager::isInitialized() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return initialized_;
}

const char*
Go2WMotionManager::modeToString(MotionMode mode) const
{
    switch (mode)
    {
        case MotionMode::NORMAL:
            return "NORMAL";

        case MotionMode::TERRAIN:
            return "TERRAIN";

        case MotionMode::CLIMB:
            return "CLIMB";

        default:
            return "UNKNOWN";
    }
}

}  // namespace multi_floor_navigation

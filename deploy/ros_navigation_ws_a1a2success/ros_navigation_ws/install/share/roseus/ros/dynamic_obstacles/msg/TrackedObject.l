;; Auto-generated. Do not edit!


(when (boundp 'dynamic_obstacles::TrackedObject)
  (if (not (find-package "DYNAMIC_OBSTACLES"))
    (make-package "DYNAMIC_OBSTACLES"))
  (shadow 'TrackedObject (find-package "DYNAMIC_OBSTACLES")))
(unless (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT")
  (make-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))

(in-package "ROS")
;;//! \htmlinclude TrackedObject.msg.html
(if (not (find-package "GEOMETRY_MSGS"))
  (ros::roseus-add-msgs "geometry_msgs"))
(if (not (find-package "UUID_MSGS"))
  (ros::roseus-add-msgs "uuid_msgs"))


(intern "*TRACK_UNKNOWN*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*TRACK_UNKNOWN* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*TRACK_UNKNOWN* 0)
(intern "*TRACK_TENTATIVE*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*TRACK_TENTATIVE* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*TRACK_TENTATIVE* 1)
(intern "*TRACK_CONFIRMED*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*TRACK_CONFIRMED* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*TRACK_CONFIRMED* 2)
(intern "*TRACK_COASTED*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*TRACK_COASTED* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*TRACK_COASTED* 3)
(intern "*MOTION_UNKNOWN*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*MOTION_UNKNOWN* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*MOTION_UNKNOWN* 0)
(intern "*MOTION_MOVING*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*MOTION_MOVING* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*MOTION_MOVING* 1)
(intern "*MOTION_STATIONARY*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*MOTION_STATIONARY* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*MOTION_STATIONARY* 2)
(intern "*ORIENTATION_UNAVAILABLE*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*ORIENTATION_UNAVAILABLE* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*ORIENTATION_UNAVAILABLE* 0)
(intern "*ORIENTATION_SIGN_UNKNOWN*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*ORIENTATION_SIGN_UNKNOWN* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*ORIENTATION_SIGN_UNKNOWN* 1)
(intern "*ORIENTATION_AVAILABLE*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*ORIENTATION_AVAILABLE* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*ORIENTATION_AVAILABLE* 2)
(intern "*SHAPE_UNKNOWN*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*SHAPE_UNKNOWN* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*SHAPE_UNKNOWN* 0)
(intern "*SHAPE_BOUNDING_BOX*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*SHAPE_BOUNDING_BOX* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*SHAPE_BOUNDING_BOX* 1)
(intern "*SHAPE_CYLINDER*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*SHAPE_CYLINDER* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*SHAPE_CYLINDER* 2)
(intern "*SHAPE_POLYGON*" (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(shadow '*SHAPE_POLYGON* (find-package "DYNAMIC_OBSTACLES::TRACKEDOBJECT"))
(defconstant dynamic_obstacles::TrackedObject::*SHAPE_POLYGON* 3)

(defun dynamic_obstacles::TrackedObject-to-symbol (const)
  (cond
        ((= const 0) 'dynamic_obstacles::TrackedObject::*TRACK_UNKNOWN*)
        ((= const 1) 'dynamic_obstacles::TrackedObject::*TRACK_TENTATIVE*)
        ((= const 2) 'dynamic_obstacles::TrackedObject::*TRACK_CONFIRMED*)
        ((= const 3) 'dynamic_obstacles::TrackedObject::*TRACK_COASTED*)
        ((= const 0) 'dynamic_obstacles::TrackedObject::*MOTION_UNKNOWN*)
        ((= const 1) 'dynamic_obstacles::TrackedObject::*MOTION_MOVING*)
        ((= const 2) 'dynamic_obstacles::TrackedObject::*MOTION_STATIONARY*)
        ((= const 0) 'dynamic_obstacles::TrackedObject::*ORIENTATION_UNAVAILABLE*)
        ((= const 1) 'dynamic_obstacles::TrackedObject::*ORIENTATION_SIGN_UNKNOWN*)
        ((= const 2) 'dynamic_obstacles::TrackedObject::*ORIENTATION_AVAILABLE*)
        ((= const 0) 'dynamic_obstacles::TrackedObject::*SHAPE_UNKNOWN*)
        ((= const 1) 'dynamic_obstacles::TrackedObject::*SHAPE_BOUNDING_BOX*)
        ((= const 2) 'dynamic_obstacles::TrackedObject::*SHAPE_CYLINDER*)
        ((= const 3) 'dynamic_obstacles::TrackedObject::*SHAPE_POLYGON*)
        (t nil)))

(defclass dynamic_obstacles::TrackedObject
  :super ros::object
  :slots (_object_id _classification _track_state _tracking_time _time_since_update _pose _has_pose_covariance _twist _has_linear_velocity _has_angular_velocity _has_linear_covariance _has_angular_covariance _acceleration _has_acceleration _has_acceleration_covariance _motion_state _orientation_availability _shape_type _dimensions _footprint ))

(defmethod dynamic_obstacles::TrackedObject
  (:init
   (&key
    ((:object_id __object_id) (instance uuid_msgs::UniqueID :init))
    ((:classification __classification) (instance dynamic_obstacles::ObjectClassification :init))
    ((:track_state __track_state) 0)
    ((:tracking_time __tracking_time) 0.0)
    ((:time_since_update __time_since_update) 0.0)
    ((:pose __pose) (instance geometry_msgs::PoseWithCovariance :init))
    ((:has_pose_covariance __has_pose_covariance) nil)
    ((:twist __twist) (instance geometry_msgs::TwistWithCovariance :init))
    ((:has_linear_velocity __has_linear_velocity) nil)
    ((:has_angular_velocity __has_angular_velocity) nil)
    ((:has_linear_covariance __has_linear_covariance) nil)
    ((:has_angular_covariance __has_angular_covariance) nil)
    ((:acceleration __acceleration) (instance geometry_msgs::AccelWithCovariance :init))
    ((:has_acceleration __has_acceleration) nil)
    ((:has_acceleration_covariance __has_acceleration_covariance) nil)
    ((:motion_state __motion_state) 0)
    ((:orientation_availability __orientation_availability) 0)
    ((:shape_type __shape_type) 0)
    ((:dimensions __dimensions) (instance geometry_msgs::Vector3 :init))
    ((:footprint __footprint) (instance geometry_msgs::Polygon :init))
    )
   (send-super :init)
   (setq _object_id __object_id)
   (setq _classification __classification)
   (setq _track_state (round __track_state))
   (setq _tracking_time (float __tracking_time))
   (setq _time_since_update (float __time_since_update))
   (setq _pose __pose)
   (setq _has_pose_covariance __has_pose_covariance)
   (setq _twist __twist)
   (setq _has_linear_velocity __has_linear_velocity)
   (setq _has_angular_velocity __has_angular_velocity)
   (setq _has_linear_covariance __has_linear_covariance)
   (setq _has_angular_covariance __has_angular_covariance)
   (setq _acceleration __acceleration)
   (setq _has_acceleration __has_acceleration)
   (setq _has_acceleration_covariance __has_acceleration_covariance)
   (setq _motion_state (round __motion_state))
   (setq _orientation_availability (round __orientation_availability))
   (setq _shape_type (round __shape_type))
   (setq _dimensions __dimensions)
   (setq _footprint __footprint)
   self)
  (:object_id
   (&rest __object_id)
   (if (keywordp (car __object_id))
       (send* _object_id __object_id)
     (progn
       (if __object_id (setq _object_id (car __object_id)))
       _object_id)))
  (:classification
   (&rest __classification)
   (if (keywordp (car __classification))
       (send* _classification __classification)
     (progn
       (if __classification (setq _classification (car __classification)))
       _classification)))
  (:track_state
   (&optional __track_state)
   (if __track_state (setq _track_state __track_state)) _track_state)
  (:tracking_time
   (&optional __tracking_time)
   (if __tracking_time (setq _tracking_time __tracking_time)) _tracking_time)
  (:time_since_update
   (&optional __time_since_update)
   (if __time_since_update (setq _time_since_update __time_since_update)) _time_since_update)
  (:pose
   (&rest __pose)
   (if (keywordp (car __pose))
       (send* _pose __pose)
     (progn
       (if __pose (setq _pose (car __pose)))
       _pose)))
  (:has_pose_covariance
   (&optional (__has_pose_covariance :null))
   (if (not (eq __has_pose_covariance :null)) (setq _has_pose_covariance __has_pose_covariance)) _has_pose_covariance)
  (:twist
   (&rest __twist)
   (if (keywordp (car __twist))
       (send* _twist __twist)
     (progn
       (if __twist (setq _twist (car __twist)))
       _twist)))
  (:has_linear_velocity
   (&optional (__has_linear_velocity :null))
   (if (not (eq __has_linear_velocity :null)) (setq _has_linear_velocity __has_linear_velocity)) _has_linear_velocity)
  (:has_angular_velocity
   (&optional (__has_angular_velocity :null))
   (if (not (eq __has_angular_velocity :null)) (setq _has_angular_velocity __has_angular_velocity)) _has_angular_velocity)
  (:has_linear_covariance
   (&optional (__has_linear_covariance :null))
   (if (not (eq __has_linear_covariance :null)) (setq _has_linear_covariance __has_linear_covariance)) _has_linear_covariance)
  (:has_angular_covariance
   (&optional (__has_angular_covariance :null))
   (if (not (eq __has_angular_covariance :null)) (setq _has_angular_covariance __has_angular_covariance)) _has_angular_covariance)
  (:acceleration
   (&rest __acceleration)
   (if (keywordp (car __acceleration))
       (send* _acceleration __acceleration)
     (progn
       (if __acceleration (setq _acceleration (car __acceleration)))
       _acceleration)))
  (:has_acceleration
   (&optional (__has_acceleration :null))
   (if (not (eq __has_acceleration :null)) (setq _has_acceleration __has_acceleration)) _has_acceleration)
  (:has_acceleration_covariance
   (&optional (__has_acceleration_covariance :null))
   (if (not (eq __has_acceleration_covariance :null)) (setq _has_acceleration_covariance __has_acceleration_covariance)) _has_acceleration_covariance)
  (:motion_state
   (&optional __motion_state)
   (if __motion_state (setq _motion_state __motion_state)) _motion_state)
  (:orientation_availability
   (&optional __orientation_availability)
   (if __orientation_availability (setq _orientation_availability __orientation_availability)) _orientation_availability)
  (:shape_type
   (&optional __shape_type)
   (if __shape_type (setq _shape_type __shape_type)) _shape_type)
  (:dimensions
   (&rest __dimensions)
   (if (keywordp (car __dimensions))
       (send* _dimensions __dimensions)
     (progn
       (if __dimensions (setq _dimensions (car __dimensions)))
       _dimensions)))
  (:footprint
   (&rest __footprint)
   (if (keywordp (car __footprint))
       (send* _footprint __footprint)
     (progn
       (if __footprint (setq _footprint (car __footprint)))
       _footprint)))
  (:serialization-length
   ()
   (+
    ;; uuid_msgs/UniqueID _object_id
    (send _object_id :serialization-length)
    ;; dynamic_obstacles/ObjectClassification _classification
    (send _classification :serialization-length)
    ;; uint8 _track_state
    1
    ;; float32 _tracking_time
    4
    ;; float32 _time_since_update
    4
    ;; geometry_msgs/PoseWithCovariance _pose
    (send _pose :serialization-length)
    ;; bool _has_pose_covariance
    1
    ;; geometry_msgs/TwistWithCovariance _twist
    (send _twist :serialization-length)
    ;; bool _has_linear_velocity
    1
    ;; bool _has_angular_velocity
    1
    ;; bool _has_linear_covariance
    1
    ;; bool _has_angular_covariance
    1
    ;; geometry_msgs/AccelWithCovariance _acceleration
    (send _acceleration :serialization-length)
    ;; bool _has_acceleration
    1
    ;; bool _has_acceleration_covariance
    1
    ;; uint8 _motion_state
    1
    ;; uint8 _orientation_availability
    1
    ;; uint8 _shape_type
    1
    ;; geometry_msgs/Vector3 _dimensions
    (send _dimensions :serialization-length)
    ;; geometry_msgs/Polygon _footprint
    (send _footprint :serialization-length)
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uuid_msgs/UniqueID _object_id
       (send _object_id :serialize s)
     ;; dynamic_obstacles/ObjectClassification _classification
       (send _classification :serialize s)
     ;; uint8 _track_state
       (write-byte _track_state s)
     ;; float32 _tracking_time
       (sys::poke _tracking_time (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; float32 _time_since_update
       (sys::poke _time_since_update (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;; geometry_msgs/PoseWithCovariance _pose
       (send _pose :serialize s)
     ;; bool _has_pose_covariance
       (if _has_pose_covariance (write-byte -1 s) (write-byte 0 s))
     ;; geometry_msgs/TwistWithCovariance _twist
       (send _twist :serialize s)
     ;; bool _has_linear_velocity
       (if _has_linear_velocity (write-byte -1 s) (write-byte 0 s))
     ;; bool _has_angular_velocity
       (if _has_angular_velocity (write-byte -1 s) (write-byte 0 s))
     ;; bool _has_linear_covariance
       (if _has_linear_covariance (write-byte -1 s) (write-byte 0 s))
     ;; bool _has_angular_covariance
       (if _has_angular_covariance (write-byte -1 s) (write-byte 0 s))
     ;; geometry_msgs/AccelWithCovariance _acceleration
       (send _acceleration :serialize s)
     ;; bool _has_acceleration
       (if _has_acceleration (write-byte -1 s) (write-byte 0 s))
     ;; bool _has_acceleration_covariance
       (if _has_acceleration_covariance (write-byte -1 s) (write-byte 0 s))
     ;; uint8 _motion_state
       (write-byte _motion_state s)
     ;; uint8 _orientation_availability
       (write-byte _orientation_availability s)
     ;; uint8 _shape_type
       (write-byte _shape_type s)
     ;; geometry_msgs/Vector3 _dimensions
       (send _dimensions :serialize s)
     ;; geometry_msgs/Polygon _footprint
       (send _footprint :serialize s)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uuid_msgs/UniqueID _object_id
     (send _object_id :deserialize buf ptr-) (incf ptr- (send _object_id :serialization-length))
   ;; dynamic_obstacles/ObjectClassification _classification
     (send _classification :deserialize buf ptr-) (incf ptr- (send _classification :serialization-length))
   ;; uint8 _track_state
     (setq _track_state (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float32 _tracking_time
     (setq _tracking_time (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; float32 _time_since_update
     (setq _time_since_update (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;; geometry_msgs/PoseWithCovariance _pose
     (send _pose :deserialize buf ptr-) (incf ptr- (send _pose :serialization-length))
   ;; bool _has_pose_covariance
     (setq _has_pose_covariance (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; geometry_msgs/TwistWithCovariance _twist
     (send _twist :deserialize buf ptr-) (incf ptr- (send _twist :serialization-length))
   ;; bool _has_linear_velocity
     (setq _has_linear_velocity (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _has_angular_velocity
     (setq _has_angular_velocity (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _has_linear_covariance
     (setq _has_linear_covariance (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _has_angular_covariance
     (setq _has_angular_covariance (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; geometry_msgs/AccelWithCovariance _acceleration
     (send _acceleration :deserialize buf ptr-) (incf ptr- (send _acceleration :serialization-length))
   ;; bool _has_acceleration
     (setq _has_acceleration (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; bool _has_acceleration_covariance
     (setq _has_acceleration_covariance (not (= 0 (sys::peek buf ptr- :char)))) (incf ptr- 1)
   ;; uint8 _motion_state
     (setq _motion_state (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _orientation_availability
     (setq _orientation_availability (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; uint8 _shape_type
     (setq _shape_type (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; geometry_msgs/Vector3 _dimensions
     (send _dimensions :deserialize buf ptr-) (incf ptr- (send _dimensions :serialization-length))
   ;; geometry_msgs/Polygon _footprint
     (send _footprint :deserialize buf ptr-) (incf ptr- (send _footprint :serialization-length))
   ;;
   self)
  )

(setf (get dynamic_obstacles::TrackedObject :md5sum-) "ab8923e41e784db52548f8423530ee07")
(setf (get dynamic_obstacles::TrackedObject :datatype-) "dynamic_obstacles/TrackedObject")
(setf (get dynamic_obstacles::TrackedObject :definition-)
      "# TrackedObject.msg

# ============================================================
# 坐标系与时间规范
#
# 1. 本消息自身不包含 Header，所有目标统一使用
#    TrackedObjectArray.header 中的时间戳和参考坐标系。
#
# 2. 本项目规定：
#    TrackedObjectArray.header.frame_id 必须为 \"map\"或\"odom\"。
#
# 3. pose、twist 和 acceleration 均对应
#    TrackedObjectArray.header.stamp 所表示的统一状态时刻。
#
# 4. 以下物理量均在 指定参考的 坐标系上表达：
#    - pose.position 底面中心点
#    - pose.orientation 表示目标局部坐标系到 指定参考的 坐标系的旋转
#    - twist.linear
#    - twist.angular
#    - acceleration.linear
#    - acceleration.angular
#
# 5. twist 和 acceleration 表示经过机器人自身运动补偿后的、
#    目标相对于 指定参考的 坐标系的估计运动状态，
#    不是相对机器人或相对传感器的运动状态。
#
# 6. 目标局部坐标系基础约定：
#   - 原点位于目标底面几何中心，右手坐标系
#   - X 轴沿目标长度方向，但其具体指向受以下 orientation_availability 约束
#   - Y 轴沿目标宽度方向
#   - Z 轴沿目标高度方向，指向上方
#
# 7. dimensions 和 footprint 在目标局部坐标系中定义。
#    footprint 不是 指定参考的坐标系下 的坐标；应结合 pose 转换到 指定参考的坐标系下。
#
# 8. 单位遵循 ROS 约定：
#    - 位置、尺寸：m
#    - 线速度：m/s
#    - 角速度：rad/s
#    - 线加速度：m/s^2
#    - 角加速度：rad/s^2
# ============================================================


# 1. 基础属性
uuid_msgs/UniqueID object_id           # 全局唯一的追踪 ID，轨迹删除后，不应立即复用相同 ID

ObjectClassification classification    # 当前目标置信度最高的最终语义类别

# ============================================================
# 2. 跟踪状态
# ============================================================

# TRACK_UNKNOWN：未知
# TENTATIVE：单帧新增障碍物/持续多帧尚未满足稳定确认条件，防止目标漏避
# CONFIRMED：稳定轨迹，当前有观测更新
# COASTED：当前无观测，由运动模型预测外推

uint8 TRACK_UNKNOWN=0
uint8 TRACK_TENTATIVE=1
# 当前版本的感知发布端仅输出 TRACK_CONFIRMED。
uint8 TRACK_CONFIRMED=2
uint8 TRACK_COASTED=3

uint8 track_state

# 自轨迹创建以来的持续时间，单位 s
float32 tracking_time

# 距最后一次真实观测更新的时间，单位 s
float32 time_since_update

# ==========================================
# 3. 运动学状态 (Kinematics)
# 包含了位姿、速度、加速度，以及它们对应的 6x6 协方差矩阵(表示不确定性)
# ==========================================

# 位姿 (x, y, z 及 四元数姿态) + 协方差
geometry_msgs/PoseWithCovariance pose
bool has_pose_covariance

# 速度 (线速度 v_x, v_y, v_z 及 角速度) + 协方差
geometry_msgs/TwistWithCovariance twist
bool has_linear_velocity 
bool has_angular_velocity 
bool has_linear_covariance
bool has_angular_covariance

# 加速度 (线加速度 a_x, a_y, a_z 及 角加速度) + 协方差
geometry_msgs/AccelWithCovariance acceleration
bool has_acceleration
bool has_acceleration_covariance

# 运动状态

uint8 MOTION_UNKNOWN=0
uint8 MOTION_MOVING=1
uint8 MOTION_STATIONARY=2

uint8 motion_state

# ============================================================
# 4. 朝向有效性
# ============================================================

# 无法获得可靠的几何方向或语义朝向。
# pose.orientation 必须是合法且归一化的四元数；
# 没有任何方向估计时推荐填单位四元数。
#
# 当 shape_type=SHAPE_BOUNDING_BOX 时，应输出相对于
# header.frame_id 的轴对齐保守包围盒，或改用不依赖朝向的
# SHAPE_CYLINDER。
uint8 ORIENTATION_UNAVAILABLE=0

# 只能确定目标长轴，正反方向存在 pi 弧度二义性。
# 局部 X 轴平行于目标长轴，但正方向可能与真实前方相反。
uint8 ORIENTATION_SIGN_UNKNOWN=1

# 能够完整区分目标正面和背面，局部 +X 指向前方，+Y 指向左侧，+Z 指向上方。
uint8 ORIENTATION_AVAILABLE=2

uint8 orientation_availability



# ==========================================
# 5. 几何形状 (Shape)
# 局部 x 轴为 length 方向，局部 y 轴为 width 方向，局部 z 轴为 height 方向
# polygon 点序为逆时针，首尾点闭合
# ==========================================
# 形状未知或当前无法提供可靠几何形状 
uint8 SHAPE_UNKNOWN=0
uint8 SHAPE_BOUNDING_BOX=1
uint8 SHAPE_CYLINDER=2
uint8 SHAPE_POLYGON=3

uint8 shape_type                       # 形状类型

#BOUNDING_BOX：x=length, y=width, z=height，且 footprint 应为空
#CYLINDER：x=diameter, y=diameter, z=height，且 footprint 应为空
#POLYGON：x=0, y=0, z=height，形状由 footprint 决定
geometry_msgs/Vector3 dimensions

# 仅在 shape_type=SHAPE_POLYGON 时使用；
# 点位于目标局部坐标系中
geometry_msgs/Polygon footprint

================================================================================
MSG: uuid_msgs/UniqueID
# A universally unique identifier (UUID).
#
#  http://en.wikipedia.org/wiki/Universally_unique_identifier
#  http://tools.ietf.org/html/rfc4122.html

uint8[16] uuid

================================================================================
MSG: dynamic_obstacles/ObjectClassification
# ObjectClassification.msg
#
# 类别编号按类别族分段预留。
# 已发布的类别编号不得改变，新增类别应使用对应预留区间。
#
# 0       ：未知类别
# 10~29   ：道路车辆
# 30~49   ：两轮或三轮车辆
# 50~69   ：行人
# 70~89   ：动物
# 90~109  ：交通设施及危险物
# 110~253 ：后续类别预留
# 254     ：其他类别
# 255     ：非法或不支持的类别


# 无法可靠识别类别，但目标仍可能真实存在，需要正常避障
uint8 UNKNOWN=0


# ============================================================
# 道路车辆：10~29
# ============================================================

# 小型乘用车辆，如轿车、SUV、MPV
uint8 CAR=10

# 货运车辆，如轻卡、厢式货车、重型卡车
uint8 TRUCK=11

# 载客大型车辆，如公交车、大巴、中巴
uint8 BUS=12

# 被牵引的挂车或拖车部分
uint8 TRAILER=13

# 14~29 预留给其他道路车辆
# 例如：VAN、FORKLIFT、CONSTRUCTION_VEHICLE


# ============================================================
# 两轮或三轮车辆：30~49
# ============================================================

# 摩托车、电动摩托车等机动车辆
uint8 MOTORCYCLE=30

# 普通脚踏自行车
uint8 BICYCLE=31

# 电动自行车，不包括电动摩托车
uint8 ELECTRIC_BICYCLE=32

# 33~49 预留
# 例如：TRICYCLE


# ============================================================
# 人：50~69
# ============================================================

# 普通人(无法进一步区分的人)。
uint8 PEDESTRIAN=50

#儿童
uint8 CHILD=51

#轮椅用户，需要更大的横向安全距离，且运动学模型与正常行人不同
uint8 WHEELCHAIR=52

# ============================================================
# 动物：70~89
# ============================================================
# 狗、猫等动物
uint8 ANIMAL=70


# ============================================================
# 交通设施及危险物：90~109
# ============================================================

# 交通锥、锥桶
uint8 TRAFFIC_CONE=90

# 闸机横杆
uint8 BOOM_BARRIER=91

# 门/闸机，此类障碍物的状态是可变的（开/关）
uint8 DOOR_GATE=92

# 93~109 预留
# 例如：BARRIER、BOLLARD、ROADBLOCK、DEBRIS


# ============================================================
# 特殊值
# ============================================================
# 已确认不属于当前已定义类别的其他目标。
# 若无法识别类别，应使用 UNKNOWN。
uint8 OTHER=254

# 非法、解析失败或接口不支持的类别
# 正常感知结果不应输出该值
uint8 INVALID=255


# 目标语义类别
uint8 label

# 当前类别的分类概率，范围 [0.0, 1.0]
# 不等同于目标存在概率 existence_probability
float32 probability


================================================================================
MSG: geometry_msgs/PoseWithCovariance
# This represents a pose in free space with uncertainty.

Pose pose

# Row-major representation of the 6x6 covariance matrix
# The orientation parameters use a fixed-axis representation.
# In order, the parameters are:
# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
float64[36] covariance

================================================================================
MSG: geometry_msgs/Pose
# A representation of pose in free space, composed of position and orientation. 
Point position
Quaternion orientation

================================================================================
MSG: geometry_msgs/Point
# This contains the position of a point in free space
float64 x
float64 y
float64 z

================================================================================
MSG: geometry_msgs/Quaternion
# This represents an orientation in free space in quaternion form.

float64 x
float64 y
float64 z
float64 w

================================================================================
MSG: geometry_msgs/TwistWithCovariance
# This expresses velocity in free space with uncertainty.

Twist twist

# Row-major representation of the 6x6 covariance matrix
# The orientation parameters use a fixed-axis representation.
# In order, the parameters are:
# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
float64[36] covariance

================================================================================
MSG: geometry_msgs/Twist
# This expresses velocity in free space broken into its linear and angular parts.
Vector3  linear
Vector3  angular

================================================================================
MSG: geometry_msgs/Vector3
# This represents a vector in free space. 
# It is only meant to represent a direction. Therefore, it does not
# make sense to apply a translation to it (e.g., when applying a 
# generic rigid transformation to a Vector3, tf2 will only apply the
# rotation). If you want your data to be translatable too, use the
# geometry_msgs/Point message instead.

float64 x
float64 y
float64 z
================================================================================
MSG: geometry_msgs/AccelWithCovariance
# This expresses acceleration in free space with uncertainty.

Accel accel

# Row-major representation of the 6x6 covariance matrix
# The orientation parameters use a fixed-axis representation.
# In order, the parameters are:
# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
float64[36] covariance

================================================================================
MSG: geometry_msgs/Accel
# This expresses acceleration in free space broken into its linear and angular parts.
Vector3  linear
Vector3  angular

================================================================================
MSG: geometry_msgs/Polygon
#A specification of a polygon where the first and last points are assumed to be connected
Point32[] points

================================================================================
MSG: geometry_msgs/Point32
# This contains the position of a point in free space(with 32 bits of precision).
# It is recommeded to use Point wherever possible instead of Point32.  
# 
# This recommendation is to promote interoperability.  
#
# This message is designed to take up less space when sending
# lots of points at once, as in the case of a PointCloud.  

float32 x
float32 y
float32 z
")



(provide :dynamic_obstacles/TrackedObject "ab8923e41e784db52548f8423530ee07")



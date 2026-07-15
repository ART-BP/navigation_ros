; Auto-generated. Do not edit!


(cl:in-package dynamic_obstacles-msg)


;//! \htmlinclude TrackedObject.msg.html

(cl:defclass <TrackedObject> (roslisp-msg-protocol:ros-message)
  ((object_id
    :reader object_id
    :initarg :object_id
    :type uuid_msgs-msg:UniqueID
    :initform (cl:make-instance 'uuid_msgs-msg:UniqueID))
   (classification
    :reader classification
    :initarg :classification
    :type dynamic_obstacles-msg:ObjectClassification
    :initform (cl:make-instance 'dynamic_obstacles-msg:ObjectClassification))
   (track_state
    :reader track_state
    :initarg :track_state
    :type cl:fixnum
    :initform 0)
   (tracking_time
    :reader tracking_time
    :initarg :tracking_time
    :type cl:float
    :initform 0.0)
   (time_since_update
    :reader time_since_update
    :initarg :time_since_update
    :type cl:float
    :initform 0.0)
   (pose
    :reader pose
    :initarg :pose
    :type geometry_msgs-msg:PoseWithCovariance
    :initform (cl:make-instance 'geometry_msgs-msg:PoseWithCovariance))
   (has_pose_covariance
    :reader has_pose_covariance
    :initarg :has_pose_covariance
    :type cl:boolean
    :initform cl:nil)
   (twist
    :reader twist
    :initarg :twist
    :type geometry_msgs-msg:TwistWithCovariance
    :initform (cl:make-instance 'geometry_msgs-msg:TwistWithCovariance))
   (has_linear_velocity
    :reader has_linear_velocity
    :initarg :has_linear_velocity
    :type cl:boolean
    :initform cl:nil)
   (has_angular_velocity
    :reader has_angular_velocity
    :initarg :has_angular_velocity
    :type cl:boolean
    :initform cl:nil)
   (has_linear_covariance
    :reader has_linear_covariance
    :initarg :has_linear_covariance
    :type cl:boolean
    :initform cl:nil)
   (has_angular_covariance
    :reader has_angular_covariance
    :initarg :has_angular_covariance
    :type cl:boolean
    :initform cl:nil)
   (acceleration
    :reader acceleration
    :initarg :acceleration
    :type geometry_msgs-msg:AccelWithCovariance
    :initform (cl:make-instance 'geometry_msgs-msg:AccelWithCovariance))
   (has_acceleration
    :reader has_acceleration
    :initarg :has_acceleration
    :type cl:boolean
    :initform cl:nil)
   (has_acceleration_covariance
    :reader has_acceleration_covariance
    :initarg :has_acceleration_covariance
    :type cl:boolean
    :initform cl:nil)
   (motion_state
    :reader motion_state
    :initarg :motion_state
    :type cl:fixnum
    :initform 0)
   (orientation_availability
    :reader orientation_availability
    :initarg :orientation_availability
    :type cl:fixnum
    :initform 0)
   (shape_type
    :reader shape_type
    :initarg :shape_type
    :type cl:fixnum
    :initform 0)
   (dimensions
    :reader dimensions
    :initarg :dimensions
    :type geometry_msgs-msg:Vector3
    :initform (cl:make-instance 'geometry_msgs-msg:Vector3))
   (footprint
    :reader footprint
    :initarg :footprint
    :type geometry_msgs-msg:Polygon
    :initform (cl:make-instance 'geometry_msgs-msg:Polygon)))
)

(cl:defclass TrackedObject (<TrackedObject>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <TrackedObject>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'TrackedObject)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name dynamic_obstacles-msg:<TrackedObject> is deprecated: use dynamic_obstacles-msg:TrackedObject instead.")))

(cl:ensure-generic-function 'object_id-val :lambda-list '(m))
(cl:defmethod object_id-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:object_id-val is deprecated.  Use dynamic_obstacles-msg:object_id instead.")
  (object_id m))

(cl:ensure-generic-function 'classification-val :lambda-list '(m))
(cl:defmethod classification-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:classification-val is deprecated.  Use dynamic_obstacles-msg:classification instead.")
  (classification m))

(cl:ensure-generic-function 'track_state-val :lambda-list '(m))
(cl:defmethod track_state-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:track_state-val is deprecated.  Use dynamic_obstacles-msg:track_state instead.")
  (track_state m))

(cl:ensure-generic-function 'tracking_time-val :lambda-list '(m))
(cl:defmethod tracking_time-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:tracking_time-val is deprecated.  Use dynamic_obstacles-msg:tracking_time instead.")
  (tracking_time m))

(cl:ensure-generic-function 'time_since_update-val :lambda-list '(m))
(cl:defmethod time_since_update-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:time_since_update-val is deprecated.  Use dynamic_obstacles-msg:time_since_update instead.")
  (time_since_update m))

(cl:ensure-generic-function 'pose-val :lambda-list '(m))
(cl:defmethod pose-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:pose-val is deprecated.  Use dynamic_obstacles-msg:pose instead.")
  (pose m))

(cl:ensure-generic-function 'has_pose_covariance-val :lambda-list '(m))
(cl:defmethod has_pose_covariance-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:has_pose_covariance-val is deprecated.  Use dynamic_obstacles-msg:has_pose_covariance instead.")
  (has_pose_covariance m))

(cl:ensure-generic-function 'twist-val :lambda-list '(m))
(cl:defmethod twist-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:twist-val is deprecated.  Use dynamic_obstacles-msg:twist instead.")
  (twist m))

(cl:ensure-generic-function 'has_linear_velocity-val :lambda-list '(m))
(cl:defmethod has_linear_velocity-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:has_linear_velocity-val is deprecated.  Use dynamic_obstacles-msg:has_linear_velocity instead.")
  (has_linear_velocity m))

(cl:ensure-generic-function 'has_angular_velocity-val :lambda-list '(m))
(cl:defmethod has_angular_velocity-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:has_angular_velocity-val is deprecated.  Use dynamic_obstacles-msg:has_angular_velocity instead.")
  (has_angular_velocity m))

(cl:ensure-generic-function 'has_linear_covariance-val :lambda-list '(m))
(cl:defmethod has_linear_covariance-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:has_linear_covariance-val is deprecated.  Use dynamic_obstacles-msg:has_linear_covariance instead.")
  (has_linear_covariance m))

(cl:ensure-generic-function 'has_angular_covariance-val :lambda-list '(m))
(cl:defmethod has_angular_covariance-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:has_angular_covariance-val is deprecated.  Use dynamic_obstacles-msg:has_angular_covariance instead.")
  (has_angular_covariance m))

(cl:ensure-generic-function 'acceleration-val :lambda-list '(m))
(cl:defmethod acceleration-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:acceleration-val is deprecated.  Use dynamic_obstacles-msg:acceleration instead.")
  (acceleration m))

(cl:ensure-generic-function 'has_acceleration-val :lambda-list '(m))
(cl:defmethod has_acceleration-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:has_acceleration-val is deprecated.  Use dynamic_obstacles-msg:has_acceleration instead.")
  (has_acceleration m))

(cl:ensure-generic-function 'has_acceleration_covariance-val :lambda-list '(m))
(cl:defmethod has_acceleration_covariance-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:has_acceleration_covariance-val is deprecated.  Use dynamic_obstacles-msg:has_acceleration_covariance instead.")
  (has_acceleration_covariance m))

(cl:ensure-generic-function 'motion_state-val :lambda-list '(m))
(cl:defmethod motion_state-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:motion_state-val is deprecated.  Use dynamic_obstacles-msg:motion_state instead.")
  (motion_state m))

(cl:ensure-generic-function 'orientation_availability-val :lambda-list '(m))
(cl:defmethod orientation_availability-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:orientation_availability-val is deprecated.  Use dynamic_obstacles-msg:orientation_availability instead.")
  (orientation_availability m))

(cl:ensure-generic-function 'shape_type-val :lambda-list '(m))
(cl:defmethod shape_type-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:shape_type-val is deprecated.  Use dynamic_obstacles-msg:shape_type instead.")
  (shape_type m))

(cl:ensure-generic-function 'dimensions-val :lambda-list '(m))
(cl:defmethod dimensions-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:dimensions-val is deprecated.  Use dynamic_obstacles-msg:dimensions instead.")
  (dimensions m))

(cl:ensure-generic-function 'footprint-val :lambda-list '(m))
(cl:defmethod footprint-val ((m <TrackedObject>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:footprint-val is deprecated.  Use dynamic_obstacles-msg:footprint instead.")
  (footprint m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<TrackedObject>)))
    "Constants for message type '<TrackedObject>"
  '((:TRACK_UNKNOWN . 0)
    (:TRACK_TENTATIVE . 1)
    (:TRACK_CONFIRMED . 2)
    (:TRACK_COASTED . 3)
    (:MOTION_UNKNOWN . 0)
    (:MOTION_MOVING . 1)
    (:MOTION_STATIONARY . 2)
    (:ORIENTATION_UNAVAILABLE . 0)
    (:ORIENTATION_SIGN_UNKNOWN . 1)
    (:ORIENTATION_AVAILABLE . 2)
    (:SHAPE_UNKNOWN . 0)
    (:SHAPE_BOUNDING_BOX . 1)
    (:SHAPE_CYLINDER . 2)
    (:SHAPE_POLYGON . 3))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'TrackedObject)))
    "Constants for message type 'TrackedObject"
  '((:TRACK_UNKNOWN . 0)
    (:TRACK_TENTATIVE . 1)
    (:TRACK_CONFIRMED . 2)
    (:TRACK_COASTED . 3)
    (:MOTION_UNKNOWN . 0)
    (:MOTION_MOVING . 1)
    (:MOTION_STATIONARY . 2)
    (:ORIENTATION_UNAVAILABLE . 0)
    (:ORIENTATION_SIGN_UNKNOWN . 1)
    (:ORIENTATION_AVAILABLE . 2)
    (:SHAPE_UNKNOWN . 0)
    (:SHAPE_BOUNDING_BOX . 1)
    (:SHAPE_CYLINDER . 2)
    (:SHAPE_POLYGON . 3))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <TrackedObject>) ostream)
  "Serializes a message object of type '<TrackedObject>"
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'object_id) ostream)
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'classification) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'track_state)) ostream)
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'tracking_time))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'time_since_update))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'pose) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'has_pose_covariance) 1 0)) ostream)
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'twist) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'has_linear_velocity) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'has_angular_velocity) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'has_linear_covariance) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'has_angular_covariance) 1 0)) ostream)
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'acceleration) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'has_acceleration) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:if (cl:slot-value msg 'has_acceleration_covariance) 1 0)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'motion_state)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'orientation_availability)) ostream)
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'shape_type)) ostream)
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'dimensions) ostream)
  (roslisp-msg-protocol:serialize (cl:slot-value msg 'footprint) ostream)
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <TrackedObject>) istream)
  "Deserializes a message object of type '<TrackedObject>"
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'object_id) istream)
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'classification) istream)
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'track_state)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'tracking_time) (roslisp-utils:decode-single-float-bits bits)))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'time_since_update) (roslisp-utils:decode-single-float-bits bits)))
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'pose) istream)
    (cl:setf (cl:slot-value msg 'has_pose_covariance) (cl:not (cl:zerop (cl:read-byte istream))))
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'twist) istream)
    (cl:setf (cl:slot-value msg 'has_linear_velocity) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'has_angular_velocity) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'has_linear_covariance) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'has_angular_covariance) (cl:not (cl:zerop (cl:read-byte istream))))
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'acceleration) istream)
    (cl:setf (cl:slot-value msg 'has_acceleration) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:slot-value msg 'has_acceleration_covariance) (cl:not (cl:zerop (cl:read-byte istream))))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'motion_state)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'orientation_availability)) (cl:read-byte istream))
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'shape_type)) (cl:read-byte istream))
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'dimensions) istream)
  (roslisp-msg-protocol:deserialize (cl:slot-value msg 'footprint) istream)
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<TrackedObject>)))
  "Returns string type for a message object of type '<TrackedObject>"
  "dynamic_obstacles/TrackedObject")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'TrackedObject)))
  "Returns string type for a message object of type 'TrackedObject"
  "dynamic_obstacles/TrackedObject")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<TrackedObject>)))
  "Returns md5sum for a message object of type '<TrackedObject>"
  "ab8923e41e784db52548f8423530ee07")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'TrackedObject)))
  "Returns md5sum for a message object of type 'TrackedObject"
  "ab8923e41e784db52548f8423530ee07")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<TrackedObject>)))
  "Returns full string definition for message of type '<TrackedObject>"
  (cl:format cl:nil "# TrackedObject.msg~%~%# ============================================================~%# 坐标系与时间规范~%#~%# 1. 本消息自身不包含 Header，所有目标统一使用~%#    TrackedObjectArray.header 中的时间戳和参考坐标系。~%#~%# 2. 本项目规定：~%#    TrackedObjectArray.header.frame_id 必须为 \"map\"或\"odom\"。~%#~%# 3. pose、twist 和 acceleration 均对应~%#    TrackedObjectArray.header.stamp 所表示的统一状态时刻。~%#~%# 4. 以下物理量均在 指定参考的 坐标系上表达：~%#    - pose.position 底面中心点~%#    - pose.orientation 表示目标局部坐标系到 指定参考的 坐标系的旋转~%#    - twist.linear~%#    - twist.angular~%#    - acceleration.linear~%#    - acceleration.angular~%#~%# 5. twist 和 acceleration 表示经过机器人自身运动补偿后的、~%#    目标相对于 指定参考的 坐标系的估计运动状态，~%#    不是相对机器人或相对传感器的运动状态。~%#~%# 6. 目标局部坐标系基础约定：~%#   - 原点位于目标底面几何中心，右手坐标系~%#   - X 轴沿目标长度方向，但其具体指向受以下 orientation_availability 约束~%#   - Y 轴沿目标宽度方向~%#   - Z 轴沿目标高度方向，指向上方~%#~%# 7. dimensions 和 footprint 在目标局部坐标系中定义。~%#    footprint 不是 指定参考的坐标系下 的坐标；应结合 pose 转换到 指定参考的坐标系下。~%#~%# 8. 单位遵循 ROS 约定：~%#    - 位置、尺寸：m~%#    - 线速度：m/s~%#    - 角速度：rad/s~%#    - 线加速度：m/s^2~%#    - 角加速度：rad/s^2~%# ============================================================~%~%~%# 1. 基础属性~%uuid_msgs/UniqueID object_id           # 全局唯一的追踪 ID，轨迹删除后，不应立即复用相同 ID~%~%ObjectClassification classification    # 当前目标置信度最高的最终语义类别~%~%# ============================================================~%# 2. 跟踪状态~%# ============================================================~%~%# TRACK_UNKNOWN：未知~%# TENTATIVE：单帧新增障碍物/持续多帧尚未满足稳定确认条件，防止目标漏避~%# CONFIRMED：稳定轨迹，当前有观测更新~%# COASTED：当前无观测，由运动模型预测外推~%~%uint8 TRACK_UNKNOWN=0~%uint8 TRACK_TENTATIVE=1~%# 当前版本的感知发布端仅输出 TRACK_CONFIRMED。~%uint8 TRACK_CONFIRMED=2~%uint8 TRACK_COASTED=3~%~%uint8 track_state~%~%# 自轨迹创建以来的持续时间，单位 s~%float32 tracking_time~%~%# 距最后一次真实观测更新的时间，单位 s~%float32 time_since_update~%~%# ==========================================~%# 3. 运动学状态 (Kinematics)~%# 包含了位姿、速度、加速度，以及它们对应的 6x6 协方差矩阵(表示不确定性)~%# ==========================================~%~%# 位姿 (x, y, z 及 四元数姿态) + 协方差~%geometry_msgs/PoseWithCovariance pose~%bool has_pose_covariance~%~%# 速度 (线速度 v_x, v_y, v_z 及 角速度) + 协方差~%geometry_msgs/TwistWithCovariance twist~%bool has_linear_velocity ~%bool has_angular_velocity ~%bool has_linear_covariance~%bool has_angular_covariance~%~%# 加速度 (线加速度 a_x, a_y, a_z 及 角加速度) + 协方差~%geometry_msgs/AccelWithCovariance acceleration~%bool has_acceleration~%bool has_acceleration_covariance~%~%# 运动状态~%~%uint8 MOTION_UNKNOWN=0~%uint8 MOTION_MOVING=1~%uint8 MOTION_STATIONARY=2~%~%uint8 motion_state~%~%# ============================================================~%# 4. 朝向有效性~%# ============================================================~%~%# 无法获得可靠的几何方向或语义朝向。~%# pose.orientation 必须是合法且归一化的四元数；~%# 没有任何方向估计时推荐填单位四元数。~%#~%# 当 shape_type=SHAPE_BOUNDING_BOX 时，应输出相对于~%# header.frame_id 的轴对齐保守包围盒，或改用不依赖朝向的~%# SHAPE_CYLINDER。~%uint8 ORIENTATION_UNAVAILABLE=0~%~%# 只能确定目标长轴，正反方向存在 pi 弧度二义性。~%# 局部 X 轴平行于目标长轴，但正方向可能与真实前方相反。~%uint8 ORIENTATION_SIGN_UNKNOWN=1~%~%# 能够完整区分目标正面和背面，局部 +X 指向前方，+Y 指向左侧，+Z 指向上方。~%uint8 ORIENTATION_AVAILABLE=2~%~%uint8 orientation_availability~%~%~%~%# ==========================================~%# 5. 几何形状 (Shape)~%# 局部 x 轴为 length 方向，局部 y 轴为 width 方向，局部 z 轴为 height 方向~%# polygon 点序为逆时针，首尾点闭合~%# ==========================================~%# 形状未知或当前无法提供可靠几何形状 ~%uint8 SHAPE_UNKNOWN=0~%uint8 SHAPE_BOUNDING_BOX=1~%uint8 SHAPE_CYLINDER=2~%uint8 SHAPE_POLYGON=3~%~%uint8 shape_type                       # 形状类型~%~%#BOUNDING_BOX：x=length, y=width, z=height，且 footprint 应为空~%#CYLINDER：x=diameter, y=diameter, z=height，且 footprint 应为空~%#POLYGON：x=0, y=0, z=height，形状由 footprint 决定~%geometry_msgs/Vector3 dimensions~%~%# 仅在 shape_type=SHAPE_POLYGON 时使用；~%# 点位于目标局部坐标系中~%geometry_msgs/Polygon footprint~%~%================================================================================~%MSG: uuid_msgs/UniqueID~%# A universally unique identifier (UUID).~%#~%#  http://en.wikipedia.org/wiki/Universally_unique_identifier~%#  http://tools.ietf.org/html/rfc4122.html~%~%uint8[16] uuid~%~%================================================================================~%MSG: dynamic_obstacles/ObjectClassification~%# ObjectClassification.msg~%#~%# 类别编号按类别族分段预留。~%# 已发布的类别编号不得改变，新增类别应使用对应预留区间。~%#~%# 0       ：未知类别~%# 10~~29   ：道路车辆~%# 30~~49   ：两轮或三轮车辆~%# 50~~69   ：行人~%# 70~~89   ：动物~%# 90~~109  ：交通设施及危险物~%# 110~~253 ：后续类别预留~%# 254     ：其他类别~%# 255     ：非法或不支持的类别~%~%~%# 无法可靠识别类别，但目标仍可能真实存在，需要正常避障~%uint8 UNKNOWN=0~%~%~%# ============================================================~%# 道路车辆：10~~29~%# ============================================================~%~%# 小型乘用车辆，如轿车、SUV、MPV~%uint8 CAR=10~%~%# 货运车辆，如轻卡、厢式货车、重型卡车~%uint8 TRUCK=11~%~%# 载客大型车辆，如公交车、大巴、中巴~%uint8 BUS=12~%~%# 被牵引的挂车或拖车部分~%uint8 TRAILER=13~%~%# 14~~29 预留给其他道路车辆~%# 例如：VAN、FORKLIFT、CONSTRUCTION_VEHICLE~%~%~%# ============================================================~%# 两轮或三轮车辆：30~~49~%# ============================================================~%~%# 摩托车、电动摩托车等机动车辆~%uint8 MOTORCYCLE=30~%~%# 普通脚踏自行车~%uint8 BICYCLE=31~%~%# 电动自行车，不包括电动摩托车~%uint8 ELECTRIC_BICYCLE=32~%~%# 33~~49 预留~%# 例如：TRICYCLE~%~%~%# ============================================================~%# 人：50~~69~%# ============================================================~%~%# 普通人(无法进一步区分的人)。~%uint8 PEDESTRIAN=50~%~%#儿童~%uint8 CHILD=51~%~%#轮椅用户，需要更大的横向安全距离，且运动学模型与正常行人不同~%uint8 WHEELCHAIR=52~%~%# ============================================================~%# 动物：70~~89~%# ============================================================~%# 狗、猫等动物~%uint8 ANIMAL=70~%~%~%# ============================================================~%# 交通设施及危险物：90~~109~%# ============================================================~%~%# 交通锥、锥桶~%uint8 TRAFFIC_CONE=90~%~%# 闸机横杆~%uint8 BOOM_BARRIER=91~%~%# 门/闸机，此类障碍物的状态是可变的（开/关）~%uint8 DOOR_GATE=92~%~%# 93~~109 预留~%# 例如：BARRIER、BOLLARD、ROADBLOCK、DEBRIS~%~%~%# ============================================================~%# 特殊值~%# ============================================================~%# 已确认不属于当前已定义类别的其他目标。~%# 若无法识别类别，应使用 UNKNOWN。~%uint8 OTHER=254~%~%# 非法、解析失败或接口不支持的类别~%# 正常感知结果不应输出该值~%uint8 INVALID=255~%~%~%# 目标语义类别~%uint8 label~%~%# 当前类别的分类概率，范围 [0.0, 1.0]~%# 不等同于目标存在概率 existence_probability~%float32 probability~%~%~%================================================================================~%MSG: geometry_msgs/PoseWithCovariance~%# This represents a pose in free space with uncertainty.~%~%Pose pose~%~%# Row-major representation of the 6x6 covariance matrix~%# The orientation parameters use a fixed-axis representation.~%# In order, the parameters are:~%# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)~%float64[36] covariance~%~%================================================================================~%MSG: geometry_msgs/Pose~%# A representation of pose in free space, composed of position and orientation. ~%Point position~%Quaternion orientation~%~%================================================================================~%MSG: geometry_msgs/Point~%# This contains the position of a point in free space~%float64 x~%float64 y~%float64 z~%~%================================================================================~%MSG: geometry_msgs/Quaternion~%# This represents an orientation in free space in quaternion form.~%~%float64 x~%float64 y~%float64 z~%float64 w~%~%================================================================================~%MSG: geometry_msgs/TwistWithCovariance~%# This expresses velocity in free space with uncertainty.~%~%Twist twist~%~%# Row-major representation of the 6x6 covariance matrix~%# The orientation parameters use a fixed-axis representation.~%# In order, the parameters are:~%# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)~%float64[36] covariance~%~%================================================================================~%MSG: geometry_msgs/Twist~%# This expresses velocity in free space broken into its linear and angular parts.~%Vector3  linear~%Vector3  angular~%~%================================================================================~%MSG: geometry_msgs/Vector3~%# This represents a vector in free space. ~%# It is only meant to represent a direction. Therefore, it does not~%# make sense to apply a translation to it (e.g., when applying a ~%# generic rigid transformation to a Vector3, tf2 will only apply the~%# rotation). If you want your data to be translatable too, use the~%# geometry_msgs/Point message instead.~%~%float64 x~%float64 y~%float64 z~%================================================================================~%MSG: geometry_msgs/AccelWithCovariance~%# This expresses acceleration in free space with uncertainty.~%~%Accel accel~%~%# Row-major representation of the 6x6 covariance matrix~%# The orientation parameters use a fixed-axis representation.~%# In order, the parameters are:~%# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)~%float64[36] covariance~%~%================================================================================~%MSG: geometry_msgs/Accel~%# This expresses acceleration in free space broken into its linear and angular parts.~%Vector3  linear~%Vector3  angular~%~%================================================================================~%MSG: geometry_msgs/Polygon~%#A specification of a polygon where the first and last points are assumed to be connected~%Point32[] points~%~%================================================================================~%MSG: geometry_msgs/Point32~%# This contains the position of a point in free space(with 32 bits of precision).~%# It is recommeded to use Point wherever possible instead of Point32.  ~%# ~%# This recommendation is to promote interoperability.  ~%#~%# This message is designed to take up less space when sending~%# lots of points at once, as in the case of a PointCloud.  ~%~%float32 x~%float32 y~%float32 z~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'TrackedObject)))
  "Returns full string definition for message of type 'TrackedObject"
  (cl:format cl:nil "# TrackedObject.msg~%~%# ============================================================~%# 坐标系与时间规范~%#~%# 1. 本消息自身不包含 Header，所有目标统一使用~%#    TrackedObjectArray.header 中的时间戳和参考坐标系。~%#~%# 2. 本项目规定：~%#    TrackedObjectArray.header.frame_id 必须为 \"map\"或\"odom\"。~%#~%# 3. pose、twist 和 acceleration 均对应~%#    TrackedObjectArray.header.stamp 所表示的统一状态时刻。~%#~%# 4. 以下物理量均在 指定参考的 坐标系上表达：~%#    - pose.position 底面中心点~%#    - pose.orientation 表示目标局部坐标系到 指定参考的 坐标系的旋转~%#    - twist.linear~%#    - twist.angular~%#    - acceleration.linear~%#    - acceleration.angular~%#~%# 5. twist 和 acceleration 表示经过机器人自身运动补偿后的、~%#    目标相对于 指定参考的 坐标系的估计运动状态，~%#    不是相对机器人或相对传感器的运动状态。~%#~%# 6. 目标局部坐标系基础约定：~%#   - 原点位于目标底面几何中心，右手坐标系~%#   - X 轴沿目标长度方向，但其具体指向受以下 orientation_availability 约束~%#   - Y 轴沿目标宽度方向~%#   - Z 轴沿目标高度方向，指向上方~%#~%# 7. dimensions 和 footprint 在目标局部坐标系中定义。~%#    footprint 不是 指定参考的坐标系下 的坐标；应结合 pose 转换到 指定参考的坐标系下。~%#~%# 8. 单位遵循 ROS 约定：~%#    - 位置、尺寸：m~%#    - 线速度：m/s~%#    - 角速度：rad/s~%#    - 线加速度：m/s^2~%#    - 角加速度：rad/s^2~%# ============================================================~%~%~%# 1. 基础属性~%uuid_msgs/UniqueID object_id           # 全局唯一的追踪 ID，轨迹删除后，不应立即复用相同 ID~%~%ObjectClassification classification    # 当前目标置信度最高的最终语义类别~%~%# ============================================================~%# 2. 跟踪状态~%# ============================================================~%~%# TRACK_UNKNOWN：未知~%# TENTATIVE：单帧新增障碍物/持续多帧尚未满足稳定确认条件，防止目标漏避~%# CONFIRMED：稳定轨迹，当前有观测更新~%# COASTED：当前无观测，由运动模型预测外推~%~%uint8 TRACK_UNKNOWN=0~%uint8 TRACK_TENTATIVE=1~%# 当前版本的感知发布端仅输出 TRACK_CONFIRMED。~%uint8 TRACK_CONFIRMED=2~%uint8 TRACK_COASTED=3~%~%uint8 track_state~%~%# 自轨迹创建以来的持续时间，单位 s~%float32 tracking_time~%~%# 距最后一次真实观测更新的时间，单位 s~%float32 time_since_update~%~%# ==========================================~%# 3. 运动学状态 (Kinematics)~%# 包含了位姿、速度、加速度，以及它们对应的 6x6 协方差矩阵(表示不确定性)~%# ==========================================~%~%# 位姿 (x, y, z 及 四元数姿态) + 协方差~%geometry_msgs/PoseWithCovariance pose~%bool has_pose_covariance~%~%# 速度 (线速度 v_x, v_y, v_z 及 角速度) + 协方差~%geometry_msgs/TwistWithCovariance twist~%bool has_linear_velocity ~%bool has_angular_velocity ~%bool has_linear_covariance~%bool has_angular_covariance~%~%# 加速度 (线加速度 a_x, a_y, a_z 及 角加速度) + 协方差~%geometry_msgs/AccelWithCovariance acceleration~%bool has_acceleration~%bool has_acceleration_covariance~%~%# 运动状态~%~%uint8 MOTION_UNKNOWN=0~%uint8 MOTION_MOVING=1~%uint8 MOTION_STATIONARY=2~%~%uint8 motion_state~%~%# ============================================================~%# 4. 朝向有效性~%# ============================================================~%~%# 无法获得可靠的几何方向或语义朝向。~%# pose.orientation 必须是合法且归一化的四元数；~%# 没有任何方向估计时推荐填单位四元数。~%#~%# 当 shape_type=SHAPE_BOUNDING_BOX 时，应输出相对于~%# header.frame_id 的轴对齐保守包围盒，或改用不依赖朝向的~%# SHAPE_CYLINDER。~%uint8 ORIENTATION_UNAVAILABLE=0~%~%# 只能确定目标长轴，正反方向存在 pi 弧度二义性。~%# 局部 X 轴平行于目标长轴，但正方向可能与真实前方相反。~%uint8 ORIENTATION_SIGN_UNKNOWN=1~%~%# 能够完整区分目标正面和背面，局部 +X 指向前方，+Y 指向左侧，+Z 指向上方。~%uint8 ORIENTATION_AVAILABLE=2~%~%uint8 orientation_availability~%~%~%~%# ==========================================~%# 5. 几何形状 (Shape)~%# 局部 x 轴为 length 方向，局部 y 轴为 width 方向，局部 z 轴为 height 方向~%# polygon 点序为逆时针，首尾点闭合~%# ==========================================~%# 形状未知或当前无法提供可靠几何形状 ~%uint8 SHAPE_UNKNOWN=0~%uint8 SHAPE_BOUNDING_BOX=1~%uint8 SHAPE_CYLINDER=2~%uint8 SHAPE_POLYGON=3~%~%uint8 shape_type                       # 形状类型~%~%#BOUNDING_BOX：x=length, y=width, z=height，且 footprint 应为空~%#CYLINDER：x=diameter, y=diameter, z=height，且 footprint 应为空~%#POLYGON：x=0, y=0, z=height，形状由 footprint 决定~%geometry_msgs/Vector3 dimensions~%~%# 仅在 shape_type=SHAPE_POLYGON 时使用；~%# 点位于目标局部坐标系中~%geometry_msgs/Polygon footprint~%~%================================================================================~%MSG: uuid_msgs/UniqueID~%# A universally unique identifier (UUID).~%#~%#  http://en.wikipedia.org/wiki/Universally_unique_identifier~%#  http://tools.ietf.org/html/rfc4122.html~%~%uint8[16] uuid~%~%================================================================================~%MSG: dynamic_obstacles/ObjectClassification~%# ObjectClassification.msg~%#~%# 类别编号按类别族分段预留。~%# 已发布的类别编号不得改变，新增类别应使用对应预留区间。~%#~%# 0       ：未知类别~%# 10~~29   ：道路车辆~%# 30~~49   ：两轮或三轮车辆~%# 50~~69   ：行人~%# 70~~89   ：动物~%# 90~~109  ：交通设施及危险物~%# 110~~253 ：后续类别预留~%# 254     ：其他类别~%# 255     ：非法或不支持的类别~%~%~%# 无法可靠识别类别，但目标仍可能真实存在，需要正常避障~%uint8 UNKNOWN=0~%~%~%# ============================================================~%# 道路车辆：10~~29~%# ============================================================~%~%# 小型乘用车辆，如轿车、SUV、MPV~%uint8 CAR=10~%~%# 货运车辆，如轻卡、厢式货车、重型卡车~%uint8 TRUCK=11~%~%# 载客大型车辆，如公交车、大巴、中巴~%uint8 BUS=12~%~%# 被牵引的挂车或拖车部分~%uint8 TRAILER=13~%~%# 14~~29 预留给其他道路车辆~%# 例如：VAN、FORKLIFT、CONSTRUCTION_VEHICLE~%~%~%# ============================================================~%# 两轮或三轮车辆：30~~49~%# ============================================================~%~%# 摩托车、电动摩托车等机动车辆~%uint8 MOTORCYCLE=30~%~%# 普通脚踏自行车~%uint8 BICYCLE=31~%~%# 电动自行车，不包括电动摩托车~%uint8 ELECTRIC_BICYCLE=32~%~%# 33~~49 预留~%# 例如：TRICYCLE~%~%~%# ============================================================~%# 人：50~~69~%# ============================================================~%~%# 普通人(无法进一步区分的人)。~%uint8 PEDESTRIAN=50~%~%#儿童~%uint8 CHILD=51~%~%#轮椅用户，需要更大的横向安全距离，且运动学模型与正常行人不同~%uint8 WHEELCHAIR=52~%~%# ============================================================~%# 动物：70~~89~%# ============================================================~%# 狗、猫等动物~%uint8 ANIMAL=70~%~%~%# ============================================================~%# 交通设施及危险物：90~~109~%# ============================================================~%~%# 交通锥、锥桶~%uint8 TRAFFIC_CONE=90~%~%# 闸机横杆~%uint8 BOOM_BARRIER=91~%~%# 门/闸机，此类障碍物的状态是可变的（开/关）~%uint8 DOOR_GATE=92~%~%# 93~~109 预留~%# 例如：BARRIER、BOLLARD、ROADBLOCK、DEBRIS~%~%~%# ============================================================~%# 特殊值~%# ============================================================~%# 已确认不属于当前已定义类别的其他目标。~%# 若无法识别类别，应使用 UNKNOWN。~%uint8 OTHER=254~%~%# 非法、解析失败或接口不支持的类别~%# 正常感知结果不应输出该值~%uint8 INVALID=255~%~%~%# 目标语义类别~%uint8 label~%~%# 当前类别的分类概率，范围 [0.0, 1.0]~%# 不等同于目标存在概率 existence_probability~%float32 probability~%~%~%================================================================================~%MSG: geometry_msgs/PoseWithCovariance~%# This represents a pose in free space with uncertainty.~%~%Pose pose~%~%# Row-major representation of the 6x6 covariance matrix~%# The orientation parameters use a fixed-axis representation.~%# In order, the parameters are:~%# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)~%float64[36] covariance~%~%================================================================================~%MSG: geometry_msgs/Pose~%# A representation of pose in free space, composed of position and orientation. ~%Point position~%Quaternion orientation~%~%================================================================================~%MSG: geometry_msgs/Point~%# This contains the position of a point in free space~%float64 x~%float64 y~%float64 z~%~%================================================================================~%MSG: geometry_msgs/Quaternion~%# This represents an orientation in free space in quaternion form.~%~%float64 x~%float64 y~%float64 z~%float64 w~%~%================================================================================~%MSG: geometry_msgs/TwistWithCovariance~%# This expresses velocity in free space with uncertainty.~%~%Twist twist~%~%# Row-major representation of the 6x6 covariance matrix~%# The orientation parameters use a fixed-axis representation.~%# In order, the parameters are:~%# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)~%float64[36] covariance~%~%================================================================================~%MSG: geometry_msgs/Twist~%# This expresses velocity in free space broken into its linear and angular parts.~%Vector3  linear~%Vector3  angular~%~%================================================================================~%MSG: geometry_msgs/Vector3~%# This represents a vector in free space. ~%# It is only meant to represent a direction. Therefore, it does not~%# make sense to apply a translation to it (e.g., when applying a ~%# generic rigid transformation to a Vector3, tf2 will only apply the~%# rotation). If you want your data to be translatable too, use the~%# geometry_msgs/Point message instead.~%~%float64 x~%float64 y~%float64 z~%================================================================================~%MSG: geometry_msgs/AccelWithCovariance~%# This expresses acceleration in free space with uncertainty.~%~%Accel accel~%~%# Row-major representation of the 6x6 covariance matrix~%# The orientation parameters use a fixed-axis representation.~%# In order, the parameters are:~%# (x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)~%float64[36] covariance~%~%================================================================================~%MSG: geometry_msgs/Accel~%# This expresses acceleration in free space broken into its linear and angular parts.~%Vector3  linear~%Vector3  angular~%~%================================================================================~%MSG: geometry_msgs/Polygon~%#A specification of a polygon where the first and last points are assumed to be connected~%Point32[] points~%~%================================================================================~%MSG: geometry_msgs/Point32~%# This contains the position of a point in free space(with 32 bits of precision).~%# It is recommeded to use Point wherever possible instead of Point32.  ~%# ~%# This recommendation is to promote interoperability.  ~%#~%# This message is designed to take up less space when sending~%# lots of points at once, as in the case of a PointCloud.  ~%~%float32 x~%float32 y~%float32 z~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <TrackedObject>))
  (cl:+ 0
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'object_id))
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'classification))
     1
     4
     4
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'pose))
     1
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'twist))
     1
     1
     1
     1
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'acceleration))
     1
     1
     1
     1
     1
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'dimensions))
     (roslisp-msg-protocol:serialization-length (cl:slot-value msg 'footprint))
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <TrackedObject>))
  "Converts a ROS message object to a list"
  (cl:list 'TrackedObject
    (cl:cons ':object_id (object_id msg))
    (cl:cons ':classification (classification msg))
    (cl:cons ':track_state (track_state msg))
    (cl:cons ':tracking_time (tracking_time msg))
    (cl:cons ':time_since_update (time_since_update msg))
    (cl:cons ':pose (pose msg))
    (cl:cons ':has_pose_covariance (has_pose_covariance msg))
    (cl:cons ':twist (twist msg))
    (cl:cons ':has_linear_velocity (has_linear_velocity msg))
    (cl:cons ':has_angular_velocity (has_angular_velocity msg))
    (cl:cons ':has_linear_covariance (has_linear_covariance msg))
    (cl:cons ':has_angular_covariance (has_angular_covariance msg))
    (cl:cons ':acceleration (acceleration msg))
    (cl:cons ':has_acceleration (has_acceleration msg))
    (cl:cons ':has_acceleration_covariance (has_acceleration_covariance msg))
    (cl:cons ':motion_state (motion_state msg))
    (cl:cons ':orientation_availability (orientation_availability msg))
    (cl:cons ':shape_type (shape_type msg))
    (cl:cons ':dimensions (dimensions msg))
    (cl:cons ':footprint (footprint msg))
))

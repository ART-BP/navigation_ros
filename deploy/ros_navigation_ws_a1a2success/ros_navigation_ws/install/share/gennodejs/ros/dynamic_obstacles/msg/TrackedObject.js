// Auto-generated. Do not edit!

// (in-package dynamic_obstacles.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let ObjectClassification = require('./ObjectClassification.js');
let geometry_msgs = _finder('geometry_msgs');
let uuid_msgs = _finder('uuid_msgs');

//-----------------------------------------------------------

class TrackedObject {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.object_id = null;
      this.classification = null;
      this.track_state = null;
      this.tracking_time = null;
      this.time_since_update = null;
      this.pose = null;
      this.has_pose_covariance = null;
      this.twist = null;
      this.has_linear_velocity = null;
      this.has_angular_velocity = null;
      this.has_linear_covariance = null;
      this.has_angular_covariance = null;
      this.acceleration = null;
      this.has_acceleration = null;
      this.has_acceleration_covariance = null;
      this.motion_state = null;
      this.orientation_availability = null;
      this.shape_type = null;
      this.dimensions = null;
      this.footprint = null;
    }
    else {
      if (initObj.hasOwnProperty('object_id')) {
        this.object_id = initObj.object_id
      }
      else {
        this.object_id = new uuid_msgs.msg.UniqueID();
      }
      if (initObj.hasOwnProperty('classification')) {
        this.classification = initObj.classification
      }
      else {
        this.classification = new ObjectClassification();
      }
      if (initObj.hasOwnProperty('track_state')) {
        this.track_state = initObj.track_state
      }
      else {
        this.track_state = 0;
      }
      if (initObj.hasOwnProperty('tracking_time')) {
        this.tracking_time = initObj.tracking_time
      }
      else {
        this.tracking_time = 0.0;
      }
      if (initObj.hasOwnProperty('time_since_update')) {
        this.time_since_update = initObj.time_since_update
      }
      else {
        this.time_since_update = 0.0;
      }
      if (initObj.hasOwnProperty('pose')) {
        this.pose = initObj.pose
      }
      else {
        this.pose = new geometry_msgs.msg.PoseWithCovariance();
      }
      if (initObj.hasOwnProperty('has_pose_covariance')) {
        this.has_pose_covariance = initObj.has_pose_covariance
      }
      else {
        this.has_pose_covariance = false;
      }
      if (initObj.hasOwnProperty('twist')) {
        this.twist = initObj.twist
      }
      else {
        this.twist = new geometry_msgs.msg.TwistWithCovariance();
      }
      if (initObj.hasOwnProperty('has_linear_velocity')) {
        this.has_linear_velocity = initObj.has_linear_velocity
      }
      else {
        this.has_linear_velocity = false;
      }
      if (initObj.hasOwnProperty('has_angular_velocity')) {
        this.has_angular_velocity = initObj.has_angular_velocity
      }
      else {
        this.has_angular_velocity = false;
      }
      if (initObj.hasOwnProperty('has_linear_covariance')) {
        this.has_linear_covariance = initObj.has_linear_covariance
      }
      else {
        this.has_linear_covariance = false;
      }
      if (initObj.hasOwnProperty('has_angular_covariance')) {
        this.has_angular_covariance = initObj.has_angular_covariance
      }
      else {
        this.has_angular_covariance = false;
      }
      if (initObj.hasOwnProperty('acceleration')) {
        this.acceleration = initObj.acceleration
      }
      else {
        this.acceleration = new geometry_msgs.msg.AccelWithCovariance();
      }
      if (initObj.hasOwnProperty('has_acceleration')) {
        this.has_acceleration = initObj.has_acceleration
      }
      else {
        this.has_acceleration = false;
      }
      if (initObj.hasOwnProperty('has_acceleration_covariance')) {
        this.has_acceleration_covariance = initObj.has_acceleration_covariance
      }
      else {
        this.has_acceleration_covariance = false;
      }
      if (initObj.hasOwnProperty('motion_state')) {
        this.motion_state = initObj.motion_state
      }
      else {
        this.motion_state = 0;
      }
      if (initObj.hasOwnProperty('orientation_availability')) {
        this.orientation_availability = initObj.orientation_availability
      }
      else {
        this.orientation_availability = 0;
      }
      if (initObj.hasOwnProperty('shape_type')) {
        this.shape_type = initObj.shape_type
      }
      else {
        this.shape_type = 0;
      }
      if (initObj.hasOwnProperty('dimensions')) {
        this.dimensions = initObj.dimensions
      }
      else {
        this.dimensions = new geometry_msgs.msg.Vector3();
      }
      if (initObj.hasOwnProperty('footprint')) {
        this.footprint = initObj.footprint
      }
      else {
        this.footprint = new geometry_msgs.msg.Polygon();
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type TrackedObject
    // Serialize message field [object_id]
    bufferOffset = uuid_msgs.msg.UniqueID.serialize(obj.object_id, buffer, bufferOffset);
    // Serialize message field [classification]
    bufferOffset = ObjectClassification.serialize(obj.classification, buffer, bufferOffset);
    // Serialize message field [track_state]
    bufferOffset = _serializer.uint8(obj.track_state, buffer, bufferOffset);
    // Serialize message field [tracking_time]
    bufferOffset = _serializer.float32(obj.tracking_time, buffer, bufferOffset);
    // Serialize message field [time_since_update]
    bufferOffset = _serializer.float32(obj.time_since_update, buffer, bufferOffset);
    // Serialize message field [pose]
    bufferOffset = geometry_msgs.msg.PoseWithCovariance.serialize(obj.pose, buffer, bufferOffset);
    // Serialize message field [has_pose_covariance]
    bufferOffset = _serializer.bool(obj.has_pose_covariance, buffer, bufferOffset);
    // Serialize message field [twist]
    bufferOffset = geometry_msgs.msg.TwistWithCovariance.serialize(obj.twist, buffer, bufferOffset);
    // Serialize message field [has_linear_velocity]
    bufferOffset = _serializer.bool(obj.has_linear_velocity, buffer, bufferOffset);
    // Serialize message field [has_angular_velocity]
    bufferOffset = _serializer.bool(obj.has_angular_velocity, buffer, bufferOffset);
    // Serialize message field [has_linear_covariance]
    bufferOffset = _serializer.bool(obj.has_linear_covariance, buffer, bufferOffset);
    // Serialize message field [has_angular_covariance]
    bufferOffset = _serializer.bool(obj.has_angular_covariance, buffer, bufferOffset);
    // Serialize message field [acceleration]
    bufferOffset = geometry_msgs.msg.AccelWithCovariance.serialize(obj.acceleration, buffer, bufferOffset);
    // Serialize message field [has_acceleration]
    bufferOffset = _serializer.bool(obj.has_acceleration, buffer, bufferOffset);
    // Serialize message field [has_acceleration_covariance]
    bufferOffset = _serializer.bool(obj.has_acceleration_covariance, buffer, bufferOffset);
    // Serialize message field [motion_state]
    bufferOffset = _serializer.uint8(obj.motion_state, buffer, bufferOffset);
    // Serialize message field [orientation_availability]
    bufferOffset = _serializer.uint8(obj.orientation_availability, buffer, bufferOffset);
    // Serialize message field [shape_type]
    bufferOffset = _serializer.uint8(obj.shape_type, buffer, bufferOffset);
    // Serialize message field [dimensions]
    bufferOffset = geometry_msgs.msg.Vector3.serialize(obj.dimensions, buffer, bufferOffset);
    // Serialize message field [footprint]
    bufferOffset = geometry_msgs.msg.Polygon.serialize(obj.footprint, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type TrackedObject
    let len;
    let data = new TrackedObject(null);
    // Deserialize message field [object_id]
    data.object_id = uuid_msgs.msg.UniqueID.deserialize(buffer, bufferOffset);
    // Deserialize message field [classification]
    data.classification = ObjectClassification.deserialize(buffer, bufferOffset);
    // Deserialize message field [track_state]
    data.track_state = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [tracking_time]
    data.tracking_time = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [time_since_update]
    data.time_since_update = _deserializer.float32(buffer, bufferOffset);
    // Deserialize message field [pose]
    data.pose = geometry_msgs.msg.PoseWithCovariance.deserialize(buffer, bufferOffset);
    // Deserialize message field [has_pose_covariance]
    data.has_pose_covariance = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [twist]
    data.twist = geometry_msgs.msg.TwistWithCovariance.deserialize(buffer, bufferOffset);
    // Deserialize message field [has_linear_velocity]
    data.has_linear_velocity = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [has_angular_velocity]
    data.has_angular_velocity = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [has_linear_covariance]
    data.has_linear_covariance = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [has_angular_covariance]
    data.has_angular_covariance = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [acceleration]
    data.acceleration = geometry_msgs.msg.AccelWithCovariance.deserialize(buffer, bufferOffset);
    // Deserialize message field [has_acceleration]
    data.has_acceleration = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [has_acceleration_covariance]
    data.has_acceleration_covariance = _deserializer.bool(buffer, bufferOffset);
    // Deserialize message field [motion_state]
    data.motion_state = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [orientation_availability]
    data.orientation_availability = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [shape_type]
    data.shape_type = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [dimensions]
    data.dimensions = geometry_msgs.msg.Vector3.deserialize(buffer, bufferOffset);
    // Deserialize message field [footprint]
    data.footprint = geometry_msgs.msg.Polygon.deserialize(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += geometry_msgs.msg.Polygon.getMessageSize(object.footprint);
    return length + 1080;
  }

  static datatype() {
    // Returns string type for a message object
    return 'dynamic_obstacles/TrackedObject';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'ab8923e41e784db52548f8423530ee07';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # TrackedObject.msg
    
    # ============================================================
    # 坐标系与时间规范
    #
    # 1. 本消息自身不包含 Header，所有目标统一使用
    #    TrackedObjectArray.header 中的时间戳和参考坐标系。
    #
    # 2. 本项目规定：
    #    TrackedObjectArray.header.frame_id 必须为 "map"或"odom"。
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
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new TrackedObject(null);
    if (msg.object_id !== undefined) {
      resolved.object_id = uuid_msgs.msg.UniqueID.Resolve(msg.object_id)
    }
    else {
      resolved.object_id = new uuid_msgs.msg.UniqueID()
    }

    if (msg.classification !== undefined) {
      resolved.classification = ObjectClassification.Resolve(msg.classification)
    }
    else {
      resolved.classification = new ObjectClassification()
    }

    if (msg.track_state !== undefined) {
      resolved.track_state = msg.track_state;
    }
    else {
      resolved.track_state = 0
    }

    if (msg.tracking_time !== undefined) {
      resolved.tracking_time = msg.tracking_time;
    }
    else {
      resolved.tracking_time = 0.0
    }

    if (msg.time_since_update !== undefined) {
      resolved.time_since_update = msg.time_since_update;
    }
    else {
      resolved.time_since_update = 0.0
    }

    if (msg.pose !== undefined) {
      resolved.pose = geometry_msgs.msg.PoseWithCovariance.Resolve(msg.pose)
    }
    else {
      resolved.pose = new geometry_msgs.msg.PoseWithCovariance()
    }

    if (msg.has_pose_covariance !== undefined) {
      resolved.has_pose_covariance = msg.has_pose_covariance;
    }
    else {
      resolved.has_pose_covariance = false
    }

    if (msg.twist !== undefined) {
      resolved.twist = geometry_msgs.msg.TwistWithCovariance.Resolve(msg.twist)
    }
    else {
      resolved.twist = new geometry_msgs.msg.TwistWithCovariance()
    }

    if (msg.has_linear_velocity !== undefined) {
      resolved.has_linear_velocity = msg.has_linear_velocity;
    }
    else {
      resolved.has_linear_velocity = false
    }

    if (msg.has_angular_velocity !== undefined) {
      resolved.has_angular_velocity = msg.has_angular_velocity;
    }
    else {
      resolved.has_angular_velocity = false
    }

    if (msg.has_linear_covariance !== undefined) {
      resolved.has_linear_covariance = msg.has_linear_covariance;
    }
    else {
      resolved.has_linear_covariance = false
    }

    if (msg.has_angular_covariance !== undefined) {
      resolved.has_angular_covariance = msg.has_angular_covariance;
    }
    else {
      resolved.has_angular_covariance = false
    }

    if (msg.acceleration !== undefined) {
      resolved.acceleration = geometry_msgs.msg.AccelWithCovariance.Resolve(msg.acceleration)
    }
    else {
      resolved.acceleration = new geometry_msgs.msg.AccelWithCovariance()
    }

    if (msg.has_acceleration !== undefined) {
      resolved.has_acceleration = msg.has_acceleration;
    }
    else {
      resolved.has_acceleration = false
    }

    if (msg.has_acceleration_covariance !== undefined) {
      resolved.has_acceleration_covariance = msg.has_acceleration_covariance;
    }
    else {
      resolved.has_acceleration_covariance = false
    }

    if (msg.motion_state !== undefined) {
      resolved.motion_state = msg.motion_state;
    }
    else {
      resolved.motion_state = 0
    }

    if (msg.orientation_availability !== undefined) {
      resolved.orientation_availability = msg.orientation_availability;
    }
    else {
      resolved.orientation_availability = 0
    }

    if (msg.shape_type !== undefined) {
      resolved.shape_type = msg.shape_type;
    }
    else {
      resolved.shape_type = 0
    }

    if (msg.dimensions !== undefined) {
      resolved.dimensions = geometry_msgs.msg.Vector3.Resolve(msg.dimensions)
    }
    else {
      resolved.dimensions = new geometry_msgs.msg.Vector3()
    }

    if (msg.footprint !== undefined) {
      resolved.footprint = geometry_msgs.msg.Polygon.Resolve(msg.footprint)
    }
    else {
      resolved.footprint = new geometry_msgs.msg.Polygon()
    }

    return resolved;
    }
};

// Constants for message
TrackedObject.Constants = {
  TRACK_UNKNOWN: 0,
  TRACK_TENTATIVE: 1,
  TRACK_CONFIRMED: 2,
  TRACK_COASTED: 3,
  MOTION_UNKNOWN: 0,
  MOTION_MOVING: 1,
  MOTION_STATIONARY: 2,
  ORIENTATION_UNAVAILABLE: 0,
  ORIENTATION_SIGN_UNKNOWN: 1,
  ORIENTATION_AVAILABLE: 2,
  SHAPE_UNKNOWN: 0,
  SHAPE_BOUNDING_BOX: 1,
  SHAPE_CYLINDER: 2,
  SHAPE_POLYGON: 3,
}

module.exports = TrackedObject;

// Auto-generated. Do not edit!

// (in-package dynamic_obstacles.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;
let TrackedObject = require('./TrackedObject.js');
let std_msgs = _finder('std_msgs');

//-----------------------------------------------------------

class TrackedObjectArray {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.header = null;
      this.objects = null;
    }
    else {
      if (initObj.hasOwnProperty('header')) {
        this.header = initObj.header
      }
      else {
        this.header = new std_msgs.msg.Header();
      }
      if (initObj.hasOwnProperty('objects')) {
        this.objects = initObj.objects
      }
      else {
        this.objects = [];
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type TrackedObjectArray
    // Serialize message field [header]
    bufferOffset = std_msgs.msg.Header.serialize(obj.header, buffer, bufferOffset);
    // Serialize message field [objects]
    // Serialize the length for message field [objects]
    bufferOffset = _serializer.uint32(obj.objects.length, buffer, bufferOffset);
    obj.objects.forEach((val) => {
      bufferOffset = TrackedObject.serialize(val, buffer, bufferOffset);
    });
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type TrackedObjectArray
    let len;
    let data = new TrackedObjectArray(null);
    // Deserialize message field [header]
    data.header = std_msgs.msg.Header.deserialize(buffer, bufferOffset);
    // Deserialize message field [objects]
    // Deserialize array length for message field [objects]
    len = _deserializer.uint32(buffer, bufferOffset);
    data.objects = new Array(len);
    for (let i = 0; i < len; ++i) {
      data.objects[i] = TrackedObject.deserialize(buffer, bufferOffset)
    }
    return data;
  }

  static getMessageSize(object) {
    let length = 0;
    length += std_msgs.msg.Header.getMessageSize(object.header);
    object.objects.forEach((val) => {
      length += TrackedObject.getMessageSize(val);
    });
    return length + 4;
  }

  static datatype() {
    // Returns string type for a message object
    return 'dynamic_obstacles/TrackedObjectArray';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'ba2939615e19076197d03cd4e0e914f7';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
    # TrackedObjectArray.msg
    # Version history:
    #
    # v0.0.3 20260618
    # - 目标pose是底面中心。
    # - 感知仅输出track_state=TRACK_CONFIRMED，没检出使用普通障碍物解决。
    # - 速度标志位区分线速度与角速度及其协方差。
    # - 静止目标运动状态感知输出完全是 0 m/s。
    #
    # v0.0.2 20260618
    # - 重新划分类别编号区间并扩充各类别族的预留空间。
    # - 将 classifications 数组修改为单个 Top-1 classification。
    # - 删除无法可靠提供的 existence_probability 字段。
    # - 完善坐标系、时间戳、跟踪状态和运动状态的定义。
    # - 完善目标局部坐标系、朝向有效性及几何形状约定。
    # - 明确 dimensions 和 footprint 使用目标局部坐标系表达。
    # - 本版本与 v0.0.1 不兼容，相关 ROS 节点需要重新编译。
    #
    # v0.0.1 20260616
    # - 首次发布。
    # - 所有追踪目标统一使用数组级 Header。
    # - 支持目标唯一标识、语义分类、运动学状态、跟踪状态、运动状态和几何形状。
    #
    
    std_msgs/Header header         # 包含 timestamp 和 frame_id (通常是 odom 或 map)
    TrackedObject[] objects        # 追踪目标数组
    
    ================================================================================
    MSG: std_msgs/Header
    # Standard metadata for higher-level stamped data types.
    # This is generally used to communicate timestamped data 
    # in a particular coordinate frame.
    # 
    # sequence ID: consecutively increasing ID 
    uint32 seq
    #Two-integer timestamp that is expressed as:
    # * stamp.sec: seconds (stamp_secs) since epoch (in Python the variable is called 'secs')
    # * stamp.nsec: nanoseconds since stamp_secs (in Python the variable is called 'nsecs')
    # time-handling sugar is provided by the client library
    time stamp
    #Frame this data is associated with
    string frame_id
    
    ================================================================================
    MSG: dynamic_obstacles/TrackedObject
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
    const resolved = new TrackedObjectArray(null);
    if (msg.header !== undefined) {
      resolved.header = std_msgs.msg.Header.Resolve(msg.header)
    }
    else {
      resolved.header = new std_msgs.msg.Header()
    }

    if (msg.objects !== undefined) {
      resolved.objects = new Array(msg.objects.length);
      for (let i = 0; i < resolved.objects.length; ++i) {
        resolved.objects[i] = TrackedObject.Resolve(msg.objects[i]);
      }
    }
    else {
      resolved.objects = []
    }

    return resolved;
    }
};

module.exports = TrackedObjectArray;

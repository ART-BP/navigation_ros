; Auto-generated. Do not edit!


(cl:in-package dynamic_obstacles-msg)


;//! \htmlinclude ObjectClassification.msg.html

(cl:defclass <ObjectClassification> (roslisp-msg-protocol:ros-message)
  ((label
    :reader label
    :initarg :label
    :type cl:fixnum
    :initform 0)
   (probability
    :reader probability
    :initarg :probability
    :type cl:float
    :initform 0.0))
)

(cl:defclass ObjectClassification (<ObjectClassification>)
  ())

(cl:defmethod cl:initialize-instance :after ((m <ObjectClassification>) cl:&rest args)
  (cl:declare (cl:ignorable args))
  (cl:unless (cl:typep m 'ObjectClassification)
    (roslisp-msg-protocol:msg-deprecation-warning "using old message class name dynamic_obstacles-msg:<ObjectClassification> is deprecated: use dynamic_obstacles-msg:ObjectClassification instead.")))

(cl:ensure-generic-function 'label-val :lambda-list '(m))
(cl:defmethod label-val ((m <ObjectClassification>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:label-val is deprecated.  Use dynamic_obstacles-msg:label instead.")
  (label m))

(cl:ensure-generic-function 'probability-val :lambda-list '(m))
(cl:defmethod probability-val ((m <ObjectClassification>))
  (roslisp-msg-protocol:msg-deprecation-warning "Using old-style slot reader dynamic_obstacles-msg:probability-val is deprecated.  Use dynamic_obstacles-msg:probability instead.")
  (probability m))
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql '<ObjectClassification>)))
    "Constants for message type '<ObjectClassification>"
  '((:UNKNOWN . 0)
    (:CAR . 10)
    (:TRUCK . 11)
    (:BUS . 12)
    (:TRAILER . 13)
    (:MOTORCYCLE . 30)
    (:BICYCLE . 31)
    (:ELECTRIC_BICYCLE . 32)
    (:PEDESTRIAN . 50)
    (:CHILD . 51)
    (:WHEELCHAIR . 52)
    (:ANIMAL . 70)
    (:TRAFFIC_CONE . 90)
    (:BOOM_BARRIER . 91)
    (:DOOR_GATE . 92)
    (:OTHER . 254)
    (:INVALID . 255))
)
(cl:defmethod roslisp-msg-protocol:symbol-codes ((msg-type (cl:eql 'ObjectClassification)))
    "Constants for message type 'ObjectClassification"
  '((:UNKNOWN . 0)
    (:CAR . 10)
    (:TRUCK . 11)
    (:BUS . 12)
    (:TRAILER . 13)
    (:MOTORCYCLE . 30)
    (:BICYCLE . 31)
    (:ELECTRIC_BICYCLE . 32)
    (:PEDESTRIAN . 50)
    (:CHILD . 51)
    (:WHEELCHAIR . 52)
    (:ANIMAL . 70)
    (:TRAFFIC_CONE . 90)
    (:BOOM_BARRIER . 91)
    (:DOOR_GATE . 92)
    (:OTHER . 254)
    (:INVALID . 255))
)
(cl:defmethod roslisp-msg-protocol:serialize ((msg <ObjectClassification>) ostream)
  "Serializes a message object of type '<ObjectClassification>"
  (cl:write-byte (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'label)) ostream)
  (cl:let ((bits (roslisp-utils:encode-single-float-bits (cl:slot-value msg 'probability))))
    (cl:write-byte (cl:ldb (cl:byte 8 0) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 8) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 16) bits) ostream)
    (cl:write-byte (cl:ldb (cl:byte 8 24) bits) ostream))
)
(cl:defmethod roslisp-msg-protocol:deserialize ((msg <ObjectClassification>) istream)
  "Deserializes a message object of type '<ObjectClassification>"
    (cl:setf (cl:ldb (cl:byte 8 0) (cl:slot-value msg 'label)) (cl:read-byte istream))
    (cl:let ((bits 0))
      (cl:setf (cl:ldb (cl:byte 8 0) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 8) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 16) bits) (cl:read-byte istream))
      (cl:setf (cl:ldb (cl:byte 8 24) bits) (cl:read-byte istream))
    (cl:setf (cl:slot-value msg 'probability) (roslisp-utils:decode-single-float-bits bits)))
  msg
)
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql '<ObjectClassification>)))
  "Returns string type for a message object of type '<ObjectClassification>"
  "dynamic_obstacles/ObjectClassification")
(cl:defmethod roslisp-msg-protocol:ros-datatype ((msg (cl:eql 'ObjectClassification)))
  "Returns string type for a message object of type 'ObjectClassification"
  "dynamic_obstacles/ObjectClassification")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql '<ObjectClassification>)))
  "Returns md5sum for a message object of type '<ObjectClassification>"
  "b67f0ffe50a7c47e827671307d1e4a55")
(cl:defmethod roslisp-msg-protocol:md5sum ((type (cl:eql 'ObjectClassification)))
  "Returns md5sum for a message object of type 'ObjectClassification"
  "b67f0ffe50a7c47e827671307d1e4a55")
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql '<ObjectClassification>)))
  "Returns full string definition for message of type '<ObjectClassification>"
  (cl:format cl:nil "# ObjectClassification.msg~%#~%# 类别编号按类别族分段预留。~%# 已发布的类别编号不得改变，新增类别应使用对应预留区间。~%#~%# 0       ：未知类别~%# 10~~29   ：道路车辆~%# 30~~49   ：两轮或三轮车辆~%# 50~~69   ：行人~%# 70~~89   ：动物~%# 90~~109  ：交通设施及危险物~%# 110~~253 ：后续类别预留~%# 254     ：其他类别~%# 255     ：非法或不支持的类别~%~%~%# 无法可靠识别类别，但目标仍可能真实存在，需要正常避障~%uint8 UNKNOWN=0~%~%~%# ============================================================~%# 道路车辆：10~~29~%# ============================================================~%~%# 小型乘用车辆，如轿车、SUV、MPV~%uint8 CAR=10~%~%# 货运车辆，如轻卡、厢式货车、重型卡车~%uint8 TRUCK=11~%~%# 载客大型车辆，如公交车、大巴、中巴~%uint8 BUS=12~%~%# 被牵引的挂车或拖车部分~%uint8 TRAILER=13~%~%# 14~~29 预留给其他道路车辆~%# 例如：VAN、FORKLIFT、CONSTRUCTION_VEHICLE~%~%~%# ============================================================~%# 两轮或三轮车辆：30~~49~%# ============================================================~%~%# 摩托车、电动摩托车等机动车辆~%uint8 MOTORCYCLE=30~%~%# 普通脚踏自行车~%uint8 BICYCLE=31~%~%# 电动自行车，不包括电动摩托车~%uint8 ELECTRIC_BICYCLE=32~%~%# 33~~49 预留~%# 例如：TRICYCLE~%~%~%# ============================================================~%# 人：50~~69~%# ============================================================~%~%# 普通人(无法进一步区分的人)。~%uint8 PEDESTRIAN=50~%~%#儿童~%uint8 CHILD=51~%~%#轮椅用户，需要更大的横向安全距离，且运动学模型与正常行人不同~%uint8 WHEELCHAIR=52~%~%# ============================================================~%# 动物：70~~89~%# ============================================================~%# 狗、猫等动物~%uint8 ANIMAL=70~%~%~%# ============================================================~%# 交通设施及危险物：90~~109~%# ============================================================~%~%# 交通锥、锥桶~%uint8 TRAFFIC_CONE=90~%~%# 闸机横杆~%uint8 BOOM_BARRIER=91~%~%# 门/闸机，此类障碍物的状态是可变的（开/关）~%uint8 DOOR_GATE=92~%~%# 93~~109 预留~%# 例如：BARRIER、BOLLARD、ROADBLOCK、DEBRIS~%~%~%# ============================================================~%# 特殊值~%# ============================================================~%# 已确认不属于当前已定义类别的其他目标。~%# 若无法识别类别，应使用 UNKNOWN。~%uint8 OTHER=254~%~%# 非法、解析失败或接口不支持的类别~%# 正常感知结果不应输出该值~%uint8 INVALID=255~%~%~%# 目标语义类别~%uint8 label~%~%# 当前类别的分类概率，范围 [0.0, 1.0]~%# 不等同于目标存在概率 existence_probability~%float32 probability~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:message-definition ((type (cl:eql 'ObjectClassification)))
  "Returns full string definition for message of type 'ObjectClassification"
  (cl:format cl:nil "# ObjectClassification.msg~%#~%# 类别编号按类别族分段预留。~%# 已发布的类别编号不得改变，新增类别应使用对应预留区间。~%#~%# 0       ：未知类别~%# 10~~29   ：道路车辆~%# 30~~49   ：两轮或三轮车辆~%# 50~~69   ：行人~%# 70~~89   ：动物~%# 90~~109  ：交通设施及危险物~%# 110~~253 ：后续类别预留~%# 254     ：其他类别~%# 255     ：非法或不支持的类别~%~%~%# 无法可靠识别类别，但目标仍可能真实存在，需要正常避障~%uint8 UNKNOWN=0~%~%~%# ============================================================~%# 道路车辆：10~~29~%# ============================================================~%~%# 小型乘用车辆，如轿车、SUV、MPV~%uint8 CAR=10~%~%# 货运车辆，如轻卡、厢式货车、重型卡车~%uint8 TRUCK=11~%~%# 载客大型车辆，如公交车、大巴、中巴~%uint8 BUS=12~%~%# 被牵引的挂车或拖车部分~%uint8 TRAILER=13~%~%# 14~~29 预留给其他道路车辆~%# 例如：VAN、FORKLIFT、CONSTRUCTION_VEHICLE~%~%~%# ============================================================~%# 两轮或三轮车辆：30~~49~%# ============================================================~%~%# 摩托车、电动摩托车等机动车辆~%uint8 MOTORCYCLE=30~%~%# 普通脚踏自行车~%uint8 BICYCLE=31~%~%# 电动自行车，不包括电动摩托车~%uint8 ELECTRIC_BICYCLE=32~%~%# 33~~49 预留~%# 例如：TRICYCLE~%~%~%# ============================================================~%# 人：50~~69~%# ============================================================~%~%# 普通人(无法进一步区分的人)。~%uint8 PEDESTRIAN=50~%~%#儿童~%uint8 CHILD=51~%~%#轮椅用户，需要更大的横向安全距离，且运动学模型与正常行人不同~%uint8 WHEELCHAIR=52~%~%# ============================================================~%# 动物：70~~89~%# ============================================================~%# 狗、猫等动物~%uint8 ANIMAL=70~%~%~%# ============================================================~%# 交通设施及危险物：90~~109~%# ============================================================~%~%# 交通锥、锥桶~%uint8 TRAFFIC_CONE=90~%~%# 闸机横杆~%uint8 BOOM_BARRIER=91~%~%# 门/闸机，此类障碍物的状态是可变的（开/关）~%uint8 DOOR_GATE=92~%~%# 93~~109 预留~%# 例如：BARRIER、BOLLARD、ROADBLOCK、DEBRIS~%~%~%# ============================================================~%# 特殊值~%# ============================================================~%# 已确认不属于当前已定义类别的其他目标。~%# 若无法识别类别，应使用 UNKNOWN。~%uint8 OTHER=254~%~%# 非法、解析失败或接口不支持的类别~%# 正常感知结果不应输出该值~%uint8 INVALID=255~%~%~%# 目标语义类别~%uint8 label~%~%# 当前类别的分类概率，范围 [0.0, 1.0]~%# 不等同于目标存在概率 existence_probability~%float32 probability~%~%~%~%"))
(cl:defmethod roslisp-msg-protocol:serialization-length ((msg <ObjectClassification>))
  (cl:+ 0
     1
     4
))
(cl:defmethod roslisp-msg-protocol:ros-message-to-list ((msg <ObjectClassification>))
  "Converts a ROS message object to a list"
  (cl:list 'ObjectClassification
    (cl:cons ':label (label msg))
    (cl:cons ':probability (probability msg))
))

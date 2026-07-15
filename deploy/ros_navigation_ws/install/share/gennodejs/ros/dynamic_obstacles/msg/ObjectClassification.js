// Auto-generated. Do not edit!

// (in-package dynamic_obstacles.msg)


"use strict";

const _serializer = _ros_msg_utils.Serialize;
const _arraySerializer = _serializer.Array;
const _deserializer = _ros_msg_utils.Deserialize;
const _arrayDeserializer = _deserializer.Array;
const _finder = _ros_msg_utils.Find;
const _getByteLength = _ros_msg_utils.getByteLength;

//-----------------------------------------------------------

class ObjectClassification {
  constructor(initObj={}) {
    if (initObj === null) {
      // initObj === null is a special case for deserialization where we don't initialize fields
      this.label = null;
      this.probability = null;
    }
    else {
      if (initObj.hasOwnProperty('label')) {
        this.label = initObj.label
      }
      else {
        this.label = 0;
      }
      if (initObj.hasOwnProperty('probability')) {
        this.probability = initObj.probability
      }
      else {
        this.probability = 0.0;
      }
    }
  }

  static serialize(obj, buffer, bufferOffset) {
    // Serializes a message object of type ObjectClassification
    // Serialize message field [label]
    bufferOffset = _serializer.uint8(obj.label, buffer, bufferOffset);
    // Serialize message field [probability]
    bufferOffset = _serializer.float32(obj.probability, buffer, bufferOffset);
    return bufferOffset;
  }

  static deserialize(buffer, bufferOffset=[0]) {
    //deserializes a message object of type ObjectClassification
    let len;
    let data = new ObjectClassification(null);
    // Deserialize message field [label]
    data.label = _deserializer.uint8(buffer, bufferOffset);
    // Deserialize message field [probability]
    data.probability = _deserializer.float32(buffer, bufferOffset);
    return data;
  }

  static getMessageSize(object) {
    return 5;
  }

  static datatype() {
    // Returns string type for a message object
    return 'dynamic_obstacles/ObjectClassification';
  }

  static md5sum() {
    //Returns md5sum for a message object
    return 'b67f0ffe50a7c47e827671307d1e4a55';
  }

  static messageDefinition() {
    // Returns full string definition for message
    return `
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
    
    
    `;
  }

  static Resolve(msg) {
    // deep-construct a valid message object instance of whatever was passed in
    if (typeof msg !== 'object' || msg === null) {
      msg = {};
    }
    const resolved = new ObjectClassification(null);
    if (msg.label !== undefined) {
      resolved.label = msg.label;
    }
    else {
      resolved.label = 0
    }

    if (msg.probability !== undefined) {
      resolved.probability = msg.probability;
    }
    else {
      resolved.probability = 0.0
    }

    return resolved;
    }
};

// Constants for message
ObjectClassification.Constants = {
  UNKNOWN: 0,
  CAR: 10,
  TRUCK: 11,
  BUS: 12,
  TRAILER: 13,
  MOTORCYCLE: 30,
  BICYCLE: 31,
  ELECTRIC_BICYCLE: 32,
  PEDESTRIAN: 50,
  CHILD: 51,
  WHEELCHAIR: 52,
  ANIMAL: 70,
  TRAFFIC_CONE: 90,
  BOOM_BARRIER: 91,
  DOOR_GATE: 92,
  OTHER: 254,
  INVALID: 255,
}

module.exports = ObjectClassification;

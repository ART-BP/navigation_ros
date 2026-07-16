;; Auto-generated. Do not edit!


(when (boundp 'dynamic_obstacles::ObjectClassification)
  (if (not (find-package "DYNAMIC_OBSTACLES"))
    (make-package "DYNAMIC_OBSTACLES"))
  (shadow 'ObjectClassification (find-package "DYNAMIC_OBSTACLES")))
(unless (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION")
  (make-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))

(in-package "ROS")
;;//! \htmlinclude ObjectClassification.msg.html


(intern "*UNKNOWN*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*UNKNOWN* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*UNKNOWN* 0)
(intern "*CAR*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*CAR* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*CAR* 10)
(intern "*TRUCK*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*TRUCK* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*TRUCK* 11)
(intern "*BUS*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*BUS* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*BUS* 12)
(intern "*TRAILER*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*TRAILER* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*TRAILER* 13)
(intern "*MOTORCYCLE*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*MOTORCYCLE* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*MOTORCYCLE* 30)
(intern "*BICYCLE*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*BICYCLE* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*BICYCLE* 31)
(intern "*ELECTRIC_BICYCLE*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*ELECTRIC_BICYCLE* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*ELECTRIC_BICYCLE* 32)
(intern "*PEDESTRIAN*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*PEDESTRIAN* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*PEDESTRIAN* 50)
(intern "*CHILD*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*CHILD* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*CHILD* 51)
(intern "*WHEELCHAIR*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*WHEELCHAIR* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*WHEELCHAIR* 52)
(intern "*ANIMAL*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*ANIMAL* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*ANIMAL* 70)
(intern "*TRAFFIC_CONE*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*TRAFFIC_CONE* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*TRAFFIC_CONE* 90)
(intern "*BOOM_BARRIER*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*BOOM_BARRIER* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*BOOM_BARRIER* 91)
(intern "*DOOR_GATE*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*DOOR_GATE* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*DOOR_GATE* 92)
(intern "*OTHER*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*OTHER* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*OTHER* 254)
(intern "*INVALID*" (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(shadow '*INVALID* (find-package "DYNAMIC_OBSTACLES::OBJECTCLASSIFICATION"))
(defconstant dynamic_obstacles::ObjectClassification::*INVALID* 255)

(defun dynamic_obstacles::ObjectClassification-to-symbol (const)
  (cond
        ((= const 0) 'dynamic_obstacles::ObjectClassification::*UNKNOWN*)
        ((= const 10) 'dynamic_obstacles::ObjectClassification::*CAR*)
        ((= const 11) 'dynamic_obstacles::ObjectClassification::*TRUCK*)
        ((= const 12) 'dynamic_obstacles::ObjectClassification::*BUS*)
        ((= const 13) 'dynamic_obstacles::ObjectClassification::*TRAILER*)
        ((= const 30) 'dynamic_obstacles::ObjectClassification::*MOTORCYCLE*)
        ((= const 31) 'dynamic_obstacles::ObjectClassification::*BICYCLE*)
        ((= const 32) 'dynamic_obstacles::ObjectClassification::*ELECTRIC_BICYCLE*)
        ((= const 50) 'dynamic_obstacles::ObjectClassification::*PEDESTRIAN*)
        ((= const 51) 'dynamic_obstacles::ObjectClassification::*CHILD*)
        ((= const 52) 'dynamic_obstacles::ObjectClassification::*WHEELCHAIR*)
        ((= const 70) 'dynamic_obstacles::ObjectClassification::*ANIMAL*)
        ((= const 90) 'dynamic_obstacles::ObjectClassification::*TRAFFIC_CONE*)
        ((= const 91) 'dynamic_obstacles::ObjectClassification::*BOOM_BARRIER*)
        ((= const 92) 'dynamic_obstacles::ObjectClassification::*DOOR_GATE*)
        ((= const 254) 'dynamic_obstacles::ObjectClassification::*OTHER*)
        ((= const 255) 'dynamic_obstacles::ObjectClassification::*INVALID*)
        (t nil)))

(defclass dynamic_obstacles::ObjectClassification
  :super ros::object
  :slots (_label _probability ))

(defmethod dynamic_obstacles::ObjectClassification
  (:init
   (&key
    ((:label __label) 0)
    ((:probability __probability) 0.0)
    )
   (send-super :init)
   (setq _label (round __label))
   (setq _probability (float __probability))
   self)
  (:label
   (&optional __label)
   (if __label (setq _label __label)) _label)
  (:probability
   (&optional __probability)
   (if __probability (setq _probability __probability)) _probability)
  (:serialization-length
   ()
   (+
    ;; uint8 _label
    1
    ;; float32 _probability
    4
    ))
  (:serialize
   (&optional strm)
   (let ((s (if strm strm
              (make-string-output-stream (send self :serialization-length)))))
     ;; uint8 _label
       (write-byte _label s)
     ;; float32 _probability
       (sys::poke _probability (send s :buffer) (send s :count) :float) (incf (stream-count s) 4)
     ;;
     (if (null strm) (get-output-stream-string s))))
  (:deserialize
   (buf &optional (ptr- 0))
   ;; uint8 _label
     (setq _label (sys::peek buf ptr- :char)) (incf ptr- 1)
   ;; float32 _probability
     (setq _probability (sys::peek buf ptr- :float)) (incf ptr- 4)
   ;;
   self)
  )

(setf (get dynamic_obstacles::ObjectClassification :md5sum-) "b67f0ffe50a7c47e827671307d1e4a55")
(setf (get dynamic_obstacles::ObjectClassification :datatype-) "dynamic_obstacles/ObjectClassification")
(setf (get dynamic_obstacles::ObjectClassification :definition-)
      "# ObjectClassification.msg
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


")



(provide :dynamic_obstacles/ObjectClassification "b67f0ffe50a7c47e827671307d1e4a55")



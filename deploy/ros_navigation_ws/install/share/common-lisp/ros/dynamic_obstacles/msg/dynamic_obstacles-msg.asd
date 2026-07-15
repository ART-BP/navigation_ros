
(cl:in-package :asdf)

(defsystem "dynamic_obstacles-msg"
  :depends-on (:roslisp-msg-protocol :roslisp-utils :geometry_msgs-msg
               :std_msgs-msg
               :uuid_msgs-msg
)
  :components ((:file "_package")
    (:file "ObjectClassification" :depends-on ("_package_ObjectClassification"))
    (:file "_package_ObjectClassification" :depends-on ("_package"))
    (:file "TrackedObject" :depends-on ("_package_TrackedObject"))
    (:file "_package_TrackedObject" :depends-on ("_package"))
    (:file "TrackedObjectArray" :depends-on ("_package_TrackedObjectArray"))
    (:file "_package_TrackedObjectArray" :depends-on ("_package"))
  ))
#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};


#[link(name = "robot_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__GpsFix() -> *const std::ffi::c_void;
}

#[link(name = "robot_msgs__rosidl_generator_c")]
extern "C" {
    fn robot_msgs__msg__GpsFix__init(msg: *mut GpsFix) -> bool;
    fn robot_msgs__msg__GpsFix__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<GpsFix>, size: usize) -> bool;
    fn robot_msgs__msg__GpsFix__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<GpsFix>);
    fn robot_msgs__msg__GpsFix__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<GpsFix>, out_seq: *mut rosidl_runtime_rs::Sequence<GpsFix>) -> bool;
}

// Corresponds to robot_msgs__msg__GpsFix
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// GPS 定位数据

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct GpsFix {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,

    /// 纬度 (deg)
    pub latitude: f64,

    /// 经度 (deg)
    pub longitude: f64,

    /// 海拔 (m)
    pub altitude: f32,

    /// m/s
    pub speed: f32,

    /// 航向角 (deg, 0~360)
    pub heading: f32,

    /// 0=no fix, 1=GPS, 2=DGPS, 4=RTK, 5=RTK float
    pub fix_type: u8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub satellites: u8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub hdop: f32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub vdop: f32,

}



impl Default for GpsFix {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !robot_msgs__msg__GpsFix__init(&mut msg as *mut _) {
        panic!("Call to robot_msgs__msg__GpsFix__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for GpsFix {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__GpsFix__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__GpsFix__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__GpsFix__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for GpsFix {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for GpsFix where Self: Sized {
  const TYPE_NAME: &'static str = "robot_msgs/msg/GpsFix";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__GpsFix() }
  }
}


#[link(name = "robot_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__BmsStatus() -> *const std::ffi::c_void;
}

#[link(name = "robot_msgs__rosidl_generator_c")]
extern "C" {
    fn robot_msgs__msg__BmsStatus__init(msg: *mut BmsStatus) -> bool;
    fn robot_msgs__msg__BmsStatus__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<BmsStatus>, size: usize) -> bool;
    fn robot_msgs__msg__BmsStatus__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<BmsStatus>);
    fn robot_msgs__msg__BmsStatus__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<BmsStatus>, out_seq: *mut rosidl_runtime_rs::Sequence<BmsStatus>) -> bool;
}

// Corresponds to robot_msgs__msg__BmsStatus
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// 电池管理系统状态

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct BmsStatus {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,

    /// 剩余电量 %
    pub soc: u8,

    /// mV
    pub voltage: i16,

    /// mA, 正=放电 负=充电
    pub current: i16,

    /// 0.1°C
    pub temp: i16,


    // This member is not documented.
    #[allow(missing_docs)]
    pub charging: bool,

    /// 0~100 %
    pub health: u8,

}



impl Default for BmsStatus {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !robot_msgs__msg__BmsStatus__init(&mut msg as *mut _) {
        panic!("Call to robot_msgs__msg__BmsStatus__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for BmsStatus {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__BmsStatus__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__BmsStatus__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__BmsStatus__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for BmsStatus {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for BmsStatus where Self: Sized {
  const TYPE_NAME: &'static str = "robot_msgs/msg/BmsStatus";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__BmsStatus() }
  }
}


#[link(name = "robot_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__OrderEvent() -> *const std::ffi::c_void;
}

#[link(name = "robot_msgs__rosidl_generator_c")]
extern "C" {
    fn robot_msgs__msg__OrderEvent__init(msg: *mut OrderEvent) -> bool;
    fn robot_msgs__msg__OrderEvent__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<OrderEvent>, size: usize) -> bool;
    fn robot_msgs__msg__OrderEvent__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<OrderEvent>);
    fn robot_msgs__msg__OrderEvent__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<OrderEvent>, out_seq: *mut rosidl_runtime_rs::Sequence<OrderEvent>) -> bool;
}

// Corresponds to robot_msgs__msg__OrderEvent
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// 订单事件（平板 → 车端）

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct OrderEvent {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,


    // This member is not documented.
    #[allow(missing_docs)]
    pub order_id: rosidl_runtime_rs::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub tablet_id: rosidl_runtime_rs::String,

    /// 1=START_DELIVERY, 2=PAUSE, 3=RESUME, 4=CANCEL, 5=ARRIVED
    pub type_: u8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub target_lat: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub target_lon: f64,

    /// 订单详情 JSON
    pub payload_json: rosidl_runtime_rs::String,

}



impl Default for OrderEvent {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !robot_msgs__msg__OrderEvent__init(&mut msg as *mut _) {
        panic!("Call to robot_msgs__msg__OrderEvent__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for OrderEvent {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__OrderEvent__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__OrderEvent__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__OrderEvent__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for OrderEvent {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for OrderEvent where Self: Sized {
  const TYPE_NAME: &'static str = "robot_msgs/msg/OrderEvent";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__OrderEvent() }
  }
}


#[link(name = "robot_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__Alarm() -> *const std::ffi::c_void;
}

#[link(name = "robot_msgs__rosidl_generator_c")]
extern "C" {
    fn robot_msgs__msg__Alarm__init(msg: *mut Alarm) -> bool;
    fn robot_msgs__msg__Alarm__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<Alarm>, size: usize) -> bool;
    fn robot_msgs__msg__Alarm__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<Alarm>);
    fn robot_msgs__msg__Alarm__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<Alarm>, out_seq: *mut rosidl_runtime_rs::Sequence<Alarm>) -> bool;
}

// Corresponds to robot_msgs__msg__Alarm
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// 告警消息

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct Alarm {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,

    /// 告警码
    pub alarm_code: u16,

    /// 1=INFO 2=WARN 3=ERROR 4=FATAL
    pub level: u8,

    /// 触发源 (e.g. "driver_bms")
    pub source: rosidl_runtime_rs::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub message: rosidl_runtime_rs::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub cleared: bool,

}



impl Default for Alarm {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !robot_msgs__msg__Alarm__init(&mut msg as *mut _) {
        panic!("Call to robot_msgs__msg__Alarm__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for Alarm {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__Alarm__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__Alarm__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__Alarm__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for Alarm {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for Alarm where Self: Sized {
  const TYPE_NAME: &'static str = "robot_msgs/msg/Alarm";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__Alarm() }
  }
}


#[link(name = "robot_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__ChassisCmd() -> *const std::ffi::c_void;
}

#[link(name = "robot_msgs__rosidl_generator_c")]
extern "C" {
    fn robot_msgs__msg__ChassisCmd__init(msg: *mut ChassisCmd) -> bool;
    fn robot_msgs__msg__ChassisCmd__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<ChassisCmd>, size: usize) -> bool;
    fn robot_msgs__msg__ChassisCmd__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<ChassisCmd>);
    fn robot_msgs__msg__ChassisCmd__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<ChassisCmd>, out_seq: *mut rosidl_runtime_rs::Sequence<ChassisCmd>) -> bool;
}

// Corresponds to robot_msgs__msg__ChassisCmd
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// 底盘速度指令

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct ChassisCmd {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,

    /// m/s
    pub linear_x: f32,

    /// m/s (麦轮可用)
    pub linear_y: f32,

    /// rad/s
    pub angular_z: f32,

    /// 0=STOP 1=FORWARD 2=REVERSE
    pub gear: u8,

}



impl Default for ChassisCmd {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !robot_msgs__msg__ChassisCmd__init(&mut msg as *mut _) {
        panic!("Call to robot_msgs__msg__ChassisCmd__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for ChassisCmd {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__ChassisCmd__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__ChassisCmd__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__ChassisCmd__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for ChassisCmd {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for ChassisCmd where Self: Sized {
  const TYPE_NAME: &'static str = "robot_msgs/msg/ChassisCmd";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__ChassisCmd() }
  }
}


#[link(name = "robot_msgs__rosidl_typesupport_c")]
extern "C" {
    fn rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__ChassisState() -> *const std::ffi::c_void;
}

#[link(name = "robot_msgs__rosidl_generator_c")]
extern "C" {
    fn robot_msgs__msg__ChassisState__init(msg: *mut ChassisState) -> bool;
    fn robot_msgs__msg__ChassisState__Sequence__init(seq: *mut rosidl_runtime_rs::Sequence<ChassisState>, size: usize) -> bool;
    fn robot_msgs__msg__ChassisState__Sequence__fini(seq: *mut rosidl_runtime_rs::Sequence<ChassisState>);
    fn robot_msgs__msg__ChassisState__Sequence__copy(in_seq: &rosidl_runtime_rs::Sequence<ChassisState>, out_seq: *mut rosidl_runtime_rs::Sequence<ChassisState>) -> bool;
}

// Corresponds to robot_msgs__msg__ChassisState
#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]

/// 底盘反馈

#[repr(C)]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct ChassisState {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::rmw::Header,


    // This member is not documented.
    #[allow(missing_docs)]
    pub odom_x: f32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub odom_y: f32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub odom_theta: f32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub vel_linear: f32,


    // This member is not documented.
    #[allow(missing_docs)]
    pub vel_angular: f32,

    /// 0=IDLE 1=RUNNING 2=ERROR
    pub state: u8,

    /// cm
    pub ultrasonic_front: u8,

    /// cm
    pub ultrasonic_rear: u8,

}



impl Default for ChassisState {
  fn default() -> Self {
    unsafe {
      let mut msg = std::mem::zeroed();
      if !robot_msgs__msg__ChassisState__init(&mut msg as *mut _) {
        panic!("Call to robot_msgs__msg__ChassisState__init() failed");
      }
      msg
    }
  }
}

impl rosidl_runtime_rs::SequenceAlloc for ChassisState {
  fn sequence_init(seq: &mut rosidl_runtime_rs::Sequence<Self>, size: usize) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__ChassisState__Sequence__init(seq as *mut _, size) }
  }
  fn sequence_fini(seq: &mut rosidl_runtime_rs::Sequence<Self>) {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__ChassisState__Sequence__fini(seq as *mut _) }
  }
  fn sequence_copy(in_seq: &rosidl_runtime_rs::Sequence<Self>, out_seq: &mut rosidl_runtime_rs::Sequence<Self>) -> bool {
    // SAFETY: This is safe since the pointer is guaranteed to be valid/initialized.
    unsafe { robot_msgs__msg__ChassisState__Sequence__copy(in_seq, out_seq as *mut _) }
  }
}

impl rosidl_runtime_rs::Message for ChassisState {
  type RmwMsg = Self;
  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> { msg_cow }
  fn from_rmw_message(msg: Self::RmwMsg) -> Self { msg }
}

impl rosidl_runtime_rs::RmwMessage for ChassisState where Self: Sized {
  const TYPE_NAME: &'static str = "robot_msgs/msg/ChassisState";
  fn get_type_support() -> *const std::ffi::c_void {
    // SAFETY: No preconditions for this function.
    unsafe { rosidl_typesupport_c__get_message_type_support_handle__robot_msgs__msg__ChassisState() }
  }
}



#[cfg(feature = "serde")]
use serde::{Deserialize, Serialize};



// Corresponds to robot_msgs__msg__GpsFix
/// GPS 定位数据

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct GpsFix {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,

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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::GpsFix::default())
  }
}

impl rosidl_runtime_rs::Message for GpsFix {
  type RmwMsg = super::msg::rmw::GpsFix;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        latitude: msg.latitude,
        longitude: msg.longitude,
        altitude: msg.altitude,
        speed: msg.speed,
        heading: msg.heading,
        fix_type: msg.fix_type,
        satellites: msg.satellites,
        hdop: msg.hdop,
        vdop: msg.vdop,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
      latitude: msg.latitude,
      longitude: msg.longitude,
      altitude: msg.altitude,
      speed: msg.speed,
      heading: msg.heading,
      fix_type: msg.fix_type,
      satellites: msg.satellites,
      hdop: msg.hdop,
      vdop: msg.vdop,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      latitude: msg.latitude,
      longitude: msg.longitude,
      altitude: msg.altitude,
      speed: msg.speed,
      heading: msg.heading,
      fix_type: msg.fix_type,
      satellites: msg.satellites,
      hdop: msg.hdop,
      vdop: msg.vdop,
    }
  }
}


// Corresponds to robot_msgs__msg__BmsStatus
/// 电池管理系统状态

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct BmsStatus {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,

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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::BmsStatus::default())
  }
}

impl rosidl_runtime_rs::Message for BmsStatus {
  type RmwMsg = super::msg::rmw::BmsStatus;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        soc: msg.soc,
        voltage: msg.voltage,
        current: msg.current,
        temp: msg.temp,
        charging: msg.charging,
        health: msg.health,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
      soc: msg.soc,
      voltage: msg.voltage,
      current: msg.current,
      temp: msg.temp,
      charging: msg.charging,
      health: msg.health,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      soc: msg.soc,
      voltage: msg.voltage,
      current: msg.current,
      temp: msg.temp,
      charging: msg.charging,
      health: msg.health,
    }
  }
}


// Corresponds to robot_msgs__msg__OrderEvent
/// 订单事件（平板 → 车端）

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct OrderEvent {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,


    // This member is not documented.
    #[allow(missing_docs)]
    pub order_id: std::string::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub tablet_id: std::string::String,

    /// 1=START_DELIVERY, 2=PAUSE, 3=RESUME, 4=CANCEL, 5=ARRIVED
    pub type_: u8,


    // This member is not documented.
    #[allow(missing_docs)]
    pub target_lat: f64,


    // This member is not documented.
    #[allow(missing_docs)]
    pub target_lon: f64,

    /// 订单详情 JSON
    pub payload_json: std::string::String,

}



impl Default for OrderEvent {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::OrderEvent::default())
  }
}

impl rosidl_runtime_rs::Message for OrderEvent {
  type RmwMsg = super::msg::rmw::OrderEvent;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        order_id: msg.order_id.as_str().into(),
        tablet_id: msg.tablet_id.as_str().into(),
        type_: msg.type_,
        target_lat: msg.target_lat,
        target_lon: msg.target_lon,
        payload_json: msg.payload_json.as_str().into(),
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
        order_id: msg.order_id.as_str().into(),
        tablet_id: msg.tablet_id.as_str().into(),
      type_: msg.type_,
      target_lat: msg.target_lat,
      target_lon: msg.target_lon,
        payload_json: msg.payload_json.as_str().into(),
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      order_id: msg.order_id.to_string(),
      tablet_id: msg.tablet_id.to_string(),
      type_: msg.type_,
      target_lat: msg.target_lat,
      target_lon: msg.target_lon,
      payload_json: msg.payload_json.to_string(),
    }
  }
}


// Corresponds to robot_msgs__msg__Alarm
/// 告警消息

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct Alarm {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,

    /// 告警码
    pub alarm_code: u16,

    /// 1=INFO 2=WARN 3=ERROR 4=FATAL
    pub level: u8,

    /// 触发源 (e.g. "driver_bms")
    pub source: std::string::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub message: std::string::String,


    // This member is not documented.
    #[allow(missing_docs)]
    pub cleared: bool,

}



impl Default for Alarm {
  fn default() -> Self {
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::Alarm::default())
  }
}

impl rosidl_runtime_rs::Message for Alarm {
  type RmwMsg = super::msg::rmw::Alarm;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        alarm_code: msg.alarm_code,
        level: msg.level,
        source: msg.source.as_str().into(),
        message: msg.message.as_str().into(),
        cleared: msg.cleared,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
      alarm_code: msg.alarm_code,
      level: msg.level,
        source: msg.source.as_str().into(),
        message: msg.message.as_str().into(),
      cleared: msg.cleared,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      alarm_code: msg.alarm_code,
      level: msg.level,
      source: msg.source.to_string(),
      message: msg.message.to_string(),
      cleared: msg.cleared,
    }
  }
}


// Corresponds to robot_msgs__msg__ChassisCmd
/// 底盘速度指令

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct ChassisCmd {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,

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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::ChassisCmd::default())
  }
}

impl rosidl_runtime_rs::Message for ChassisCmd {
  type RmwMsg = super::msg::rmw::ChassisCmd;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        linear_x: msg.linear_x,
        linear_y: msg.linear_y,
        angular_z: msg.angular_z,
        gear: msg.gear,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
      linear_x: msg.linear_x,
      linear_y: msg.linear_y,
      angular_z: msg.angular_z,
      gear: msg.gear,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      linear_x: msg.linear_x,
      linear_y: msg.linear_y,
      angular_z: msg.angular_z,
      gear: msg.gear,
    }
  }
}


// Corresponds to robot_msgs__msg__ChassisState
/// 底盘反馈

#[cfg_attr(feature = "serde", derive(Deserialize, Serialize))]
#[derive(Clone, Debug, PartialEq, PartialOrd)]
pub struct ChassisState {

    // This member is not documented.
    #[allow(missing_docs)]
    pub header: std_msgs::msg::Header,


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
    <Self as rosidl_runtime_rs::Message>::from_rmw_message(super::msg::rmw::ChassisState::default())
  }
}

impl rosidl_runtime_rs::Message for ChassisState {
  type RmwMsg = super::msg::rmw::ChassisState;

  fn into_rmw_message(msg_cow: std::borrow::Cow<'_, Self>) -> std::borrow::Cow<'_, Self::RmwMsg> {
    match msg_cow {
      std::borrow::Cow::Owned(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Owned(msg.header)).into_owned(),
        odom_x: msg.odom_x,
        odom_y: msg.odom_y,
        odom_theta: msg.odom_theta,
        vel_linear: msg.vel_linear,
        vel_angular: msg.vel_angular,
        state: msg.state,
        ultrasonic_front: msg.ultrasonic_front,
        ultrasonic_rear: msg.ultrasonic_rear,
      }),
      std::borrow::Cow::Borrowed(msg) => std::borrow::Cow::Owned(Self::RmwMsg {
        header: std_msgs::msg::Header::into_rmw_message(std::borrow::Cow::Borrowed(&msg.header)).into_owned(),
      odom_x: msg.odom_x,
      odom_y: msg.odom_y,
      odom_theta: msg.odom_theta,
      vel_linear: msg.vel_linear,
      vel_angular: msg.vel_angular,
      state: msg.state,
      ultrasonic_front: msg.ultrasonic_front,
      ultrasonic_rear: msg.ultrasonic_rear,
      })
    }
  }

  fn from_rmw_message(msg: Self::RmwMsg) -> Self {
    Self {
      header: std_msgs::msg::Header::from_rmw_message(msg.header),
      odom_x: msg.odom_x,
      odom_y: msg.odom_y,
      odom_theta: msg.odom_theta,
      vel_linear: msg.vel_linear,
      vel_angular: msg.vel_angular,
      state: msg.state,
      ultrasonic_front: msg.ultrasonic_front,
      ultrasonic_rear: msg.ultrasonic_rear,
    }
  }
}



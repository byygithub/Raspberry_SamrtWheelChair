Page({
  data: {
    Direction_Control: "S",
    deviceProperties: null,
    gps: { lat: "22.543100", lng: "113.940205" },
    heartRate: "78"
  },

  config: {
    authorization: "version=2018-10-31&res=products%2FH9At1TTBP4%2Fdevices%2Fclient&et=1900800000&method=md5&sign=CjzWJ2qR1SBv1wEqdS83dw%3D%3D",
    product_id: "H9At1TTBP4",
    device_name: "client",
    setUrl: "https://iot-api.heclouds.com/thingmodel/set-device-property",
    getUrl: "https://iot-api.heclouds.com/thingmodel/query-device-property?product_id=H9At1TTBP4&device_name=client"
  },

  sendDirection(direction) {
    wx.request({
      url: this.config.setUrl,
      method: "POST",
      header: {
        "authorization": this.config.authorization,
        "Content-Type": "application/json"
      },
      data: {
        product_id: this.config.product_id,
        device_name: this.config.device_name,
        params: {
          Direction_Control: direction
        }
      },
      success: (res) => {
        console.log("📤 指令下发结果：", res.data);
        if (res.data.code === 0) {
          wx.showToast({ title: "指令已发送：" + direction, icon: "success" });
          this.setData({ Direction_Control: direction });
        } else {
          wx.showToast({ title: "失败：" + res.data.msg, icon: "none" });
        }
      },
      fail: (err) => {
        console.error(err);
        wx.showToast({ title: "网络异常", icon: "none" });
      }
    });
  },

  onForward() { this.sendDirection("F"); },
  onBackward() { this.sendDirection("B"); },
  onLeft() { this.sendDirection("L"); },
  onRight() { this.sendDirection("R"); },
  onStop() { this.sendDirection("S"); },

  onAccelerate() { 
    wx.showToast({ title: "加速 +", icon: "none" });
    this.sendDirection("A"); 
  },
  onDecelerate() { 
    wx.showToast({ title: "减速 -", icon: "none" });
    this.sendDirection("D"); 
  },

  getDeviceStatus() {
    wx.request({
      url: this.config.getUrl,
      header: { authorization: this.config.authorization },
      success: (res) => {
        console.log("📥 平台最新属性：", res.data);
        if (res.data.code === 0) {
          const list = res.data.data;
          this.setData({ deviceProperties: list });
          const dir = list.find(i => i.identifier === "Direction_Control");
          if (dir) this.setData({ Direction_Control: dir.value });
        }
      }
    });
  },

  onLoad() {
    this.getDeviceStatus();
  }
});
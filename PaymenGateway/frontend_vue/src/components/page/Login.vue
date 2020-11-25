<template>
  <div class="login-wrap">
    <div class="ms-login">
      <!-- <div class="ms-title">第三方后台管理系统</div> -->
      <el-form
        :model="searchForm"
        :rules="rules"
        ref="searchForm"
        label-width="0px"
        class="ms-content"
        v-show="!firstLogin"
      >
        <el-form-item prop="pro_id">
          <el-input v-model="searchForm.pro_id" placeholder="请输入账号">
            <i slot="prepend" class="el-icon-s-custom"></i>
          </el-input>
        </el-form-item>
        <el-form-item prop="tel_no">
          <el-input v-model="searchForm.tel_no" placeholder="手机号" v-interger="11" autocomplete="new-password">
            <i slot="prepend" class="el-icon-mobile-phone"></i>
          </el-input>
        </el-form-item>
        <el-form-item prop="password">
          <el-input
            type="password"
            placeholder="请输入密码"
            v-model="searchForm.password"
            autocomplete="new-password"
            @keyup.enter.native="submitForm('searchForm')"
          >
            <i slot="prepend" class="el-icon-lock"></i>
          </el-input>
        </el-form-item>
        <el-form-item>
          <el-input
            placeholder="请输入谷歌验证码"
            v-model="searchForm.code"
            v-interger="6"
            @keyup.enter.native="submitForm('searchForm')"
          >
            <i slot="prepend" class="el-icon-key"></i>
          </el-input>
        </el-form-item>
        <div class="login-btn">
          <el-button type="primary" @click="submitForm('searchForm')">登录</el-button>
        </div>
      </el-form>
      <el-form v-show="firstLogin" class="ms-content" style="color: #ffffff" :model="aragin" :rules="rules2" ref="again">
        <div>请绑定谷歌验证后，输入验证码继续登录</div>
        <div id="qrcode" class="qr-code"></div>
        <div style="text-align:center">秘钥：{{googleKey}}</div>
        <el-form-item prop="code">
          <el-input
            style="margin-top: 20px"
            placeholder="请输入谷歌验证码"
            v-model="aragin.code"
            v-interger="6"
            @keyup.enter.native="submitForm('searchForm')"
          >
            <i slot="prepend" class="el-icon-key"></i>
          </el-input>
        </el-form-item>
        <div class="login-btn" style="margin-top: 30px">
          <el-button type="primary" @click="againLogin">登录</el-button>
        </div>
      </el-form>
    </div>
  </div>
</template>

<script>
import { setLocalStorageLogin, toMd5 } from "@/utils";
import md5 from 'js-md5'
import QRCode from "qrcodejs2";
let qrCode
export default {
  data() {
    let validatePass = (rule, value, callback) => {
      if (!/^[0-9A-Za-z]{8,16}$/.test(value)) {
        callback(new Error("密码为8-16位字母数字组合"));
      }
      if (value === "") {
        callback(new Error("请输入密码"));
      } else {
        if (this.searchForm.password !== "") {
          this.$refs.searchForm.validateField("checkPass");
        }
        callback();
      }
    };
    let validateTel = (rule, value, callback) => {
      if (!/^1[3456789]\d{9}$/.test(value)) {
        callback(new Error("手机号格式不正确"));
      }
      if (value === "") {
        callback(new Error("请输入手机号"));
      } else {
        if (this.searchForm.password !== "") {
          this.$refs.searchForm.validateField("checkPass");
        }
        callback();
      }
    };
    return {
      setLocalStorageLogin,
      QRCode,
      firstLogin: false,
      gcode: "",
      googleKey: "",
      searchForm: {
        pro_id: "",
        tel_no: "",
        password: "",
        code: ""
      },
      aragin: {
        code: ""
      },
      rules: {
        pro_id: [{ required: true, message: "请输入项目方名称", trigger: "blur" }],
        tel_no: [{ validator: validateTel, required: true, trigger: "blur" }],
        password: [{ validator: validatePass, required: true, trigger: "blur" }]
      },
      rules2: {
        code: [{ required: true, message: "请输入谷歌验证码", trigger: "blur" }]
      }
    };
  },
  mounted() {
    this.getqrCode()
  },
  methods: {
    getqrCode( ) {
      qrCode = new QRCode("qrcode", {
        width: 120,
        height: 120,
        text: this.gcode
      });
    },
    submitForm(formName) {
      let params = {
        ...this.searchForm
      }
      if (!this.searchForm.code) {
        delete params.code
      }
      params.password = toMd5(params.password)
      console.log(params.password)
      this.$refs[formName].validate(async valid => {
        if (valid) {
          const { status, data } = await this.$axios.post(
            "/client/login/",
            params
          );
          if (status === 200) {
            if (data.status === 200) {
              if (data.code === "success") {
                this.setLocalStorageLogin(data.data);
                // 存入刷新token的时间戳
                let refreshTime = new Date().getTime() + 59 * 60 * 1000 
                sessionStorage.refreshTime = refreshTime
                this.$router.push("/myinfor");
              } else {
                this.$msg('请绑定谷歌验证后，重新登录', "error");
                this.gcode = "otpauth://totp/shabao?secret=" + data.data.gcode;
                this.googleKey = data.data.gcode
                this.searchForm.code = ''
                this.$nextTick(() => {
                  qrCode.makeCode(this.gcode)
                });
                this.firstLogin = true;
              }
            }
          } else {
            this.$msg(data.data.detail, "error");
          }
        } else {
          return false;
        }
      });
    },
    againLogin () {
      this.$refs["again"].validate(async valid => {
        if (valid) {
          this.searchForm.code = this.aragin.code
          this.submitForm('searchForm')
        }
      })
    }
  }
};
</script>

<style scoped lang="scss">
.login-wrap {
  position: relative;
  width: 100%;
  height: 100%;
  background: url("../../assets/img/login-bg.jpg") no-repeat center bottom;
  background-size: 100% auto;
  background-color: rgba(4, 9, 38, 1);
}
/*     .ms-title{
    width:100%;
    line-height: 50px;
    text-align: center;
    font-size:20px;
    color: #fff;
    border-bottom: 1px solid #ddd;
} */
.logo-cont {
  position: absolute;
  left: 80px;
  top: 80px;
  width: 200px;
  height: 100px;
  .logo {
    width: 100%;
    height: 80px;
    margin-bottom: 20px;
    background: url("../../assets/img/logo.png") no-repeat left center;
    background-size: auto 100%;
  }
  .des {
    color: #fff;
  }
}
.ms-login {
  position: absolute;
  left: 50%;
  top: 50%;
  width: 350px;
  margin: -190px 0 0 -175px;
  border-radius: 5px;
  background: rgba(255, 255, 255, 0.3);
  overflow: hidden;
}
.ms-content {
  padding: 30px 30px;
}
.login-btn {
  text-align: center;
}
.login-btn button {
  width: 100%;
  height: 36px;
  /* margin-bottom: 10px; */
}
.login-tips {
  font-size: 12px;
  line-height: 30px;
  color: #fff;
}
.qr-code {
  width: 120px;
  height: 120px;
  margin: 10px auto;
  padding: 10px;
  background: #ffffff;
}
</style>
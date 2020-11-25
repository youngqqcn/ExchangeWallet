<template>
  <div class="wrapper">
    <v-head></v-head>
    <v-sidebar></v-sidebar>
    <div class="content-box" :class="{'content-collapse':collapse}">
      <v-tags></v-tags>
      <div class="content">
        <transition name="move" mode="out-in">
          <!-- <keep-alive :include="tagsList"> -->
          <router-view></router-view>
          <!-- </keep-alive> -->
        </transition>
        <el-dialog title="修改密码（每天只能修改一次）" :center="true" :visible.sync="changePwdshow">
          <el-form
            :model="form"
            label-width="100px"
            ref="changeform"
            label-position="left"
            :rules="rules"
          >
            <el-form-item label="新密码:" prop="new_password">
              <el-input type="password" v-model="form.new_password" autocomplete="off"></el-input>
            </el-form-item>
            <el-form-item label="确认密码:" prop="password_verify">
              <el-input type="password" v-model="form.password_verify" autocomplete="off"></el-input>
            </el-form-item>
            <!-- <el-form-item label="账号:" prop="pro_id">
              <el-input v-model="form.pro_id" autocomplete="off"></el-input>
            </el-form-item> -->
            <el-form-item label="手机号:" prop="tel_no">
              <el-input v-model="form.tel_no" autocomplete="off" v-interger="11"></el-input>
            </el-form-item>
            <el-form-item label="原密码:" prop="password">
              <el-input type="password" v-model="form.password" autocomplete="off"></el-input>
            </el-form-item>
            <el-form-item label="谷歌验证码:" prop="code">
              <el-input v-model="form.code" autocomplete="off" v-interger="6"></el-input>
            </el-form-item>
          </el-form>
          <div slot="footer" class="dialog-footer">
            <el-button @click="changePwdshow = false">取 消</el-button>
            <el-button type="primary" @click="modifyPwd">确 定</el-button>
          </div>
        </el-dialog>
      </div>
    </div>
  </div>
</template>

<script>
import vHead from "./Header.vue";
import vSidebar from "./Sidebar.vue";
import vTags from "./Tags.vue";
import bus from "./bus";
import { removeLocalStorageLogin, toMd5, setLocalStorageLogin, logout } from "@/utils";
export default {
  data() {
    let reg = /^[0-9A-Za-z]{8,16}$/;
    let validatePass = (rule, value, callback) => {
      if (!reg.test(value)) {
        callback(new Error("密码为8-16位字母数字组合"));
      }
      if (value === "") {
        callback(new Error("请输入新密码"));
      } else {
        callback();
      }
    };
    let validatePass2 = (rule, value, callback) => {
      if (!reg.test(value)) {
        callback(new Error("密码为8-16位字母数字组合"));
      }
      if (value === "") {
        callback(new Error("请再次新输入密码"));
      } else if (value !== this.form.new_password) {
        callback(new Error("两次输入密码不一致!"));
      } else {
        callback();
      }
    };
    let validatePass3 = (rule, value, callback) => {
      if (!reg.test(value)) {
        callback(new Error("密码为8-16位字母数字组合"));
      }
      if (value === "") {
        callback(new Error("请输入原密码"));
      } else {
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
        callback();
      }
    };
    return {
      removeLocalStorageLogin,
      setLocalStorageLogin,
      tagsList: [],
      collapse: false,
      changePwdshow: false,
      form: {
        new_password: "",
        password_verify: "",
        // pro_id: "",
        tel_no: "",
        password: "",
        code: ""
      },
      rules: {
        new_password: [
          { validator: validatePass, required: true, trigger: "blur" }
        ],
        password_verify: [
          { validator: validatePass2, required: true, trigger: "blur" }
        ],
        // pro_id: [{ required: true, message: "请输入项目方名称", trigger: "blur" }],
        tel_no: [{ validator: validateTel, required: true, trigger: "blur" }],
        password: [
          { validator: validatePass3, required: true, trigger: "blur" }
        ],
        code: [{ required: true, message: "请输入谷歌验证码", trigger: "blur" }]
      },
      timer: ''
    };
  },
  components: {
    vHead,
    vSidebar,
    vTags
  },
  watch: {
    changePwdshow(v) {
      if (!v) {
        this.$refs.changeform.resetFields();
      }
    }
  },
  created() {
    bus.$on("collapse", msg => {
      this.collapse = msg;
    });

    bus.$on("changePwd", msg => {
      this.changePwdshow = msg;
    });

    // 只有在标签页列表里的页面才使用keep-alive，即关闭标签之后就不保存到内存中了。
    bus.$on("tags", msg => {
      let arr = [];
      for (let i = 0, len = msg.length; i < len; i++) {
        msg[i].name && arr.push(msg[i].name);
      }
      this.tagsList = arr;
    });
    // this.refresh(localStorage.token);
    this.timer = setInterval(() => {
      console.log('执行定时器')
      if (new Date().getTime() > sessionStorage.refreshTime) {
        console.log('执行刷新token')
        this.refresh(localStorage.token);
      }
    }, 30 * 1000)
  },
  beforeDestroy () {
    clearInterval(this.timer)
  },
  methods: {
    async refresh (token) {
      console.log('刷新token')
      const { status, data } = await this.$axios.post("/client/refresh/", {token})
      if (status === 200) {
        this.setLocalStorageLogin(data.data);
        let refreshTime = new Date().getTime() + 59 * 60 * 1000
        sessionStorage.refreshTime = refreshTime
      }
    },
    modifyPwd() {
      this.$refs["changeform"].validate(valid => {
        if (valid) {
          this.$confirm("确定修改登录密码?", "提示", {
            confirmButtonText: "确定",
            cancelButtonText: "取消",
            type: "warning",
            center: true
          })
            .then(async () => {
              let params = {
                ... this.form
              }
              params.new_password = toMd5(this.form.new_password)
              params.password_verify = toMd5(this.form.password_verify)
              params.password = toMd5(this.form.password)
              const { status, data } = await this.$axios.put("/client/reset/", {
                ...params
              });
              if (status === 200) {
                this.$msg(data.code);
                // logout();
                removeLocalStorageLogin();
                this.$router.push("/login");
              } else {
                this.$msg(data.code, "error");
              }
            })
            .catch(() => {
              // this.changePwdshow = false
            });
        }
      });
    }
  }
};
</script>

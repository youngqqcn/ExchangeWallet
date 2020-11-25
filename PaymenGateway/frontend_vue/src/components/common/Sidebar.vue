<template>
  <div class="sidebar">
    <el-menu
      class="sidebar-el-menu"
      :default-active="onRoutes"
      :collapse="collapse"
      background-color="#324157"
      text-color="#bfcbd9"
      active-text-color="#20a0ff"
      unique-opened
      router
    >
      <template v-for="item in items">
        <template v-if="item.subs">
          <el-submenu :index="item.index" :key="item.index">
            <template slot="title">
              <i :class="item.icon"></i>
              <span slot="title">{{ item.title }}</span>
            </template>
            <template v-for="subItem in item.subs">
              <el-submenu v-if="subItem.subs" :index="subItem.index" :key="subItem.index">
                <template slot="title">{{ subItem.title }}</template>
                <el-menu-item
                  v-for="(threeItem,i) in subItem.subs"
                  :key="i"
                  :index="threeItem.index"
                >{{ threeItem.title }}</el-menu-item>
              </el-submenu>
              <el-menu-item v-else :index="subItem.index" :key="subItem.index">{{ subItem.title }}</el-menu-item>
            </template>
          </el-submenu>
        </template>
        <template v-else>
          <el-menu-item :index="item.index" :key="item.index">
            <i :class="item.icon"></i>
            <span slot="title">{{ item.title }}</span>
          </el-menu-item>
        </template>
      </template>
    </el-menu>
  </div>
</template>

<script>
import bus from "../common/bus";

export default {
  data() {
    return {
      collapse: false,
      items: [
        // {
        //   icon: "el-icon-lx-home",
        //   index: "dashboard",
        //   title: "系统首页"
        // },
        {
          icon: "el-icon-lx-cascades",
          index: "myinfor",
          title: "我的信息"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "address_management",
          title: "地址管理"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "user_assets",
          title: "用户资产"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "recharge_record",
          title: "充值记录"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "collection_record",
          title: "归集记录"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "withdrawal_record",
          title: "提币记录"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "assetdaily_report",
          title: "资产日报表"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "c2c_order",
          title: "法币订单"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "c2c_recharge_record",
          title: "法币充值记录"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "c2c_withdrawal_record",
          title: "法币提币记录"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "c2c_change_report",
          title: "法币交易日报表"
        },
        {
          icon: "el-icon-lx-cascades",
          index: "user_log",
          title: "用户操作日志"
        }
      ]
    };
  },
  computed: {
    onRoutes() {
      return this.$route.path.replace("/", "");
    }
  },
  created() {
    // 通过 Event Bus 进行组件间通信，来折叠侧边栏
    bus.$on("collapse", msg => {
      this.collapse = msg;
    });
    // let menuList = [
    //   {
    //     icon: 'el-icon-lx-home',
    //     index: 'member_list',
    //     title: '会员列表'
    //   },
    //   {
    //     icon: 'el-icon-lx-cascades',
    //     index: 'member_import_audit',
    //     title: '会员导入'
    //   },
    //   {
    //     icon: 'el-icon-lx-copy',
    //     index: 'name_config',
    //     title: '会员名称配置'
    //   },
    //   {
    //     icon: 'el-icon-lx-calendar',
    //     index: 'unlock_strategy',
    //     title: '会员解锁策略列表'
    //   },
    //   {
    //     icon: 'el-icon-lx-favor',
    //     index: 'unlock_record',
    //     title: '资产解锁记录'
    //   },
    //   {
    //     icon: 'el-icon-lx-warn',
    //     index: 'lock_record',
    //     title: '资产锁定记录'
    //   },
    //   {
    //     icon: 'el-icon-rank',
    //     index: 'manual_check',
    //     title: '解锁人工审核'
    //   },
    //   {
    //     icon: 'el-icon-lx-warn',
    //     index: 'manual_check_record',
    //     title: '解锁人工审核记录'
    //   },
    //   {
    //     icon: 'el-icon-lx-emoji',
    //     index: 'invitation_detail',
    //     title: '解锁邀请详情'
    //   },
    //   {
    //     icon: 'el-icon-lx-warn',
    //     index: 'bibi_unlockRecord',
    //     title: '币币交易解锁'
    //   },
    //   {
    //     icon: 'el-icon-lx-warn',
    //     index: 'fabi_unlockRecord',
    //     title: '法币交易解锁'
    //   },
    //   {
    //     icon: 'el-icon-lx-home',
    //     index: 'high_list',
    //     title: '高级认证解锁'
    //   }
    // ]
    // if(localStorage.username.toLocaleUpperCase().indexOf('SPQQ') === -1){
    //   this.items = menuList
    // }
  }
};
</script>

<style scoped>
.sidebar {
  display: block;
  position: absolute;
  left: 0;
  top: 70px;
  bottom: 0;
  overflow-y: scroll;
}

.sidebar::-webkit-scrollbar {
  width: 0;
}

.sidebar-el-menu:not(.el-menu--collapse) {
  width: 250px;
}

.sidebar > ul {
  height: 100%;
}
</style>

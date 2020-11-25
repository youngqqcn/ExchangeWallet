import Vue from 'vue';
import Router from 'vue-router';

const originalPush = Router.prototype.push
Router.prototype.push = function push (location) {
  return originalPush.call(this, location).catch(err => err)
}

Vue.use(Router);

export default new Router({
  routes: [
    {
      path: '/',
      redirect: '/login'
    },
    {
      path: '/',
      component: resolve => require(['../components/common/Home.vue'], resolve),
      meta: { title: 'home' },
      children:[
        // {
        //   path: '/dashboard',
        //   component: resolve => require(['../components/page/dashboard.vue'], resolve),
        //   meta: { title: '主页' }
        // },
        {
          path: '/myinfor',
          component: resolve => require(['../components/ClientManage/myInformation.vue'], resolve),
          meta: { title: '我的信息' }
        },
        {
          path: '/address_management',
          component: resolve => require(['../components/ClientManage/addressManagement.vue'], resolve),
          meta: { title: '地址管理' }
        },
        {
          path: '/subaddress_query',
          component: resolve => require(['../components/ClientManage/subaddressQuery.vue'], resolve),
          meta: { title: '子地址查询' }
        },
        {
          path: '/user_assets',
          component: resolve => require(['../components/ClientManage/userAssets.vue'], resolve),
          meta: { title: '用户资产' }
        },
        {
          path: '/assets_detail',
          component: resolve => require(['../components/ClientManage/assetsDetail.vue'], resolve),
          meta: { title: '资产详情' }
        },
        {
          path: '/recharge_record',
          component: resolve => require(['../components/ClientManage/rechargeRecord.vue'], resolve),
          meta: { title: '充值记录' }
        },
        {
          path: '/collection_record',
          component: resolve => require(['../components/ClientManage/collectionRecord.vue'], resolve),
          meta: { title: '归集记录' }
        },
        {
          path: '/withdrawal_record',
          component: resolve => require(['../components/ClientManage/withdrawalRecord.vue'], resolve),
          meta: { title: '提币记录' }
        },
        {
          path: '/assetdaily_report',
          component: resolve => require(['../components/ClientManage/assetdailyReport.vue'], resolve),
          meta: { title: '资产日报表' }
        },
        {
          path: '/c2c_order',
          component: resolve => require(['../components/C2cManage/C2cOrder.vue'], resolve),
          meta: { title: '法币订单' }
        },
        {
          path: '/c2c_recharge_record',
          component: resolve => require(['../components/C2cManage/C2cRechargeRecord.vue'], resolve),
          meta: { title: '法币充值记录' }
        },
        {
          path: '/c2c_withdrawal_record',
          component: resolve => require(['../components/C2cManage/C2cWithdrawalRecord.vue'], resolve),
          meta: { title: '法币提币记录' }
        },
        {
          path: '/c2c_change_report',
          component: resolve => require(['../components/C2cManage/C2cChangeReport.vue'], resolve),
          meta: { title: '法币交易日报表' }
        },
        {
          path: '/user_log',
          component: resolve => require(['../components/ClientManage/userLog.vue'], resolve),
          meta: { title: '用户操作日志' }
        },
        {
          path: '/404',
          component: resolve => require(['../components/page/404.vue'], resolve),
          meta: { title: '404' }
        },
        {
          path: '/403',
          component: resolve => require(['../components/page/403.vue'], resolve),
          meta: { title: '403' }
        }
      ]
    },
    {
      path: '/login',
      component: resolve => require(['../components/page/Login.vue'], resolve)
    },
    {
      path: '*',
      redirect: '/404'
    }
  ]
})

import Vue from 'vue';
import App from './App';
import router from './router';
import axios from 'axios';
import store from './utils/store';
import config from '@/config'
import $excel from './excel/outExcel'
import ElementUI from 'element-ui';
import { removeLocalStorageLogin, logout } from '@/utils'
import './utils/directive.main.js' // 指令
import 'element-ui/lib/theme-chalk/index.css';    // 默认主题
// import '../static/css/theme-green/index.css';       // 浅绿色主题
import HCont from './components/common/HCont.vue'
import HitContainer from '@/components/common/HitContainer';
import '../static/css/icon.css';
import "babel-polyfill";

Vue.use(ElementUI, { size: 'small' });
Vue.use($excel, {})
Vue.component('h-cont', HCont)
Vue.component('hit-container', HitContainer)
Vue.prototype.$msg = (msg = '', type = 'success', center = true) => {
  Vue.prototype.$message({
    type: type,
    center: center,
    message: msg
  })
}

axios.defaults.baseURL = config.apiPrefix
// axios.defaults.withCredentials = true
// axios.defaults.baseURL = "/api"

axios.interceptors.request.use(config => {
  config.headers['Authorization'] = localStorage.token ? 'JWT ' + localStorage.token : ''
  // config.headers['xsrfCookieName'] = 'XMLHttpRequest'
  // const regex = /.*csrftoken=([^;.]).*$/
  // config.headers['X-CSRFToken'] = document.cookie.match(regex) === null ? null : document.cookie
  return config
}, err => {
  return Promise.reject(err)
})

axios.interceptors.response.use(res => {
  return {data: res.data, status: res.status}
}, (err) => {
  if (err.response.status === 401) {
    removeLocalStorageLogin()
    router.push("/login")
    // logout()
  }
  if (err.response.status === 429) {
    Vue.prototype.$msg('请求超过了限速。', 'error')
  }
  return Promise.resolve({data: err.response.data, status: err.response.status})
})

// axios.defaults.withCredentials = true
Vue.prototype.$axios = axios // 掛到vue

router.beforeEach((to, from, next) => {
  // console.log('beforeEach', to, from)
  const role = localStorage.token ? 1 : 0
  let path = 'login'
  if (role) { // 已登录
    if (to.path === '/login') {
      return next(`/myinfor`)
    }
    next()
  } else { // 未登录
    path = 'login'
    if (to.path !== '/login') {
      // console.log(path)
      return next(`/${path}`)
    }
    next()
  }
})

const trigger = (el, type) => {
  const e = document.createEvent('HTMLEvents')
  e.initEvent(type, true, true)
  el.dispatchEvent(e)
}
Vue.directive('decimal', { // 小数精度，默认8位小数精度，证书精度默认9位，可传入配置小数位、这数位精度, 非必选，传入格式为 '3|4' 3位整数位长度，4为小数位精度，'5' 只有一个就为小数位精度
  bind: (el, binding) => { // 动态精度配置，使用data-float配置小数位精度，使用data-int配置整数位精度
    // console.log(binding, el.getElementsByTagName('input')[0])
    el = el.getElementsByTagName('input')[0]
    let tempArr = []
    let lengDec = 8 // 小数位默认8位长度
    let lengInt = 9 // 整数位默认9位长度
    if (binding.expression) tempArr = binding.expression.split('|')
    if (tempArr.length === 1) lengDec = tempArr[0] * 1 // 只传一个数字长度就为小数位精度
    if (tempArr.length === 2) {
      lengInt = tempArr[0]
      lengDec = tempArr[1]
    }
    el.oninput = () => {
      let dataset = el.dataset
      if (dataset.float) lengDec = dataset.float // 配置了动态精度则取用动态精度
      if (dataset.int) lengInt = dataset.int
      let regDec = new RegExp('^([1-9]\\d*|0)(\\.\\d{1,' + lengDec + '})(\\.|\\d{1})?$')
      let regInt = new RegExp('^(\\d{1,' + lengInt + '})(\\d*)(\\.\\d*)?$')
      // 可防止回删小数点不能输入的问题
      // console.log(el.value)
      el.value = el.value.replace(/[^\d.。]/g, '').replace(/。/g, '.').replace(/^\./, '0').replace(/\.{2}/g, '.').replace(/^0\d{1}/g, '0').replace(regDec, '$1$2').replace(regInt, '$1$3')
    }
    el.onkeyup = () => {
      trigger(el, 'input') // 解决value不更新的问题
    }
  }
})

new Vue({
  router,
  store,
  render: h => h(App)
}).$mount('#app');

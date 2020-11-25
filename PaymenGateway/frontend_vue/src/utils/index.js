import md5 from 'js-md5'
import Vue from 'vue'
import router from '../router';
import axios from 'axios'
import config from '@/config'

// 设置session的登录信息
export const setLocalStorageLogin = data => {
  localStorage.token = data.token
  localStorage.username = data.username
  localStorage.pro_id = data.pro_id
  localStorage.tel_no = data.tel_no
  localStorage.email = data.email
}

// 删掉session的登录信息
export const removeLocalStorageLogin = () => {
  localStorage.removeItem('token')
  localStorage.removeItem('username')
  localStorage.removeItem('pro_id')
  localStorage.removeItem('tel_no')
  localStorage.removeItem('email')
}

// 处理时间格式
const add0 = (time) => { return time < 10 ? '0' + time : time }

export const handleTimeFormat = (timestamp, type) => {
  if (!timestamp) return ''
  timestamp = timestamp * 1
  // if(!type==='st'){
  // }
  let date = new Date(timestamp)
  let y = date.getFullYear()
  let m = date.getMonth() + 1
  let d = date.getDate()
  let h = date.getHours()
  let mm = date.getMinutes()
  let s = date.getSeconds()
  if (type === 'date') {
    return add0(y) + '-' + add0(m) + '-' + add0(d)
  } else if (type === 'datetime') {
    return add0(y) + '-' + add0(m) + '-' + add0(d) + ' ' + add0(h) + ':' + add0(mm) + ':' + add0(s)
  } else {
    return add0(y) + '-' + add0(m) + '-' + add0(d) + ' ' + add0(h) + ':' + add0(mm) + ':' + add0(s)
  }
}

export const toMd5 = (value) => {
  let hash = md5.create()
  hash.update(value)
  return hash.hex()
}

export const logout = async () => {
  const { status, data } = await axios.post(config.apiPrefix + '/client/loginout/', {})
  if (status === 200) {
    setTimeout(() => {
      removeLocalStorageLogin()
      router.push('/login')
    }, 1000);
  }
}

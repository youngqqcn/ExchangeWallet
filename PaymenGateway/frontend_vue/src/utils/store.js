import Vue from 'vue'
import Vuex from 'vuex'
// console.log(process.env.NODE_ENV)
const localData = JSON.parse(localStorage.getItem('state')) || {} 
// console.log(localData)
const state = {
  uid: null,
  token: null,
  sliders: localData['sildeBars'] || [], // 左侧导航栏
  authUrls: localData['authUrls'] || [] // 个人权限地址
}

const mutations = {
  changeState(state, dataObj) {
    state[dataObj.name] = dataObj.value
    localStorage.setItem('state', JSON.stringify(state))
  }
}

const getters = {
  newValue(state) {
    return state.value + 1
  }
}

const actions = {
  test({ commit, state }, value) {
    commit('test', 1)
  },
  async getData({ commit, state }) {
    const { errCode, errMsg, data } = await Vue.prototype.$axios.post('utl', {
      value: state.value,
    })
    if (errCode) {
      // Vue.prototype.$layer.msg(errMsg)
    } else {
      commit('changeState', {
        name: '',
        value: ''
      })
    }
    return { errCode, errMsg, data }
  }
}

Vue.use(Vuex)

let store
export default store = new Vuex.Store({
  state,
  mutations,
  actions,
  getters
})

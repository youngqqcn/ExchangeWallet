import Vue from 'vue'

let reg = /[\-\_\,\！\!\@\|\~\`\(\)\#\$\%\^\&\*\{\}\:\;\"\<\>\?\！\￥\（\）\—\+\。\’\；\、\“\”\：\》\,\=\】\【\‘\’\[\]\'\.\/\\]/g

const getInputEl = (el) => { // 获取封装组件内要绑定的input
  return (el.nodeName).toLowerCase() === 'input' ? el : el.getElementsByTagName('input')[0]
}

const trigger = (el, type) => {
  const e = document.createEvent('HTMLEvents')
  e.initEvent(type, true, true)
  el.dispatchEvent(e)
}

Vue.directive('_num1', {
  bind: (el, binding, vnode) => {
    el.oninput = () => {
      let maxInteger = 8 // 最大整数
      let maxDecimals = 8 // 最大小数
      let bindVal = ''
      // console.log(binding)
      if (binding.value) {
        if (binding.value.rule) {
          maxInteger = binding.value.rule.split('|')[0]
          maxDecimals = binding.value.rule.split('|')[1]
        }
        bindVal = binding.value.val
      }
      let putEl = el.getElementsByTagName('input')[0]
      let val = putEl.value.replace(/\s+/g, '')
      let el1 = val
      if (!((val * 1) > -1)) {
        while (!((el1 * 1) > -1) && el1.length) {
          el1 = el1.substring(0, el1.length - 1)
        }
        putEl.value = el1.substring(0, maxInteger)
        eval(`(vnode.context.${bindVal}='${putEl.value}')`)
        return
      } else {
        putEl.value = el1.substring(0, maxInteger)
        eval(`(vnode.context.${bindVal}='${putEl.value}')`)
      }
      if (el1.indexOf('.') !== -1) {
        let integer = el1.split('.')[0].substring(0, maxInteger)
        let decimals = el1.split('.')[1].substring(0, maxDecimals)
        putEl.value = (integer + '.' + decimals)
        // console.log(putEl.value)
        eval(`(vnode.context.${bindVal}='${putEl.value}')`)
      }
    }
  }
})

Vue.directive('_rate1', {
  bind: (el, binding, vnode) => {
    el.oninput = () => {
      let putEl = el.getElementsByTagName('input')[0]
      let val = (putEl.value + '').replace(/\s+/g, '')
      let el1 = val
      if (!((val * 1) > -1)) {
        while (!((el1 * 1) > -1) && el1.length) {
          el1 = el1.substring(0, el1.length - 1)
        }
        putEl.value = el1
      } else {
        if (el1 * 1 > 100) {
          putEl.value = 100
          eval(`(vnode.context.${binding.expression}=100)`)
          return
        }
      }
      if (el1.indexOf('.') !== -1) {
        let integer = el1.split('.')[0]
        let decimals = el1.split('.')[1].substring(0, 2)
        if (((integer + '.' + decimals) * 1) > 100) {
          putEl.value = 100
          eval(`(vnode.context.${binding.expression}=100)`)
          return
        }
        let finalValue = integer + '.' + decimals
        putEl.value = finalValue
        eval(`(vnode.context.${binding.expression}='${finalValue}')`)
      } else {
        putEl.value = el1
      }
    }
  }
})

Vue.directive('g_code', {
  bind: (el, binding) => {
    el.oninput = () => {
      let max = binding.value
      let putEl = el.getElementsByTagName('input')[0]
      let val = (putEl.value + '').replace(/\s+/g, '')
      putEl.value = val.replace(/[^0-9]+/g, '').substring(0, max)
    }
  }
})

Vue.directive('noSymbol', {
  bind: (el, binding, vnode) => {
    el.oninput = () => {
      let max = 20
      let bindVal = binding.value.val
      if(binding.value.max){
        max = binding.value.max
      }
      let putEl = el.getElementsByTagName('input')[0]
      let val = putEl.value
      putEl.value = val.replace(reg, '').replace(/\s+/g, '').substring(0, max)
      eval(`(vnode.context.${bindVal}='${putEl.value}')`)
    }
  }
})


Vue.directive('onlyNum', {
  bind: (el, binding, vnode) => {
    el.oninput = () => {
      let max = 30
      let bindVal = ''
      // console.log(binding)
      if (binding.value) {
        if (binding.value.max) {
          max = binding.value.max * 1
        }
        bindVal = binding.value.val
      }
      let putEl = el.getElementsByTagName('input')[0]
      let val = putEl.value.replace(/\s+/g, '').replace(/[.]/g, '')
      let el1 = val
      if (!((val * 1) > 0)) {
        while (!((el1 * 1) > -1) && el1.length) {
          el1 = el1.substring(0, el1.length - 1)
        }
        putEl.value = el1.substring(0, max)
        eval(`(vnode.context.${bindVal}='${putEl.value}')`)
      } else {
        putEl.value = el1.substring(0, max)
        eval(`(vnode.context.${bindVal}='${putEl.value}')`)
      }
    }
  }
})

Vue.directive('onlyNumLetter', {
  bind: (el, binding, vnode) => {
    el.oninput = () => {
      // console.log(binding)
      let bindVal = binding.value
      let putEl = el.getElementsByTagName('input')[0]
      putEl.value = putEl.value.replace(/[^\w\.]/ig,'')
      eval(`(vnode.context.${bindVal}='${putEl.value}')`)
    }
  }
})

Vue.directive('decimal', { // 小数精度，默认8位小数精度，证书精度默认9位，可传入配置小数位、这数位精度, 非必选，传入格式为 '3|4' 3位整数位长度，4为小数位精度，'5' 只有一个就为小数位精度
  bind: (el, binding) => { // 动态精度配置，使用data-float配置小数位精度，使用data-int配置整数位精度
    // console.log(binding, el.dataset, el.value)
    el = getInputEl(el)
    let tempArr = []
    let lengDec = 8 // 小数位默认8位长度
    let lengInt = 9 // 整数位默认9位长度
    let max = 0 // 默认最大值为0
    if (binding.expression) tempArr = binding.expression.split('|')
    if (tempArr.length === 1) lengDec = tempArr[0] * 1 // 只传一个数字长度就为小数位精度
    if (tempArr.length === 2) {
      lengInt = tempArr[0]
      lengDec = tempArr[1]
    }
    let flag = true
    el.addEventListener('compositionstart', () => { // 事件触发于文字的输入之前
      flag = false
    })
    el.addEventListener('compositionend', () => { // 事件触发于文字的输入组成完成或取消时
      flag = true
    })
    el.oninput = () => {
      if (flag) {
        let dataset = el.dataset
        if (dataset.max) { // 最大值限制
          max = dataset.max
          if (el.value * 1 >= max) {
            el.value = max
          }
        }
        if (dataset.float) lengDec = dataset.float // 配置了动态精度则取用动态精度
        if (dataset.int) lengInt = dataset.int
        let regDec = new RegExp('^([1-9]\\d*|0)(\\.\\d{1,' + lengDec + '})(\\.|\\d{1})?$')
        let regInt = new RegExp('^(\\d{1,' + lengInt + '})(\\d*)(\\.\\d*)?$')
        // 可防止回删小数点不能输入的问题
        // console.log(el.value)
        el.value = el.value.replace(/[^\d.。]/g, '').replace(/。/g, '.').replace(/^\./, '0').replace(/\.{2}/g, '.').replace(/^0\d{1}/g, '0').replace(regDec, '$1$2').replace(regInt, '$1$3')
      }
    }
    el.onkeyup = () => {
      trigger(el, 'input') // 解决value不更新的问题
    }
  }
})

Vue.directive('interger', { // 正整数，验证码，可配置长度限制，默认6位
  bind: (el, binding) => {
    el = getInputEl(el)
    let leng = binding.value || 6
    el.oninput = () => {
      el.value = el.value.replace(/\D/g, '').substring(0, leng)
    }
    el.onkeyup = () => {
      trigger(el, 'input')
    }
  }
})

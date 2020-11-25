let apiPrefix = ''
if (process.env.NODE_ENV === 'development') {
  apiPrefix = 'http://192.168.10.174:59333' // sit
  // apiPrefix = 'https://test.shbao.me:8777' // uat
  // apiPrefix = 'http://192.168.10.231' // pro
} else {
  console.log('正在打包：', process.env.type)
  switch (process.env.type) {
    case 'sit':
      apiPrefix = 'http://192.168.10.174:59333' // sit
      break
    case 'dev2':
      // apiPrefix = 'http://192.168.10.53:9101/' // dev
      break
    case 'uat':
      apiPrefix = 'https://test.shbao.me:8777' // uat
      break
    case 'pro':
      apiPrefix = 'http://192.168.10.231' // pro_hetbicom
      break
    default:
      apiPrefix = 'http://192.168.10.52:9101/' // sit
      break
  }
  console.log(process.env.type, 'pro', apiPrefix)
}

let pageSizes = [15, 30, 45, 60]
let limit = 10
let pageSize = 15

export default {
  apiPrefix,
  pageSizes,
  limit,
  pageSize
}

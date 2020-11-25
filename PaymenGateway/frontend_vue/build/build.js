'use strict'
require('./check-versions')()

process.env.NODE_ENV = 'production'

const ora = require('ora')
const rm = require('rimraf')
const path = require('path')
const chalk = require('chalk')
const webpack = require('webpack')
const config = require('../config')
const webpackConfig = require('./webpack.prod.conf')
const client = require('scp2')

const spinner = ora('building for production...')
spinner.start()

rm(path.join(config.build.assetsRoot, config.build.assetsSubDirectory), err => {
  if (err) throw err
  webpack(webpackConfig, (err, stats) => {
    spinner.stop()
    if (err) throw err
    process.stdout.write(stats.toString({
      colors: true,
      modules: false,
      children: false, // If you are using ts-loader, setting this to true will make TypeScript errors show up during build.
      chunks: false,
      chunkModules: false
    }) + '\n\n')

    if (stats.hasErrors()) {
      console.log(chalk.red('  Build failed with errors.\n'))
      process.exit(1)
    }

    console.log(chalk.cyan('  Build complete.\n'))
    console.log(chalk.yellow(
      '  Tip: built files are meant to be served over an HTTP server.\n' +
      '  Opening index.html over file:// won\'t work.\n'
    ))
    console.log(chalk.green(process.env.type + ' 环境打包完毕!'))
    if (process.env.NODE_ENV === 'production') {
      if (process.env.type === '"sit"') { // 测试环境
        console.log(chalk.green(process.env.type + ' 开始上传!'))
        client.scp('dist/', { // 打包后上传服务器
          host: '192.168.10.174',
          username: 'root',
          password: '123456',
          path: '/data/www/'
        }, function (err) {
          if (err) {
            return console.log(err)
          }
          console.log(chalk.green('Finished,' + process.env.type + ' 上传成功!'))
        })
      } else if (process.env.type === '"dev2"') {
        console.log(chalk.green(process.env.type + ' 开始上传!'))
        client.scp('dist/', { // 打包后上传服务器
          host: '192.168.10.22',
          username: 'root',
          password: 'qq1007059411',
          path: '/root/static/dev2-htdf-third-admin/'
        }, function (err) {
          if (err) {
            return console.log(err)
          }
          console.log(chalk.green('Finished,' + process.env.type + ' 上传成功!'))
        })
      }
    }
  })
})

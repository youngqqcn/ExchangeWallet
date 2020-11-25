<template>
    <div id="app">
        <router-view></router-view>
    </div>
</template>
<style>
    @import "../static/css/main.css";
    @import "../static/css/color-dark.css";     /*深色主题*/
    /*@import "../static/css/theme-green/color-green.css";   浅绿色主题*/
    .content-cont {
      padding: 20px;
    }
    .total-cont {
      margin-bottom: 20px;
    }
    .el-pagination {
      padding: 20px 0;
      text-align: center;
    }
</style>
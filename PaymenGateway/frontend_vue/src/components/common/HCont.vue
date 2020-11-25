<template>
<div class="hit-container">
  <!-- 容器组件 -->
  <div class="title-cont" v-if="!hideTitle">
    <ul
      class="router-cont"
      v-if="routerList.length">
      <router-link
        v-for="(item, index) in routerList"
        :key="index"
        :class="[
          'router-item',
          {
            'cur-router': item.index === $route.path
          }
        ]"
        tag="li"
        :to="item.index">{{item.name}}</router-link>
    </ul>
    <div v-else>{{realTitle}}</div>
    <slot name="titleRight"></slot>
  </div>
  <slot></slot>
</div>
</template>

<script>
export default {
  data () {
    return {
      realTitle: ''
    }
  },
  props: {
    title: {
      type: String,
      required: false
    },
    hideTitle: {
      type: Boolean,
      default: false
    },
    routerList: {
      type: Array,
      default: function () {
        return []
      }
    }
  },
  created () {
    this.realTitle = this.title || this.$route.meta.title
  }
}
</script>

<style lang="scss">
.hit-container {
  border: 1px solid #dcdfe6;
  border-radius: 5px;
  overflow: hidden;
  background-color: #fff;
  .title-cont {
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 10px 20px;
    border-bottom: 1px solid #dcdfe6;
  }
  .router-cont {
    .router-item {
      float: left;
      list-style: none;
      height: 30px;
      margin-right: 30px;
      cursor: pointer;
    }
    &:last-child {
      margin-right: 0;
    }
    & .cur-router {
      position: relative;
      color: #409EFF;
      &::after {
        position: absolute;
        content: '';
        bottom: 0;
        left: 0;
        height: 2px;
        width: 100%;
        background-color: #409EFF;
      }
    }
  }
/*   .el-table {
    margin: 20px;
  } */
}
</style>

<template>
  <div class="user-assets-content">
    <hit-container>
      <div class="user-assets-box">
        <el-tabs type="border-card" @tab-click="chagneTabs" v-model="searchForm.type">
          <el-tab-pane label="币币资产" name="0"></el-tab-pane>
          <el-tab-pane label="法币资产" name="1"></el-tab-pane>
          <el-form :inline="true" :model="searchForm">
            <el-form-item>
              <el-input
                v-model="searchForm.search"
                @keydown.enter.native="getSearch"
                clearable
                placeholder="请输入币种进行搜索"
              ></el-input>
            </el-form-item>
            <el-form-item>
              <el-button type="primary" @click="getSearch">搜索</el-button>
            </el-form-item>
            <el-form-item>
              <el-button type="success" @click="exportTable">导出</el-button>
            </el-form-item>
          </el-form>
          <div v-show="fileUrl" style="padding: 10px 20px 0 20px;font-size: 14px;">文件已生成，如果浏览器没有自动下载，请 <a id="downEx" :download='getName()' style="color: red" :href="fileUrl">点击此处</a> 下载文件</div>
          <el-table border :data="userBalances" style="margin: 0px;width: 100%">
            <!-- <el-table-column prop="uid" label="用户UID" align="center"></el-table-column> -->
            <el-table-column prop="token_name" label="币种" align="center" :resizable="false" width="150"></el-table-column>
            <!-- <el-table-column prop="all_balance" label="数量" align="center"></el-table-column> -->
            <el-table-column prop="withdraw_address" label="出币地址" align="center" :resizable="false"></el-table-column>
            <el-table-column prop="withdraw_balance" label="出币地址当前余额" align="center" :resizable="false"></el-table-column>
            <el-table-column label="操作" align="center" :resizable="false">
              <template slot-scope="props">
                <el-button
                  type="text"
                  @click="$router.push({path: 'assets_detail', query: {token_name: props.row.token_name}})"
                >查看详情</el-button>
              </template>
            </el-table-column>
          </el-table>
          <el-pagination
            @current-change="handleCurrentChange"
            @size-change="handleSizeChange"
            :current-page.sync="searchForm.page"
            :page-sizes="[10, 50, 100, 500, 1000, 10000]"
            :page-size="searchForm.pageSize"
            layout="total, sizes, prev, pager, next, jumper"
            :total="total"
          ></el-pagination>
        </el-tabs>
      </div>
    </hit-container>
  </div>
</template>
<script>
export default {
  data() {
    return {
      userBalances: [],
      fileUrl: "",
      searchForm: {
        type: "0",
        search: "",
        page: 1,
        pageSize: 10
      },
      total: 0
    };
  },
  created () {
    this.handleSearch()
  },
  methods: {
    getSearch() {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    getName() {
      return `用户资产表-${new Date().getTime()}.xlsx`
    },
    chagneTabs () {
      console.log(11)
    },
    handleCurrentChange(page) {
      this.searchForm.page = page;
      this.handleSearch()
    },
    handleSizeChange(pageSize) {
      this.searchForm.pageSize = pageSize;
      this.handleSearch()
    },
    async handleSearch() {
      const { data, status } = await this.$axios({
        methods: "get",
        url: "/client/UserTokenBalances/",
        params: {
          page: this.searchForm.page,
          limit: this.searchForm.pageSize,
          search: this.searchForm.search.toLocaleUpperCase()
        }
      });
      if (status === 200) {
        this.userBalances = data.results;
        this.total = data.count;
      } else {
        this.$msg(data.detail, 'error')
      }
    },
    exportTable() {
      let outData = this.userBalances.map(item => {
        item = {
          "token_name": item.token_name,
          "all_balance": item.all_balance,
          "withdraw_address": item.withdraw_address,
          "withdraw_balance": item.withdraw_balance,
        }
        return item
      })
      let exData = new this.$excel(outData, {
        "token_name": "币种",
        "all_balance": "数量",
        "withdraw_address": "提币支出地址",
        "withdraw_balance": "提币地址持币数量"
      })
      this.fileUrl = exData.exInfo.fileUrl
      this.$nextTick(()=>{
        this.isLoading = false
        document.getElementById('downEx').click()
      })
    }
  }
};
</script>
<style lang="scss">
.user-assets-content {
  .user-assets-box {
    padding: 15px 20px;
  }
}
</style>

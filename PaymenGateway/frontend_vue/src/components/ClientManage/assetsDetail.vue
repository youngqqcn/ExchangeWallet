<template>
  <div class="subaddress-query-content">
    <hit-container title="子地址资产信息">
      <div class="subaddress-query-box">
        <el-form :inline="true" :model="searchForm">
          <el-form-item>
            <el-input v-model="searchForm.search" clearable @keydown.enter.native="getSearch"></el-input>
          </el-form-item>
          <el-form-item>
            <el-button type="primary" @click="getSearch">搜索</el-button>
          </el-form-item>
          <el-form-item>
            <el-button type="success" @click="exportTable">导出</el-button>
          </el-form-item>
        </el-form>
        <div v-show="fileUrl" style="padding: 10px 20px 0 20px;font-size: 14px;">文件已生成，如果浏览器没有自动下载，请 <a id="downEx" :download='getName()' style="color: red" :href="fileUrl">点击此处</a> 下载文件</div>
        <el-table border :data="UserAddressBalances" style="margin: 0px;width: 100%">
          <el-table-column type="index" label="序号" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="balance" label="数量" align="center" :resizable="false" width="200"></el-table-column>
          <el-table-column prop="address" label="地址" align="center" :resizable="false"></el-table-column>
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
      </div>
    </hit-container>
  </div>
</template>
<script>
export default {
  data() {
    return {
      uname: "",
      token_name: "",
      fileUrl: "",
      UserAddressBalances: [],
      searchForm: {
        search: "",
        page: 1,
        pageSize: 10
      },
      total: 30
    };
  },
  created() {
    this.searchForm.search = this.token_name = this.$route.query.token_name;
    this.uname = localStorage.username;
    this.handleSearch()
  },
  methods: {
    getSearch() {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    getName() {
      return `${this.token_name}资产信息表-${new Date().getTime()}.xlsx`
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
        url: "/client/UserAddressBalances/",
        params: {
          first_time: '',
          end_time: '',
          offset: (this.searchForm.page - 1) * this.searchForm.pageSize,
          limit: this.searchForm.pageSize,
          search: this.searchForm.search.toLocaleUpperCase()
        }
      });
      if (status === 200) {
        this.UserAddressBalances= data.results.map(item => {
          item.update_time = item.update_time.replace("T", " ")
          return item
        });
        this.total = data.count;
        this.token_name = this.searchForm.search.toLocaleUpperCase()
      } else {
        this.$msg(data.detail, 'error')
      }
    },
    exportTable() {
      let exData = new this.$excel(this.UserAddressBalances, {
        "token_name": "币种",
        "balance": "充值金额",
        "address": "源地址",
        "update_time": "更新时间"
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
.subaddress-query-content {
  .subaddress-query-box {
    padding: 15px 20px;
  }
}
</style>

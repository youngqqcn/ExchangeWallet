<template>
  <div class="subaddress-query-content">
    <hit-container>
      <div class="subaddress-query-box">
        <el-form :inline="true" :model="searchForm">
          <!-- <el-form-item label="客户名">
            <el-select v-model="searchForm.name" clearable>
              <el-option label="全部" value></el-option>
              <el-option label="链上科技" value="1"></el-option>
              <el-option label="火币" value="2"></el-option>
              <el-option label="火币2" value="2"></el-option>
            </el-select>
          </el-form-item>-->
          <el-form-item label="币种">
            <el-input v-model="searchForm.search" @keydown.enter.native="getSearch" clearable></el-input>
          </el-form-item>
          <el-form-item>
            <el-button type="primary" @click="getSearch">搜索</el-button>
          </el-form-item>
        </el-form>
        <el-table border :data="subaddressList" style="margin: 0px;width: 100%">
          <el-table-column type="index" label="序号" align="center" :resizable="false"></el-table-column>
          <el-table-column
            prop="token_name"
            label="币种"
            align="center"
            :resizable="false"
            width="100"
          ></el-table-column>
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
      subaddressList: [],
      searchForm: {
        search: "",
        page: 1,
        pageSize: 10
      },
      total: 0
    };
  },
  created() {
    this.searchForm.search = this.$route.query.token_name;
    this.handleSearch();
  },
  methods: {
    getSearch() {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    handleCurrentChange(page) {
      this.searchForm.page = page;
      this.handleSearch();
    },
    handleSizeChange(pageSize) {
      this.searchForm.pageSize = pageSize;
      this.handleSearch();
    },
    async handleSearch() {
      const { data, status } = await this.$axios({
        methods: "get",
        url: "/client/Subaddress/",
        params: {
          offset: (this.searchForm.page - 1) * this.searchForm.pageSize,
          first_time: "",
          end_time: "",
          limit: this.searchForm.pageSize,
          search: this.searchForm.search.toLocaleUpperCase()
        }
      });
      if (status === 200) {
        this.subaddressList = data.results;
        this.total = data.count;
      } else {
        this.$msg(data.detail, "error");
      }
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

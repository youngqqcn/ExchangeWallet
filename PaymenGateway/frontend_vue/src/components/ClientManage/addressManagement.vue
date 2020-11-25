<template>
  <div class="address-management-content">
    <hit-container>
      <div class="address-management-box">
        <el-form :inline="true" :model="searchForm">
          <el-form-item label="币种">
            <el-input v-model="searchForm.search" @keydown.enter.native="getSearch" placeholder="请输入币种进行搜索"></el-input>
          </el-form-item>
          <el-form-item>
            <el-button type="primary" @click="getSearch">搜索</el-button>
          </el-form-item>
        </el-form>
        <el-table border :data="addressList" style="margin: 0px;width: 100%">
          <el-table-column prop="token_name" label="币种" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="address_nums" label="地址总数" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="uncharged_address_nums" label="未充值地址数" align="center" :resizable="false"></el-table-column>
          <el-table-column label="操作" align="center" :resizable="false">
            <template slot-scope="props">
              <el-button
                type="text"
                @click="$router.push({path: 'subaddress_query', query: {token_name: props.row.token_name}})"
              >查看子地址</el-button>
            </template>
          </el-table-column>
        </el-table>
        <el-pagination
          @current-change="handleCurrentChange"
          @size-change="handleSizeChange"
          :current-page.sync="searchForm.page"
          :page-sizes="[10, 20, 50, 100]"
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
      addressList: [],
      searchForm: {
        search: "",
        page: 1,
        pageSize: 10
      },
      total: 0
    };
  },
  created() {
    this.handleSearch();
  },
  methods: {
    getSearch () {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    handleCurrentChange(page) {
      this.searchForm.page = page;
      this.handleSearch()
    },
    handleSizeChange(pageSize) {
      this.searchForm.pageSize= pageSize;
      this.handleSearch()
    },
    async handleSearch() {
      const { data, status } = await this.$axios({
        methods: "get",
        url: "/client/Address/",
        params: {
          page: this.searchForm.page,
          limit: this.searchForm.pageSize,
          search: this.searchForm.search.toLocaleUpperCase()
        }
      });
      if (status === 200) {
        this.addressList = data.results
        this.total = data.count
      } else {
        this.$msg(data.detail, 'error')
      }
    }
  }
};
</script>
<style lang="scss">
.address-management-content {
  .address-management-box {
    padding: 15px 20px;
  }
}
</style>

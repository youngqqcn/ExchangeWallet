<template>
  <div class="assetdaily-report-content">
    <hit-container>
      <div class="assetdaily-report-box">
        <el-form :inline="true" :model="searchForm">
          <el-form-item label="币种">
            <el-input type="text" v-model="searchForm.search" clearable placeholder="请输入币种进行搜索" @keydown.enter.native="getSearch"></el-input>
          </el-form-item>
          <el-form-item label="时间">
            <el-date-picker
              v-model="searchForm.first_time"
              type="date"
              format="yyyy-MM-dd"
              value-format="yyyy-MM-dd"
              default-time="00:00:00"
              placeholder="开始日期"
            ></el-date-picker>
            -
            <el-date-picker
              v-model="searchForm.end_time"
              type="date"
              format="yyyy-MM-dd"
              value-format="yyyy-MM-dd"
              default-time="00:00:00"
              placeholder="结束日期"
            ></el-date-picker>
          </el-form-item>
          <el-form-item>
            <el-button type="primary" @click="getSearch">搜索</el-button>
          </el-form-item>
          <el-form-item>
            <el-button type="success" @click="exportTable">导出</el-button>
          </el-form-item>
        </el-form>
        <div v-show="fileUrl" style="padding: 10px 20px 0 20px;font-size: 14px;">文件已生成，如果浏览器没有自动下载，请 <a id="downEx" :download='getName()' style="color: red" :href="fileUrl">点击此处</a> 下载文件</div>
        <el-table border :data="assetDailyList" style="margin: 0px;width: 100%">
          <el-table-column prop="token_name" label="币种" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="update_time" label="时间" align="center" :resizable="false">
            <!-- <template slot-scope="props">{{handleTimeFormat(props.row.update_time)}}</template> -->
          </el-table-column>
          <el-table-column prop="all_balance" label="子地址币总资产" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="deposit_amount" label="当日充值数量" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="withdraw_amount" label="当日提币数量" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="collectionRecords_amount" label="当日归集数量" align="center" :resizable="false"></el-table-column>
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
import { handleTimeFormat } from "@/utils";
export default {
  data() {
    return {
      handleTimeFormat,
      fileUrl: "",
      assetDailyList: [],
      searchForm: {
        search: "",
        first_time: "",
        end_time: "",
        page: 1,
        pageSize: 10
      },
      total: 30
    };
  },
  created() {
    this.handleSearch();
  },
  methods: {
    getSearch() {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    getName() {
      return `资产日报表-${new Date().getTime()}.xlsx`
    },
    handleCurrentChange(page) {
      this.searchForm.page = page;
      this.handleSearch();
    },
    handleSizeChange(pageSize) {
      this.searchForm.pageSize = pageSize;
    },
    async handleSearch() {
      let search = [];
      if (this.searchForm.token_name) {
        search.push(this.searchForm.token_name.toLocaleUpperCase());
      }
      if (this.searchForm.time) {
        search.push(this.searchForm.time);
      }
      const { data, status } = await this.$axios({
        methods: "get",
        url: "/client/AssetDailyReport/",
        params: {
          first_time: this.searchForm.first_time || '',
          end_time: this.searchForm.end_time || '',
          offset: (this.searchForm.page - 1) * this.searchForm.pageSize,
          limit: this.searchForm.pageSize,
          search: this.searchForm.search.toLocaleUpperCase()
        }
      });
      if (status === 200) {
        this.assetDailyList = data.results.map(item => {
          item.update_time = this.handleTimeFormat(new Date(item.update_time).getTime());
          return item;
        });
        this.total = data.count;
      } else {
        this.$msg(data.detail, 'error')
      }
    },
    exportTable() {
      let exData = new this.$excel(this.assetDailyList, {
        "token_name": "币种",
        "update_time": "时间",
        "all_balance": "资产",
        "deposit_amount": "当日充值数量",
        "withdraw_amount": "当日提币数量",
        "collectionRecords_amount": "当日归集数量"
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
.assetdaily-report-content {
  .assetdaily-report-box {
    padding: 15px 20px;
  }
}
</style>

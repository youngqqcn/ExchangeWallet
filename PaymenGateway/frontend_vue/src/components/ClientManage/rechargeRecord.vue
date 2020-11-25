<template>
  <div class="recharge-record-content">
    <hit-container>
      <div class="recharge-record-box">
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
              placeholder="开始日期"></el-date-picker>
            -
            <el-date-picker
              v-model="searchForm.end_time"
              type="date"
              format="yyyy-MM-dd"
              value-format="yyyy-MM-dd"
              default-time="00:00:00"
              placeholder="结束日期"></el-date-picker>
          </el-form-item>
          <el-form-item>
            <el-button type="primary" @click="getSearch">搜索</el-button>
          </el-form-item>
          <el-form-item>
            <el-button type="success" @click="exportTable">导出</el-button>
          </el-form-item>
        </el-form>
        <div v-show="fileUrl" style="padding: 10px 20px 0 20px;font-size: 14px;">文件已生成，如果浏览器没有自动下载，请 <a id="downEx" :download='getName()' style="color: red" :href="fileUrl">点击此处</a> 下载文件</div>
        <el-table border :data="depositList" style="margin: 0px;width: 100%">
          <el-table-column prop="token_name" label="币种" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="amount" label="充值金额" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="from_addr" label="源地址" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="to_addr" label="目的地址" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="block_time" label="区块时间" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="tx_confirmations" label="区块确认数" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="block_height" label="区块高度" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="tx_hash" label="交易HASH" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="memo" label="交易备注(memo)" align="center" :resizable="false"></el-table-column>
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
      depositList: [],
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
    this.handleSearch()
  },
  methods: {
    getSearch() {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    getName() {
      return `充值记录表-${new Date().getTime()}.xlsx`
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
        url: "/client/Deposit/",
        params: {
          first_time: this.searchForm.first_time || "",
          end_time: this.searchForm.end_time || "",
          offset: (this.searchForm.page - 1) * this.searchForm.pageSize,
          limit: this.searchForm.pageSize,
          search: this.searchForm.search.toLocaleUpperCase(),
        }
      });
      if (status === 200) {
        this.depositList = data.results.map(item => {
          item.block_time = item.block_time.replace('T', ' ')
          return item
        });
        this.total = data.count;
      } else {
        this.$msg(data.detail, 'error')
      }
    },
    exportTable() {
      // let outData = this.depositList.map(item => {
      //   item = {
      //     token_name: item.token_name,
      //     amount: item.amount,
      //     from_addr: item.from_addr,
      //     to_addr: item.to_addr,
      //     block_time: item.block_time,
      //     tx_confirmations: item.tx_confirmations,
      //     block_height: item.block_height,
      //     tx_hash: item.tx_hash,

      //   }
      // })
      let exData = new this.$excel(this.depositList, {
        "token_name": "币种",
        "amount": "充值金额",
        "from_addr": "源地址",
        "to_addr": "目的地址",
        "block_time": "区块时间",
        "tx_confirmations": "区块确认数",
        "block_height": "区块高度",
        "tx_hash": "交易HASH",
        "memo": "交易备注"
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
.recharge-record-content {
  .recharge-record-box {
    padding: 15px 20px;
  }
}
</style>

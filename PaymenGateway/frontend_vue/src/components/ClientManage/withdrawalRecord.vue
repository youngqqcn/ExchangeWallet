<template>
  <div class="withdrawal-record-content">
    <hit-container>
      <div class="withdrawal-record-box">
        <el-form :inline="true" :model="searchForm">
          <el-form-item label="币种">
            <el-input type="text" v-model="searchForm.search" clearable placeholder="请输入币种进行搜索" @keyup.enter.native="getSearch"></el-input>
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
        <el-table border :data="withdrawList" style="margin: 0px;width: 100%">
          <el-table-column prop="serial_id" label="流水号" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="order_id" label="订单号" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="token_name" label="币种" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="amount" label="金额" align="center" :resizable="false"></el-table-column>
          <el-table-column label="订单状态" align="center" :resizable="false">
            <template slot-scope="props">{{orderSlist[props.row.order_status]}}</template>
          </el-table-column>
          <el-table-column label="交易状态" align="center" :resizable="false">
            <template slot-scope="props">{{transactionSlist[props.row.transaction_status]}}</template>
          </el-table-column>
          <el-table-column label="通知状态" align="center" :resizable="false">
            <!-- <template
              slot-scope="props"
            >{{`第${props.row.notify_times}次${props.row.notify_status.includes("SUCCESS") ? "成功":""}`}}</template>-->
            <template slot-scope="props">{{notifySlist[props.row.notify_status]}}</template>
          </el-table-column>
          <el-table-column prop="notify_times" label="通知次数" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="block_height" label="区块高度" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="block_time" label="区块时间" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="tx_confirmations" label="区块确认数" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="from_addr" label="源地址" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="to_addr" label="目的地址" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="tx_hash" label="交易哈希" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="complete_time" label="完成时间" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="memo" label="交易备注(memo)" align="center" :resizable="false"></el-table-column>
          <!-- <el-table-column prop="remark" label="备注" align="center" :resizable="false"></el-table-column> -->
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
      orderSlist: {
        PROCESSING: "处理中",
        SUCCESS: "成功",
        FAIL: "失败"
      },
      transactionSlist: {
        NOTYET: "未汇出",
        PENDING: "已汇出",
        FAIL: "交易失败",
        SUCCESS: "交易成功"
      },
      notifySlist: {
        NOTYET: "尚未通知",
        FIRSTSUCCESS: "第一次通知成功",
        FIRSTFAIL: "第一次通知失败",
        SECONDSUCCESS: "第二次通知成功",
        SECONDFAIL: "第二次通知失败"
      },
      withdrawList: [],
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
  async created() {
    this.handleSearch();
    //  const {status, data} = await this.$axios.get("/client/login/");
  },
  methods: {
    getSearch() {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    getName() {
      return `提币记录表-${new Date().getTime()}.xlsx`
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
        url: "/client/WithdrawOrder/",
        params: {
          first_time: this.searchForm.first_time || "",
          end_time: this.searchForm.end_time || "",
          offset: (this.searchForm.page - 1) * this.searchForm.pageSize,
          limit: this.searchForm.pageSize,
          search: this.searchForm.search.toLocaleUpperCase()
        }
      });
      if (status === 200) {
        this.withdrawList = data.results.map(item => {
          if (item.complete_time) item.complete_time = item.complete_time.replace("T", " ");
          if (item.block_time) item.block_time = item.block_time.replace("T", " ");
          return item;
        });
        this.total = data.count;
      } else {
        this.$msg(data.detail, 'error')
      }
    },
    exportTable() {
      // let outData = this.withdrawList.map(item => {
      //   item = {
      //     order_id: item.order_id,
      //     token_name: item.token_name,
      //     amount: item.amount,
      //     order_status: this.orderSlist[item.order_status],
      //     transaction_status: this.transactionSlist[item.transaction_status],
      //     notify_status: this.notifySlist[item.notify_status],
      //     notify_times: item.notify_times,
      //     block_height: item.block_height,
      //     tx_confirmations: item.tx_confirmations,
      //     from_addr: item.from_addr,
      //     to_addr: item.to_addr,
      //     tx_hash: item.tx_hash,
      //     complete_time: item.complete_time,
      //     remark: item.remark,
      //     memo: item.memo
      //   }
      //   return item
      // })
      let exData = new this.$excel(this.withdrawList, {
        "serial_id": "流水号",
        "order_id": "订单号",
        "token_name": "币种",
        "amount": "金额",
        "order_status": "订单状态",
        "transaction_status": "交易状态",
        "notify_status": "通知状态",
        "notify_times": "通知次数",
        "block_height": "区块高度",
        "block_time": "区块时间",
        "tx_confirmations": "区块确认数",
        "from_addr": "源地址",
        "to_addr": "目的地址",
        "tx_hash": "交易哈希",
        "complete_time": "完成时间",
        "remark": "交易备注"
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
.withdrawal-record-content {
  .withdrawal-record-box {
    padding: 15px 20px;
  }
}
</style>

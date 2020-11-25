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
          <el-table-column prop="token_name" label="订单编号" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="token_name" label="会员UID" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="token_name" label="会员姓名" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="token_name" label="交易币种" align="center" :resizable="false"></el-table-column>
          <el-table-column label="交易类型" align="center" :resizable="false">
            <template slot-scope="props">{{props.row.a1}}</template>
          </el-table-column>
          <el-table-column prop="token_name" label="交易数量" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="token_name" label="交易单价" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="token_name" label="交易总额" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="token_name" label="支付方式" align="center" :resizable="false">
            <template slot-scope="props">{{props.row.a1}}</template>
          </el-table-column>
          <el-table-column prop="token_name" label="交易时间" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="token_name" label="订单状态" align="center" :resizable="false">
            <template slot-scope="props">{{props.row.a1}}</template>
          </el-table-column>
          <el-table-column prop="token_name" label="交易商家" align="center" :resizable="false"></el-table-column>
          <el-table-column label="操作" align="center" :resizable="false">
            <template slot-scope="props">
              <el-button type="text" @click="openDialog(props.row)">操作</el-button>
            </template>
          </el-table-column>
        </el-table>
        <el-dialog :visible.sync="Dialog" width="50%" center>
          <el-form :model="dialogForm" label-position="left">
            <el-form-item label="订单编号：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="订单状态：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="下单时间：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="交易类型：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="会员UID：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="会员姓名：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="交易币种：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="交易数量：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="交易价格：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="交易总额：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="支付方式：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="开户行：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="开户支行：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="收款人姓名：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="银行卡号：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="支付宝账号：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="微信账号：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="商家UID：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="商家姓名：" label-width="180px">{{12345}}</el-form-item>
            <el-form-item label="付款时间：" label-width="180px">{{12345}}</el-form-item>
          </el-form>
          <div slot="footer" class="dialog-footer">
            <el-button @click="Dialog = false">返回</el-button>
          </div>
        </el-dialog>
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
      Dialog: false,
      dialogForm: {},
      transactionType: {
        a1: '买入',
        a2: '卖出'
      },
      paymentMethod: {
        a1: '支付宝',
        a2: '银行卡',
        a3: '微信'
      },
      orderStauts: {
        a1: "待付款",
        a2: '待收款',
        a3: '已取消'
      },
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
    // this.handleSearch()
  },
  methods: {
    getSearch() {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    getName() {
      return `表-${new Date().getTime()}.xlsx`
    },
    openDialog (row) {
      this.Dialog = true
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
        url: "",
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
      let exData = new this.$excel(this.depositList, {
        "a1": "币种",
        "a2": "充值金额",
        "a3": "源地址",
        "a4": "目的地址",
        "a5": "区块时间",
        "a6": "区块确认数",
        "a7": "区块高度"
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

<template>
  <div class="collection-record-content">
    <hit-container>
      <div class="collection-record-box">
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
        <el-table border :data="collectionList" style="margin: 0px;width: 100%">
          <el-table-column prop="token_name" label="币种" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="amount" label="归集金额" align="center" :resizable="false"></el-table-column>
          <el-table-column label="操作类型" align="center" :resizable="false">
            <template slot-scope="props">{{operaTion[props.row.collection_type]}}</template>
          </el-table-column>
          <el-table-column prop="from_address" label="源地址" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="to_address" label="目的地址" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="complete_time" label="归集完成时间" align="center" :resizable="false"></el-table-column>
          <el-table-column label="归集状态" align="center">
            <template slot-scope="props">{{collectionStatus[props.row.transaction_status]}}</template>
          </el-table-column>
          <el-table-column prop="tx_confirmations" label="区块确认数" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="block_height" label="区块高度" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="tx_hash" label="交易HASH" align="center" :resizable="false"></el-table-column>
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
      operaTion: {
        AUTO: "自动归集",
        MANUAL: "手动归集",
        FEE: "补手续费"
      },
      collectionStatus: {
        NOTYET: "未汇出",
        PENDING: "已汇出",
        FAIL: "交易失败",
        SUCCESS: "交易成功"
      },
      collectionList: [],
      coinTypeList: [],
      searchForm: {
        search: "",
        first_time: "",
        end_time: "",
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
    getSearch() {
      this.searchForm.page = 1;
      this.handleSearch();
    },
    getName() {
      return `归集记录表-${new Date().getTime()}.xlsx`
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
      let search = [];
      if (this.searchForm.token_name) {
        search.push(this.searchForm.token_name.toLocaleUpperCase());
      }
      if (this.searchForm.time) {
        search.push(this.searchForm.time);
      }
      const { data, status } = await this.$axios({
        methods: "get",
        url: "/client/CollectionRecords/",
        params: {
          first_time: this.searchForm.first_time || "",
          end_time: this.searchForm.end_time || "",
          offset: (this.searchForm.page - 1) * this.searchForm.pageSize,
          limit: this.searchForm.pageSize,
          search: this.searchForm.search.toLocaleUpperCase(),
        }
      });
      if (status === 200) {
        this.collectionList = data.results.map(item => {
          item.complete_time = item.complete_time.replace("T", " ");
          return item;
        });
        this.total = data.count;
      } else {
        this.$msg(data.detail, 'error')
      }
    },
    exportTable() {
      let outData = this.collectionList.map(item => {
        item = {
          token_name: item.token_name,
          amount: item.amount,
          collection_type: this.operaTion[item.collection_type],
          from_address: item.from_address,
          to_address: item.to_address,
          complete_time: item.complete_time,
          transaction_status: this.collectionStatus[item.transaction_status],
          tx_confirmations: item.tx_confirmations,
          block_height: item.block_height,
          tx_hash: item.tx_hash,
        }
        return item
      })
      let exData = new this.$excel(outData, {
        "token_name": "币种",
        "amount": "数量",
        "collection_type": "操作类型",
        "from_address": "源地址",
        "to_address": "目的地址",
        "complete_time": "归集完成时间",
        "transaction_status": "归集状态",
        "tx_confirmations": "区块确认数",
        "block_height": "区块高度",
        "tx_hash": "交易HASH"
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
.collection-record-content {
  .collection-record-box {
    padding: 15px 20px;
  }
}
</style>

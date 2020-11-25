<template>
  <div class="user-log-content">
    <hit-container>
      <div class="user-log-box">
        <el-form :inline="true" :model="searchForm">
          <el-form-item label="功能名称">
            <el-input type="text" v-model="searchForm.search" clearable placeholder="请输入功能名称进行搜索" @keyup.enter.native="getSearch"></el-input>
          </el-form-item>
          <el-form-item label="操作时间">
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
        <el-table border :data="userlogList" style="margin: 0px;width: 100%">
          <el-table-column prop="operation_time" label="操作时间" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="function_name" label="功能名称" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="operation_type" label="操作类型" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="update_before_value" label="修改前的值" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="last_after_value" label="修改后的值" align="center" :resizable="false"></el-table-column>
          <el-table-column prop="operation_status" label="操作状态" align="center" :resizable="false"></el-table-column>
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
      operationStatus: {
        SUCCESS: "成功",
        FAIL: "失败"
      },
      operationType: {
        CREAT: "新增",
        QUERY: "查询",
        DELETE: "删除",
        UPDATE: "修改",
        LOGIN: "登录",
        LOGIN_NO_GCODE: "登录没有验证码"
      },
      userlogList: [],
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
      return `用户操作日志-${new Date().getTime()}.xlsx`
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
        url: "/client/UserOperationLog/",
        params: {
          first_time: this.searchForm.first_time || "",
          end_time: this.searchForm.end_time || "",
          offset: (this.searchForm.page - 1) * this.searchForm.pageSize,
          limit: this.searchForm.pageSize,
          search: this.searchForm.search
        }
      });
      if (status === 200) {
        this.userlogList = data.results.map(item => {
          if (item.operation_time) item.operation_time = this.handleTimeFormat(new Date(item.operation_time).getTime());;
          if (item.operation_type) item.operation_type = this.operationType[item.operation_type]
          if (item.operation_status) item.operation_status = this.operationStatus[item.operation_status]
          return item;
        });
        this.total = data.count;
      } else {
        this.$msg(data.detail, 'error')
      }
    },
    exportTable() {
      let exData = new this.$excel(this.userlogList, {
        "operation_time": "操作时间",
        "function_name": "功能名称",
        "operation_type": "操作类型",
        "update_before_value": "修改前的值",
        "last_after_value": "修改后的值",
        "operation_status": "操作状态"
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
.user-log-content {
  .user-log-box {
    padding: 15px 20px;
  }
}
</style>

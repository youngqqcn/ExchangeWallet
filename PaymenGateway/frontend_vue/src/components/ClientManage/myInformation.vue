<template>
  <div class="my-information-content">
    <hit-container>
      <div class="my-information-box">
        <div class="title">用户信息</div>
        <el-form label-width="80px" label-position="left">
          <el-form-item label="公司名:">{{user.pro_name}}</el-form-item>
          <el-form-item label="手机号:">{{user.tel_no}}</el-form-item>
          <el-form-item label="邮箱:">{{user.email}}</el-form-item>
        </el-form>
        <div class="title">
          归集配置
          <span style="color: #949494;font-size:14px;margin-left:10px">(提示：每个币种每天只能修改一次)</span>
        </div>
        <el-table border :data="collectionList" style="margin: 0px;width: 100%">
          <el-table-column prop="token_name" label="币种" :resizable="false"></el-table-column>
          <el-table-column prop="min_amount_to_collect" label="最小归集金额" :resizable="false">
            <template slot-scope="props">
              <el-input v-if="modifyC === props.$index" v-model="editCollection" v-decimal data-int="20" data-float="8"></el-input>
              <div v-else>{{props.row.min_amount_to_collect}}</div>
            </template>
          </el-table-column>
          <el-table-column prop="collection_dst_addr" label="归集地址" :resizable="false"></el-table-column>
          <el-table-column label="操作" align="center" :resizable="false">
            <template slot-scope="props">
              <el-button type="text" @click="modifyCOn(props)" v-show="modifyC !== props.$index">操作</el-button>
              <el-button type="text" @click="modifyC=''" v-show="modifyC === props.$index">取消</el-button>
              <el-button
                type="text"
                style="margin: 0; margin-left: 20px"
                v-show="modifyC === props.$index"
                @click="saveC(props.row)"
              >保存</el-button>
            </template>
          </el-table-column>
        </el-table>
        <div class="title" v-if="handFee">手续费配置(当USDT归集时,被归集ETH的余额不够，该地址用来补手续费)</div>
        <el-table border :data="collectionFeeList" style="margin: 0px;width: 100%" v-if="handFee">
          <el-table-column prop="token_name" label="币种" :resizable="false"></el-table-column>
          <el-table-column prop="address" label="手续费地址" :resizable="false"></el-table-column>
        </el-table>
        <div class="title" v-if="handFee">法币账户地址(法币资产充值时的地址)</div>
        <el-table border :data="collectionFeeList" style="margin: 0px;width: 100%" v-if="handFee">
          <el-table-column prop="token_name" label="币种" :resizable="false"></el-table-column>
          <el-table-column prop="address" label="地址" :resizable="false"></el-table-column>
        </el-table>
        <div class="title">
          提币配置
          <span style="color: #949494;font-size:14px;margin-left:10px">(提示：每个币种每天只能修改一次)</span>
        </div>
        <el-table border :data="withdrawList" style="margin: 0px;width: 100%">
          <el-table-column prop="token_name" label="币种" :resizable="false"></el-table-column>
          <el-table-column label="最小提币金额" :resizable="false">
            <template slot-scope="props">
              <el-input v-if="modify === props.$index" v-model="editCollectionFee.min_amount" v-decimal data-int="20" data-float="8"></el-input>
              <div v-else>{{props.row.min_amount}}</div>
            </template>
          </el-table-column>
          <el-table-column label="最大提币金额" :resizable="false">
            <template slot-scope="props">
              <el-input v-if="modify === props.$index" v-model="editCollectionFee.max_amount" v-decimal data-int="20" data-float="8"></el-input>
              <div v-else>{{props.row.max_amount}}</div>
            </template>
          </el-table-column>
          <el-table-column label="短信通知阈值" :resizable="false">
            <template slot-scope="props">
              <el-input
                v-if="modify === props.$index"
                v-model="editCollectionFee.balance_threshold_to_sms"
                v-decimal
                data-int="20"
                data-float="8"
              ></el-input>
              <div v-else>{{props.row.balance_threshold_to_sms}}</div>
            </template>
          </el-table-column>
          <el-table-column prop="address" label="出币地址" :resizable="false" width="300"></el-table-column>
          <el-table-column label="操作" align="center">
            <template slot-scope="props">
              <el-button type="text" @click="modifyOn(props)" v-show="modify !== props.$index">操作</el-button>
              <el-button type="text" @click="modify=''" v-show="modify === props.$index">取消</el-button>
              <el-button
                type="text"
                style="margin: 0; margin-left: 20px"
                v-show="modify === props.$index"
                @click="save(props.row)"
              >保存</el-button>
            </template>
          </el-table-column>
        </el-table>
      </div>
    </hit-container>
  </div>
</template>
<script>
export default {
  data() {
    return {
      handFee: false,
      user: {
        pro_name: "",
        tel_no: "",
        email: ""
      },
      editCollectionFee: {
        min_amount: "",
        max_amount: "",
        balance_threshold_to_sms: ""
      },
      editCollection: "",
      modify: "", // 提币配置修改开关
      modifyC: "", // 归集配置修改开关
      collectionList: [],
      collectionFeeList: [],
      withdrawList: []
    };
  },
  created() {
    this.getUsers();
    this.getCollectionConfig();
    this.getCollectionFeeConfig();
    this.getWithdrawConfig();
  },
  methods: {
    modifyOn(props) {
      this.modify = props.$index;
      this.editCollectionFee = {
        min_amount: props.row.min_amount,
        max_amount: props.row.max_amount,
        balance_threshold_to_sms: props.row.balance_threshold_to_sms
      };
    },
    modifyCOn(props) {
      this.modifyC = props.$index;
      this.editCollection = props.row.min_amount_to_collect;
    },
    // modifyOff () {
    //   this.modify = ""
    // },
    // modifyCOff () {
    //   this.modifyC = ""
    // },
    save(row) {
      this.$confirm("确定修改提币配置?", "提示", {
        confirmButtonText: "确定",
        cancelButtonText: "取消",
        type: "warning",
        center: true
      })
        .then(async () => {
          const { data, status } = await this.$axios.put(
            `/client/WithdrawConfig/${row.id}/`,
            {
              // id: row.id,
              token_name: row.token_name,
              min_amount: this.editCollectionFee.min_amount,
              max_amount: this.editCollectionFee.max_amount,
              balance_threshold_to_sms: this.editCollectionFee
                .balance_threshold_to_sms
            }
          );
          this.modify = "";
          if (status === 200) {
            this.$msg("修改成功");
          } else {
            this.$msg(data.code, "error");
          }
          this.getWithdrawConfig();
        })
        .catch(() => {
          this.modify = "";
        });
    },
    async saveC(row) {
      this.$confirm("确定修改归集配置吗?", "提示", {
        confirmButtonText: "确定",
        cancelButtonText: "取消",
        type: "warning",
        center: true
      })
        .then(async () => {
          const { data, status } = await this.$axios.put(
            `/client/CollectionConfig/${row.id}/`,
            {
              token_name: row.token_name,
              min_amount_to_collect: this.editCollection
            }
          );
          this.modifyC = "";
          if (status === 200) {
            this.$msg("修改成功");
          } else {
            this.$msg(data.code, "error");
          }
          this.getCollectionConfig();
        })
        .catch(() => {
          this.modifyC = "";
        });
    },
    async getUsers() {
      // 用户信息
      const { data, status } = await this.$axios.get("/client/Users/");
      if (status === 200) {
        Object.assign(this.user, data[0]);
      } else {
        this.$msg(data.detail, "error");
      }
    },
    async getCollectionConfig() {
      // 归集配置
      const { data, status } = await this.$axios.get(
        "/client/CollectionConfig/"
      );
      if (status === 200) {
        this.handFee = data.some(item => item.token_name === "USDT");
        this.collectionList = data;
      }
    },
    async getCollectionFeeConfig() {
      // 手续费配置
      const { data, status } = await this.$axios.get(
        "/client/CollectionFeeConfig/"
      );
      if (status === 200) {
        this.collectionFeeList = data;
      } else {
        this.$msg(data.detail, "error");
      }
    },
    async getWithdrawConfig() {
      // 提币配置
      const { data, status } = await this.$axios.get("/client/WithdrawConfig/");
      if (status === 200) {
        this.withdrawList = data.map(item => {
          item.modify = false;
          return item;
        });
      } else {
        this.$msg(data.detail, "error");
      }
    }
  }
};
</script>
<style lang="scss">
.my-information-content {
  .my-information-box {
    padding: 15px 20px;
    .title {
      margin: 20px 0 10px;
    }
  }
}
</style>

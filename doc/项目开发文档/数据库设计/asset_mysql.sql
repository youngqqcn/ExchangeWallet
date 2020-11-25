drop table if exists tb_addr;

drop table if exists tb_admin;

drop table if exists tb_admin_type;

drop table if exists tb_auth_flag;

drop table if exists tb_auto_withdraw;

drop table if exists tb_big_account;

drop table if exists tb_coin_type;

drop table if exists tb_collection;

drop table if exists tb_eth_tx_nonce;

drop table if exists tb_finance_flow;

drop table if exists tb_operation_log;

drop table if exists tb_recharge;

drop table if exists tb_sms_num_set;

drop table if exists tb_user;

create table tb_addr
(
   add_no               int not null,
   lead_in_datetime     varchar(24),
   new_charge_time      varchar(24),
   coin_type            varchar(20) not null,
   operate_personal     varchar(20),
   addr                 varchar(100) not null,
   primary key (add_no)
);

create table tb_admin
(
   admin_id             int not null,
   admin_type_id        int not null,
   login_name           varchar(32) not null,
   password             varchar(32) not null,
   family_name          varchar(32) not null,
   tel                  varchar(20) not null,
   status               int not null,
   create_time          bigint not null,
   login_status         int,
   primary key (admin_id)
);

create table tb_admin_type
(
   admin_type_id        int not null,
   type_name            varchar(32) not null,
   status               varchar(10) not null,
   departement          varchar(32) not null,
   create_time          bigint not null,
   remark               varchar(100),
   primary key (admin_type_id)
);

create table tb_auth_flag
(
   admin_type_id        int not null,
   charge_list          int not null,
   addr_count_list      int not null,
   addr_detail          int not null,
   collection_create_raw_Tx int not null,
   collection_broadcast int not null,
   big_account_list     int not null,
   collection_log       int not null,
   auto_withdraw        int not null,
   manual_withdraw1     int not null,
   manual_withdraw2     int not null,
   user_assert_data     int not null,
   user_statements      int not null,
   manual_withdraw_create_raw_Tx int not null,
   manual_withdraw_broadcast int not null,
   withdraw_list        int not null,
   admin_type_list      int not null,
   add_admin_type       int not null,
   admin_list           int not null,
   admin_log            int not null,
   exchange_statements  int not null,
   exchange_charge_daily int not null,
   exchange_withdraw_daily int not null,
   add_coin             int not null,
   airdrop_charge       int not null,
   primary key (admin_type_id)
);

create table tb_auto_withdraw
(
   order_id             varchar(50) not null,
   user_id              varchar(20) not null,
   coin_type            varchar(20) not null,
   amount               varchar(20) not null,
   trade_fee            varchar(20) not null,
   tx_fee               varchar(20) not null,
   txid                 varchar(100),
   dst_addr             varchar(100) not null,
   src_addr             varchar(100) not null,
   withdraw_way         int not null,
   audit_status         int not null,
   tx_status            int not null,
   order_status         int not null,
   upload_status        int not null,
   create_time          bigint not null,
   audit_time           bigint not null,
   remarks              varchar(64),
   auditor              varchar(32),
   complete_time        bigint not null,
   primary key (order_id)
);

create table tb_big_account
(
   account_no           int not null,
   coin_type            varchar(20) not null,
   balance              numeric(20,10) not null,
   addr                 varchar(100) not null,
   last_query_time      bigint not null,
   primary key (account_no)
);

create table tb_coin_type
(
   type_no              int not null,
   coin_nick            varchar(32),
   withdraw_src_addr    varchar(100) not null,
   addr_count           int,
   encrypted_privkey    varchar(256) not null,
   decode_pwd           varchar(30),
   min_withdraw_count   varchar(20),
   day_withdraw_count   varchar(20),
   tx_fee               varchar(100) not null,
   charge_threshold     varchar(20),
   primary key (type_no)
);

create table tb_collection
(
   order_id             varchar(50) not null,
   admin_id             int,
   coin_type            varchar(20) not null,
   src_addr             varchar(100) not null,
   dst_addr             varchar(100) not null,
   amount               varchar(20) not null,
   token_amount         varchar(20),
   tx_fee               varchar(20) not null,
   status               int not null,
   complete_time        bigint not null,
   txid                 varchar(100) not null,
   primary key (order_id)
);

create table tb_eth_tx_nonce
(
   order_id             varchar(50) not null,
   src_addr             varchar(50) not null,
   txid                 varchar(100) not null,
   nonce                bigint not null,
   primary key (order_id)
);

create table tb_finance_flow
(
   flow_no              varchar(50) not null,
   coin_type            varchar(20) not null,
   coin_count           numeric(20,10) not null,
   flow_type            varchar(32) not null,
   flow_time            bigint not null,
   primary key (flow_no)
);

create table tb_operation_log
(
   log_no               int not null,
   admin_id             int,
   admin_login_name     varchar(30),
   admin_family_name    varchar(30),
   tel                  varchar(20),
   operation_type       varchar(100),
   operation_time       bigint,
   primary key (log_no)
);

create table tb_recharge
(
   charge_no            int not null,
   order_id             varchar(50) not null,
   admin_id             int not null,
   coin_type            varchar(20) not null,
   recharge_count       varchar(20) not null,
   recharge_status      int not null,
   upload_status        int not null,
   audit_status         int,
   auditor              varchar(32),
   audit_remark         varchar(64),
   src_addr             varchar(100) not null,
   receive_addr         varchar(100) not null,
   txid                 varchar(100) not null,
   recharge_time        bigint not null,
   block_number         bigint not null,
   collection_status    int,
   primary key (charge_no)
);

create table tb_sms_num_set
(
   no                   int not null,
   phone_number_1       varchar(15),
   phone_number_2       varchar(15),
   phone_number_3       varchar(15),
   phone_number_4       varchar(15),
   phone_number_5       varchar(15),
   primary key (no)
);

create table tb_user
(
   user_id              varchar(20) not null,
   user_nick            varchar(32) not null,
   user_family_name     varchar(32) not null,
   tel                  varchar(15) not null,
   email                varchar(100) not null,
   primary key (user_id)
);

alter table tb_admin add constraint FK_Reference_1 foreign key (admin_type_id)
      references tb_admin_type (admin_type_id) on delete restrict on update restrict;

alter table tb_collection add constraint FK_Reference_2 foreign key (admin_id)
      references tb_admin (admin_id) on delete restrict on update restrict;

alter table tb_operation_log add constraint FK_Reference_5 foreign key (admin_id)
      references tb_admin (admin_id) on delete restrict on update restrict;

alter table tb_recharge add constraint FK_Reference_4 foreign key (admin_id)
      references tb_admin (admin_id) on delete restrict on update restrict;


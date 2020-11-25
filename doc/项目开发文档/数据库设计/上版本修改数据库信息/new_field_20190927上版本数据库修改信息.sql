ALTER TABLE tb_coin_type ADD COLUMN charge_threshold VARCHAR(20) DEFAULT NULL;
ALTER TABLE tb_coin_type MODIFY COLUMN tx_fee VARCHAR(100);
ALTER TABLE tb_recharge ADD COLUMN audit_remark varchar(64) DEFAULT NULL COMMENT '审核备注信息' after upload_status;
ALTER TABLE tb_recharge ADD COLUMN auditor varchar(32) DEFAULT NULL COMMENT '审核人' after upload_status;
ALTER TABLE tb_recharge ADD COLUMN audit_status int(11) DEFAULT NULL COMMENT '审核状态' after upload_status;

update tb_coin_type set charge_threshold = '10000' where coin_nick = 'HTDF';
update tb_coin_type set charge_threshold = '10000' where coin_nick = 'USDP';
update tb_coin_type set charge_threshold = '10000000' where coin_nick = 'HET';
update tb_coin_type set charge_threshold = '100000' where coin_nick = 'BJC';
update tb_coin_type set charge_threshold = '100000' where coin_nick = 'BEI';
update tb_coin_type set charge_threshold = '10' where coin_nick = 'BTC';
update tb_coin_type set charge_threshold = '100' where coin_nick = 'ETH';
update tb_coin_type set charge_threshold = '80' where coin_nick = 'BCH';
update tb_coin_type set charge_threshold = '100' where coin_nick = 'BSV';
update tb_coin_type set charge_threshold = '100' where coin_nick = 'LTC';
update tb_coin_type set charge_threshold = '100' where coin_nick = 'DASH';
update tb_coin_type set charge_threshold = '10000' where coin_nick = 'USDT';
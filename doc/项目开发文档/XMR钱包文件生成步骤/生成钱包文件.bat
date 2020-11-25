::作者 : yqq
::日期 : 2020-04-10
::说明 : 生成Monero(XMR) 门罗币的钱包文件
::--electrum-seed="anybody snout itinerary lava awoken sulking typist fixate ouch bygones dilute rebel novelty also cage boat boxes urchins enraged code rewind vocal tomorrow utensils ouch"

::#################################【 请修改助记词 】##########################################
SET  自动出币地址助记词="anybody snout itinerary lava awoken sulking typist fixate ouch bygones dilute rebel novelty also cage boat boxes urchins enraged code rewind vocal tomorrow utensils ouch"


SET  人工出币地址助记词="anybody snout itinerary lava awoken sulking typist fixate ouch bygones dilute rebel novelty also cage boat boxes urchins enraged code rewind vocal tomorrow utensils ouch"
::###########################################################################

:menu
cls
@echo off
echo.&echo.&echo.
echo #########################【 XMR 钱包文件生成 】################################

SET  AUTO_WITHDRAW_DIR=".\auto_withdraw"
SET  MANUAL_WITHDRAW_DIR=".\manual_withdraw"

echo.&echo.
echo 【1】 生成自动出币钱包文件
echo 【2】 生成人工出币钱包文件
echo 【3】 退出
echo.&echo.
set /p a=请输入选择的操作编号 :
if "%a%"=="1" goto gen_auto
if "%a%"=="2" goto gen_manual
if "%a%"=="3" goto end

:gen_auto
echo 尝试创建目录 %AUTO_WITHDRAW_DIR% ....
if not exist %AUTO_WITHDRAW_DIR% (
	mkdir %AUTO_WITHDRAW_DIR%
	echo 创建 %AUTO_WITHDRAW_DIR% 目录成功!
	echo 启动钱包客户程序....
	echo.&echo.&echo.
	echo ========================自动出币地址助记词===================
	echo  %自动出币地址助记词%
	echo =============================================================
	echo.&echo.&echo.
	monero-wallet-cli   --offline  --restore-deterministic-wallet  --subaddress-lookahead  2:50000  --generate-new-wallet=./auto_withdraw/cold_wallet --electrum-seed=%自动出币地址助记词%
	goto end
)else (
	echo %AUTO_WITHDRAW_DIR%  目录已存在!无法继续创建
	pause
	goto menu
)

:gen_manual
echo 尝试创建目录 %MANUAL_WITHDRAW_DIR% ....
if not exist %MANUAL_WITHDRAW_DIR% (
	mkdir %MANUAL_WITHDRAW_DIR%
	echo 创建 %MANUAL_WITHDRAW_DIR% 目录成功!
	echo 启动钱包客户程序....
	echo.&echo.&echo.
	echo ====================人工出币地址助记词========================
	echo  %人工出币地址助记词%
	echo ==============================================================
	echo.&echo.&echo.
	monero-wallet-cli    --offline  --restore-deterministic-wallet   --subaddress-lookahead  2:50000   --generate-new-wallet=./manual_withdraw/cold_wallet  --electrum-seed=%人工出币地址助记词%
	goto end
)else (
	echo %MANUAL_WITHDRAW_DIR%  目录已存在! 无法继续创建
	pause
	goto menu
)

:end
echo 已退出
pause

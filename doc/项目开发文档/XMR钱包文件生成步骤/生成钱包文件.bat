::���� : yqq
::���� : 2020-04-10
::˵�� : ����Monero(XMR) ���ޱҵ�Ǯ���ļ�
::--electrum-seed="anybody snout itinerary lava awoken sulking typist fixate ouch bygones dilute rebel novelty also cage boat boxes urchins enraged code rewind vocal tomorrow utensils ouch"

::#################################�� ���޸����Ǵ� ��##########################################
SET  �Զ����ҵ�ַ���Ǵ�="anybody snout itinerary lava awoken sulking typist fixate ouch bygones dilute rebel novelty also cage boat boxes urchins enraged code rewind vocal tomorrow utensils ouch"


SET  �˹����ҵ�ַ���Ǵ�="anybody snout itinerary lava awoken sulking typist fixate ouch bygones dilute rebel novelty also cage boat boxes urchins enraged code rewind vocal tomorrow utensils ouch"
::###########################################################################

:menu
cls
@echo off
echo.&echo.&echo.
echo #########################�� XMR Ǯ���ļ����� ��################################

SET  AUTO_WITHDRAW_DIR=".\auto_withdraw"
SET  MANUAL_WITHDRAW_DIR=".\manual_withdraw"

echo.&echo.
echo ��1�� �����Զ�����Ǯ���ļ�
echo ��2�� �����˹�����Ǯ���ļ�
echo ��3�� �˳�
echo.&echo.
set /p a=������ѡ��Ĳ������ :
if "%a%"=="1" goto gen_auto
if "%a%"=="2" goto gen_manual
if "%a%"=="3" goto end

:gen_auto
echo ���Դ���Ŀ¼ %AUTO_WITHDRAW_DIR% ....
if not exist %AUTO_WITHDRAW_DIR% (
	mkdir %AUTO_WITHDRAW_DIR%
	echo ���� %AUTO_WITHDRAW_DIR% Ŀ¼�ɹ�!
	echo ����Ǯ���ͻ�����....
	echo.&echo.&echo.
	echo ========================�Զ����ҵ�ַ���Ǵ�===================
	echo  %�Զ����ҵ�ַ���Ǵ�%
	echo =============================================================
	echo.&echo.&echo.
	monero-wallet-cli   --offline  --restore-deterministic-wallet  --subaddress-lookahead  2:50000  --generate-new-wallet=./auto_withdraw/cold_wallet --electrum-seed=%�Զ����ҵ�ַ���Ǵ�%
	goto end
)else (
	echo %AUTO_WITHDRAW_DIR%  Ŀ¼�Ѵ���!�޷���������
	pause
	goto menu
)

:gen_manual
echo ���Դ���Ŀ¼ %MANUAL_WITHDRAW_DIR% ....
if not exist %MANUAL_WITHDRAW_DIR% (
	mkdir %MANUAL_WITHDRAW_DIR%
	echo ���� %MANUAL_WITHDRAW_DIR% Ŀ¼�ɹ�!
	echo ����Ǯ���ͻ�����....
	echo.&echo.&echo.
	echo ====================�˹����ҵ�ַ���Ǵ�========================
	echo  %�˹����ҵ�ַ���Ǵ�%
	echo ==============================================================
	echo.&echo.&echo.
	monero-wallet-cli    --offline  --restore-deterministic-wallet   --subaddress-lookahead  2:50000   --generate-new-wallet=./manual_withdraw/cold_wallet  --electrum-seed=%�˹����ҵ�ַ���Ǵ�%
	goto end
)else (
	echo %MANUAL_WITHDRAW_DIR%  Ŀ¼�Ѵ���! �޷���������
	pause
	goto menu
)

:end
echo ���˳�
pause

#!coding:utf8

#author:yqq
#date:2020/5/6 0006 16:48
#description:   测试 modle
# 关于  unittest 使用方法:  https://blog.51cto.com/2681882/2123613
# 1) 使用 TestSuite 可以控制测试用例的执行顺序
# 2) 如何让多个用例共用setup、teardown , 使用 setUpClass, tearDownClass
# 3) 如何跳过用例, unittest.skip(reason)、unittest.skipIf(condition,reason)、 unittest.skipUnless(condition,reason)
# 4) 如何生成html格式的测试报告:  使用HTMLTestRunner代替默认的TextTestRunner()




#sqlalchemy使用, 请参考: https://www.cnblogs.com/liu-yao/p/5342656.html
from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker

# from .model import ORMBase, WithdrawOrder, Project
from src.config.constant import MYSQL_CONNECT_INFO
from src.model.model import  ORMBase, Deposit

# import pytest
import unittest


class TestModels(unittest.TestCase):

    # def addCleanup(self):
    #     这是清理函数
    # pass

    def setUp(self):
        """
        每个用例执行前调用此方法
        :return:
        """
        print('setUp()')
        pass

    def tearDown(self):
        print('tearDown()')

    @classmethod
    def tearDownClass(cls):
        """
        类
        :return:
        """
        print('tearDownClass()')
        pass

    # def shortDescription(self):
    #     pass

    @classmethod
    def setUpClass(cls):
        """
        在初始化类之前, 执行这个函数
        :return:
        "Hook method for setting up class fixture before running tests in the class."
        """
        cls.engine = create_engine(MYSQL_CONNECT_INFO,
                                   max_overflow=0,
                                   pool_size=5)

        SessionCls = sessionmaker(bind=cls.engine,
                                  autoflush=True,
                                  # autocommit=True  # 自动提交
                                  autocommit=False  # 自动提交
                                  )

        cls.session = SessionCls()
        print('setUpClass()')



    def test_create_all_table(self):
        """
        创建所有表表结构, 如果表已经存在,则忽略
        :return:
        """

        print('test_create_all_table')
        ORMBase.metadata.create_all(self.engine)  # 创建所有表结构

        # assert 1 == 1

    def test_insert_data_Deposit(self):
        """
        测试 想 tb_deposit 表中插入数据
        :return:
        """
        print('test_insert_data_Project')


        import random
        import string

        try:
            all_instances = []
            import datetime
            for i in range(10000):
                # order = Project(  # pro_id 不设置,  默认自增即可
                #     name=''.join(random.sample(population=string.ascii_lowercase * 10, k=43)),
                #     tel_no=''.join(random.sample(population=string.digits * 10, k=11)),
                #     email='test@' + ''.join(random.sample(population=string.ascii_lowercase * 10, k=11)),
                #     api_key_hash=''.join(random.sample(population=string.ascii_lowercase * 10, k=64)),
                #     app_id=''.join(random.sample(population=string.ascii_lowercase * 10, k=20)),
                #     create_time=datetime.datetime.now(),
                #     account_status=random.randint(0, 4)
                # )

                # session.add(instance=order, _warn=True)

                deposit_tx = Deposit(pro_id=random.randint(0, 100),
                                     token_name=random.choice(['BTC', 'HTDF', 'ETH', 'USDT']),
                                     tx_hash='0x' + ''.join( random.sample(population=string.hexdigits * 10, k=64) ),
                                     from_addr=''.join(random.sample(population=string.ascii_lowercase * 10, k=43)),
                                     to_addr=''.join(random.sample(population=string.ascii_lowercase * 10, k=43)),
                                     memo='',
                                     amount=random.uniform(0.00001, 9999999),  #生成浮点数
                                     block_height= random.randint(0, 199999999),
                                     block_time=datetime.datetime.now(),
                                     notify_status=random.randint(0, 3),
                                     tx_confirmations=  random.randint(0, 199999),
                                     )


                all_instances.append(deposit_tx)

            self.session.add_all(instances=all_instances)

            self.session.commit()

        except Exception as e:
            print(f"error: {e}")
            pass

        pass

    # @unittest.skipIf(1 == 1, reason='skip' )
    def test_query_data_Deposit(self):
        """
        测试从 tb_deposit 表中查询数据
        :return:
        """
        # deposit_txs  = self.session.query(Deposit).filter_by(pro_id=3).all()
        # print(deposit_txs)
        # for tx in deposit_txs:
        #     print(tx)



        #关于更多的查询:  https://www.jianshu.com/p/d08a63170714
        start_height = 0
        end_height = 999999999
        token_name = 'ETH'
        pro_id = 3

        deposit_txs = self.session.query(Deposit).filter_by(pro_id=pro_id, token_name=token_name) \
            .filter(Deposit.block_height >= start_height , Deposit.block_height < end_height).all()

        print(type(deposit_txs[0]))
        print(len(deposit_txs))


        pass


if __name__ == '__main__':

    test_funcs = [ TestModels('test_create_all_table'),
                   TestModels('test_insert_data_Deposit'),
                   TestModels('test_query_data_Deposit') ]

    suite = unittest.TestSuite(  )
    suite.addTests(test_funcs)


    runner = unittest.TextTestRunner()
    runner.run(test=suite)

    unittest.main()


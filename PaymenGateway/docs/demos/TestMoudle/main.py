#!coding:utf8

#author:yqq
#date:2020/9/10 0010 11:03
#description:


# import mypkg
# from mypkg import  mul
# import mypkg

from multiprocessing import Process
import time



class Worker:

    def __init__(self):
        self.setting = 999
        self.wp =  Process(target=self.do_work, kwargs={'a':1, 'b':3, 'c':5} ) #ok
        pass

    def show(self):
        print(self.setting)

    def do_work(self, a, b, c):
        while True:
            time.sleep(1)
            self.show()
            print('a = {}, b = {}, c = {}'.format(a, b, c))
        pass

    def start(self):
        # wp = Process(target=self.do_work,  args=(1, 3, 5))  #ok

        # wp.daemon = True
        self.wp.start()
        self.wp.join(timeout=1)
        print('--------')



def main():

    # print( mul(1, 2) )
    worker = Worker()
    worker.start()

    pass


if __name__ == '__main__':

    main()
    print('main end')
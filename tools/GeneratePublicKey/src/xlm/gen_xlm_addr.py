#!coding:utf8

#author:yqq
#date:2020/1/13 0013 16:20
#description: 

from nacl.signing import SigningKey

import os


def main():


    seed = os.urandom(32)

    sk = SigningKey(seed)
    print("{}".format( sk.verify_key))


    

    pass


if __name__ == '__main__':

    main()
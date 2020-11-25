#!coding:utf8

#author:yqq
#date:2020/7/2 0002 20:57
#description:


import urllib3


def main():

    try:
        raise urllib3.exceptions.ConnectionError
    except Exception as e:
        print('{}', e)



    pass


if __name__ == '__main__':

    main()
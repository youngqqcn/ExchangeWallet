"""
Django settings for PG_Admin project.

Generated by 'django-admin startproject' using Django 3.0.6.

For more information on this file, see
https://docs.djangoproject.com/en/3.0/topics/settings/

For the full list of settings and their values, see
https://docs.djangoproject.com/en/3.0/ref/settings/
"""
import json
import os

from config import config
from config.config import config as conf


ENV_NAME = conf.ENV_NAME
# ENV_NAME = 'dev'

# Build paths inside the project like this: os.path.join(BASE_DIR, ...)
BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))


# Quick-start development settings - unsuitable for production
# See https://docs.djangoproject.com/en/3.0/howto/deployment/checklist/

# SECURITY WARNING: keep the secret key used in production secret!
SECRET_KEY = 'ayevh^k8%je^4v-$y9(#qgd3dpj3!xh0z9n6oo7q$2g6d4^w49'

# SECURITY WARNING: don't run with debug turned on in production!

DEBUG = conf.DEBUG
# DEBUG = True

ALLOWED_HOSTS = ['*',]


# Application definition

INSTALLED_APPS = [
    'simpleui',
    'import_export',
    'django.contrib.admin',
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.messages',
    'django.contrib.staticfiles',
    'pgadmin',
    # 'debug_toolbar',

    'mdeditor'
]

MIDDLEWARE = [
    'django.middleware.security.SecurityMiddleware',
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
    'django.contrib.messages.middleware.MessageMiddleware',
    'django.middleware.clickjacking.XFrameOptionsMiddleware',
    # 'debug_toolbar.middleware.DebugToolbarMiddleware',
]


# INTERNAL_IPS = ('127.0.0.1',)
ROOT_URLCONF = 'PG_Admin.urls'

TEMPLATES = [
    {
        'BACKEND': 'django.template.backends.django.DjangoTemplates',
        'DIRS': [os.path.join(BASE_DIR, 'templates')],
        'APP_DIRS': True,
        'OPTIONS': {
            'context_processors': [
                'django.template.context_processors.debug',
                'django.template.context_processors.request',
                'django.contrib.auth.context_processors.auth',
                'django.contrib.messages.context_processors.messages',
            ],
        },
    },
]

WSGI_APPLICATION = 'PG_Admin.wsgi.application'


# Database
# https://docs.djangoproject.com/en/3.0/ref/settings/#databases

DATABASES = {
     'default': {
        'ENGINE': 'django.db.backends.mysql',
        'NAME': f'pg_database_{ENV_NAME}',
        # 'NAME' : 'pg_database',
        'HOST':conf.MYSQL_HOST,
        'PORT':conf.MYSQL_PORT,
        'USER':conf.MYSQL_USERNAME,
        'PASSWORD':conf.MYSQL_PWD,

    }
}


# Password validation
# https://docs.djangoproject.com/en/3.0/ref/settings/#auth-password-validators

AUTH_PASSWORD_VALIDATORS = [
    {
        'NAME': 'django.contrib.auth.password_validation.UserAttributeSimilarityValidator',
    },
    {
        'NAME': 'django.contrib.auth.password_validation.MinimumLengthValidator',
    },
    {
        'NAME': 'django.contrib.auth.password_validation.CommonPasswordValidator',
    },
    {
        'NAME': 'django.contrib.auth.password_validation.NumericPasswordValidator',
    },
]


# Internationalization
# https://docs.djangoproject.com/en/3.0/topics/i18n/

LANGUAGE_CODE = 'zh-hans'

TIME_ZONE = 'Asia/Shanghai'

USE_I18N = True

USE_L10N = False
DATETIME_FORMAT = 'Y-m-d H:i:s'

USE_TZ = False


# Static files (CSS, JavaScript, Images)
# https://docs.djangoproject.com/en/3.0/howto/static-files/

STATIC_URL = '/static/'
# STATIC_ROOT = os.path.join(BASE_DIR, "static")
MEDIA_ROOT = os.path.join(BASE_DIR, 'media')

STATICFILES_DIRS = (
    os.path.join(BASE_DIR, 'static'),
)


# 设置前缀
MEDIA_URL = '/media/'

SIMPLEUI_LOGIN_PARTICLES = True



# 图标设置，图标参考：
SIMPLEUI_ICON = {
    '系统管理': 'fab fa-apple',
    '员工管理': 'fas fa-user-tie'
}

g_MNEMONIC = conf.MNEMONIC

#是否是主网
g_IS_MAINNET = conf.IS_MAINNET

REDIS_HOST = conf.REDIS_HOST
REDIS_PORT = conf.REDIS_PORT
REDIS_API_KEY_DB_NAME = 2
REDIS_API_KEY_DB_NAME_CACHE = 7

RABBIT_MQ_IP = conf.RABBIT_MQ_HOST
RABBIT_MQ_PORT = conf.RABBIT_MQ_PORT
RABIIT_MQ_USER_NAME = conf.RABBIT_MQ_USER_NAME
RABIIT_MQ_PASSWORD = conf.RABBIT_MQ_PASSWORD

RABBIT_MQ_VRIATUAL_HOST = ENV_NAME
RABBIT_DIRECT_MODE = 'direct'
RABBIT_DELIVERY_MODE = 2

ADDRESS_GENERATE_EXCHANGE = 'address_generate_exchange'
Q_ADDRESS_GENERATE = 'q_address_generate'
ADDRESS_GENERATE_ROUTINGKEY = 'address_generate'

# mq 的心跳超时时间, 10分钟之内, client一直没有回心跳, sever就连接
RABBIT_MQ_HEARTBEAT_TIME= 60 * 10
RABBIT_BLOCKED_CONNECTION_TIMEOUT = 60*50


# 手机号码正则表达式
REGEX_MOBILE = "^1[358]\d{9}$|^147\d{8}$|^176\d{8}$"

ETH_FULL_NODE_HOST = config.Config.ETH_FULL_NODE_HOST
ETH_FULL_NODE_PORT = config.Config.ETH_FULL_NODE_PORT

BTC_API_HOST = conf.BTC_NODE_API_HOST
BTC_API_PORT = conf.BTC_NODE_API_PORT

HTDF_NODE_RPC_HOST = conf.HTDF_NODE_HOST
HTDF_NODE_RPC_PORT = conf.HTDF_NODE_PORT


EIP20_ABI = json.loads('[{"constant":true,"inputs":[],"name":"name","outputs":[{"name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_spender","type":"address"},{"name":"_value","type":"uint256"}],"name":"approve","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"totalSupply","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_from","type":"address"},{"name":"_to","type":"address"},{"name":"_value","type":"uint256"}],"name":"transferFrom","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[],"name":"decimals","outputs":[{"name":"","type":"uint8"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[{"name":"_owner","type":"address"}],"name":"balanceOf","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":true,"inputs":[],"name":"symbol","outputs":[{"name":"","type":"string"}],"payable":false,"stateMutability":"view","type":"function"},{"constant":false,"inputs":[{"name":"_to","type":"address"},{"name":"_value","type":"uint256"}],"name":"transfer","outputs":[{"name":"","type":"bool"}],"payable":false,"stateMutability":"nonpayable","type":"function"},{"constant":true,"inputs":[{"name":"_owner","type":"address"},{"name":"_spender","type":"address"}],"name":"allowance","outputs":[{"name":"","type":"uint256"}],"payable":false,"stateMutability":"view","type":"function"},{"anonymous":false,"inputs":[{"indexed":true,"name":"_from","type":"address"},{"indexed":true,"name":"_to","type":"address"},{"indexed":false,"name":"_value","type":"uint256"}],"name":"Transfer","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"name":"_owner","type":"address"},{"indexed":true,"name":"_spender","type":"address"},{"indexed":false,"name":"_value","type":"uint256"}],"name":"Approval","type":"event"}]')  # noqa: 501

if g_IS_MAINNET:
#TODO: 生产环境需要更改
    ERC20_USDT_CONTRACT_ADDRESS = '0xdac17f958d2ee523a2206206994597c13d831ec7'  # 主网USDT
else:
    ERC20_USDT_CONTRACT_ADDRESS = '0x1f2648f4437edf90240810e30aa561e1a8b2b802'  # ropsten的测试


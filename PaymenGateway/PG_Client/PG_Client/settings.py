"""
Django settings for PG_Client project.

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
SECRET_KEY = 'e&ik^uu365a6vf*_g$^uf_^xu^twn3r1^4-54dbns6rzc1du)u'

# SECURITY WARNING: don't run with debug turned on in production!
DEBUG = False

ALLOWED_HOSTS = ['*']


# Application definition

AUTH_USER_MODEL = "clientadmin.Project"

INSTALLED_APPS = [
    'django.contrib.admin',
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.messages',
    'django.contrib.staticfiles',
    'corsheaders',
    'rest_framework',  #使用django-restframework
    'clientadmin',
]

MIDDLEWARE = [
    'corsheaders.middleware.CorsMiddleware',  # 需注意与其他中间件顺序，这里放在最前面即可
    'django.middleware.security.SecurityMiddleware',
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
    'django.contrib.messages.middleware.MessageMiddleware',
    'django.middleware.clickjacking.XFrameOptionsMiddleware',
]

ROOT_URLCONF = 'PG_Client.urls'

# # 支持跨域配置开始
CORS_ORIGIN_ALLOW_ALL = True
CORS_ALLOW_CREDENTIALS = True

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

WSGI_APPLICATION = 'PG_Client.wsgi.application'


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

USE_L10N = True

USE_TZ = False


# Static files (CSS, JavaScript, Images)
# https://docs.djangoproject.com/en/3.0/howto/static-files/

STATIC_URL = '/static/'
MEDIA_ROOT = os.path.join(BASE_DIR, 'media')
STATIC_ROOT = os.path.join(BASE_DIR, "static")
# STATICFILES_DIRS = (
#     os.path.join(BASE_DIR, 'static'),
# )

# 设置前缀

MEDIA_URL = '/media/'


REST_FRAMEWORK = {
    #筛选功能
    'DEFAULT_FILTER_BACKENDS':
    [
        'django_filters.rest_framework.DjangoFilterBackend'
    ],
    #文档插件
    'DEFAULT_SCHEMA_CLASS': 'rest_framework.schemas.coreapi.AutoSchema',
    #drf认证
    'DEFAULT_AUTHENTICATION_CLASSES': (
        #drf-jwt
        'rest_framework_jwt.authentication.JSONWebTokenAuthentication',
        #django自身验证
        'rest_framework.authentication.SessionAuthentication',#携带csrf验证
        'rest_framework.authentication.BasicAuthentication',
    ),
    'DEFAULT_RENDERER_CLASSES': [
        'rest_framework.renderers.JSONRenderer',
    ],
    #只能json访问
    'DEFAULT_PARSER_CLASSES': [
        'rest_framework.parsers.JSONParser',
    ],

    #自定义限频
    'DEFAULT_THROTTLE_CLASSES': [
        'rest_framework.throttling.AnonRateThrottle',
        'rest_framework.throttling.UserRateThrottle',
        'clientadmin.mythrottle.LoginBeforeThrottle',
    ],
    'DEFAULT_THROTTLE_RATES': {
        'login_before': '10/m',
        'anon': '0/day',
        'user': '41/m',
    }

}

import datetime

#jwt自定义配置
JWT_AUTH = {

    'JWT_EXPIRATION_DELTA': datetime.timedelta(hours=1), # 有效期设置
    'JWT_REFRESH_EXPIRATION_DELTA': datetime.timedelta(days=7),
    #允许刷新token令牌
    'JWT_ALLOW_REFRESH': True,
    # 重写成功返回参数
    'JWT_RESPONSE_PAYLOAD_HANDLER': 'clientadmin.utils.jwt_response_payload_handler',
}

# 手机号码正则表达式
REGEX_MOBILE = "^1[358]\d{9}$|^147\d{8}$|^176\d{8}$"
#日期匹配
REGEX_KEY = "\d{4}-\d{1,2}-\d{1,2}"

AUTHENTICATION_BACKENDS = [
    # 默认值：['django.contrib.auth.backends.ModelBackend']
    'clientadmin.views.CustomBackend',
    'clientadmin.serializers.CustomJWTSerializer',
]

# DRF扩展缓存时间
REST_FRAMEWORK_EXTENSIONS = {
    # 缓存时间
    'DEFAULT_CACHE_RESPONSE_TIMEOUT': 2 * 60,
    # 缓存存储
    'DEFAULT_USE_CACHE': 'default',
}

REDIS_HOST = conf.REDIS_HOST
REDIS_PORT = conf.REDIS_PORT
REDIS_API_KEY_DB_NAME = 6
REDIS_API_KEY_DB_NAME_CACHE = 7

CACHES = {
    "default": {
        "BACKEND": "django_redis.cache.RedisCache",
        "LOCATION": f"redis://{REDIS_HOST}:{REDIS_PORT}/{REDIS_API_KEY_DB_NAME}",
        "OPTIONS": {
            "CLIENT_CLASS": "django_redis.client.DefaultClient",
        }
    }
}

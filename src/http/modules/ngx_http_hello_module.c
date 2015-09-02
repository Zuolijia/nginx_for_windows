#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct
{
    ngx_str_t hello_string;
} ngx_http_hello_loc_conf_t;

static ngx_int_t ngx_http_hello_handler(ngx_http_request_t *r);
static char *ngx_http_hello(ngx_conf_t *cf, ngx_command_t *cmd, void *conf); //����ָ��

static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf);               //������http{}�ڵ������������ص�
static ngx_int_t ngx_http_hello_preconfiguration(ngx_conf_t *cf);   //�ڽ���http{}�ڵ�������ǰ�ص�
/*
** �������ڴ洢HTTPȫ��������Ľṹ�壬�ýṹ���еĳ�Ա������ֱ����http{}��������������
** �����ڽ���main������ǰ����
*/
static void *ngx_http_hello_create_main_conf(ngx_conf_t *cf);
static char *ngx_http_hello_init_main_conf(ngx_conf_t *cf, void *conf); //������main�������ص�
/*
** �������ڴ洢��ͬʱ������main/srv����������Ľṹ�壬�ýṹ���еĳ�Ա��server�������������
*/
static void *ngx_http_hello_create_srv_conf(ngx_conf_t *cf);
/*
** create_srv_conf�����Ľṹ����Ҫ���������������ͬʱ������main/srv�����У�
** merge_srv_conf�������԰ѳ�����main�����е�������ֵ�ϲ���srv������������
*/
static char *ngx_http_hello_merge_srv_conf(ngx_conf_t *cf, void *prev, void *conf);
/*
** �������ڴ洢��ͬʱ������main��srv��loc����Ľṹ�壬�ýṹ���еĳ�Ա��location�������������
*/
static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf);
/*
** create_loc_conf�����Ľṹ����Ҫ��������������ܳ�����main/srv/loc�����У�
** merge_loc_conf�������Էֱ�ѳ�����main��srv�����������ϲ���loc�������������
*/
static char *ngx_http_hello_merge_loc_conf(ngx_conf_t *cf, void *prev, void *conf);

static ngx_command_t ngx_http_hello_commands[] =
{
    {
        ngx_string("hello_string"),     /* ���������� */
        NGX_HTTP_MAIN_CONF | NGX_HTTP_LOC_CONF | NGX_HTTP_LMT_CONF | NGX_CONF_TAKE1,
        ngx_http_hello,             /* ����������Ĳ��� */
        NGX_HTTP_LOC_CONF_OFFSET,   /* �����ļ��е�ƫ���� */
        offsetof(ngx_http_hello_loc_conf_t, hello_string),
        NULL
    },
    ngx_null_command
};

/*
** 8���׶εĵ���˳���붨��˳��ͬ��
** HTTP��ܵ�����Щ�ص�������ʵ��˳����nginx.conf�������й�
*/
static ngx_http_module_t ngx_http_hello_module_ctx =
{
    ngx_http_hello_preconfiguration,        /* preconfiguration */
    ngx_http_hello_init,                    /* postconfiguration */
    ngx_http_hello_create_main_conf,        /* create main configuration */
    ngx_http_hello_init_main_conf,          /* init main configuration */
    ngx_http_hello_create_srv_conf,         /* create server configuration */
    ngx_http_hello_merge_srv_conf,          /* merge server configuration */
    ngx_http_hello_create_loc_conf,         /* create location configuration */
    ngx_http_hello_merge_loc_conf,          /* merge location configuration */
};

ngx_module_t ngx_http_hello_module =
{
    NGX_MODULE_V1,              /* ��䴦�� */
    &ngx_http_hello_module_ctx, /* module context */
    ngx_http_hello_commands,    /* module directives */
    NGX_HTTP_MODULE,            /* module type */
    NULL,                       /* init master */
    NULL,                       /* init module */
    NULL,                       /* init process */
    NULL,                       /* init thread */
    NULL,                       /* exit thread */
    NULL,                       /* exit process */
    NULL,                       /* exit master */
    NGX_MODULE_V1_PADDING       /* ��䴦�� */
};
static ngx_int_t ngx_http_hello_preconfiguration(ngx_conf_t *cf)
{
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_hello_preconfiguration is called");
    return NGX_OK;
}

static ngx_int_t ngx_http_hello_init(ngx_conf_t *cf)
{
    ngx_http_handler_pt        *h;
    ngx_http_core_main_conf_t  *cmcf;
	ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_hello_postconfiguration is called");
    cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module);
    h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers);
    if (h == NULL)
    {
        return NGX_ERROR;
    }
    *h = ngx_http_hello_handler;
    return NGX_OK;
}

static void *ngx_http_hello_create_main_conf(ngx_conf_t *cf)
{
    ngx_http_hello_loc_conf_t *main_conf = NULL;
	ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_hello_create_main_conf is called");
    main_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_loc_conf_t));
    if (main_conf == NULL)
    {
        return NULL;
    }
    ngx_str_null(&main_conf->hello_string);
    return main_conf;
}
static char *ngx_http_hello_init_main_conf(ngx_conf_t *cf, void *conf)
{
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_hello_init_main_conf is called");
    return NULL;
}
static void *ngx_http_hello_create_srv_conf(ngx_conf_t *cf)
{
    ngx_http_hello_loc_conf_t *srv_conf = NULL;
	ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_hello_create_srv_conf is called");
    srv_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_loc_conf_t));
    if (srv_conf == NULL)
    {
        return NULL;
    }
    ngx_str_null(&srv_conf->hello_string);
    return srv_conf;
}
static char *ngx_http_hello_merge_srv_conf(ngx_conf_t *cf, void *prev, void *conf)
{
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_hello_merge_srv_conf is called");
    return NULL;
}
static void *ngx_http_hello_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_hello_loc_conf_t *local_conf = NULL;
    local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_hello_loc_conf_t));
    if (local_conf == NULL)
    {
        return NULL;
    }
    ngx_str_null(&local_conf->hello_string);
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_hello_create_loc_conf is called");
    return local_conf;
}
static char *ngx_http_hello_merge_loc_conf(ngx_conf_t *cf, void *prev, void *conf)
{
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "ngx_http_hello_merge_loc_conf is called");
    return NULL;
}
static char *ngx_http_hello(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    /*ngx_http_core_loc_conf_t *clcf;
    clcf = ngx_http_conf_get_module_loc_conf(cf,ngx_http_core_module);
    clcf->handler = ngx_http_hello_handler;
    return NGX_CONF_OK;*/
    ngx_http_hello_loc_conf_t *local_conf;
	char *rv;
    local_conf = conf;
    rv = ngx_conf_set_str_slot(cf, cmd, conf);
    ngx_conf_log_error(NGX_LOG_EMERG, cf, 0, "hello_string:%s", local_conf->hello_string.data);
    return rv;
}

static ngx_int_t ngx_http_hello_handler(ngx_http_request_t *r)
{
    ngx_int_t rc;
    ngx_buf_t *b;
    ngx_chain_t out;
    ngx_http_hello_loc_conf_t *my_conf;
    u_char ngx_hello_string[1024] = {0};
    ngx_uint_t content_length = 0;
	ngx_str_t type = ngx_string("text/plain");

    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_hello_handler is called");
    my_conf = ngx_http_get_module_loc_conf(r, ngx_http_hello_module);
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "get ngx_http_hello_loc_conf_t");
    if (my_conf == NULL)
    {
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "my_conf is null");
    }
    if (my_conf->hello_string.len == 0)
    {
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "empty!");
        return NGX_DECLINED;
    }
    else
    {
        ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "string is not empty");
        ngx_sprintf(ngx_hello_string, "%V", &my_conf->hello_string);
    }
    content_length = ngx_strlen(ngx_hello_string);
    ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "els");
    if (!(r->method & (NGX_HTTP_GET | NGX_HTTP_HEAD)))
    {
        return NGX_HTTP_NOT_ALLOWED;
    }
    rc = ngx_http_discard_request_body(r);
    if (rc != NGX_OK)
    {
        return rc;
    }
    
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = content_length;
    r->headers_out.content_type = type;
    /* HTTP����ṩ�ķ���HTTPͷ���ķ��� */
    /* ����NGX_ERROR�򷵻�ֵ����0�ͱ�ʾ������ */
    rc = ngx_http_send_header(r);
    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only)
    {
        return rc;
    }
    b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL)
    {
        return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }
    b->pos = ngx_hello_string;
    b->last = ngx_hello_string + content_length;
    b->memory = 1;
    b->last_buf = 1;
    out.buf = b;
    out.next = NULL;
    return ngx_http_output_filter(r, &out);
}
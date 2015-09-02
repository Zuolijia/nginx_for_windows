/*
	Copyright (C) Paul 
	rwhsysu@163.com

	for nginx.conf 
	location /sendfile {
		sendflag on;
		filename ...;
	} 
*/

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

typedef struct 
{
	ngx_flag_t sendflag;
	ngx_str_t filename;
}ngx_http_sendfile_loc_conf_t;

static ngx_int_t ngx_http_sendfile_init(ngx_conf_t *cf); //handler���غ���,  content phase handlers

static void * ngx_http_sendfile_create_loc_conf(ngx_conf_t *cf); //����local_conf

static ngx_int_t ngx_http_sendfile_handler(ngx_http_request_t *r); //handler ��������




static ngx_command_t ngx_http_sendfile_commands[]={
	{
		ngx_string("sendflag"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1,/*
		NGX_HTTP_LOC_CONF: ���Գ�����http server�������location����ָ���
		NGX_CONF_NOARGS������ָ������κβ�����
		or  NGX_CONF_TAKE1������ָ�����1��������
        */
        ngx_conf_set_flag_slot, /*���ô�����

		nginx�Ѿ�Ĭ���ṩ�˶�һЩ��׼���͵Ĳ������ж�ȡ�ĺ���������ֱ�Ӹ�ֵ��set�ֶ�ʹ�á���������һ����Щ�Ѿ�ʵ�ֵ�set���ͺ�����
		ngx_conf_set_flag_slot�� ��ȡNGX_CONF_FLAG���͵Ĳ�����
		ngx_conf_set_str_slot:��ȡ�ַ������͵Ĳ�����
		ngx_conf_set_str_array_slot: ��ȡ�ַ����������͵Ĳ�����
		ngx_conf_set_keyval_slot�� ��ȡ��ֵ�����͵Ĳ�����
		ngx_conf_set_num_slot: ��ȡ��������(�з�������ngx_int_t)�Ĳ�����
		ngx_conf_set_size_slot:��ȡsize_t���͵Ĳ�����Ҳ�����޷�������
		ngx_conf_set_off_slot: ��ȡoff_t���͵Ĳ�����
		ngx_conf_set_msec_slot: ��ȡ����ֵ���͵Ĳ�����
		ngx_conf_set_sec_slot: ��ȡ��ֵ���͵Ĳ�����
		ngx_conf_set_bufs_slot�� ��ȡ�Ĳ���ֵ��2����һ����buf�ĸ�����һ����buf�Ĵ�С�����磺 output_buffers 1 128k;
		ngx_conf_set_enum_slot: ��ȡö�����͵Ĳ���������ת��������ngx_uint_t���͡�
		ngx_conf_set_bitmask_slot: ��ȡ������ֵ��������Щ������ֵ��bitλ����ʽ�洢�����磺HttpDavModuleģ���dav_methodsָ�
        */
        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_sendfile_loc_conf_t, sendflag),
        NULL
	},
	{
		ngx_string("filename"),
        NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS|NGX_CONF_TAKE1,/*
		NGX_HTTP_LOC_CONF: ���Գ�����http server�������location����ָ���
		NGX_CONF_NOARGS������ָ������κβ�����
		or  NGX_CONF_TAKE1������ָ�����1��������
        */
        ngx_conf_set_str_slot,

        NGX_HTTP_LOC_CONF_OFFSET,
        offsetof(ngx_http_sendfile_loc_conf_t, filename),
        NULL
	},
	ngx_null_command
};

static ngx_http_module_t ngx_http_sendfile_module_ctx = {
        NULL,                          /* preconfiguration */
        ngx_http_sendfile_init,           /* postconfiguration */

        NULL,                          /* create main configuration */
        NULL,                          /* init main configuration */

        NULL,                          /* create server configuration */
        NULL,                          /* merge server configuration */

        ngx_http_sendfile_create_loc_conf, /* create location configuration */
        NULL                            /* merge location configuration */
};

ngx_module_t ngx_http_sendfile_module = {
        NGX_MODULE_V1,
        &ngx_http_sendfile_module_ctx,    /* module context */
        ngx_http_sendfile_commands,       /* module directives */
        NGX_HTTP_MODULE,               /* module type */
        NULL,                          /* init master */
        NULL,                          /* init module */
        NULL,                          /* init process */
        NULL,                          /* init thread */
        NULL,                          /* exit thread */
        NULL,                          /* exit process */
        NULL,                          /* exit master */
        NGX_MODULE_V1_PADDING
};

static ngx_int_t ngx_http_sendfile_init(ngx_conf_t *cf)
{
	ngx_http_handler_pt  *h;
	ngx_http_core_main_conf_t *cmcf;
	cmcf = ngx_http_conf_get_module_main_conf(cf, ngx_http_core_module); //ȡ��core_module��cf

        h = ngx_array_push(&cmcf->phases[NGX_HTTP_CONTENT_PHASE].handlers); // ���غ�������Ӧ����׶�
        if (h == NULL) {
                return NGX_ERROR;
        }

        *h = ngx_http_sendfile_handler; //������ָ��ָ��handler����

        return NGX_OK;
}

static void *ngx_http_sendfile_create_loc_conf(ngx_conf_t *cf)
{
        ngx_http_sendfile_loc_conf_t* local_conf = NULL;
        local_conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_sendfile_loc_conf_t));
        if (local_conf == NULL)
        {
                return NULL;
        }

        ngx_str_null(&local_conf->filename);/*
		conf->teststr.len = 0;
        conf->teststr.data = NULL;
        //*/
        local_conf->sendflag = NGX_CONF_UNSET;
        //local_conf->testnum = NGX_CONF_UNSET;
        //local_conf->testsize = NGX_CONF_UNSET_SIZE;
        /*
		��Щ������Ϣһ��Ĭ�϶�Ӧ����Ϊһ��δ��ʼ����ֵ������������Nginx������һϵ�еĺ궨�������������������Ӧ�������͵�δ��ʼ��ֵ�����£�
		#define NGX_CONF_UNSET       -1
		#define NGX_CONF_UNSET_UINT  (ngx_uint_t) -1
		#define NGX_CONF_UNSET_PTR   (void *) -1
		#define NGX_CONF_UNSET_SIZE  (size_t) -1
		#define NGX_CONF_UNSET_MSEC  (ngx_msec_t) -1
        */

        return local_conf;
}

static ngx_int_t ngx_http_sendfile_handler(ngx_http_request_t *r)
{
	ngx_int_t rc;
	ngx_buf_t *b;
	ngx_chain_t out;
	ngx_http_sendfile_loc_conf_t *my_cf;
	u_char namestring[1024] = {0};
	ngx_uint_t content_length = 0;
	

	ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "ngx_http_sendfile_handler is called!");

	my_cf = ngx_http_get_module_loc_conf(r,ngx_http_sendfile_module);
	// if (my_cf->sendflag == NGX_CONF_UNSET )
 //        {
 //                ngx_log_error(NGX_LOG_EMERG, r->connection->log, 0, "sendfile disabled");
 //                return NGX_DECLINED;
 //        }
    ngx_sprintf(namestring, "%V\0", &my_cf->filename);

    /* we response to 'GET' and 'HEAD' requests only */
        if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
                return NGX_HTTP_NOT_ALLOWED;
        }

        /* discard request body, since we don't need it here */
        rc = ngx_http_discard_request_body(r);

        if (rc != NGX_OK) {
                return rc;
        }

        /* allocate a buffer for your response body */
        b = ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
        if (b == NULL) {
                return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }

        /*��һ�ַ�ʽ
        ���ڴ���temporary�ֶ�Ϊ1��buf�����������ݿ��Ա�������filterģ������޸ģ�������ֱ��ʹ�ú���ngx_create_temp_buf���д�����

		ngx_buf_t *ngx_create_temp_buf(ngx_pool_t *pool, size_t size);
		�ú�������һ��ngx_but_t���͵Ķ��󣬲�����ָ����������ָ�룬����ʧ�ܷ���NULL��

		���ڴ����������������start��endָ���·����ڴ濪ʼ�ͽ����ĵط���pos��last��ָ������·����ڴ�Ŀ�ʼ���������������Ĳ�������������·�����ڴ��ϴ������ݡ�*/
        
        /* adjust the pointers of the buffer */

        b->in_file = 1;
        b->file = ngx_pcalloc(r->pool, sizeof(ngx_file_t));
        b->file->fd = ngx_open_file(namestring, NGX_FILE_RDONLY|NGX_FILE_NONBLOCK, NGX_FILE_OPEN, 0);
        b->file->log = r->connection->log;
        ngx_str_set(&b->file->name,namestring);
        if(b->file->fd <= 0){
        	return NGX_HTTP_NOT_FOUND;
        }
        if (ngx_file_info(namestring,&b->file->info) == NGX_FILE_ERROR){
        	return NGX_HTTP_INTERNAL_SERVER_ERROR;
        }
        content_length = b->file->info.st_size;
        //b->memory = 1;    /* this buffer is in memory */
        b->file_pos = 0;
        b->file_last = b->file->info.st_size;
        b->last_buf = 1;  /* this is the last buffer in the buffer chain */

        ngx_pool_cleanup_t *cln = ngx_pool_cleanup_add(r->pool, sizeof(ngx_pool_cleanup_file_t));
        if(cln == NULL)
        {
        	return NGX_ERROR;
        }

        cln->handler = ngx_pool_cleanup_file;
        ngx_pool_cleanup_file_t *clnf = cln->data;

        clnf->fd = b->file->fd;
        clnf->name = b->file->name.data;
        clnf->log = r->pool->log;

		/* set the 'Content-type' header */
        /*
         *r->headers_out.content_type.len = sizeof("text/html") - 1;
         *r->headers_out.content_type.data = (u_char *)"text/html";
         */
        ngx_str_set(&r->headers_out.content_type, "text/plain");
        r->headers_out.status = NGX_HTTP_OK;
        r->headers_out.content_length_n = content_length;
        /* send the header only, if the request type is http 'HEAD' */
        if (r->method == NGX_HTTP_HEAD) {
            	return ngx_http_send_header(r);
        }

        /* attach this buffer to the buffer chain */
        out.buf = b;
        out.next = NULL;
        /* send the headers of your response */
        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
                return rc;
        }

        /* send the buffer chain of your response */
        return ngx_http_output_filter(r, &out);
}

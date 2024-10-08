#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#pragma warning( disable : 4996)

static void clear_cr(char *tmp)
{
	int i;
	int len=strlen(tmp);
	if(0 == len){
		return;
	}
	len--;
	for(i=0; i < len; i++){
		char a=tmp[i];
		if(('\n' == a)){
			tmp[i+1]=0;
			break;
		}
	}
}

static void debug(const char *fmt,...)
{
#ifdef _DEBUG
	char tmp[256]={0};
	va_list ap;
	va_start(ap,fmt);
	vsnprintf(tmp,sizeof(tmp),fmt,ap);
	tmp[sizeof(tmp) - 1]=0;
	clear_cr(tmp);
	OutputDebugStringA(tmp);
#endif
}

static void put_char(char a,char *out,const int out_len,int *out_index)
{
	int index=out_index[0];
	if((index+1) >= out_len){
		return;
	}
	out[index]=a;
	out_index[0]=index + 1;
}

static int is_wspace(char a)
{
	if((' ' == a) || ('\t' == a) || ('\r' == a) || ('\n' == a)){
		return 1;
	}
	return 0;
}
static int is_crlf(char a)
{
	if(('\r' == a) || ('\n' == a)){
		return 1;
	}
	return 0;
}
static int char_in_str(const char a,const char *str)
{
	int result=0;
	int len=strlen(str);
	int i;
	for(i=0; i < len; i++){
		if(a == str[i]){
			result=1;
			break;
		}
	}
	return result;
}
static void write_indent(const int indent,char *out,const int out_len,int *out_index)
{
	int i;
	int level=indent;
	for(i=0; i < level; i++){
		put_char('\t',out,out_len,out_index);
	}
}
static void write_crlf(char *out,const int out_len,int *out_index)
{
	put_char('\r',out,out_len,out_index);
	put_char('\n',out,out_len,out_index);
}
static int is_state(const int *states,const int state)
{
	int result=0;
	int i;
	for(i=0; i < 100; i++){
		const int s=states[i];
		if(s < 0){
			break;
		}
		if(s == state){
			result=1;
			break;
		}
	}
	return result;
}
enum{
	WAITING_KEY=0,
	WAITING_COLON,
	WAITING_VALUE,
	IN_KEY,
	IN_VALUE,
	IN_VALUE_STR,
	IN_COMMENT,
	IN_COMMENT_BLOCK,
	AFTER_VALUE,
	AFTER_COMMA,
	NEW_LINE,
	IGNORE_CHAR,
};

static const char *get_enum_name(const int val)
{
	const char *result="UNKNOWN";
#ifdef _DEBUG
	typedef struct{
		int val;
		const char *name;
	}KEY_MAP;
	static KEY_MAP key_map[]={
		{WAITING_KEY,      "WAITING_KEY"},
		{WAITING_COLON,	   "WAITING_COLON"},
		{WAITING_VALUE,	   "WAITING_VALUE"},
		{IN_KEY,		   "IN_KEY"},
		{IN_VALUE,		   "IN_VALUE"},
		{IN_VALUE_STR,	   "IN_VALUE_STR"},
		{IN_COMMENT,	   "IN_COMMENT"},
		{IN_COMMENT_BLOCK, "IN_COMMENT_BLOCK"},
		{AFTER_VALUE,	   "AFTER_VALUE"},
		{AFTER_COMMA,	   "AFTER_COMMA"},
		{NEW_LINE,		   "NEW_LINE"},
		{IGNORE_CHAR,	   "IGNORE_CHAR"}
	};
	int i,count=sizeof(key_map) / sizeof(key_map[0]);

	for(i=0; i < count; i++){
		if(val == key_map[i].val){
			result=key_map[i].name;
			break;
		}
	}
#endif
	return result;
}
void do_format_json(char *data,int data_len,char *out,int out_len)
{
	int state=0;
	int i;
	int out_index=0;
	int indent=0;
	for(i=0; i < data_len; i++){
		int write_char=0;
		char a=data[i];
		char b=data[i + 1];
		char *dptr=data + i;
		//debug("%s:%.20s\n",get_enum_name(state),data + i);
		switch(a){
		case '#':
		case '/':
			if(IN_KEY == state){

			}
			else if(IN_VALUE_STR == state){

			}
			else{
				const int post_states[]={
					AFTER_VALUE,AFTER_COMMA,-1
				};
				if('/' == b){
					if(is_state(post_states,state)){
						put_char(' ',out,out_len,&out_index);
					}
					else if(!(IN_COMMENT_BLOCK == state)){
						if(WAITING_KEY == state){
							write_indent(indent,out,out_len,&out_index);
						}
						state=IN_COMMENT;
					}
				}
				else if('*' == b){
					if(is_state(post_states,state)){
						put_char(' ',out,out_len,&out_index);
					}
					else if(!(IN_COMMENT == state)){
						if(WAITING_KEY == state){
							write_indent(indent,out,out_len,&out_index);
						}
						state=IN_COMMENT_BLOCK;
					}
				}
				else if('#' == a){
					if(is_state(post_states,state)){
						put_char(' ',out,out_len,&out_index);
					}
					else if(!(IN_COMMENT_BLOCK == state)){
						if(WAITING_KEY == state){
							write_indent(indent,out,out_len,&out_index);
						}
						state=IN_COMMENT;
					}
				}
			}
		break;
		case '*':
			if('/' == b){
				if(IN_COMMENT_BLOCK == state){
					put_char('*',out,out_len,&out_index);
					put_char('/',out,out_len,&out_index);
					write_crlf(out,out_len,&out_index);
					state=WAITING_KEY;
					i++;
					a=' ';
				}
			}
			break;
		case '\r':
			continue;
			break;
		case '\n':
			if(IN_COMMENT==state){
				state=NEW_LINE;
			}
			else if(AFTER_VALUE==state){
				state=NEW_LINE;
			}
			break;
		case '"':
			if(IN_KEY == state){
				state=WAITING_COLON;
			}
			else if(WAITING_KEY==state){
				write_indent(indent,out,out_len,&out_index);
				state=IN_KEY;
			}
			else if(WAITING_VALUE==state){
				state=IN_VALUE_STR;
			}
			else if(IN_VALUE_STR == state){
				state=AFTER_VALUE;
			}
			else if(AFTER_COMMA == state){
				write_crlf(out,out_len,&out_index);
				write_indent(indent,out,out_len,&out_index);
				state=IN_KEY;
			}
			break;
		case ':':
			if(WAITING_COLON == state){
				state=WAITING_VALUE;
			}
			break;
		case '{':
		case '}':
		case '[':
		case ']':
			switch(state){
			case IN_KEY:
			case IN_VALUE_STR:
			case IN_COMMENT:
			case IN_COMMENT_BLOCK:
				break;
			default:
			{
				char tmp=a;
				if(char_in_str(tmp,"}")){
					indent--;
					if(indent < 0){
						indent=0;
					}
				}
				if(char_in_str(a,"{}[]")){
					write_crlf(out,out_len,&out_index);
					write_indent(indent,out,out_len,&out_index);
					put_char(a,out,out_len,&out_index);
					write_crlf(out,out_len,&out_index);
					a=' ';
				}
				if(char_in_str(tmp,"{")){
					indent++;
				}
				state=WAITING_KEY;
			}
			}
			break;
		case ',':
			if(IN_VALUE == state){
				state=AFTER_COMMA;
			}
			else if(AFTER_VALUE == state){
				state=AFTER_COMMA;
			}
			else if(WAITING_COLON == state){
				state=AFTER_COMMA;
			}
			else if(WAITING_KEY==state){
				write_crlf(out,out_len,&out_index);
				write_indent(indent,out,out_len,&out_index);
				put_char(a,out,out_len,&out_index);
				write_crlf(out,out_len,&out_index);
				a=' ';
			}
			break;
		default:
			if(WAITING_VALUE==state){
				if(!is_wspace(a)){
					state=IN_VALUE;
				}
			}
			else if(IN_VALUE == state){
				if(is_wspace(a)){
					state=AFTER_VALUE;
				}
			}
			break;
		}

		write_char=0;
		switch(state){
		case IN_KEY:
		case IN_VALUE:
		case IN_VALUE_STR:
		case IN_COMMENT:
		case IN_COMMENT_BLOCK:
			write_char=1;
			break;
		case NEW_LINE:
			if(!is_wspace(a)){
				put_char(a,out,out_len,&out_index);
			}
			write_crlf(out,out_len,&out_index);
			write_char=0;
			state=WAITING_KEY;
			break;
		default:
			if(!is_wspace(a)){
				write_char=1;
			}
			break;
		}
		if(write_char){
			put_char(a,out,out_len,&out_index);
		}

	}
	if(out_len>0){
		out[out_len - 1]=0;
	}
}

void remove_empty_lines(char *in,const int in_len,char *out,const int out_len)
{
	int i;
	int out_index=0;
	int found_text=0;
	int start_index=0;
	for(i=0; i < in_len; i++){
		unsigned char a=in[i];
		if('\n' == a){
			if(found_text){
				start_index=out_index+1;
				found_text=0;
			}
			else{
				out_index=start_index;
				continue;
			}
		}
		else if(a>' '){
			found_text=1;
		}
		put_char(a,out,out_len,&out_index);
		if(0 == a){
			break;
		}
	}
	if(out_len > 0){
		out[out_len - 1]=0;
	}
}

static void *read_file_data(const char *fname,int *out_len)
{
	FILE *f;
	int len;
	char *data=0;
	f=fopen(fname,"rb");
	if(f){
		fseek(f,0,SEEK_END);
		len=ftell(f);
		fseek(f,0,SEEK_SET);
		data=calloc(len + 10,1);
		if(data){
			fread(data,1,len,f);
			*out_len=len;
		}
		fclose(f);
	}
	return data;
}

static void write_fdata(char *data,int len,const char *fname)
{
	FILE *f;
	f=fopen(fname,"wb");
	if(f){
		fwrite(data,1,len,f);
		fclose(f);
	}
}

int main(int argc,char **argv)
{
	char *data=0;
	int data_len=0;
	char *tmp=0;
	int tmp_len;
	data=read_file_data("b:\\test_data.jsonc",&data_len);
	if(data){
		tmp_len=data_len * 10;
		tmp=calloc(tmp_len,1);
		if(tmp){
			char *tmp2;
			int tmp2_len=tmp_len;
			do_format_json(data,data_len,tmp,tmp_len);
			tmp2=calloc(tmp2_len,1);
			if(tmp2){
				remove_empty_lines(tmp,tmp_len,tmp2,tmp2_len);
				free(tmp);
				tmp=tmp2;
				tmp2=0;
			}
			int len=strlen(tmp);
			write_fdata(tmp,len,"b:\\out.txt");
		}
	}
    return 0;
}


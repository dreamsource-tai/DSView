/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 * Copyright (C) 2014 DreamSourceLab <support@dreamsourcelab.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include "AnnotationResTable.h"
#include <assert.h>
#include "../../dsvdef.h"

#define DECODER_MAX_DATA_BLOCK_LEN 25

const char g_bin_cvt_table[] = "0000000100100011010001010110011110001001101010111100110111101111";
char g_bin_format_tmp_buffer[DECODER_MAX_DATA_BLOCK_LEN * 4 + 2];
char g_oct_format_tmp_buffer[DECODER_MAX_DATA_BLOCK_LEN * 3 + 2];
char g_number_tmp_64[30];
 
 char* bin2oct_string(char *buf, int size, const char *bin, int len){
	char *wr = buf + size - 1;
	*wr = 0; //end flag

	char *rd = (char*)bin + len - 1; //move to last byte
	char tmp[3]; 

	while (rd >= bin && wr > buf)
	{  
		wr--;
		int num = 0;

		while (rd >= bin && num < 3)
		{
			tmp[2-num] = *rd;			 
			rd--;
			num++;
		}

		//fill
		while (num < 3)
		{
			tmp[2-num] = '0';
			++num;
		}		

	    if (strncmp(tmp, "000", 3) == 0)
			*wr = '0';
		else if (strncmp(tmp, "001", 3) == 0)
			*wr = '1';
		else if (strncmp(tmp, "010", 3) == 0)
			*wr = '2';
		else if (strncmp(tmp, "011", 3) == 0)
			*wr = '3';
		else if (strncmp(tmp, "100", 3) == 0)
			*wr = '4';
		else if (strncmp(tmp, "101", 3) == 0)
			*wr = '5';
		else if (strncmp(tmp, "110", 3) == 0)
			*wr = '6';
		else if (strncmp(tmp, "111", 3) == 0)
			*wr = '7';
	} 

	return wr;
}

long long bin2long_string(const char *bin, int len)
{
	char *rd = (char *)bin + len - 1; //move to last byte
	int dex = 0;
	long long value = 0;
	long long bv = 0;

    while (rd >= bin)
	{
		if (*rd == '1')
		{
			bv = 1 << dex;
			value += bv;
		}
		rd--;
		++dex;
	}

	return value;
}

 
int AnnotationResTable::MakeIndex(const std::string &key, AnnotationSourceItem* &newItem)
{   
    auto fd = m_indexs.find(key);
    if (fd != m_indexs.end()){
        return (*fd).second;
    } 
  
    AnnotationSourceItem *item = new AnnotationSourceItem();
    m_resourceTable.push_back(item);

    item->cur_display_format = -1;
    item->is_numeric = false;
    newItem = item;
   
    int dex = m_indexs.size();
    m_indexs[key] = dex;
    return dex;
}

AnnotationSourceItem* AnnotationResTable::GetItem(int index){
    assert(index >= 0 && index < m_resourceTable.size());
    return m_resourceTable[index];
}

const char* AnnotationResTable::format_numberic(const char *hex_str, int fmt)
{
    assert(hex_str);

    //flow, convert to oct\dec\bin format
	 const char *data = hex_str;
	 if (data[0] == 0 || fmt == DecoderDataFormat::hex){
		 return data;
	 }
	
	 //convert to bin format
	 char *buf = g_bin_format_tmp_buffer + sizeof(g_bin_format_tmp_buffer) - 2;
	 buf[1] = 0; //set the end flag
	 buf[0] = 0;

	 int len = strlen(data);
	  //buffer is not enough
	 if (len > DECODER_MAX_DATA_BLOCK_LEN){
		 return data;
	 }

     char *rd = (char*)data + len - 1; //move to last byte
	 char c = 0;
	 int dex = 0;

	 while (rd >= data)
	 {
		 c = *rd;
		 dex = (int)(c <= '9' ? (c - '0') : (c - 'A' + 10));
         char *ptable = (char*)g_bin_cvt_table + dex * 4;

		 buf -= 4; //move to left for 4 bytes
		 buf[0] = ptable[0];
		 buf[1] = ptable[1];
		 buf[2] = ptable[2];
		 buf[3] = ptable[3];
	
		 rd--;
	 } 

	 //get bin format 
	 if (fmt == DecoderDataFormat::bin){
		  return buf;
	 }

	 //get oct format
	 if (fmt == DecoderDataFormat::oct){
		 char *oct_buf = bin2oct_string(g_oct_format_tmp_buffer, 
		                  sizeof(g_oct_format_tmp_buffer), buf,  len * 4);
		 return oct_buf;
	 }

	//64 bit integer
	 if (fmt == DecoderDataFormat::dec && len * 4 <= 64){
         long long lv = bin2long_string(buf, len * 4);
		 g_number_tmp_64[0] = 0;
    	 sprintf(g_number_tmp_64, "%lld", lv);
         return g_number_tmp_64;
	 }
	 
	 //ascii
	 if (fmt == DecoderDataFormat::ascii && len < 30 - 3){
		 if (len == 2){
             int lv = (int)bin2long_string(buf, len * 4);
			 //can display chars
			 if (lv >= 33 && lv <= 126){
				 sprintf(g_number_tmp_64, "%c", (char)lv);
				 return g_number_tmp_64;
			 }
		 }
		 char * const wr_buf = g_number_tmp_64;
         g_number_tmp_64[0] = '[';
         strcpy(g_number_tmp_64 + 1, data);
         g_number_tmp_64[len+1] = ']';
         g_number_tmp_64[len+2] = 0;
         return g_number_tmp_64;
	 }

    return data;    
}
 

/*****************************************************************
** �ļ���:sps_ppsģ��		sps_pps.c
** Copyright (c) 2014 xxxxxxxxxxxxxxxxxx

** ������: Yoto
** ��  ��: 2013-11-02
** ��  ��: sps_ppsģ����Ҫ��ɶ�H264��sps��pps����ȡ����������������Ϣ��һ����������
			1;��ǰ�汾�Ƚϼ򵥾��Ƕ�sps_ppsһЩ���õĺ������м򵥷�װ
			2:����������һЩ���ܺ������߱Ƚϸ�Ч�ĺ����ӿ�
** ��  ��: V1.0

** �޸���:
** ��  ��:
** �޸�����:
** ��  ��:
******************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> /* for uint32_t, etc */
#include <UtilS_SPS_PPS.h>
//#include "Zyfly_Interface.h"

#ifdef __cplusplus
extern "C" {
#endif
/* report level */
#define RPT_ERR 		(1) // error, system error
#define RPT_WRN 		(2) // warning, maybe wrong, maybe OK
#define RPT_INF 		(4) // important information
#define RPT_DBG 		(8) // debug information

static int rpt_lvl =RPT_ERR; /* report level: ERR, WRN, INF, DBG */

/* report micro */
#define RPT(lvl, ...) \
    do { \
        if(lvl & rpt_lvl) { \
            switch(lvl) { \
                case RPT_ERR: \
                    fprintf(stderr, "\"%s\" line %d [err]: ", __FILE__, __LINE__); \
                    break; \
                case RPT_WRN: \
                    fprintf(stderr, "\"%s\" line %d [wrn]: ", __FILE__, __LINE__); \
                    break; \
                case RPT_INF: \
                    fprintf(stderr, "\"%s\" line %d [inf]: ", __FILE__, __LINE__); \
                    break; \
                case RPT_DBG: \
                    fprintf(stderr, "\"%s\" line %d [dbg]: ", __FILE__, __LINE__); \
                    break; \
                default: \
                    fprintf(stderr, "\"%s\" line %d [???]: ", __FILE__, __LINE__); \
                    break; \
                } \
                fprintf(stderr, __VA_ARGS__); \
                fprintf(stderr, "\n"); \
        } \
    } while(0)

/********************************************
*define here
********************************************/
#define SPS_PPS_DEBUG
#undef  SPS_PPS_DEBUG

#define MAX_LEN 32


/********************************************
*functions
********************************************/
/**
 *  @brief Function get_1bit()   ��1��bit
 *  @param[in]     h     get_bit_context structrue
 *  @retval        0: success, -1 : failure
 *  @pre
 *  @post
 */
static int get_1bit(void *h)
{
    get_bit_context *ptr = (get_bit_context *)h;
    int ret = 0;
    uint8_t *cur_char = NULL;
    uint8_t shift;

    if(NULL == ptr)
    {
        RPT(RPT_ERR, "NULL pointer");
        ret = -1;
        goto exit;
    }

    cur_char = ptr->buf + (ptr->bit_pos >> 3);
    shift = 7 - (ptr->cur_bit_pos);
    ptr->bit_pos++;
    ptr->cur_bit_pos = ptr->bit_pos & 0x7;
    ret = ((*cur_char) >> shift) & 0x01;

exit:
    return ret;
}


/**
 *  @brief Function get_bits()  ��n��bits��n���ܳ���32
 *  @param[in]     h     get_bit_context structrue
 *  @param[in]     n     how many bits you want?
 *  @retval        0: success, -1 : failure
 *  @pre
 *  @post
 */
static int get_bits(void *h, int n)
{
    get_bit_context *ptr = (get_bit_context *)h;
    uint8_t temp[5] = {0};
    uint8_t *cur_char = NULL;
    uint8_t nbyte = 0;
    uint8_t shift = 0;
    uint32_t result =0;
    uint64_t ret = 0;

    if(NULL == ptr)
    {
        RPT(RPT_ERR, "NULL pointer");
        ret = -1;
        goto exit;
    }

    if(n > MAX_LEN)
    {
        n = MAX_LEN;
    }

    if((ptr->bit_pos + n) > ptr->total_bit)
    {
        n = ptr->total_bit- ptr->bit_pos;
    }

    cur_char = ptr->buf+ (ptr->bit_pos>>3);
    nbyte = (ptr->cur_bit_pos + n + 7) >> 3;
    shift = (8 - (ptr->cur_bit_pos + n))& 0x07;

    if(n == MAX_LEN)
    {
        RPT(RPT_DBG, "12(ptr->bit_pos(:%d) + n(:%d)) > ptr->total_bit(:%d)!!! ",\
                ptr->bit_pos, n, ptr->total_bit);
        RPT(RPT_DBG, "0x%x 0x%x 0x%x 0x%x", (*cur_char), *(cur_char+1),*(cur_char+2),*(cur_char+3));
    }

    memcpy(&temp[5-nbyte], cur_char, nbyte);
    ret = (uint32_t)temp[0] << 24;
    ret = ret << 8;
    ret = ((uint32_t)temp[1]<<24)|((uint32_t)temp[2] << 16)\
                        |((uint32_t)temp[3] << 8)|temp[4];

    ret = (ret >> shift) & (((uint64_t)1<<n) - 1);

    result = ret;
    ptr->bit_pos += n;
    ptr->cur_bit_pos = ptr->bit_pos & 0x7;

exit:
    return result;
}


/**
 *  @brief Function parse_codenum() ָ�����ײ�����������ο�h264��׼��9��
 *  @param[in]     buf
 *  @retval        code_num
 *  @pre
 *  @post
 */
static int parse_codenum(void *buf)
{
    uint8_t leading_zero_bits = -1;
    uint8_t b;
    uint32_t code_num = 0;

    for(b=0; !b; leading_zero_bits++)
    {
        b = get_1bit(buf);
    }

    code_num = ((uint32_t)1 << leading_zero_bits) - 1 + get_bits(buf, leading_zero_bits);

    return code_num;
}

/**
 *  @brief Function parse_ue() ָ�����ײ�������� ue(),�ο�h264��׼��9��
 *  @param[in]     buf       sps_pps parse buf
 *  @retval        code_num
 *  @pre
 *  @post
 */
static int parse_ue(void *buf)
{
    return parse_codenum(buf);
}


/**
 *  @brief Function parse_se() ָ�����ײ�������� se(), �ο�h264��׼��9��
 *  @param[in]     buf       sps_pps parse buf
 *  @retval        code_num
 *  @pre
 *  @post
 */
static int parse_se(void *buf)
{
    int ret = 0;
    int code_num;

    code_num = parse_codenum(buf);
    ret = (code_num + 1) >> 1;
    ret = (code_num & 0x01)? ret : -ret;

    return ret;
}


/**
 *  @brief Function get_bit_context_free()  �����get_bit_context�ṹ�ڴ��ͷ�
 *  @param[in]     buf       get_bit_context buf
 *  @retval        none
 *  @pre
 *  @post
 */
static void get_bit_context_free(void *buf)
{
    get_bit_context *ptr = (get_bit_context *)buf;

    if(ptr)
    {
        if(ptr->buf)
        {
            free(ptr->buf);
        }

        free(ptr);
    }
}


/**
 *  @brief Function de_emulation_prevention()  �⾺������
 *  @param[in]     buf       get_bit_context buf
 *  @retval        none
 *  @pre
 *  @post
 *  @note:
 *      ����ʱ���Ƿ���vui.time_scaleֵ�ر���֣�����16777216��������ѯԭ������:
 *      http://www.cnblogs.com/eustoma/archive/2012/02/13/2415764.html
 *      H.264����ʱ����ÿ��NALǰ�����ʼ�� 0x000001���������������м�⵽��ʼ�룬��ǰNAL������
 *      Ϊ�˷�ֹNAL�ڲ�����0x000001�����ݣ�h.264�����'��ֹ���� emulation prevention"���ƣ�
 *      �ڱ�����һ��NALʱ�������������������0x00�ֽڣ����ں������һ��0x03��
 *      ����������NAL�ڲ���⵽0x000003�����ݣ��Ͱ�0x03�������ָ�ԭʼ���ݡ�
 *      0x000000  >>>>>>  0x00000300
 *      0x000001  >>>>>>  0x00000301
 *      0x000002  >>>>>>  0x00000302
 *      0x000003  >>>>>>  0x00000303
 */
static void *de_emulation_prevention(void *buf)
{
    get_bit_context *ptr = NULL;
    get_bit_context *buf_ptr = (get_bit_context *)buf;
    int i = 0, j = 0;
    uint8_t *tmp_ptr = NULL;
    int tmp_buf_size = 0;
    int val = 0;
    if(NULL == buf_ptr)
    {
        RPT(RPT_ERR, "NULL ptr");
        goto exit;
    }
    ptr = (get_bit_context *)malloc(sizeof(get_bit_context));
    if(NULL == ptr)
    {
        RPT(RPT_ERR, "NULL ptr");
        goto exit;
    }
    memcpy(ptr, buf_ptr, sizeof(get_bit_context));
    ptr->buf = (uint8_t *)malloc(ptr->buf_size);
    if(NULL == ptr->buf)
    {
        RPT(RPT_ERR, "NULL ptr ");
        goto exit;
    }
    memcpy(ptr->buf, buf_ptr->buf, buf_ptr->buf_size);

    tmp_ptr = ptr->buf;
    tmp_buf_size = ptr->buf_size;
    for(i=0; i<(tmp_buf_size-2); i++)
    {
        /*���0x000003*/
        val = (tmp_ptr[i]^0x00) + (tmp_ptr[i+1]^0x00) + (tmp_ptr[i+2]^0x03);
        if(val == 0)
        {
            /*�޳�0x03*/
            for(j=i+2; j<tmp_buf_size-1; j++)
            {
                tmp_ptr[j] = tmp_ptr[j+1];
            }

            /*��Ӧ��bufsizeҪ��С*/
            ptr->buf_size--;
        }
    }

    /*���¼���total_bit*/
	ptr->bit_pos = 0;
	ptr->cur_bit_pos = 0;
    ptr->total_bit = ptr->buf_size << 3;
    return (void *)ptr;

exit:
    get_bit_context_free(ptr);
    return NULL;
}


/**
 *  @brief Function get_bit_context_free()  VUI_parameters ������ԭ��ο�h264��׼
 *  @param[in]     buf       get_bit_context buf
 *  @param[in]     vui_ptr   vui�������
 *  @retval        0: success, -1 : failure
 *  @pre
 *  @post
 */
static int vui_parameters_set(void *buf, vui_parameters_t *vui_ptr)
{
    int ret = 0;
    int SchedSelIdx = 0;

    if(NULL == vui_ptr || NULL == buf)
    {
        RPT(RPT_ERR,"ERR null pointer\n");
        ret = -1;
        goto exit;
    }

    vui_ptr->aspect_ratio_info_present_flag = get_1bit(buf);
    if(vui_ptr->aspect_ratio_info_present_flag)
    {
        vui_ptr->aspect_ratio_idc = get_bits(buf, 8);
        if(vui_ptr->aspect_ratio_idc == Extended_SAR)
        {
            vui_ptr->sar_width = get_bits(buf, 16);
            vui_ptr->sar_height = get_bits(buf, 16);
        }
    }

    vui_ptr->overscan_info_present_flag = get_1bit(buf);
    if(vui_ptr->overscan_info_present_flag)
    {
        vui_ptr->overscan_appropriate_flag = get_1bit(buf);
    }

    vui_ptr->video_signal_type_present_flag = get_1bit(buf);
    if(vui_ptr->video_signal_type_present_flag)
    {
        vui_ptr->video_format = get_bits(buf, 3);
        vui_ptr->video_full_range_flag = get_1bit(buf);

        vui_ptr->colour_description_present_flag = get_1bit(buf);
        if(vui_ptr->colour_description_present_flag)
        {
            vui_ptr->colour_primaries = get_bits(buf, 8);
            vui_ptr->transfer_characteristics = get_bits(buf, 8);
            vui_ptr->matrix_coefficients = get_bits(buf, 8);
        }
    }

    vui_ptr->chroma_loc_info_present_flag = get_1bit(buf);
    if(vui_ptr->chroma_loc_info_present_flag)
    {
        vui_ptr->chroma_sample_loc_type_top_field = parse_ue(buf);
        vui_ptr->chroma_sample_loc_type_bottom_field = parse_ue(buf);
    }

    vui_ptr->timing_info_present_flag = get_1bit(buf);
    if(vui_ptr->timing_info_present_flag)
    {
        vui_ptr->num_units_in_tick = get_bits(buf, 32);
        vui_ptr->time_scale = get_bits(buf, 32);
        vui_ptr->fixed_frame_rate_flag = get_1bit(buf);
    }

    vui_ptr->nal_hrd_parameters_present_flag = get_1bit(buf);
    if(vui_ptr->nal_hrd_parameters_present_flag)
    {
        vui_ptr->cpb_cnt_minus1 = parse_ue(buf);
        vui_ptr->bit_rate_scale = get_bits(buf, 4);
        vui_ptr->cpb_size_scale = get_bits(buf, 4);

        for(SchedSelIdx=0; SchedSelIdx<=vui_ptr->cpb_cnt_minus1; SchedSelIdx++)
        {
            vui_ptr->bit_rate_value_minus1[SchedSelIdx] = parse_ue(buf);
            vui_ptr->cpb_size_value_minus1[SchedSelIdx] = parse_ue(buf);
            vui_ptr->cbr_flag[SchedSelIdx] = get_1bit(buf);
        }

        vui_ptr->initial_cpb_removal_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->cpb_removal_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->dpb_output_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->time_offset_length = get_bits(buf, 5);
    }


    vui_ptr->vcl_hrd_parameters_present_flag = get_1bit(buf);
    if(vui_ptr->vcl_hrd_parameters_present_flag)
    {
        vui_ptr->cpb_cnt_minus1 = parse_ue(buf);
        vui_ptr->bit_rate_scale = get_bits(buf, 4);
        vui_ptr->cpb_size_scale = get_bits(buf, 4);

        for(SchedSelIdx=0; SchedSelIdx<=vui_ptr->cpb_cnt_minus1; SchedSelIdx++)
        {
            vui_ptr->bit_rate_value_minus1[SchedSelIdx] = parse_ue(buf);
            vui_ptr->cpb_size_value_minus1[SchedSelIdx] = parse_ue(buf);
            vui_ptr->cbr_flag[SchedSelIdx] = get_1bit(buf);
        }

        vui_ptr->initial_cpb_removal_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->cpb_removal_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->dpb_output_delay_length_minus1 = get_bits(buf, 5);
        vui_ptr->time_offset_length = get_bits(buf, 5);
    }

    if(vui_ptr->nal_hrd_parameters_present_flag \
           || vui_ptr->vcl_hrd_parameters_present_flag)
    {
        vui_ptr->low_delay_hrd_flag = get_1bit(buf);
    }

    vui_ptr->pic_struct_present_flag = get_1bit(buf);

    vui_ptr->bitstream_restriction_flag = get_1bit(buf);
    if(vui_ptr->bitstream_restriction_flag)
    {
        vui_ptr->motion_vectors_over_pic_boundaries_flag = get_1bit(buf);
        vui_ptr->max_bytes_per_pic_denom = parse_ue(buf);
        vui_ptr->max_bits_per_mb_denom = parse_ue(buf);
        vui_ptr->log2_max_mv_length_horizontal= parse_ue(buf);
        vui_ptr->log2_max_mv_length_vertical = parse_ue(buf);
        vui_ptr->num_reorder_frames = parse_ue(buf);
        vui_ptr->max_dec_frame_buffering = parse_ue(buf);
    }

exit:
    return ret;
}


/**
 *  @brief Function sps_info_print() ������Ϣ��ӡ
 *  @param[in]     sps_ptr      sps��Ϣ�ṹ��ָ��
 *  @retval
 *  @pre
 *  @post
 */
#ifdef SPS_PPS_DEBUG
/*SPS ��Ϣ��ӡ������ʹ��*/
static void sps_info_print(SPS* sps_ptr)
{
    if(NULL != sps_ptr)
    {
        RPT(RPT_DBG, "profile_idc: %d", sps_ptr->profile_idc);
        RPT(RPT_DBG, "constraint_set0_flag: %d", sps_ptr->constraint_set0_flag);
        RPT(RPT_DBG, "constraint_set1_flag: %d", sps_ptr->constraint_set1_flag);
        RPT(RPT_DBG, "constraint_set2_flag: %d", sps_ptr->constraint_set2_flag);
        RPT(RPT_DBG, "constraint_set3_flag: %d", sps_ptr->constraint_set3_flag);
        RPT(RPT_DBG, "reserved_zero_4bits: %d", sps_ptr->reserved_zero_4bits);
        RPT(RPT_DBG, "level_idc: %d", sps_ptr->level_idc);
        RPT(RPT_DBG, "seq_parameter_set_id: %d", sps_ptr->seq_parameter_set_id);
        RPT(RPT_DBG, "chroma_format_idc: %d", sps_ptr->chroma_format_idc);
        RPT(RPT_DBG, "separate_colour_plane_flag: %d", sps_ptr->separate_colour_plane_flag);
        RPT(RPT_DBG, "bit_depth_luma_minus8: %d", sps_ptr->bit_depth_luma_minus8);
        RPT(RPT_DBG, "bit_depth_chroma_minus8: %d", sps_ptr->bit_depth_chroma_minus8);
        RPT(RPT_DBG, "qpprime_y_zero_transform_bypass_flag: %d", sps_ptr->qpprime_y_zero_transform_bypass_flag);
        RPT(RPT_DBG, "seq_scaling_matrix_present_flag: %d", sps_ptr->seq_scaling_matrix_present_flag);
        //RPT(RPT_INF, "seq_scaling_list_present_flag:%d", sps_ptr->seq_scaling_list_present_flag);
        RPT(RPT_DBG, "log2_max_frame_num_minus4: %d", sps_ptr->log2_max_frame_num_minus4);
        RPT(RPT_DBG, "pic_order_cnt_type: %d", sps_ptr->pic_order_cnt_type);
        RPT(RPT_DBG, "num_ref_frames: %d", sps_ptr->num_ref_frames);
        RPT(RPT_DBG, "gaps_in_frame_num_value_allowed_flag: %d", sps_ptr->gaps_in_frame_num_value_allowed_flag);
        RPT(RPT_DBG, "pic_width_in_mbs_minus1: %d", sps_ptr->pic_width_in_mbs_minus1);
        RPT(RPT_DBG, "pic_height_in_map_units_minus1: %d", sps_ptr->pic_height_in_map_units_minus1);
        RPT(RPT_DBG, "frame_mbs_only_flag: %d", sps_ptr->frame_mbs_only_flag);
        RPT(RPT_DBG, "mb_adaptive_frame_field_flag: %d", sps_ptr->mb_adaptive_frame_field_flag);
        RPT(RPT_DBG, "direct_8x8_inference_flag: %d", sps_ptr->direct_8x8_inference_flag);
        RPT(RPT_DBG, "frame_cropping_flag: %d", sps_ptr->frame_cropping_flag);
        RPT(RPT_DBG, "frame_crop_left_offset: %d", sps_ptr->frame_crop_left_offset);
        RPT(RPT_DBG, "frame_crop_right_offset: %d", sps_ptr->frame_crop_right_offset);
        RPT(RPT_DBG, "frame_crop_top_offset: %d", sps_ptr->frame_crop_top_offset);
        RPT(RPT_DBG, "frame_crop_bottom_offset: %d", sps_ptr->frame_crop_bottom_offset);
        RPT(RPT_DBG, "vui_parameters_present_flag: %d", sps_ptr->vui_parameters_present_flag);

        if(sps_ptr->vui_parameters_present_flag)
        {
            RPT(RPT_DBG, "aspect_ratio_info_present_flag: %d", sps_ptr->vui_parameters.aspect_ratio_info_present_flag);
            RPT(RPT_DBG, "aspect_ratio_idc: %d", sps_ptr->vui_parameters.aspect_ratio_idc);
            RPT(RPT_DBG, "sar_width: %d", sps_ptr->vui_parameters.sar_width);
            RPT(RPT_DBG, "sar_height: %d", sps_ptr->vui_parameters.sar_height);
            RPT(RPT_DBG, "overscan_info_present_flag: %d", sps_ptr->vui_parameters.overscan_info_present_flag);
            RPT(RPT_DBG, "overscan_info_appropriate_flag: %d", sps_ptr->vui_parameters.overscan_appropriate_flag);
            RPT(RPT_DBG, "video_signal_type_present_flag: %d", sps_ptr->vui_parameters.video_signal_type_present_flag);
            RPT(RPT_DBG, "video_format: %d", sps_ptr->vui_parameters.video_format);
            RPT(RPT_DBG, "video_full_range_flag: %d", sps_ptr->vui_parameters.video_full_range_flag);
            RPT(RPT_DBG, "colour_description_present_flag: %d", sps_ptr->vui_parameters.colour_description_present_flag);
            RPT(RPT_DBG, "colour_primaries: %d", sps_ptr->vui_parameters.colour_primaries);
            RPT(RPT_DBG, "transfer_characteristics: %d", sps_ptr->vui_parameters.transfer_characteristics);
            RPT(RPT_DBG, "matrix_coefficients: %d", sps_ptr->vui_parameters.matrix_coefficients);
            RPT(RPT_DBG, "chroma_loc_info_present_flag: %d", sps_ptr->vui_parameters.chroma_loc_info_present_flag);
            RPT(RPT_DBG, "chroma_sample_loc_type_top_field: %d", sps_ptr->vui_parameters.chroma_sample_loc_type_top_field);
            RPT(RPT_DBG, "chroma_sample_loc_type_bottom_field: %d", sps_ptr->vui_parameters.chroma_sample_loc_type_bottom_field);
            RPT(RPT_DBG, "timing_info_present_flag: %d", sps_ptr->vui_parameters.timing_info_present_flag);
            RPT(RPT_DBG, "num_units_in_tick: %d", sps_ptr->vui_parameters.num_units_in_tick);
            RPT(RPT_DBG, "time_scale: %d", sps_ptr->vui_parameters.time_scale);
            RPT(RPT_DBG, "fixed_frame_rate_flag: %d", sps_ptr->vui_parameters.fixed_frame_rate_flag);
            RPT(RPT_DBG, "nal_hrd_parameters_present_flag: %d", sps_ptr->vui_parameters.nal_hrd_parameters_present_flag);
            RPT(RPT_DBG, "cpb_cnt_minus1: %d", sps_ptr->vui_parameters.cpb_cnt_minus1);
            RPT(RPT_DBG, "bit_rate_scale: %d", sps_ptr->vui_parameters.bit_rate_scale);
            RPT(RPT_DBG, "cpb_size_scale: %d", sps_ptr->vui_parameters.cpb_size_scale);
            RPT(RPT_DBG, "initial_cpb_removal_delay_length_minus1: %d", sps_ptr->vui_parameters.initial_cpb_removal_delay_length_minus1);
            RPT(RPT_DBG, "cpb_removal_delay_length_minus1: %d", sps_ptr->vui_parameters.cpb_removal_delay_length_minus1);
            RPT(RPT_DBG, "dpb_output_delay_length_minus1: %d", sps_ptr->vui_parameters.dpb_output_delay_length_minus1);
            RPT(RPT_DBG, "time_offset_length: %d", sps_ptr->vui_parameters.time_offset_length);
            RPT(RPT_DBG, "vcl_hrd_parameters_present_flag: %d", sps_ptr->vui_parameters.vcl_hrd_parameters_present_flag);
            RPT(RPT_DBG, "low_delay_hrd_flag: %d", sps_ptr->vui_parameters.low_delay_hrd_flag);
            RPT(RPT_DBG, "pic_struct_present_flag: %d", sps_ptr->vui_parameters.pic_struct_present_flag);
            RPT(RPT_DBG, "bitstream_restriction_flag: %d", sps_ptr->vui_parameters.bitstream_restriction_flag);
            RPT(RPT_DBG, "motion_vectors_over_pic_boundaries_flag: %d", sps_ptr->vui_parameters.motion_vectors_over_pic_boundaries_flag);
            RPT(RPT_DBG, "max_bytes_per_pic_denom: %d", sps_ptr->vui_parameters.max_bytes_per_pic_denom);
            RPT(RPT_DBG, "max_bits_per_mb_denom: %d", sps_ptr->vui_parameters.max_bits_per_mb_denom);
            RPT(RPT_DBG, "log2_max_mv_length_horizontal: %d", sps_ptr->vui_parameters.log2_max_mv_length_horizontal);
            RPT(RPT_DBG, "log2_max_mv_length_vertical: %d", sps_ptr->vui_parameters.log2_max_mv_length_vertical);
            RPT(RPT_DBG, "num_reorder_frames: %d", sps_ptr->vui_parameters.num_reorder_frames);
            RPT(RPT_DBG, "max_dec_frame_buffering: %d", sps_ptr->vui_parameters.max_dec_frame_buffering);
        }

    }
}
#endif

/**
 *  @brief Function h264dec_seq_parameter_set()  h264 SPS infomation ����
 *  @param[in]     buf       get_bit_context�ṹ��buf ptr, ��ͬ��00 00 00 01 X7����
 *  @param[in]     sps_ptr   spsָ�룬����SPS��Ϣ
 *  @retval        0: success, -1 : failure
 *  @pre
 *  @post
 *  @note:
 *      �÷�:(α����)
 *          get_bit_context buffer;    //����һ��get_bit_context�ṹbuffer
 *          SPS sps_buffer;            //����һ��SPS�ṹsps_buffer
 *          .......
 *          buffer.buf = SPS_start     //esͷ��ʶΪ00 00 00 01 x7�Ժ�����ݣ���Ҫ��ǰ5�ֽ�
 *          buffer.buf_size = SPS_len  //SPS���������ݳ���
 *          ......
 *          h264dec_seq_parameter_set(&buffer, &sps_buffer);
 *
 */

/**
 *  @brief Function h264dec_seq_parameter_set()  h264 SPS infomation ����
 *  @param[in]     buf       buf ptr, ��ͬ��00 00 00 01 X7����
 *  @param[in]     sps_ptr   spsָ�룬����SPS��Ϣ
 *  @retval        0: success, -1 : failure
 *  @pre
 *  @post
 */
int h264dec_seq_parameter_set(void *buf_ptr, SPS *sps_ptr)
{
    SPS *sps = sps_ptr;
    int ret = 0;
    int profile_idc = 0;
    int i,j,last_scale, next_scale, delta_scale;
    void *buf = NULL;

    if(NULL == buf_ptr || NULL == sps)
    {
        RPT(RPT_ERR,"ERR null pointer\n");
        ret = -1;
        goto exit;
    }

    memset((void *)sps, 0, sizeof(SPS));
    buf = de_emulation_prevention(buf_ptr);
    if(NULL == buf)
    {
        RPT(RPT_ERR,"ERR null pointer\n");
        ret = -1;
        goto exit;
    }

    sps->profile_idc          = get_bits(buf, 8);
    sps->constraint_set0_flag = get_1bit(buf);
    sps->constraint_set1_flag = get_1bit(buf);
    sps->constraint_set2_flag = get_1bit(buf);
    sps->constraint_set3_flag = get_1bit(buf);
    sps->reserved_zero_4bits  = get_bits(buf, 4);
    sps->level_idc            = get_bits(buf, 8);
    sps->seq_parameter_set_id = parse_ue(buf);
    profile_idc = sps->profile_idc;
    if( (profile_idc == 100) || (profile_idc == 110) || (profile_idc == 122) || (profile_idc == 244)
    || (profile_idc  == 44) || (profile_idc == 83) || (profile_idc  == 86) || (profile_idc == 118) ||\
    (profile_idc == 128))
    {
        sps->chroma_format_idc = parse_ue(buf);
        if(sps->chroma_format_idc == 3)
        {
            sps->separate_colour_plane_flag = get_1bit(buf);
        }

        sps->bit_depth_luma_minus8 = parse_ue(buf);
        sps->bit_depth_chroma_minus8 = parse_ue(buf);
        sps->qpprime_y_zero_transform_bypass_flag = get_1bit(buf);
        sps->seq_scaling_matrix_present_flag = get_1bit(buf);

        if(sps->seq_scaling_matrix_present_flag)
        {

            for(i=0; i<((sps->chroma_format_idc != 3)?8:12); i++)
            {
                sps->seq_scaling_list_present_flag[i] = get_1bit(buf);
                if(sps->seq_scaling_list_present_flag[i])
                {
                    if(i<6)
                    {
                        for(j=0; j<16; j++)
                        {
                            last_scale = 8;
                            next_scale = 8;
                            if(next_scale != 0)
                            {
                                delta_scale = parse_se(buf);
                                next_scale = (last_scale + delta_scale + 256)%256;
                                sps->UseDefaultScalingMatrix4x4Flag[i] = ((j == 0) && (next_scale == 0));
                            }
                            sps->ScalingList4x4[i][j] = (next_scale == 0)?last_scale:next_scale;
                            last_scale = sps->ScalingList4x4[i][j];
                        }
                    }
                    else
                    {
                        int ii = i-6;
                        next_scale = 8;
                        last_scale = 8;
                        for(j=0; j<64; j++)
                        {
                            if(next_scale != 0)
                            {
                                delta_scale = parse_se(buf);
                                next_scale = (last_scale + delta_scale + 256)%256;
                                sps->UseDefaultScalingMatrix8x8Flag[ii] = ((j == 0) && (next_scale == 0));
                            }
                            sps->ScalingList8x8[ii][j] = (next_scale == 0)?last_scale:next_scale;
                            last_scale = sps->ScalingList8x8[ii][j];
                        }
                    }
                }
            }
        }
    }
    sps->log2_max_frame_num_minus4 = parse_ue(buf);
    sps->pic_order_cnt_type = parse_ue(buf);

    if(sps->pic_order_cnt_type == 0)
    {
        sps->log2_max_pic_order_cnt_lsb_minus4 = parse_ue(buf);
    }
    else if(sps->pic_order_cnt_type == 1)
    {
        sps->delta_pic_order_always_zero_flag = get_1bit(buf);
        sps->offset_for_non_ref_pic = parse_se(buf);
        sps->offset_for_top_to_bottom_field = parse_se(buf);

        sps->num_ref_frames_in_pic_order_cnt_cycle = parse_ue(buf);
        for(i=0; i<sps->num_ref_frames_in_pic_order_cnt_cycle; i++)
        {
            sps->offset_for_ref_frame_array[i] = parse_se(buf);
        }
    }
    sps->num_ref_frames = parse_ue(buf);
    sps->gaps_in_frame_num_value_allowed_flag = get_1bit(buf);
    sps->pic_width_in_mbs_minus1 = parse_ue(buf);
    sps->pic_height_in_map_units_minus1 = parse_ue(buf);
    sps->frame_mbs_only_flag = get_1bit(buf);

    if(!sps->frame_mbs_only_flag)
    {
        sps->mb_adaptive_frame_field_flag = get_1bit(buf);
    }
    sps->direct_8x8_inference_flag = get_1bit(buf);

    sps->frame_cropping_flag = get_1bit(buf);
    if(sps->frame_cropping_flag)
    {
        sps->frame_crop_left_offset = parse_ue(buf);
        sps->frame_crop_right_offset = parse_ue(buf);
        sps->frame_crop_top_offset = parse_ue(buf);
        sps->frame_crop_bottom_offset = parse_ue(buf);
    }
    sps->vui_parameters_present_flag = get_1bit(buf);
    if(sps->vui_parameters_present_flag)
    {
        vui_parameters_set(buf, &sps->vui_parameters);
    }

#ifdef SPS_PPS_DEBUG
    sps_info_print(sps);
#endif

exit:
    get_bit_context_free(buf);
    return ret;
}


/**
 *  @brief Function more_rbsp_data()  ����pps�����һ��Ϊ1�ı���λ������Ǳ���0�ĸ���
 *  @param[in]     buf       get_bit_context structure
 *  @retval
 *  @pre
 *  @post
 *  @note  ��δ����������ѵİ�������û����֤��ʹ��ʱ��ע��
 */
static int more_rbsp_data(void *buf)
{
    get_bit_context *ptr = (get_bit_context *)buf;
    get_bit_context tmp;

    if(NULL == buf)
    {
        RPT(RPT_ERR, "NULL pointer, err");
        return -1;
    }

    memset(&tmp, 0, sizeof(get_bit_context));
    memcpy(&tmp, ptr, sizeof(get_bit_context));

    for(tmp.bit_pos = ptr->total_bit - 1;tmp.bit_pos > ptr->bit_pos;tmp.bit_pos -= 2)
    {
        if(get_1bit(&tmp))
        {
            break;
        }
    }

    return tmp.bit_pos == ptr->bit_pos? 0:1;
}

/**
 *  @brief Function h264dec_picture_parameter_set()  h264 PPS infomation ����
 *  @param[in]     buf       buf ptr, ��ͬ��00 00 00 01 X8����
 *  @param[in]     pps_ptr   ppsָ�룬����pps��Ϣ
 *  @retval        0: success, -1 : failure
 *  @pre
 *  @post
 *  @note: �÷��ο�sps����
 */
int h264dec_picture_parameter_set(void *buf_ptr, PPS *pps_ptr)
{
    PPS *pps = pps_ptr;
    int ret = 0;
    void *buf = NULL;
    int iGroup = 0;
    int i,j,last_scale, next_scale, delta_scale;

    if(NULL == buf_ptr || NULL == pps_ptr)
    {
        RPT(RPT_ERR, "NULL pointer\n");
        ret = -1;
        goto exit;
    }

    memset((void *)pps, 0, sizeof(PPS));

    buf = de_emulation_prevention(buf_ptr);
    if(NULL == buf)
    {
        RPT(RPT_ERR,"ERR null pointer\n");
        ret = -1;
        goto exit;
    }

    pps->pic_parameter_set_id = parse_ue(buf);
    pps->seq_parameter_set_id = parse_ue(buf);
    pps->entropy_coding_mode_flag = get_1bit(buf);
    pps->pic_order_present_flag = get_1bit(buf);

    pps->num_slice_groups_minus1 = parse_ue(buf);
    if(pps->num_slice_groups_minus1 > 0)
    {
        pps->slice_group_map_type = parse_ue(buf);
        if(pps->slice_group_map_type == 0)
        {
            for(iGroup=0; iGroup<=pps->num_slice_groups_minus1; iGroup++)
            {
                pps->run_length_minus1[iGroup] = parse_ue(buf);
            }
        }
        else if(pps->slice_group_map_type == 2)
        {
            for(iGroup=0; iGroup<=pps->num_slice_groups_minus1; iGroup++)
            {
                pps->top_left[iGroup] = parse_ue(buf);
                pps->bottom_right[iGroup] = parse_ue(buf);
            }
        }
        else if(pps->slice_group_map_type == 3 \
                ||pps->slice_group_map_type == 4\
                ||pps->slice_group_map_type == 5)
        {
            pps->slice_group_change_direction_flag = get_1bit(buf);
            pps->slice_group_change_rate_minus1 = parse_ue(buf);
        }
        else if(pps->slice_group_map_type == 6)
        {
            pps->pic_size_in_map_units_minus1 = parse_ue(buf);
            for(i=0; i<pps->pic_size_in_map_units_minus1; i++)
            {
                /*��ط����������⣬��u(v)���ƫ��*/
                pps->slice_group_id[i] = get_bits(buf, pps->pic_size_in_map_units_minus1);
            }
        }
    }

    pps->num_ref_idx_10_active_minus1 = parse_ue(buf);
    pps->num_ref_idx_11_active_minus1 = parse_ue(buf);
    pps->weighted_pred_flag = get_1bit(buf);
    pps->weighted_bipred_idc = get_bits(buf, 2);
    pps->pic_init_qp_minus26 = parse_se(buf); /*relative26*/
    pps->pic_init_qs_minus26 = parse_se(buf); /*relative26*/
    pps->chroma_qp_index_offset = parse_se(buf);
    pps->deblocking_filter_control_present_flag = get_1bit(buf);
    pps->constrained_intra_pred_flag = get_1bit(buf);
    pps->redundant_pic_cnt_present_flag = get_1bit(buf);

    if(more_rbsp_data(buf))
    {
        pps->transform_8x8_mode_flag = get_1bit(buf);
        pps->pic_scaling_matrix_present_flag = get_1bit(buf);
        if(pps->pic_scaling_matrix_present_flag)
        {
            for(i=0; i<6+2*pps->transform_8x8_mode_flag; i++)
            {
                pps->pic_scaling_list_present_flag[i] = get_1bit(buf);
                if(pps->pic_scaling_list_present_flag[i])
                {
                    if(i<6)
                    {
                        for(j=0; j<16; j++)
                        {
                            next_scale = 8;
                            last_scale = 8;
                            if(next_scale != 0)
                            {
                                delta_scale = parse_se(buf);
                                next_scale = (last_scale + delta_scale + 256)%256;
                                pps->UseDefaultScalingMatrix4x4Flag[i] = ((j == 0) && (next_scale == 0));
                            }
                            pps->ScalingList4x4[i][j] = (next_scale == 0)?last_scale:next_scale;
                            last_scale = pps->ScalingList4x4[i][j];
                        }
                    }
                    else
                    {
                        int ii = i-6;
                        next_scale = 8;
                        last_scale = 8;
                        for(j=0; j<64; j++)
                        {
                            if(next_scale != 0)
                            {
                                delta_scale = parse_se(buf);
                                next_scale = (last_scale + delta_scale + 256)%256;
                                pps->UseDefaultScalingMatrix8x8Flag[ii] = ((j == 0) && (next_scale == 0));
                            }
                            pps->ScalingList8x8[ii][j] = (next_scale == 0)?last_scale:next_scale;
                            last_scale = pps->ScalingList8x8[ii][j];
                        }
                    }
                }
            }

            pps->second_chroma_qp_index_offset = parse_se(buf);
        }
    }

exit:
    get_bit_context_free(buf);
    return ret;
}


// calculation width height and framerate
int h264_get_width(SPS *sps_ptr)
{
    return (sps_ptr->pic_width_in_mbs_minus1 + 1) * 16;
}

int h264_get_height(SPS *sps_ptr)
{
	//printf("fun = %s line = %d sps_ptr->frame_mbs_only_flag=%d \n", __FUNCTION__, __LINE__, sps_ptr->frame_mbs_only_flag);
    return (sps_ptr->pic_height_in_map_units_minus1 + 1) * 16 * (2 - sps_ptr->frame_mbs_only_flag);
}

int h264_get_format(SPS *sps_ptr)
{
    return sps_ptr->frame_mbs_only_flag;
}


/*
1 - 23.98
2 - 24
3 - 25
4 - 29.97
5 - 30
6 - 50
7 - 59.94
8 - 60
9 - 6
10 - 8
11 - 12
12 - 15
13 - 10.
*/
int h264_get_framerate(float *framerate,SPS *sps_ptr)
{
    int fr;
    int fr_int = 0;
    if(sps_ptr->vui_parameters.timing_info_present_flag)
    {
        if(sps_ptr->frame_mbs_only_flag)
        {
            //*framerate = (float)sps_ptr->vui_parameters.time_scale / (float)sps_ptr->vui_parameters.num_units_in_tick;
		*framerate = (float)sps_ptr->vui_parameters.time_scale / (float)sps_ptr->vui_parameters.num_units_in_tick/2.0;
            //fr_int = sps_ptr->vui_parameters.time_scale / sps_ptr->vui_parameters.num_units_in_tick;
        }else
        {
            *framerate = (float)sps_ptr->vui_parameters.time_scale / (float)sps_ptr->vui_parameters.num_units_in_tick / 2.0;
            //fr_int = sps_ptr->vui_parameters.time_scale / sps_ptr->vui_parameters.num_units_in_tick / 2;
        }
        fr_int = sps_ptr->vui_parameters.time_scale / sps_ptr->vui_parameters.num_units_in_tick / 2;
    }
    switch(fr_int)
    {
        case 23:// 23.98
            fr = 1;
            break;
        case 24:
            fr = 2;
            break;
        case 25:
            fr = 3;
            break;
        case 29://29.97
            fr = 4;
            break;
        case 30:
            fr = 5;
            break;
        case 50:
            fr = 6;
            break;
        case 59://59.94
            fr = 7;
            break;
        case 60:
            fr = 8;
            break;
        case 6:
            fr = 10;
            break;
        case 8:
            fr = 11;
            break;
        case 12:
            fr = 12;
            break;
        case 15:
            fr = 13;
            break;
        case 10:
            fr = 14;
            break;

        default:
            fr = 0;
            break;
    }

    return fr;
}


// for mpeg-2
void memcpy_sps_data(uint8_t *dst,uint8_t *src,int len)
{
    int tmp;
    for(tmp = 0; tmp < len; tmp++)
    {
        //printf("0x%02x ", src[tmp]);
        dst[(tmp/4)*4+(3 - (tmp % 4))] = src[tmp];
    }
}
/*_*/
#ifdef __cplusplus
}
#endif

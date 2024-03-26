// SPDX-License-Identifier: LGPL-3.0-or-later
/*
 * vim:noexpandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright 2018 Jeff Layton <jlayton@redhat.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * ---------------------------------------
 */

#include "config.h"
#include "log.h"
#include "fsal.h"
#include "nfs_core.h"
#include "sal_functions.h"
#include "nfs_proto_functions.h"
#include "nfs_proto_tools.h"
#include "nfs_convert.h"
#include "fsal_pnfs.h"
#include "server_stats.h"
#include "export_mgr.h"

/**
 * @file nfs4_op_copy.c
 * @brief Routines used for managing the NFS4 COMPOUND functions.
 *
 * Routines used for managing the NFS4 COMPOUND functions COPY.
 */

/**
 * @brief The NFS4_OP_ALLOCATE operation
 * This functions handles the NFS4_OP_ALLOCATE operation in NFSv4.2. This
 * function can be called only from nfs4_Compound.
 *
 * @param[in]     op    Arguments for nfs4_op
 * @param[in,out] data  Compound request's data
 * @param[out]    resp  Results for nfs4_op
 *
 */
enum nfs_req_result nfs4_op_copy(struct nfs_argop4 *op,
				     compound_data_t *data,
				     struct nfs_resop4 *resp)
{
    COPY4args * const arg_COPY4 = &op->nfs_argop4_u.opcopy;
    COPY4res * const res_COPY4 = &resp->nfs_resop4_u.opcopy;

    state_t *state = NULL;
    struct fsal_obj_handle *src_obj = NULL;
    struct fsal_obj_handle *dst_obj = NULL;
    fsal_status_t fsal_status = {0, 0};
    bool is_intra = true;

    LogInfo(COMPONENT_NFS_V4, "[VICENTE DBG]: calling nfs4_op_copy with src offset: %lu, dst offset: %lu", arg_COPY4->ca_src_offset, arg_COPY4->ca_dst_offset);
    LogInfo(COMPONENT_NFS_V4, "[VICENTE DBG]: calling nfs4_op_copy with count: %lu, sync: %u", arg_COPY4->ca_count, arg_COPY4->ca_sync);
	resp->resop = NFS4_OP_COPY;
	res_COPY4->cr_status = NFS4ERR_NOTSUPP;

    // only 4.2 support ssc
    if (data->minorversion < 2) {
	    res_COPY4->cr_status = NFS4ERR_NOTSUPP;
        return NFS_REQ_ERROR;
	}

    LogInfo(COMPONENT_NFS_V4, "[VICENTE DBG]: check copy file is regular file");
    res_COPY4->cr_status = nfs4_sanity_check_FH(data, REGULAR_FILE, false);
    if (res_COPY4->cr_status != NFS4_OK) {
        res_COPY4->cr_status = NFS4ERR_NOTSUPP;
		return NFS_REQ_ERROR; 
    }

    /* 
     * SAVED_FH: source file
     * CURRENT_FH: destination file or directory
     */
    src_obj = data->saved_obj;
    dst_obj = data->current_obj;

    if (op_ctx->ctx_export != NULL && data->saved_export != NULL &&
        op_ctx->ctx_export->export_id != data->saved_export->export_id) {
            is_intra = false; 
    }
    LogInfo(COMPONENT_NFS_V4, "[VICENTE DBG]: %s", is_intra ? "SSC intra" : "SSC inter");

    if (is_intra) {
        // check src stateid for read
        LogInfo(COMPONENT_NFS_V4, "[VICENTE DBG]: check src stateid for write");
        res_COPY4->cr_status = nfs4_Check_Stateid(&arg_COPY4->ca_src_stateid, src_obj, &state, data,
			       STATEID_SPECIAL_ANY, 0, false, "COPY");
        if (res_COPY4->cr_status != NFS4_OK) {
            res_COPY4->cr_status = NFS4ERR_NOTSUPP;
            return NFS_REQ_ERROR;
        }
        LogInfo(COMPONENT_NFS_V4, "[VICENTE DBG]: check dst stateid for write");
        // check dst stateid for write
        res_COPY4->cr_status = nfs4_Check_Stateid(&arg_COPY4->ca_dst_stateid, dst_obj, &state, data,
                   STATEID_SPECIAL_ANY, 0, false, "COPY");
        if (res_COPY4->cr_status != NFS4_OK) {
            res_COPY4->cr_status = NFS4ERR_NOTSUPP;
            return NFS_REQ_ERROR;
        }
    } else {
        LogInfo(COMPONENT_NFS_V4, "[VICENTE DBG]: NOT SUPPORT INTER SSC.");
        res_COPY4->cr_status = NFS4ERR_NOTSUPP;
        return NFS_REQ_ERROR;
    }
    fsal_status = src_obj->obj_ops->copy_file_range(src_obj, arg_COPY4->ca_src_offset, dst_obj, arg_COPY4->ca_dst_offset, arg_COPY4->ca_count); 
    LogInfo(COMPONENT_NFS_V4, "[VICENTE DBG]: copy_file_range return %s", fsal_err_txt(fsal_status));
    res_COPY4->cr_status = NFS4ERR_NOTSUPP;

	return nfsstat4_to_nfs_req_result(res_COPY4->cr_status);
}

void nfs4_op_copy_Free(nfs_resop4 *resp)
{
	/* Nothing to be done */
}
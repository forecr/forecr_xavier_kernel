/*
 * Copyright (c) 2016-2022, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __NVIDIA_DRM_CRTC_H__
#define __NVIDIA_DRM_CRTC_H__

#include "nvidia-drm-conftest.h"

#if defined(NV_DRM_ATOMIC_MODESET_AVAILABLE)

#include "nvidia-drm-helper.h"

#if defined(NV_DRM_DRMP_H_PRESENT)
#include <drm/drmP.h>
#endif

#include <drm/drm_crtc.h>

#include "nvtypes.h"
#include "nvkms-kapi.h"

struct nv_drm_crtc {
    NvU32 head;

    bool vblank_enabled;
    /**
     * @flip_list:
     *
     * List of flips pending to get processed by __nv_drm_handle_flip_event().
     * Protected by @vblank_q_lock.
     */
    struct list_head flip_list;

    struct mutex vblank_q_lock;

    /**
     * @modeset_permission_filep:
     *
     * The filep using this crtc with DRM_IOCTL_NVIDIA_GRANT_PERMISSIONS.
     */
    struct drm_file *modeset_permission_filep;

    struct NvKmsKapiVblankIntrCallback *vblankIntrCallback;

    nv_kthread_q_item_t disable_vblank_q_item;
    nv_kthread_q_item_t enable_vblank_q_item;

    struct drm_crtc base;
};

/**
 * struct nv_drm_flip - flip state
 *
 * This state is getting used to consume DRM completion event associated
 * with each crtc state from atomic commit.
 *
 * Function nv_drm_atomic_apply_modeset_config() consumes DRM completion
 * event, save it into flip state associated with crtc and queue flip state into
 * crtc's flip list and commits atomic update to hardware.
 */
struct nv_drm_flip {
    /**
     * @event:
     *
     * Optional pointer to a DRM event to signal upon completion of
     * the state update.
     */
    struct drm_pending_vblank_event *event;

/**
 * @plane_mask
 *
 * Bitmask of drm_plane_mask(plane) of planes attached to the completion of
 * the state update.
 */
uint32_t plane_mask;

/**
 * @pending_events_plane_mask
 *
 * Bitmask of drm_plane_mask(plane) of planes for which HW events are
 * pending before signaling the completion of the state update.
 */
uint32_t pending_events_plane_mask;

/**
 * @list_entry:
 *
 * Entry on the per-CRTC &nv_drm_crtc.flip_list. Protected by
 * &nv_drm_crtc.vblank_q_lock.
 */
struct list_head list_entry;
};

struct nv_drm_crtc_state {
    /**
     * @base:
     *
     * Base DRM crtc state object for this.
     */
    struct drm_crtc_state base;

    /**
     * @head_req_config:
     *
     * Requested head's modeset configuration corresponding to this crtc state.
     */
    struct NvKmsKapiHeadRequestedConfig req_config;

    /**
     * @nv_flip:
     *
     * Flip state associated with this crtc state, gets allocated
     * by nv_drm_atomic_crtc_duplicate_state(), on successful commit it gets
     * consumed and queued into flip list by
     * nv_drm_atomic_apply_modeset_config() and finally gets destroyed
     * by __nv_drm_handle_flip_event() after getting processed.
     *
     * In case of failure of atomic commit, this flip state getting destroyed by
     * nv_drm_atomic_crtc_destroy_state().
     */
    struct nv_drm_flip *nv_flip;
};

static inline struct nv_drm_crtc_state *to_nv_crtc_state(struct drm_crtc_state *state)
{
    return container_of(state, struct nv_drm_crtc_state, base);
}

struct nv_drm_plane {
    /**
     * @base:
     *
     * Base DRM plane object for this plane.
     */
    struct drm_plane base;

    /**
     * @defaultCompositionMode:
     *
     * Default composition blending mode of this plane.
     */
    enum NvKmsCompositionBlendingMode defaultCompositionMode;

    NvU32 head;

    /**
     * @layer_idx
     *
     * Index of this plane in the per head array of layers.
     */
    uint32_t layer_idx;
};

static inline struct nv_drm_plane *to_nv_plane(struct drm_plane *plane)
{
    if (plane == NULL) {
        return NULL;
    }
    return container_of(plane, struct nv_drm_plane, base);
}

struct nv_drm_plane_state {
    struct drm_plane_state base;
    s32 __user *fd_user_ptr;
    enum NvKmsInputColorSpace input_colorspace;
    enum NvKmsInputColorRange input_colorrange;
#if defined(NV_DRM_HAS_HDR_OUTPUT_METADATA)
    struct drm_property_blob *hdr_output_metadata;
#endif
};

static inline struct nv_drm_plane_state *to_nv_drm_plane_state(struct drm_plane_state *state)
{
    return container_of(state, struct nv_drm_plane_state, base);
}

static inline const struct nv_drm_plane_state *to_nv_drm_plane_state_const(const struct drm_plane_state *state)
{
    return container_of(state, const struct nv_drm_plane_state, base);
}

static inline struct nv_drm_crtc *to_nv_crtc(struct drm_crtc *crtc)
{
    if (crtc == NULL) {
        return NULL;
    }
    return container_of(crtc, struct nv_drm_crtc, base);
}

/*
 * CRTCs are static objects, list does not change once after initialization and
 * before teardown of device. Initialization/teardown paths are single
 * threaded, so no locking required.
 */
static inline
struct nv_drm_crtc *nv_drm_crtc_lookup(struct nv_drm_device *nv_dev, NvU32 head)
{
    struct drm_crtc *crtc;
    nv_drm_for_each_crtc(crtc, nv_dev->dev) {
        struct nv_drm_crtc *nv_crtc = to_nv_crtc(crtc);

        if (nv_crtc->head == head)  {
            return nv_crtc;
        }
    }
    return NULL;
}

static inline
struct nv_drm_plane *nv_drm_plane_lookup(struct nv_drm_device *nv_dev, NvU32 head, NvU32 layer)
{
    struct drm_plane *plane;
    nv_drm_for_each_plane(plane, nv_dev->dev) {
        struct nv_drm_plane *nv_plane = to_nv_plane(plane);

        if ((nv_plane->head == head) && (nv_plane->layer_idx == layer))  {
            return nv_plane;
        }
    }
    return NULL;
}

/**
 * nv_drm_crtc_enqueue_flip - Enqueue nv_drm_flip object to flip_list of crtc.
 */
static inline void nv_drm_crtc_enqueue_flip(struct nv_drm_crtc *nv_crtc,
                                            struct nv_drm_flip *nv_flip)
{
    spin_lock(&nv_crtc->base.dev->event_lock);
    list_add(&nv_flip->list_entry, &nv_crtc->flip_list);
    spin_unlock(&nv_crtc->base.dev->event_lock);
}

void nv_drm_enumerate_crtcs_and_planes(
    struct nv_drm_device *nv_dev,
    const struct NvKmsKapiDeviceResourcesInfo *pResInfo);

int nv_drm_get_crtc_crc32_ioctl(struct drm_device *dev,
                                void *data, struct drm_file *filep);

int nv_drm_get_crtc_crc32_v2_ioctl(struct drm_device *dev,
                                   void *data, struct drm_file *filep);

#endif /* NV_DRM_ATOMIC_MODESET_AVAILABLE */

#endif /* __NVIDIA_DRM_CRTC_H__ */

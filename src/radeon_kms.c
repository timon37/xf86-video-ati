/*
 * Copyright © 2009 Red Hat, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Dave Airlie <airlied@redhat.com>
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <sys/ioctl.h>
/* Driver data structures */
#include "radeon.h"
#include "radeon_reg.h"
#include "radeon_probe.h"
#include "micmap.h"

#include "shadow.h"

#include "atipciids.h"


#ifdef XF86DRM_MODE

#if 0
}
#endif

#include "radeon_chipset_gen.h"
#include "radeon_chipinfo_gen.h"

#define CURSOR_WIDTH	64
#define CURSOR_HEIGHT	64

#include "radeon_bo_gem.h"
#include "radeon_cs_gem.h"
#include "radeon_vbo.h"


#define dHack(fmt)	\
	do {	\
		xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, fmt);	\
	}while(0)
/*
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, fmt);	\
*/

#define dHackP(fmt,...)	\
	do {	\
		char buf[256];	\
		sprintf (buf, fmt,##__VA_ARGS__);	\
		xf86DrvMsgVerb(0, X_INFO, RADEON_LOGLEVEL_DEBUG, buf);	\
	}while(0)
/*
static int
drm_intel_gem_bo_flink(drm_intel_bo *bo, uint32_t * name)
{
	drm_intel_bufmgr_gem *bufmgr_gem = (drm_intel_bufmgr_gem *) bo->bufmgr;
	drm_intel_bo_gem *bo_gem = (drm_intel_bo_gem *) bo;
	int ret;

	if (!bo_gem->global_name) {
		struct drm_gem_flink flink;

		VG_CLEAR(flink);
		flink.handle = bo_gem->gem_handle;

		ret = drmIoctl(bufmgr_gem->fd, DRM_IOCTL_GEM_FLINK, &flink);
		if (ret != 0)
			return -errno;

		bo_gem->global_name = flink.name;
		bo_gem->reusable = false;

		DRMLISTADDTAIL(&bo_gem->name_list, &bufmgr_gem->named);
	}

	*name = bo_gem->global_name;
	return 0;
}
/*
int drm_intel_bo_flink(drm_intel_bo *bo, uint32_t * name)
{
	if (bo->bufmgr->bo_flink)
		return bo->bufmgr->bo_flink(bo, name);

	return -ENODEV;
}/**/

#define xf86DrvMsgVerb(...)	do{}while(0);


#ifdef XORG_WAYLAND
int radeon_create_window_buffer(struct xwl_window *xwl_window, PixmapPtr pixmap)
{
	xf86DrvMsgVerb(0, X_INFO, RADEON_LOGLEVEL_DEBUG, "radeon_create_window_buffer\n");
//	dHack ("radeon_create_window_buffer\n");
	uint32_t name;
	struct radeon_bo *bo;

	xf86DrvMsgVerb(0, X_INFO, RADEON_LOGLEVEL_DEBUG, "radeon_create_window_buffer radeon_get_pixmap_bo(0x%lx)\n", pixmap);
	
//	struct radeon_bo *radeon_get_pixmap_bo(PixmapPtr pPix)
	{
		struct radeon_exa_pixmap_priv *driver_priv;
		driver_priv = exaGetPixmapDriverPrivate(pixmap);
		xf86DrvMsgVerb(0, X_INFO, RADEON_LOGLEVEL_DEBUG, "radeon_create_window_buffer driver_priv 0x%lx\n", driver_priv);
	//	if (!driver_priv)
	//		return BadDrawable;
		bo = driver_priv->bo;
	}

	bo = radeon_get_pixmap_bo(pixmap);
	xf86DrvMsgVerb(0, X_INFO, RADEON_LOGLEVEL_DEBUG, "radeon_create_window_buffer radeon_get_pixmap_bo %lx\n", bo);
	if (bo == NULL || radeon_gem_get_kernel_name(bo, &name) != 0) {
		xf86DrvMsgVerb(0, X_INFO, RADEON_LOGLEVEL_DEBUG, "radeon_create_window_buffer BadDrawable\n");
	//	dHack ("radeon_create_window_buffer return BadDrawable\n");
		return BadDrawable;
	}
//	xf86DrvMsgVerb(0, X_INFO, RADEON_LOGLEVEL_DEBUG, "radeon_create_window_buffer good\n");
	
//	dHackP ("radeon_create_window_buffer good name %d\n", radeon_gem_name_bo(bo));
//	dHackP ("radeon_create_window_buffer good kernel name %d\n", name);
	
	int ret = xwl_create_window_buffer_drm(xwl_window, pixmap, name);
	
//	dHackP ("radeon_create_window_buffer good ret %d\n", ret);
	return ret;
}

static struct xwl_driver xwl_driver = {
	.version = 1,
	.use_drm = 1,
	.create_window_buffer = radeon_create_window_buffer
};
#endif



static Bool radeon_setup_kernel_mem(ScreenPtr pScreen);

const OptionInfoRec RADEONOptions_KMS[] = {
    { OPTION_NOACCEL,        "NoAccel",          OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SW_CURSOR,      "SWcursor",         OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_PAGE_FLIP,      "EnablePageFlip",   OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_ACCEL_DFS,      "AccelDFS",         OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_IGNORE_EDID,    "IgnoreEDID",       OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_COLOR_TILING,   "ColorTiling",      OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_COLOR_TILING_2D,"ColorTiling2D",    OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_RENDER_ACCEL,   "RenderAccel",      OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SUBPIXEL_ORDER, "SubPixelOrder",    OPTV_ANYSTR,  {0}, FALSE },
    { OPTION_ACCELMETHOD,    "AccelMethod",      OPTV_STRING,  {0}, FALSE },
    { OPTION_DRI,            "DRI",       	 OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_TVSTD,          "TVStandard",       OPTV_STRING,  {0}, FALSE },
    { OPTION_EXA_VSYNC,      "EXAVSync",         OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_EXA_PIXMAPS,    "EXAPixmaps",	 OPTV_BOOLEAN,   {0}, FALSE },
    { OPTION_ZAPHOD_HEADS,   "ZaphodHeads",      OPTV_STRING,  {0}, FALSE },
    { OPTION_PAGE_FLIP,      "EnablePageFlip",   OPTV_BOOLEAN, {0}, FALSE },
    { OPTION_SWAPBUFFERS_WAIT,"SwapbuffersWait", OPTV_BOOLEAN, {0}, FALSE },
    { -1,                    NULL,               OPTV_NONE,    {0}, FALSE }
};

void radeon_cs_flush_indirect(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr  info = RADEONPTR(pScrn);
    struct radeon_accel_state *accel_state = info->accel_state;
    int ret;

    if (!info->cs->cdw)
	return;

    /* release the current VBO so we don't block on mapping it later */
    if (info->accel_state->vbo.vb_offset && info->accel_state->vbo.vb_bo) {
        radeon_vbo_put(pScrn, &info->accel_state->vbo);
        info->accel_state->vbo.vb_start_op = -1;
    }

    /* release the current VBO so we don't block on mapping it later */
    if (info->accel_state->cbuf.vb_bo) {
        radeon_vbo_put(pScrn, &info->accel_state->cbuf);
        info->accel_state->cbuf.vb_start_op = -1;
    }

    radeon_cs_emit(info->cs);
    radeon_cs_erase(info->cs);

    if (accel_state->use_vbos)
        radeon_vbo_flush_bos(pScrn);

    ret = radeon_cs_space_check_with_bo(info->cs,
					accel_state->vbo.vb_bo,
					RADEON_GEM_DOMAIN_GTT, 0);
    if (ret)
      ErrorF("space check failed in flush\n");

    if (info->reemit_current2d && info->state_2d.op)
        info->reemit_current2d(pScrn, info->state_2d.op);

    if (info->dri2.enabled) {
        info->accel_state->XInited3D = FALSE;
        info->accel_state->engineMode = EXA_ENGINEMODE_UNKNOWN;
    }

}

void radeon_ddx_cs_start(ScrnInfoPtr pScrn,
			 int n, const char *file,
			 const char *func, int line)
{
    RADEONInfoPtr  info = RADEONPTR(pScrn);

    if (info->cs->cdw + n > info->cs->ndw) {
	radeon_cs_flush_indirect(pScrn);

    }
    radeon_cs_begin(info->cs, n, file, func, line);
}


extern _X_EXPORT int gRADEONEntityIndex;

static int getRADEONEntityIndex(void)
{
    return gRADEONEntityIndex;
}

static void *
radeonShadowWindow(ScreenPtr screen, CARD32 row, CARD32 offset, int mode,
		   CARD32 *size, void *closure)
{
    ScrnInfoPtr pScrn = xf86Screens[screen->myNum];
	dHack ("RADEONCreateScreenResources_KMS\n");
    RADEONInfoPtr  info   = RADEONPTR(pScrn);
    int stride;

    stride = (pScrn->displayWidth * pScrn->bitsPerPixel) / 8;
    *size = stride;

    return ((uint8_t *)info->front_bo->ptr + row * stride + offset);
}

static Bool RADEONCreateScreenResources_KMS(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr  info   = RADEONPTR(pScrn);
    PixmapPtr pixmap;
    struct radeon_surface *surface;

    pScreen->CreateScreenResources = info->CreateScreenResources;
    if (!(*pScreen->CreateScreenResources)(pScreen))
	return FALSE;
	
	
    pScreen->CreateScreenResources = RADEONCreateScreenResources_KMS;

#ifdef XORG_WAYLAND
	if (info->xwl_screen)
		xwl_screen_init(info->xwl_screen, pScreen);
#endif
    if (!drmmode_set_desired_modes(pScrn, &info->drmmode))
	return FALSE;

    drmmode_uevent_init(pScrn, &info->drmmode);

    if (info->r600_shadow_fb) {
	pixmap = pScreen->GetScreenPixmap(pScreen);

	if (!shadowAdd(pScreen, pixmap, shadowUpdatePackedWeak(),
		       radeonShadowWindow, 0, NULL))
	    return FALSE;
    }

    if (info->dri2.enabled) {
	if (info->front_bo) {
	    PixmapPtr pPix = pScreen->GetScreenPixmap(pScreen);
	    radeon_set_pixmap_bo(pPix, info->front_bo);
	    surface = radeon_get_pixmap_surface(pPix);
	    if (surface) {
		*surface = info->front_surface;
	    }
	}
    }
    return TRUE;
}

static void RADEONBlockHandler_KMS(int i, pointer blockData,
				   pointer pTimeout, pointer pReadmask)
{
    ScreenPtr      pScreen = screenInfo.screens[i];
    ScrnInfoPtr    pScrn   = xf86Screens[i];
    RADEONInfoPtr  info    = RADEONPTR(pScrn);

    pScreen->BlockHandler = info->BlockHandler;
    (*pScreen->BlockHandler) (i, blockData, pTimeout, pReadmask);
    pScreen->BlockHandler = RADEONBlockHandler_KMS;

    if (info->VideoTimerCallback)
	(*info->VideoTimerCallback)(pScrn, currentTime.milliseconds);
    radeon_cs_flush_indirect(pScrn);
}

static void
radeon_flush_callback(CallbackListPtr *list,
		      pointer user_data, pointer call_data)
{
    ScrnInfoPtr pScrn = user_data;
//	dHack ("radeon_flush_callback\n");
    RADEONInfoPtr  info    = RADEONPTR(pScrn);

    if (pScrn->vtSema) {
        radeon_cs_flush_indirect(pScrn);
        
//	radeon_vbo_flush_bos(pScrn);
#ifdef XORG_WAYLAND
		if (info->xwl_screen) {
		//	dHack ("xwl_screen_post_damage\n");
			xwl_screen_post_damage(info->xwl_screen);
		}
#endif
    }
}

static Bool RADEONIsFusionGARTWorking(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    struct drm_radeon_info ginfo;
    int r;
    uint32_t tmp;

#ifndef RADEON_INFO_FUSION_GART_WORKING
#define RADEON_INFO_FUSION_GART_WORKING 0x0c
#endif
    memset(&ginfo, 0, sizeof(ginfo));
    ginfo.request = RADEON_INFO_FUSION_GART_WORKING;
    ginfo.value = (uintptr_t)&tmp;
    r = drmCommandWriteRead(info->dri->drmFD, DRM_RADEON_INFO, &ginfo, sizeof(ginfo));
    if (r) {
	return FALSE;
    }
    if (tmp == 1)
	return TRUE;
    return FALSE;
}

static Bool RADEONIsAccelWorking(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr info = RADEONPTR(pScrn);
    struct drm_radeon_info ginfo;
    int r;
    uint32_t tmp;

#ifndef RADEON_INFO_ACCEL_WORKING
#define RADEON_INFO_ACCEL_WORKING 0x03
#endif
#ifndef RADEON_INFO_ACCEL_WORKING2
#define RADEON_INFO_ACCEL_WORKING2 0x05
#endif

    memset(&ginfo, 0, sizeof(ginfo));
    if (info->dri->pKernelDRMVersion->version_minor >= 5)
	ginfo.request = RADEON_INFO_ACCEL_WORKING2;
    else
	ginfo.request = RADEON_INFO_ACCEL_WORKING;
    ginfo.value = (uintptr_t)&tmp;
    r = drmCommandWriteRead(info->dri->drmFD, DRM_RADEON_INFO, &ginfo, sizeof(ginfo));
    if (r) {
        /* If kernel is too old before 2.6.32 than assume accel is working */
        if (r == -EINVAL) {
            xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Kernel too old missing accel "
                       "information, assuming accel is working\n");
            return TRUE;
        }
        return FALSE;
    }
    if (tmp)
        return TRUE;
    return FALSE;
}

static Bool RADEONPreInitAccel_KMS(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr  info = RADEONPTR(pScrn);

    if (!(info->accel_state = calloc(1, sizeof(struct radeon_accel_state)))) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Unable to allocate accel_state rec!\n");
	return FALSE;
    }

    if (xf86ReturnOptValBool(info->Options, OPTION_NOACCEL, FALSE) ||
	(!RADEONIsAccelWorking(pScrn))) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "GPU accel disabled or not working, using shadowfb for KMS\n");
	info->r600_shadow_fb = TRUE;
	if (!xf86LoadSubModule(pScrn, "shadow"))
	    info->r600_shadow_fb = FALSE;
	return TRUE;
    }

    if (info->ChipFamily == CHIP_FAMILY_PALM) {
	info->accel_state->allowHWDFS = RADEONIsFusionGARTWorking(pScrn);
    } else
	info->accel_state->allowHWDFS = TRUE;

    if ((info->ChipFamily == CHIP_FAMILY_RS100) ||
	(info->ChipFamily == CHIP_FAMILY_RS200) ||
	(info->ChipFamily == CHIP_FAMILY_RS300) ||
	(info->ChipFamily == CHIP_FAMILY_RS400) ||
	(info->ChipFamily == CHIP_FAMILY_RS480) ||
	(info->ChipFamily == CHIP_FAMILY_RS600) ||
	(info->ChipFamily == CHIP_FAMILY_RS690) ||
	(info->ChipFamily == CHIP_FAMILY_RS740))
	info->accel_state->has_tcl = FALSE;
    else {
	info->accel_state->has_tcl = TRUE;
    }

    info->useEXA = TRUE;
//    info->useEXA = FALSE;

    if (info->useEXA) {
	int errmaj = 0, errmin = 0;
	info->exaReq.majorversion = EXA_VERSION_MAJOR;
	info->exaReq.minorversion = EXA_VERSION_MINOR;
	if (!LoadSubModule(pScrn->module, "exa", NULL, NULL, NULL,
			   &info->exaReq, &errmaj, &errmin)) {
	    LoaderErrorMsg(NULL, "exa", errmaj, errmin);
	    return FALSE;
	}
    }

    return TRUE;
}

static Bool RADEONPreInitChipType_KMS(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr  info   = RADEONPTR(pScrn);
    uint32_t cmd_stat;
    int i;

    info->Chipset = PCI_DEV_DEVICE_ID(info->PciInfo);
    pScrn->chipset = (char *)xf86TokenToString(RADEONChipsets, info->Chipset);
    if (!pScrn->chipset) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "ChipID 0x%04x is not recognized\n", info->Chipset);
	return FALSE;
    }

    if (info->Chipset < 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "Chipset \"%s\" is not recognized\n", pScrn->chipset);
	return FALSE;
    }
    xf86DrvMsg(pScrn->scrnIndex, X_PROBED,
	       "Chipset: \"%s\" (ChipID = 0x%04x)\n",
	       pScrn->chipset,
	       info->Chipset);

    for (i = 0; i < sizeof(RADEONCards) / sizeof(RADEONCardInfo); i++) {
	if (info->Chipset == RADEONCards[i].pci_device_id) {
	    RADEONCardInfo *card = &RADEONCards[i];
	    info->ChipFamily = card->chip_family;
	    info->IsMobility = card->mobility;
	    info->IsIGP = card->igp;
	    break;
	}
    }

    info->cardType = CARD_PCI;

    PCI_READ_LONG(info->PciInfo, &cmd_stat, PCI_CMD_STAT_REG);
    if (cmd_stat & RADEON_CAP_LIST) {
	uint32_t cap_ptr, cap_id;

	PCI_READ_LONG(info->PciInfo, &cap_ptr, RADEON_CAPABILITIES_PTR_PCI_CONFIG);
	cap_ptr &= RADEON_CAP_PTR_MASK;

	while(cap_ptr != RADEON_CAP_ID_NULL) {
	    PCI_READ_LONG(info->PciInfo, &cap_id, cap_ptr);
	    if ((cap_id & 0xff)== RADEON_CAP_ID_AGP) {
		info->cardType = CARD_AGP;
		break;
	    }
	    if ((cap_id & 0xff)== RADEON_CAP_ID_EXP) {
		info->cardType = CARD_PCIE;
		break;
	    }
	    cap_ptr = (cap_id >> 8) & RADEON_CAP_PTR_MASK;
	}
    }


    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "%s card detected\n",
	       (info->cardType==CARD_PCI) ? "PCI" :
		(info->cardType==CARD_PCIE) ? "PCIE" : "AGP");

    /* treat PCIE IGP cards as PCI */
    if (info->cardType == CARD_PCIE && info->IsIGP)
	info->cardType = CARD_PCI;

    if ((info->ChipFamily >= CHIP_FAMILY_R600) && info->IsIGP)
	info->cardType = CARD_PCIE;

    /* not sure about gart table requirements */
    if ((info->ChipFamily == CHIP_FAMILY_RS600) && info->IsIGP)
	info->cardType = CARD_PCIE;

#ifdef RENDER
    info->RenderAccel = xf86ReturnOptValBool(info->Options, OPTION_RENDER_ACCEL,
					     info->Chipset != PCI_CHIP_RN50_515E &&
					     info->Chipset != PCI_CHIP_RN50_5969);
#endif
    return TRUE;
}

static Bool radeon_alloc_dri(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr  info   = RADEONPTR(pScrn);
    if (!(info->dri = calloc(1, sizeof(struct radeon_dri)))) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,"Unable to allocate dri rec!\n");
	return FALSE;
    }

    if (!(info->cp = calloc(1, sizeof(struct radeon_cp)))) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,"Unable to allocate cp rec!\n");
	return FALSE;
    }
    return TRUE;
}

static Bool radeon_open_drm_master(ScrnInfoPtr pScrn)
{
	dHack ("radeon_open_drm_master\n");
    RADEONInfoPtr  info   = RADEONPTR(pScrn);
    RADEONEntPtr pRADEONEnt = RADEONEntPriv(pScrn);
    struct pci_device *dev = info->PciInfo;
    char *busid;
    drmSetVersion sv;
    int err;

    if (pRADEONEnt->fd) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   " reusing fd for second head\n");

	info->dri2.drm_fd = pRADEONEnt->fd;
	goto out;
    }

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,9,99,901,0)
    XNFasprintf(&busid, "pci:%04x:%02x:%02x.%d",
                dev->domain, dev->bus, dev->dev, dev->func);
#else
    busid = XNFprintf("pci:%04x:%02x:%02x.%d",
		      dev->domain, dev->bus, dev->dev, dev->func);
#endif

    info->dri2.drm_fd = drmOpen("radeon", busid);
    if (info->dri2.drm_fd == -1) {

	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "[drm] Failed to open DRM device for %s: %s\n",
		   busid, strerror(errno));
	free(busid);
	return FALSE;
    }
    free(busid);

    /* Check that what we opened was a master or a master-capable FD,
     * by setting the version of the interface we'll use to talk to it.
     * (see DRIOpenDRMMaster() in DRI1)
     */
    sv.drm_di_major = 1;
    sv.drm_di_minor = 1;
    sv.drm_dd_major = -1;
    sv.drm_dd_minor = -1;
    err = drmSetInterfaceVersion(info->dri2.drm_fd, &sv);
    if (err != 0) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "[drm] failed to set drm interface version.\n");
	drmClose(info->dri2.drm_fd);
	info->dri2.drm_fd = -1;

	return FALSE;
    }

    pRADEONEnt->fd = info->dri2.drm_fd;
 out:
    info->drmmode.fd = info->dri2.drm_fd;
    info->dri->drmFD = info->dri2.drm_fd;
    return TRUE;
}

#ifdef EXA_MIXED_PIXMAPS

static Bool r600_get_tile_config(ScrnInfoPtr pScrn)
{
    RADEONInfoPtr  info   = RADEONPTR(pScrn);
    struct drm_radeon_info ginfo;
    int r;
    uint32_t tmp;

    if (info->ChipFamily < CHIP_FAMILY_R600)
	return FALSE;

#ifndef RADEON_INFO_TILING_CONFIG
#define RADEON_INFO_TILING_CONFIG 0x6
#endif

    memset(&ginfo, 0, sizeof(ginfo));
    ginfo.request = RADEON_INFO_TILING_CONFIG;
    ginfo.value = (uintptr_t)&tmp;
    r = drmCommandWriteRead(info->dri->drmFD, DRM_RADEON_INFO, &ginfo, sizeof(ginfo));
    if (r)
	return FALSE;

    info->tile_config = tmp;
    info->r7xx_bank_op = 0;
    if (info->ChipFamily >= CHIP_FAMILY_CEDAR) {
	if (info->dri->pKernelDRMVersion->version_minor >= 7) {
	    switch (info->tile_config & 0xf) {
	    case 0:
                info->num_channels = 1;
                break;
	    case 1:
                info->num_channels = 2;
                break;
	    case 2:
                info->num_channels = 4;
                break;
	    case 3:
                info->num_channels = 8;
                break;
	    default:
                return FALSE;
	    }

	    switch((info->tile_config & 0xf0) >> 4) {
	    case 0:
		info->num_banks = 4;
		break;
	    case 1:
		info->num_banks = 8;
		break;
	    case 2:
		info->num_banks = 16;
		break;
	    default:
		return FALSE;
	    }

	    switch ((info->tile_config & 0xf00) >> 8) {
	    case 0:
                info->group_bytes = 256;
                break;
	    case 1:
                info->group_bytes = 512;
                break;
	    default:
                return FALSE;
	    }
	} else
	    return FALSE;
    } else {
	switch((info->tile_config & 0xe) >> 1) {
	case 0:
	    info->num_channels = 1;
	    break;
	case 1:
	    info->num_channels = 2;
	    break;
	case 2:
	    info->num_channels = 4;
	    break;
	case 3:
	    info->num_channels = 8;
	    break;
	default:
	    return FALSE;
	}
	switch((info->tile_config & 0x30) >> 4) {
	case 0:
	    info->num_banks = 4;
	    break;
	case 1:
	    info->num_banks = 8;
	    break;
	default:
	    return FALSE;
	}
	switch((info->tile_config & 0xc0) >> 6) {
	case 0:
	    info->group_bytes = 256;
	    break;
	case 1:
	    info->group_bytes = 512;
	    break;
	default:
	    return FALSE;
	}
    }

    info->have_tiling_info = TRUE;
    return TRUE;
}

#endif /* EXA_MIXED_PIXMAPS */

Bool RADEONPreInit_KMS(ScrnInfoPtr pScrn, int flags)
{
    RADEONInfoPtr     info;
    RADEONEntPtr pRADEONEnt;
    DevUnion* pPriv;
    Gamma  zeros = { 0.0, 0.0, 0.0 };
    uint32_t tiling = 0;
    int cpp;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONPreInit_KMS\n");
	
	{char buf[64];
	sprintf (buf, "RADEONPreInit_KMS scrnIndex %d\n", pScrn->scrnIndex);
	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, buf);
	}
	
    if (pScrn->numEntities != 1) return FALSE;
    if (!RADEONGetRec(pScrn)) return FALSE;

    info               = RADEONPTR(pScrn);
    info->MMIO         = NULL;
    info->IsSecondary  = FALSE;
    info->IsPrimary = FALSE;
    info->kms_enabled = TRUE;
    info->pEnt         = xf86GetEntityInfo(pScrn->entityList[pScrn->numEntities - 1]);
    if (info->pEnt->location.type != BUS_PCI) goto fail;

    pPriv = xf86GetEntityPrivate(pScrn->entityList[0],
				 getRADEONEntityIndex());
    pRADEONEnt = pPriv->ptr;

    if(xf86IsEntityShared(pScrn->entityList[0]))
    {
        if(xf86IsPrimInitDone(pScrn->entityList[0]))
        {
            info->IsSecondary = TRUE;
            pRADEONEnt->pSecondaryScrn = pScrn;
        }
        else
        {
	    info->IsPrimary = TRUE;
            xf86SetPrimInitDone(pScrn->entityList[0]);
            pRADEONEnt->pPrimaryScrn = pScrn;
            pRADEONEnt->HasSecondary = FALSE;
        }
    }

    info->PciInfo = xf86GetPciInfoForEntity(info->pEnt->index);
    pScrn->monitor     = pScrn->confScreen->monitor;

    if (!RADEONPreInitVisual(pScrn))
	goto fail;

    xf86CollectOptions(pScrn, NULL);
    if (!(info->Options = malloc(sizeof(RADEONOptions_KMS))))
	goto fail;

    memcpy(info->Options, RADEONOptions_KMS, sizeof(RADEONOptions_KMS));
    xf86ProcessOptions(pScrn->scrnIndex, pScrn->options, info->Options);

    if (!RADEONPreInitWeight(pScrn))
	goto fail;

    if (!RADEONPreInitChipType_KMS(pScrn))
        goto fail;

    if (!radeon_alloc_dri(pScrn))
	return FALSE;

#ifdef XORG_WAYLAND
	{
		xf86LoadSubModule(pScrn, "xwayland");
		info->xwl_screen = xwl_screen_pre_init(pScrn, 0, &xwl_driver);
		if (!info->xwl_screen) {
			xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Failed to initialize xwayland.\n");
			return FALSE;
		}

		info->dri2.drm_fd = xwl_screen_get_drm_fd(info->xwl_screen);
		info->drmmode.fd = info->dri2.drm_fd;
		info->dri->drmFD = info->dri2.drm_fd;
		RADEONEntPtr pRADEONEnt = RADEONEntPriv(pScrn);
		pRADEONEnt->fd = info->dri2.drm_fd;
	}
#endif

    if (!info->xwl_screen && radeon_open_drm_master(pScrn) == FALSE) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Kernel modesetting setup failed\n");
	goto fail;
    }

    info->dri2.enabled = FALSE;
    info->dri->pKernelDRMVersion = drmGetVersion(info->dri->drmFD);
    if (info->dri->pKernelDRMVersion == NULL) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "RADEONDRIGetVersion failed to get the DRM version\n");
	goto fail;
    }

    if (!RADEONPreInitAccel_KMS(pScrn))              goto fail;

    info->allowColorTiling2D = FALSE;

#ifdef EXA_MIXED_PIXMAPS
    /* don't enable tiling if accel is not enabled */
    if (!info->r600_shadow_fb) {
	Bool colorTilingDefault =
	    xorgGetVersion() >= XORG_VERSION_NUMERIC(1,9,4,901,0) &&
	    info->ChipFamily >= CHIP_FAMILY_R300 &&
	    info->ChipFamily <= CHIP_FAMILY_ARUBA;

	/* 2D color tiling */
	if (info->ChipFamily >= CHIP_FAMILY_R600) {
		info->allowColorTiling2D = xf86ReturnOptValBool(info->Options, OPTION_COLOR_TILING_2D, FALSE);
	}

	if (info->ChipFamily >= CHIP_FAMILY_R600) {
	    /* set default group bytes, overridden by kernel info below */
	    info->group_bytes = 256;
	    info->have_tiling_info = FALSE;
	    if (info->dri->pKernelDRMVersion->version_minor >= 6) {
		if (r600_get_tile_config(pScrn)) {
		    info->allowColorTiling = xf86ReturnOptValBool(info->Options,
								  OPTION_COLOR_TILING, colorTilingDefault);
		    /* need working DFS for tiling */
		    if ((info->ChipFamily == CHIP_FAMILY_PALM) &&
			(!info->accel_state->allowHWDFS))
			info->allowColorTiling = FALSE;
		} else
		    info->allowColorTiling = FALSE;
	    } else
		xf86DrvMsg(pScrn->scrnIndex, X_INFO,
			   "R6xx+ KMS Color Tiling requires radeon drm 2.6.0 or newer\n");
	} else
	    info->allowColorTiling = xf86ReturnOptValBool(info->Options,
							  OPTION_COLOR_TILING, colorTilingDefault);
    } else
#else
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "KMS Color Tiling requires xserver which supports EXA_MIXED_PIXMAPS\n");
#endif
	info->allowColorTiling = FALSE;
	
	info->allowColorTiling = FALSE;
	info->allowColorTiling2D = FALSE;
	info->allowColorTiling = FALSE;

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "KMS Color Tiling: %sabled\n", info->allowColorTiling ? "en" : "dis");

    if (info->dri->pKernelDRMVersion->version_minor >= 8) {
	info->allowPageFlip = xf86ReturnOptValBool(info->Options,
						   OPTION_PAGE_FLIP, TRUE);
//	info->allowPageFlip = FALSE;
	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		   "KMS Pageflipping: %sabled\n", info->allowPageFlip ? "en" : "dis");
    }

    info->swapBuffersWait = xf86ReturnOptValBool(info->Options,
						 OPTION_SWAPBUFFERS_WAIT, TRUE);
	info->swapBuffersWait = FALSE;
	
    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
	       "SwapBuffers wait for vsync: %sabled\n", info->swapBuffersWait ? "en" : "dis");

	if (!info->xwl_screen)
		info->swapBuffersWait = TRUE;
	info->swapBuffersWait = FALSE;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS 10\n");
    
    if (info->xwl_screen) {
		info->drmmode.mode_res = drmModeGetResources(info->drmmode.fd);
	    
    }else if (/*!info->xwl_screen &&*/ drmmode_pre_init(pScrn, &info->drmmode, pScrn->bitsPerPixel / 8) == FALSE) {
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS 11\n");
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "Kernel modesetting setup failed\n");
	goto fail;
    }
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS 12\n");

//    if (info->drmmode.mode_res->count_crtcs == 1)
//        pRADEONEnt->HasCRTC2 = FALSE;
//    else
        pRADEONEnt->HasCRTC2 = TRUE;


    /* fix up cloning on rn50 cards
     * since they only have one crtc sometimes the xserver doesn't assign
     * a crtc to one of the outputs even though both outputs have common modes
     * which results in only one monitor being enabled.  Assign a crtc here so
     * that both outputs light up.
     */
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS 20\n");
    if (info->ChipFamily == CHIP_FAMILY_RV100 && !pRADEONEnt->HasCRTC2) {
	xf86CrtcConfigPtr xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
	int i;

	for (i = 0; i < xf86_config->num_output; i++) {
	    xf86OutputPtr output = xf86_config->output[i];

	    /* XXX: double check crtc mode */
	    if ((output->probed_modes != NULL) && (output->crtc == NULL))
		output->crtc = xf86_config->crtc[0];
	}
    }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS 30\n");
    {
	struct drm_radeon_gem_info mminfo;

	if (!drmCommandWriteRead(info->dri->drmFD, DRM_RADEON_GEM_INFO, &mminfo, sizeof(mminfo)))
	{
	    info->vram_size = mminfo.vram_visible;
	    info->gart_size = mminfo.gart_size;
	    xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		       "mem size init: gart size :%llx vram size: s:%llx visible:%llx\n",
		       (unsigned long long)mminfo.gart_size,
		       (unsigned long long)mminfo.vram_size,
		       (unsigned long long)mminfo.vram_visible);
	}
    }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS 40\n");
    info->exa_pixmaps = xf86ReturnOptValBool(info->Options,
                                             OPTION_EXA_PIXMAPS, 
					     ((info->vram_size > (32 * 1024 * 1024) &&
					      info->RenderAccel)));
	
	
//	info->exa_pixmaps = FALSE;
    if (info->exa_pixmaps)
    	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		"EXA: Driver will allow EXA pixmaps in VRAM\n");
    else
    	xf86DrvMsg(pScrn->scrnIndex, X_INFO,
		"EXA: Driver will not allow EXA pixmaps in VRAM\n");

    /* no tiled scanout on r6xx+ yet */
    if (info->allowColorTiling) {
	if (info->ChipFamily >= CHIP_FAMILY_R600)
	    tiling |= RADEON_TILING_MICRO;
	else
	    tiling |= RADEON_TILING_MACRO;
    }
    cpp = pScrn->bitsPerPixel / 8;
    pScrn->displayWidth =
	RADEON_ALIGN(pScrn->virtualX, drmmode_get_pitch_align(pScrn, cpp, tiling));
    info->CurrentLayout.displayWidth = pScrn->displayWidth;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS 50\n");
    /* Set display resolution */
    xf86SetDpi(pScrn, 0, 0);

	/* Get ScreenInit function */
    if (!xf86LoadSubModule(pScrn, "fb")) return FALSE;

    if (!xf86SetGamma(pScrn, zeros)) return FALSE;

    if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
	if (!xf86LoadSubModule(pScrn, "ramdac")) return FALSE;
    }

    if (pScrn->modes == NULL) {
      xf86DrvMsg(pScrn->scrnIndex, X_ERROR, "No modes.\n");
      goto fail;
   }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS finish\n");
	dHack ("RADEONPreInit_KMS return true\n");
    return TRUE;
 fail:
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG, "RADEONPreInit_KMS fail\n");
    RADEONFreeRec(pScrn);
    return FALSE;

}

static Bool RADEONCursorInit_KMS(ScreenPtr pScreen)
{
    return xf86_cursors_init (pScreen, CURSOR_WIDTH, CURSOR_HEIGHT,
			      (HARDWARE_CURSOR_TRUECOLOR_AT_8BPP |
			       HARDWARE_CURSOR_AND_SOURCE_WITH_MASK |
			       HARDWARE_CURSOR_SOURCE_MASK_INTERLEAVE_1 |
			       HARDWARE_CURSOR_UPDATE_UNHIDDEN |
			       HARDWARE_CURSOR_ARGB));
}

static Bool RADEONSaveScreen_KMS(ScreenPtr pScreen, int mode)
{
    ScrnInfoPtr  pScrn = xf86Screens[pScreen->myNum];
    Bool         unblank;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONSaveScreen(%d)\n", mode);

    unblank = xf86IsUnblank(mode);
    if (unblank) SetTimeSinceLastInputEvent();

    if ((pScrn != NULL) && pScrn->vtSema) {
	if (unblank)
	    RADEONUnblank(pScrn);
	else
	    RADEONBlank(pScrn);
    }
    return TRUE;
}

/* Called at the end of each server generation.  Restore the original
 * text mode, unmap video memory, and unwrap and call the saved
 * CloseScreen function.
 */
static Bool RADEONCloseScreen_KMS(int scrnIndex, ScreenPtr pScreen)
{
    ScrnInfoPtr    pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr  info  = RADEONPTR(pScrn);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONCloseScreen\n");

    drmmode_uevent_fini(pScrn, &info->drmmode);
    if (info->cs)
      radeon_cs_flush_indirect(pScrn);

    DeleteCallback(&FlushCallback, radeon_flush_callback, pScrn);

    if (info->accel_state->exa) {
	exaDriverFini(pScreen);
	free(info->accel_state->exa);
	info->accel_state->exa = NULL;
    }

    if (info->accel_state->use_vbos)
        radeon_vbo_free_lists(pScrn);

    drmDropMaster(info->dri->drmFD);

    if (info->cursor) xf86DestroyCursorInfoRec(info->cursor);
    info->cursor = NULL;

    if (info->dri2.enabled)
	radeon_dri2_close_screen(pScreen);

    pScrn->vtSema = FALSE;
    xf86ClearPrimInitDone(info->pEnt->index);
    pScreen->BlockHandler = info->BlockHandler;
    pScreen->CloseScreen = info->CloseScreen;
    return (*pScreen->CloseScreen)(scrnIndex, pScreen);
}


void RADEONFreeScreen_KMS(int scrnIndex, int flags)
{
    ScrnInfoPtr  pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr  info  = RADEONPTR(pScrn);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONFreeScreen\n");

    /* when server quits at PreInit, we don't need do this anymore*/
    if (!info) return;

    RADEONFreeRec(pScrn);
}

Bool RADEONScreenInit_KMS(int scrnIndex, ScreenPtr pScreen,
			  int argc, char **argv)
{
    ScrnInfoPtr    pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr  info  = RADEONPTR(pScrn);
    int            subPixelOrder = SubPixelUnknown;
    char*          s;
    void *front_ptr;
    int ret;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS\n");
		   
    pScrn->fbOffset = 0;

    miClearVisualTypes();
    if (!miSetVisualTypes(pScrn->depth,
			  miGetDefaultVisualMask(pScrn->depth),
			  pScrn->rgbBits,
			  pScrn->defaultVisual)) return FALSE;
    miSetPixmapDepths ();

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 10\n");
		   
    ret = drmSetMaster(info->dri->drmFD);
    if (ret) {
        ErrorF("Unable to retrieve master\n");
        return FALSE;
    }
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 11\n");
    info->directRenderingEnabled = FALSE;
    
    if (info->r600_shadow_fb == FALSE)
        info->directRenderingEnabled = radeon_dri2_screen_init(pScreen);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 12\n");
    front_ptr = info->FB;

    info->surf_man = radeon_surface_manager_new(info->dri->drmFD);
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 13\n");
    if (!info->bufmgr)
        info->bufmgr = radeon_bo_manager_gem_ctor(info->dri->drmFD);
    if (!info->bufmgr) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "failed to initialise GEM buffer manager");
	return FALSE;
    }
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 14\n");
    drmmode_set_bufmgr(pScrn, &info->drmmode, info->bufmgr);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 15\n");
    if (!info->csm)
        info->csm = radeon_cs_manager_gem_ctor(info->dri->drmFD);
    if (!info->csm) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "failed to initialise command submission manager");
	return FALSE;
    }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 16\n");
    if (!info->cs)
        info->cs = radeon_cs_create(info->csm, RADEON_BUFFER_SIZE/4);
    if (!info->cs) {
	xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
		   "failed to initialise command submission buffer");
	return FALSE;
    }
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 17\n");

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 20\n");
    radeon_cs_set_limit(info->cs, RADEON_GEM_DOMAIN_GTT, info->gart_size);
    radeon_cs_space_set_flush(info->cs, (void(*)(void *))radeon_cs_flush_indirect, pScrn); 

    radeon_setup_kernel_mem(pScreen);
    front_ptr = info->front_bo->ptr;

    if (info->r600_shadow_fb) {
	info->fb_shadow = calloc(1,
				 pScrn->displayWidth * pScrn->virtualY *
				 ((pScrn->bitsPerPixel + 7) >> 3));
	if (info->fb_shadow == NULL) {
	    xf86DrvMsg(pScrn->scrnIndex, X_ERROR,
                       "Failed to allocate shadow framebuffer\n");
	    info->r600_shadow_fb = FALSE;
	} else {
	    if (!fbScreenInit(pScreen, info->fb_shadow,
			      pScrn->virtualX, pScrn->virtualY,
			      pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth,
			      pScrn->bitsPerPixel))
		return FALSE;
	}
    }

    if (info->r600_shadow_fb == FALSE) {
	/* Init fb layer */
	if (!fbScreenInit(pScreen, front_ptr,
			  pScrn->virtualX, pScrn->virtualY,
			  pScrn->xDpi, pScrn->yDpi, pScrn->displayWidth,
			  pScrn->bitsPerPixel))
	    return FALSE;
    }

    xf86SetBlackWhitePixels(pScreen);

    if (pScrn->bitsPerPixel > 8) {
	VisualPtr  visual;

	visual = pScreen->visuals + pScreen->numVisuals;
	while (--visual >= pScreen->visuals) {
	    if ((visual->class | DynamicClass) == DirectColor) {
		visual->offsetRed   = pScrn->offset.red;
		visual->offsetGreen = pScrn->offset.green;
		visual->offsetBlue  = pScrn->offset.blue;
		visual->redMask     = pScrn->mask.red;
		visual->greenMask   = pScrn->mask.green;
		visual->blueMask    = pScrn->mask.blue;
	    }
	}
    }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 50\n");
    /* Must be after RGB order fixed */
    fbPictureInit (pScreen, 0, 0);

#ifdef RENDER
    if ((s = xf86GetOptValString(info->Options, OPTION_SUBPIXEL_ORDER))) {
	if (strcmp(s, "RGB") == 0) subPixelOrder = SubPixelHorizontalRGB;
	else if (strcmp(s, "BGR") == 0) subPixelOrder = SubPixelHorizontalBGR;
	else if (strcmp(s, "NONE") == 0) subPixelOrder = SubPixelNone;
	PictureSetSubpixelOrder (pScreen, subPixelOrder);
    }
#endif

    pScrn->vtSema = TRUE;
    /* Backing store setup */
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "Initializing backing store\n");
    miInitializeBackingStore(pScreen);
    xf86SetBackingStore(pScreen);


    if (info->directRenderingEnabled) {
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Direct rendering enabled\n");
    } else {
	xf86DrvMsg(pScrn->scrnIndex, X_WARNING,
		   "Direct rendering disabled\n");
    }

    if (info->r600_shadow_fb) {
	xf86DrvMsg(scrnIndex, X_INFO, "Acceleration disabled\n");
	info->accelOn = FALSE;
    } else {
	xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		       "Initializing Acceleration\n");
	if (RADEONAccelInit(pScreen)) {
	    xf86DrvMsg(scrnIndex, X_INFO, "Acceleration enabled\n");
	    info->accelOn = TRUE;
	} else {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Acceleration initialization failed\n");
	    xf86DrvMsg(scrnIndex, X_INFO, "Acceleration disabled\n");
	    info->accelOn = FALSE;
	}
    }

    /* Init DPMS */
    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "Initializing DPMS\n");
    xf86DPMSInit(pScreen, xf86DPMSSet, 0);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "Initializing Cursor\n");

    /* Set Silken Mouse */
    xf86SetSilkenMouse(pScreen);

    /* Cursor setup */
    miDCInitialize(pScreen, xf86GetPointerScreenFuncs());

    if (!xf86ReturnOptValBool(info->Options, OPTION_SW_CURSOR, FALSE)) {
	if (RADEONCursorInit_KMS(pScreen)) {
	}
    }

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 60\n");
    /* DGA setup */
#ifdef XFreeXDGA
    /* DGA is dangerous on kms as the base and framebuffer location may change:
     * http://lists.freedesktop.org/archives/xorg-devel/2009-September/002113.html
     */
    /* xf86DiDGAInit(pScreen, info->LinearAddr + pScrn->fbOffset); */
#endif
    if (info->r600_shadow_fb == FALSE) {
        /* Init Xv */
        xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
                       "Initializing Xv\n");
        RADEONInitVideo(pScreen);
    }

    if (info->r600_shadow_fb == TRUE) {
        if (!shadowSetup(pScreen)) {
	    xf86DrvMsg(scrnIndex, X_ERROR,
		       "Shadowfb initialization failed\n");
            return FALSE;
        }
    }
    pScrn->pScreen = pScreen;

    /* Provide SaveScreen & wrap BlockHandler and CloseScreen */
    /* Wrap CloseScreen */
    info->CloseScreen    = pScreen->CloseScreen;
    pScreen->CloseScreen = RADEONCloseScreen_KMS;
    pScreen->SaveScreen  = RADEONSaveScreen_KMS;
    info->BlockHandler = pScreen->BlockHandler;
    pScreen->BlockHandler = RADEONBlockHandler_KMS;

    if (!AddCallback(&FlushCallback, radeon_flush_callback, pScrn))
        return FALSE;

    info->CreateScreenResources = pScreen->CreateScreenResources;
    pScreen->CreateScreenResources = RADEONCreateScreenResources_KMS;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 80\n");
   if (!xf86CrtcScreenInit (pScreen))
       return FALSE;

   /* Wrap pointer motion to flip touch screen around */
//    info->PointerMoved = pScrn->PointerMoved;
//    pScrn->PointerMoved = RADEONPointerMoved;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS 90\n");
    if (!drmmode_setup_colormap(pScreen, pScrn))
	return FALSE;

   /* Note unused options */
    if (serverGeneration == 1)
	xf86ShowUnusedOptions(pScrn->scrnIndex, pScrn->options);

    drmmode_init(pScrn, &info->drmmode);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit finished\n");

    info->accel_state->XInited3D = FALSE;
    info->accel_state->engineMode = EXA_ENGINEMODE_UNKNOWN;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONScreenInit_KMS finish\n");
    return TRUE;
}

Bool RADEONEnterVT_KMS(int scrnIndex, int flags)
{
    ScrnInfoPtr    pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr  info  = RADEONPTR(pScrn);
    int ret;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONEnterVT_KMS\n");


    ret = drmSetMaster(info->dri->drmFD);
    if (ret)
	ErrorF("Unable to retrieve master\n");
    info->accel_state->XInited3D = FALSE;
    info->accel_state->engineMode = EXA_ENGINEMODE_UNKNOWN;

    pScrn->vtSema = TRUE;

    if (!drmmode_set_desired_modes(pScrn, &info->drmmode))
	return FALSE;

    if (info->adaptor)
	RADEONResetVideo(pScrn);

    return TRUE;
}


void RADEONLeaveVT_KMS(int scrnIndex, int flags)
{
    ScrnInfoPtr    pScrn = xf86Screens[scrnIndex];
    RADEONInfoPtr  info  = RADEONPTR(pScrn);

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "RADEONLeaveVT_KMS\n");

    drmDropMaster(info->dri->drmFD);

#ifdef HAVE_FREE_SHADOW
    xf86RotateFreeShadow(pScrn);
#endif

    xf86_hide_cursors (pScrn);
    info->accel_state->XInited3D = FALSE;
    info->accel_state->engineMode = EXA_ENGINEMODE_UNKNOWN;

    xf86DrvMsgVerb(pScrn->scrnIndex, X_INFO, RADEON_LOGLEVEL_DEBUG,
		   "Ok, leaving now...\n");
}


Bool RADEONSwitchMode_KMS(int scrnIndex, DisplayModePtr mode, int flags)
{
    ScrnInfoPtr    pScrn       = xf86Screens[scrnIndex];
    Bool ret;
    ret = xf86SetSingleMode (pScrn, mode, RR_Rotate_0);
    return ret;

}

void RADEONAdjustFrame_KMS(int scrnIndex, int x, int y, int flags)
{
    ScrnInfoPtr    pScrn       = xf86Screens[scrnIndex];
    RADEONInfoPtr  info        = RADEONPTR(pScrn);
    drmmode_adjust_frame(pScrn, &info->drmmode, x, y, flags);
    return;
}

static Bool radeon_setup_kernel_mem(ScreenPtr pScreen)
{
    ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
    RADEONInfoPtr info = RADEONPTR(pScrn);
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    int cpp = info->CurrentLayout.pixel_bytes;
    int screen_size;
    int pitch, base_align;
    int total_size_bytes = 0;
    uint32_t tiling_flags = 0;
    struct radeon_surface surface;

    if (info->accel_state->exa != NULL) {
	xf86DrvMsg(pScreen->myNum, X_ERROR, "Memory map already initialized\n");
	return FALSE;
    }
    if (info->r600_shadow_fb == FALSE) {
        info->accel_state->exa = exaDriverAlloc();
        if (info->accel_state->exa == NULL)
	    return FALSE;
    }

    if (info->allowColorTiling) {
	if (info->ChipFamily >= CHIP_FAMILY_R600) {
		if (info->allowColorTiling2D) {
			tiling_flags |= RADEON_TILING_MACRO;
		} else {
			tiling_flags |= RADEON_TILING_MICRO;
		}
	} else
	    tiling_flags |= RADEON_TILING_MACRO;
    }
    pitch = RADEON_ALIGN(pScrn->displayWidth, drmmode_get_pitch_align(pScrn, cpp, tiling_flags)) * cpp;
    screen_size = RADEON_ALIGN(pScrn->virtualY, drmmode_get_height_align(pScrn, tiling_flags)) * pitch;
    base_align = drmmode_get_base_align(pScrn, cpp, tiling_flags);
	if (info->ChipFamily >= CHIP_FAMILY_R600) {
		memset(&surface, 0, sizeof(struct radeon_surface));
		surface.npix_x = pScrn->displayWidth;
		surface.npix_y = pScrn->virtualY;
		surface.npix_z = 1;
		surface.blk_w = 1;
		surface.blk_h = 1;
		surface.blk_d = 1;
		surface.array_size = 1;
		surface.last_level = 0;
		surface.bpe = cpp;
		surface.nsamples = 1;
		surface.flags = RADEON_SURF_SCANOUT;
		surface.flags |= RADEON_SURF_SET(RADEON_SURF_TYPE_2D, TYPE);
		surface.flags |= RADEON_SURF_SET(RADEON_SURF_MODE_LINEAR_ALIGNED, MODE);
		if (tiling_flags & RADEON_TILING_MICRO) {
			surface.flags = RADEON_SURF_CLR(surface.flags, MODE);
			surface.flags |= RADEON_SURF_SET(RADEON_SURF_MODE_1D, MODE);
		}
		if (tiling_flags & RADEON_TILING_MACRO) {
			surface.flags = RADEON_SURF_CLR(surface.flags, MODE);
			surface.flags |= RADEON_SURF_SET(RADEON_SURF_MODE_2D, MODE);
		}
		if (radeon_surface_best(info->surf_man, &surface)) {
			return FALSE;
		}
		if (radeon_surface_init(info->surf_man, &surface)) {
			return FALSE;
		}
		pitch = surface.level[0].pitch_bytes;
		screen_size = surface.bo_size;
		base_align = surface.bo_alignment;
		tiling_flags = 0;
		switch (surface.level[0].mode) {
		case RADEON_SURF_MODE_2D:
			tiling_flags |= RADEON_TILING_MACRO;
			tiling_flags |= surface.bankw << RADEON_TILING_EG_BANKW_SHIFT;
			tiling_flags |= surface.bankh << RADEON_TILING_EG_BANKH_SHIFT;
			tiling_flags |= surface.mtilea << RADEON_TILING_EG_MACRO_TILE_ASPECT_SHIFT;
			tiling_flags |= eg_tile_split(surface.tile_split) << RADEON_TILING_EG_TILE_SPLIT_SHIFT;
			break;
		case RADEON_SURF_MODE_1D:
			tiling_flags |= RADEON_TILING_MICRO;
			break;
		default:
			break;
		}
		info->front_surface = surface;
	}
    {
	int cursor_size = 64 * 4 * 64;
	int c;

	cursor_size = RADEON_ALIGN(cursor_size, RADEON_GPU_PAGE_SIZE);
	for (c = 0; c < xf86_config->num_crtc; c++) {
	    /* cursor objects */
            if (info->cursor_bo[c] == NULL) {
                info->cursor_bo[c] = radeon_bo_open(info->bufmgr, 0,
                                                    cursor_size, 0,
                                                    RADEON_GEM_DOMAIN_VRAM, 0);
                if (!info->cursor_bo[c]) {
                    return FALSE;
                }

                if (radeon_bo_map(info->cursor_bo[c], 1)) {
                    ErrorF("Failed to map cursor buffer memory\n");
                }

                drmmode_set_cursor(pScrn, &info->drmmode, c, info->cursor_bo[c]);
                total_size_bytes += cursor_size;
            }
        }
    }

    screen_size = RADEON_ALIGN(screen_size, RADEON_GPU_PAGE_SIZE);
    /* keep area front front buffer - but don't allocate it yet */
    total_size_bytes += screen_size;

    info->dri->textureSize = 0;

    if (info->front_bo == NULL) {
        info->front_bo = radeon_bo_open(info->bufmgr, 0, screen_size,
                                        base_align, RADEON_GEM_DOMAIN_VRAM, 0);
        if (info->r600_shadow_fb == TRUE) {
            if (radeon_bo_map(info->front_bo, 1)) {
                ErrorF("Failed to map cursor buffer memory\n");
            }
        }
#if X_BYTE_ORDER == X_BIG_ENDIAN
	switch (cpp) {
	case 4:
	    tiling_flags |= RADEON_TILING_SWAP_32BIT;
	    break;
	case 2:
	    tiling_flags |= RADEON_TILING_SWAP_16BIT;
	    break;
	}
#endif
	if (tiling_flags)
            radeon_bo_set_tiling(info->front_bo, tiling_flags, pitch);
    }

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "Front buffer size: %dK\n", info->front_bo->size/1024);
    radeon_kms_update_vram_limit(pScrn, screen_size);
    return TRUE;
}

void radeon_kms_update_vram_limit(ScrnInfoPtr pScrn, int new_fb_size)
{
    xf86CrtcConfigPtr   xf86_config = XF86_CRTC_CONFIG_PTR(pScrn);
    RADEONInfoPtr info = RADEONPTR(pScrn);
    int remain_size_bytes;
    int total_size_bytes;
    int c;

    for (c = 0; c < xf86_config->num_crtc; c++) {
	if (info->cursor_bo[c] != NULL) {
	    total_size_bytes += (64 * 4 * 64);
	}
    }

    total_size_bytes += new_fb_size;
    remain_size_bytes = info->vram_size - new_fb_size;
    remain_size_bytes = (remain_size_bytes / 10) * 9;
    radeon_cs_set_limit(info->cs, RADEON_GEM_DOMAIN_VRAM, remain_size_bytes);

    xf86DrvMsg(pScrn->scrnIndex, X_INFO, "VRAM usage limit set to %dK\n", remain_size_bytes / 1024);
}


#endif

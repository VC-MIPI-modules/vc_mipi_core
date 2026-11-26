#ifndef _VC_MIPI_CORE_H
#define _VC_MIPI_CORE_H

#define VERSION "0.7.0"

#define ENABLE_ADVANCED_CONTROL

#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/videodev2.h>

// The core is linked into the camera driver module on all platforms (Raspberry Pi, NXP,
// NVIDIA), so it exports no symbols.

// Debug level, module parameter "debug" (named vc_debug to avoid clashes with other modules)
extern int vc_debug;

#define level(level) if (vc_debug >= level)
#define vc_reg(dev, fmt, ...) level(6) dev_info(dev, fmt, ##__VA_ARGS__)
#define vc_dbg(dev, fmt, ...) level(5) dev_info(dev, fmt, ##__VA_ARGS__)
#define vc_info(dev, fmt, ...) level(4) dev_info(dev, fmt, ##__VA_ARGS__)
#define vc_notice(dev, fmt, ...) level(3) dev_notice(dev, fmt, ##__VA_ARGS__)
#define vc_warn(dev, fmt, ...) level(2) dev_warn(dev, fmt, ##__VA_ARGS__)
#define vc_err(dev, fmt, ...) level(1) dev_err(dev, fmt, ##__VA_ARGS__)

#define FLAG_RESET_ALWAYS               (1 <<  0)
#define FLAG_EXPOSURE_SONY              (1 <<  1)
#define FLAG_EXPOSURE_NORMAL            (1 <<  2)
#define FLAG_EXPOSURE_OMNIVISION        (1 <<  3)

#define FLAG_IO_ENABLED                 (1 <<  4)
#define FLAG_DOUBLE_HEIGHT              (1 <<  6)
#define FLAG_INCREASE_FRAME_RATE        (1 <<  7)

#define FLAG_TRIGGER_DISABLE            (1 <<  8)
#define FLAG_TRIGGER_EXTERNAL           (1 <<  9)
#define FLAG_TRIGGER_PULSEWIDTH         (1 << 10)
#define FLAG_TRIGGER_SELF               (1 << 11)
#define FLAG_TRIGGER_SELF_V2            (1 << 12)
#define FLAG_TRIGGER_SINGLE             (1 << 13)
#define FLAG_TRIGGER_SYNC               (1 << 14)
#define FLAG_TRIGGER_STREAM_EDGE        (1 << 15)
#define FLAG_TRIGGER_STREAM_LEVEL       (1 << 16)
#define FLAG_TRIGGER_SLAVE              (1 << 17)

#define FLAG_PREGIUS_S                  (1 << 18)
#define FLAG_USE_BINNING_INDEX          (1 << 19)
// Keep XTRIG enabled in free running mode too: the module distributes its own
// sync pulses over XTRIG (e.g. OV9281L linked sensor array)
#define FLAG_XTRIG_ALWAYS               (1 << 20)
// Overlapping (sequential) external trigger: the next exposure may start during
// the read out. Needs module modes of type MODE_TYPE_TRIGGER_OVERLAP (IMX250, IMX252).
#define FLAG_TRIGGER_OVERLAP            (1 << 21)
// Don't reset the module when a stream is restarted in the same trigger mode (IMX252)
#define FLAG_TRIGGER_NO_RESET           (1 << 22)


// Color Filter Array (Bayer pattern) of a color sensor, starting with the
// top left pixel of the first two lines.
enum vc_cfa_pattern {
        VC_CFA_RGGB = 0,
        VC_CFA_GBRG,
        VC_CFA_GRBG,
        VC_CFA_BGGR,
};

#define GAIN_DISABLED                   0
#define GAIN_LINEAR                     1
#define GAIN_LOGARITHMIC                2
#define GAIN_RECIPROCAL                 3
#define GAIN_FRACTIONAL                 4

#define FORMAT_RAW08                    0x2a
#define FORMAT_RAW10                    0x2b
#define FORMAT_RAW12                    0x2c
#define FORMAT_RAW14                    0x2d

#define MAX_VC_MODES                    16
#define MAX_EXTRA_MODE_REGS             8

#define MAX_MBUS_CODES                  5
#define MAX_VC_DESC_MODES               24

#define MAX_I2C_RETRY_COUNT             5

struct vc_desc_mode {
        __u8 data_rate[4];
        __u8 num_lanes;
        __u8 format;
        __u8 type;
        __u8 binning;
        __u8 reserved2[8];
};

struct vc_desc {
        // Module description
        __u8 magic[12];
        __u8 manuf[32];
        __u16 manuf_id;
        __u8 sen_manuf[8];
        __u8 sen_type[16];
        __u16 mod_id;
        __u16 mod_rev;
        __u16 chip_id_high;
        __u16 chip_id_low;
        __u16 chip_rev;
        // Sensor registers
        __u16 csr_mode;
        __u16 csr_h_start_h;
        __u16 csr_h_start_l;
        __u16 csr_v_start_h;
        __u16 csr_v_start_l;
        __u16 csr_h_end_h;
        __u16 csr_h_end_l;
        __u16 csr_v_end_h;
        __u16 csr_v_end_l;
        __u16 csr_o_width_h;
        __u16 csr_o_width_l;
        __u16 csr_o_height_h;
        __u16 csr_o_height_l;
        __u16 csr_exposure_h;
        __u16 csr_exposure_m;
        __u16 csr_exposure_l;
        __u16 csr_gain_h;
        __u16 csr_gain_l;
        // Exposure Settings
        __u32 clk_ext_trigger;
        __u32 clk_pixel;
        __u16 shutter_offset;
        // Reserved
        __u8 reserved[4];
        // Modes
        __u16 num_modes;
        __u16 bytes_per_mode;
        struct vc_desc_mode modes[MAX_VC_DESC_MODES];
};

typedef struct vc_control {
        __u32 min;
        __u32 max;
        __u32 def;
} vc_control;

typedef struct vc_control64 { 
        __u64 min;
        __u64 max;
        __u64 def;
} vc_control64;

typedef struct vc_gain {
        __u32 max;
        __u32 max_mdB;
        __u32 type;
        __u32 c0;
        __u32 c1;
} vc_gain;

typedef struct vc_frame {
        __u32 left;
        __u32 top;
        __u32 width;
        __u32 height;
} vc_frame;

// offset: Bit position of the value LSB within the combined register (0 = LSB of l)
// length: Number of valid value bits (0 = max length, i.e. register width - offset)
// preserve: Keep bits outside of offset/length (read-modify-write), otherwise they are written as 0
typedef struct vc_csr2 {
        __u32 l;
        __u32 m;
        __u8 offset;
        __u8 length;
        bool preserve;
} vc_csr2;

typedef struct vc_csr4 {
        __u32 l;
        __u32 m;
        __u32 h;
        __u32 u;
        __u8 offset;
        __u8 length;
        bool preserve;
} vc_csr4;

struct vc_sen_csr {
        struct vc_csr2 mode;
        __u8 mode_standby;
        __u8 mode_operating;
        struct vc_csr4 vmax;
        struct vc_csr4 hmax;
        struct vc_csr4 shs;
        struct vc_csr2 again;
        struct vc_csr2 dgain;
        struct vc_csr2 blacklevel;
        struct vc_csr2 h_start;
        struct vc_csr2 v_start;
        struct vc_csr2 h_end;
        struct vc_csr2 v_end;
        struct vc_csr2 w_width;
        struct vc_csr2 w_height;
        struct vc_csr2 o_width;
        struct vc_csr2 o_height;
        // Digital crop and scaling (e.g. IMX412), 0 = not supported
        struct vc_csr2 d_left;
        struct vc_csr2 d_top;
        struct vc_csr2 d_width;
        struct vc_csr2 d_height;
        struct vc_csr2 scale;
        // Grouped parameter hold: register changes apply together in the next frame
        struct vc_csr2 group_hold;
        // System clock divider of the video timing domain (IMX412 VTSYCK_DIV)
        struct vc_csr2 vt_syck_div;
        struct vc_csr4 flash_duration;
        struct vc_csr4 flash_offset;
};

struct vc_csr {
        struct vc_sen_csr sen;
};

typedef struct vc_reg {
        __u16 address;
        __u8 value;
} vc_reg;

// Scaling mode of sensors with a scaler (e.g. IMX412). h_factor/v_factor > 0:
// the image is scaled horizontally/vertically by scale_numerator / state->scale.
#define MAX_VC_SCALINGS                 3
typedef struct vc_scaling {
        __u8 h_factor;
        __u8 v_factor;
        struct vc_reg regs[8];
} vc_scaling;

#define SCALING_START(scaling, h, v) \
        scaling = (vc_scaling) { .h_factor = h, .v_factor = v }; \
        { const struct vc_reg regs [] = {
#define SCALING_END(scaling) \
        , {0, 0} }; memcpy(&scaling.regs, regs, sizeof(regs)); }

typedef struct vc_mode {
        __u8       num_lanes;
        __u8       format;
        __u8       binning;
        vc_control hmax;
        vc_control vmax;
        vc_control blacklevel;
        __u32      retrigger_min;
        __u32      vmax_row_margin;
        __u32      vmax_row_floor;
        __u32      vmax_row_scale;
        // Binning scale factors in x (horizontal) and y (vertical) direction.
        // The output image is h_scale x v_scale smaller than the read out
        // sensor area. 1 (or 0) means no binning.
        __u8       h_scale;
        __u8       v_scale;
        // Virtual mode: Not defined in the module (FPGA) mode table. The
        // module is set to the mode of the real mode base (index into
        // ctrl->mode[]), then the extra registers of base and of this mode are
        // written, so this mode overwrites or extends the base settings.
        bool       virt;
        __u8       base;
        // Overwrite (only with virt, -1 = none): Index of the mode this virtual
        // mode replaces while a module trigger mode (external, pulse width,
        // self, single) is active. The module is set to the STREAM mode of
        // base instead of its trigger mode, the extra registers set the sensor
        // up for triggering (e.g. OV9281: FSIN wake-up without auto sleep).
        // Not selected by lanes, format and binning (vc_core_get_mode_index()).
        __s8       overwrite;
        // Extra sensor registers written whenever this mode is selected, after
        // the ROI has been set (see vc_sen_write_extra_mode_regs() in
        // vc_mipi_core.c), e.g. AR2020's eDR-mode CSI2 data type tag or the
        // IMX56x timing registers for binning modes.
        struct vc_reg extra_regs[MAX_EXTRA_MODE_REGS];
} vc_mode;

typedef struct vc_tout {
        bool tout1_sel;
        bool tout2_sel;

        bool trig_tout1_sel;
        bool trig_tout2_sel;

        bool pulse1_en_nor;
        bool pulse1_en_trig;
        bool pulse1_pol;

        __u32 pulse1_up;
        __u32 pulse1_dn;

        bool pulse2_en_nor;
        bool pulse2_en_trig;
        bool pulse2_pol;
        __u32 pulse2_up;
        __u32 pulse2_dn;

        vc_reg toutsel_reg;
        vc_reg trigtoutsel_reg;
        vc_reg pulse1_reg;
        vc_csr4 pulse1_up_reg;
        vc_csr4 pulse1_dn_reg;
        vc_reg pulse2_reg;
        vc_csr4 pulse2_up_reg;
        vc_csr4 pulse2_dn_reg;
} vc_tout;

#define TOUT(tout, _toutsel_reg, _trigtoutsel_reg, _pulse1_reg, _pulse1_up_reg, _pulse1_dn_reg, _pulse2_reg, _pulse2_up_reg, _pulse2_dn_reg) \
    tout = (vc_tout){                                                      \
        .tout1_sel         = true,                                          \
        .tout2_sel         = true,                                          \
        .trig_tout1_sel    = true,                                          \
        .trig_tout2_sel    = true,                                          \
        .pulse1_en_nor     = true,                                          \
        .pulse2_en_nor     = true,                                          \
        .pulse1_en_trig    = true,                                          \
        .pulse2_en_trig    = true,                                          \
        .pulse1_pol        = true,                                          \
        .pulse2_pol        = true,                                          \
        .pulse1_up         = 0,                                             \
        .pulse1_dn         = 0,                                             \
        .pulse2_up         = 0,                                             \
        .pulse2_dn         = 0,                                             \
        .toutsel_reg       = (vc_reg){.address =_toutsel_reg, .value = 0b1111 },                                   \
        .trigtoutsel_reg   = (vc_reg){.address =_trigtoutsel_reg, .value = 0b0101},                               \
        .pulse1_reg        = (vc_reg){.address =_pulse1_reg, .value = 0b1011},                                    \
        .pulse1_up_reg     = _pulse1_up_reg,                                 \
        .pulse1_dn_reg     = _pulse1_dn_reg,                                 \
        .pulse2_reg        = (vc_reg){.address =_pulse2_reg, .value = 0b1011},                                    \
        .pulse2_up_reg     = _pulse2_up_reg,                                 \
        .pulse2_dn_reg     = _pulse2_dn_reg                                  \
    };
struct vc_ctrl {
        // Communication
        int mod_i2c_addr;
        struct i2c_client *client_sen;
        struct i2c_client *client_mod;
        // Controls
        struct vc_mode mode[MAX_VC_MODES];
        struct vc_control exposure;

        struct vc_gain again;
        struct vc_gain dgain;
        struct vc_control framerate;
        // Modes & Frame Formats
        struct vc_frame frame;          // Pixel
        struct vc_tout tout;

        __u8 max_binning_modes_used;
        // Scaling modes, scalings[0] is "no scaling". Only used if max_scaling_mode > 0.
        struct vc_scaling scalings[MAX_VC_SCALINGS];
        __u8 max_scaling_mode;
        __u8 scale_numerator;
        
        // Control and status registers
        struct vc_csr csr;
        // Exposure
        __u32 clk_ext_trigger;          // Hz
        __u32 clk_pixel;                // Hz
        // Flash
        __u32 flash_factor;
        __s32 flash_toffset;
        // Cropping and exposure related registers
        __u32 vmax_exposure_margin;
        // Vertical crop alignment (rows) of top and height for a colour (Bayer) output. A colour
        // output always uses an even step to keep the Bayer pattern (at least 2, see
        // vc_core_get_crop_top_step()), set more only if the sensor needs it. Mono outputs are
        // not aligned.
        __u32 crop_top_step;
        // Trigger snapshot mode (all OmniVision sensors, FLAG_EXPOSURE_OMNIVISION): On the FSIN
        // edge the sensor wakes up, exposes and sends one frame of VMAX rows, triggers during
        // this time are ignored. In trigger modes VMAX is therefore not extended to the frame
        // rate, the frame rate is set by the retrigger period only, which is at least
        // trigger_wakeup_us + (trigger_wakeup_rows + SHS + VMAX) * row period.
        // trigger_wakeup_us:   wake up time from the FSIN edge (datasheet: 16388 XVCLK)
        // trigger_wakeup_rows: rows from the wake up to the integration start (tExp_Dly) plus
        //                      rows from the integration end to the frame start (tExp_Sof)
        __u32 trigger_wakeup_us;
        __u32 trigger_wakeup_rows;
        // Color filter array (Bayer pattern) of color sensors
        enum vc_cfa_pattern cfa;
        // Special features
        __u32 flags;
        __u32 mbus_codes[MAX_MBUS_CODES];
        bool  packed_outputs;
        // If non-zero, VMAX is fixed by the module and never written by the driver.
        // Only SHS follows the exposure (e.g. OV9281L linked sensor array).
        __u32 static_vmax;
};

struct vc_state {
        __u8 mode;
        __u32 vmax;
        __u32 shs;
        __u32 exposure;                 // µs
        __u32 gain;                     // mdB
        __u32 blacklevel;
        __u32 exposure_cnt;             // Exposure in clock cycles
        __u32 retrigger_cnt;
        __u32 framerate;
        __u32 format_code;
        struct vc_frame frame;          // Pixel, output frame (V4L2 compose / format size)
        struct vc_frame crop;           // Pixel, analog crop (V4L2 crop), 0x0: follows the output frame
        __u8 scaling_mode;
        __u32 scale;                    // Scale denominator, image size * scale_numerator / scale
        __u32 d_left;                   // Digital crop offset in the analog crop (live ROI)
        __u32 d_top;
        __u8 num_lanes;
        __u8 io_mode;
        __u8 trigger_mode;
        bool trigger_overlap;           // External trigger with overlapping exposure (trigger mode 8)
        __u8 binning_mode;
        __u8 ctrl_mode;                 // Index of the active ctrl->mode[] (0xff: none)
        int power_on;
        int streaming;
        __u8 flags;
#ifdef ENABLE_ADVANCED_CONTROL
        __s32 hmax_overwrite;
        __s32 vmax_overwrite;
        __s32 width_offset;
        __s32 height_offset;
        __s32 vt_syck_div;              // 0: module default, else written to csr.sen.vt_syck_div
#endif
};

struct vc_cam {
        struct vc_desc desc;
        struct vc_ctrl ctrl;
        struct vc_state state;
        u32 force_color_mode; // Set to 1 to treat a mono sensor as color (from DT: force-color-mode)
        // Platform setting of the camera driver: additional wait after a module reset in
        // vc_sen_start_stream(), before the sensor registers are written (NVIDIA: 100-200 ms
        // measured on Tegra). 0 = no additional wait (the module ready status is always polled).
        __u32 reset_delay_ms;
};

// --- Helper functions to allow i2c communication for customization ----------
int vc_read_i2c_reg(struct i2c_client *client, const __u16 addr);
int vc_write_i2c_reg(struct i2c_client *client, const __u16 addr, const __u8 value);
int vc_write_i2c_reg2(struct i2c_client *client, struct vc_csr2 *csr, const __u32 value);
int vc_write_i2c_reg4(struct i2c_client *client, struct vc_csr4 *csr, const __u32 value);

struct i2c_client *vc_mod_get_client(struct device *dev, struct i2c_adapter *adapter, __u8 i2c_addr);

// --- Helper functions for internal data structures --------------------------
void vc_core_print_debug(struct vc_cam *cam);                                   // Only used by NVIDIA driver
struct device *vc_core_get_sen_device(struct vc_cam *cam);
vc_mode vc_core_get_mode(struct vc_cam *cam);
bool vc_mode_is_valid(const vc_mode *mode);
int vc_core_get_current_mode_index(struct vc_cam *cam);
void vc_core_update_mbus_codes(struct vc_cam *cam);
__u32 vc_core_get_num_mbus_codes(struct vc_cam *cam);
__u32 vc_core_get_mbus_code(struct vc_cam *cam, __u32 index);
int vc_core_is_mbus_code_supported(struct vc_cam *cam, __u32 code);
int vc_core_try_format(struct vc_cam *cam, __u32 code);
__u32 vc_core_get_optimized_vmax(struct vc_cam *cam, __u8 num_lanes, __u8 format, __u8 binning_mode, __u32 height);
int vc_core_set_format(struct vc_cam *cam, __u32 code);
__u32 vc_core_get_format(struct vc_cam *cam);
int vc_core_set_frame(struct vc_cam *cam, __u32 left, __u32 top, __u32 width, __u32 height);
struct vc_frame *vc_core_get_frame(struct vc_cam *cam);
// Frames as used by the NXP driver: native = sensor, crop = analog crop, out = output (= frame)
struct vc_frame *vc_core_get_native_frame(struct vc_cam *cam);
struct vc_frame *vc_core_get_crop_frame(struct vc_cam *cam);
int vc_core_set_crop_frame(struct vc_cam *cam, __u32 left, __u32 top, __u32 width, __u32 height);
struct vc_frame *vc_core_get_out_frame(struct vc_cam *cam);
int vc_core_set_out_frame(struct vc_cam *cam, __u32 left, __u32 top, __u32 width, __u32 height);
int vc_core_set_scaling_mode(struct vc_cam *cam, int mode);
int vc_core_set_scale(struct vc_cam *cam, int scale);
int vc_core_enum_mbus_code(struct vc_cam *cam, __u32 index);
__u8 vc_core_mbus_code_to_format(__u32 code);
void vc_core_get_binning_scale(struct vc_cam *cam, __u8 *h_scale, __u8 *v_scale);
int vc_core_set_num_lanes(struct vc_cam *cam, __u32 number);
__u32 vc_core_get_num_lanes(struct vc_cam *cam);
__s32 vc_core_get_lane_datarate(struct vc_cam *cam);
__u8 vc_core_get_bit_depth(__u8 format);
__u64 vc_core_get_link_freq(struct vc_cam *cam);
__u64 vc_core_get_pixel_rate(struct vc_cam *cam);
int vc_core_set_framerate(struct vc_cam *cam, __u32 framerate);
__u32 vc_core_get_framerate(struct vc_cam *cam);
__u32 vc_core_get_time_per_line_ns(struct vc_cam *cam);                         // Only used by NXP driver
int vc_core_set_binning_mode(struct vc_cam *cam, int mode);
__u64 vc_core_mdB_to_times(int mdB);                                            // Only used by NXP driver
int vc_core_times_to_mdB(__u64 times);                                          // Only used by NXP driver
int vc_core_live_roi(struct vc_cam *cam, __u32 scale, __u32 left, __u32 top);
void vc_core_get_live_roi(struct vc_cam *cam, __u32 *scale, __u32 *left, __u32 *top);
#ifdef ENABLE_ADVANCED_CONTROL
int vc_core_set_hmax_overwrite(struct vc_cam *cam, __s32 hmax_overwrite);       // Only used by NXP driver
int vc_core_set_vmax_overwrite(struct vc_cam *cam, __s32 vmax_overwrite);       // Only used by NXP driver
int vc_core_set_width_offset(struct vc_cam *cam, __s32 width_offset);           // Only used by NXP driver
int vc_core_set_vt_syck_div(struct vc_cam *cam, __s32 divider);
__u32 vc_core_get_clk_pixel(struct vc_cam *cam);
int vc_core_set_height_offset(struct vc_cam *cam, __s32 height_offset);         // Only used by NXP driver
#endif

// --- Function to initialize the vc core --------------------------------------
int vc_core_init(struct vc_cam *cam, struct i2c_client *client);
int vc_core_release(struct vc_cam *cam);
int vc_core_update_controls(struct vc_cam *cam);                                // Only used by NVIDIA driver

// --- Functions for the VC MIPI Controller Module ----------------------------
int vc_mod_set_mode(struct vc_cam *cam, int *reset);
__u8 vc_mod_get_mode(struct vc_cam *cam);
int vc_mod_set_trigger_mode(struct vc_cam *cam, int mode);
int vc_mod_get_trigger_mode(struct vc_cam *cam);                                // Only used by NVIDIA driver
int vc_mod_set_single_trigger(struct vc_cam *cam);
int vc_mod_set_io_mode(struct vc_cam *cam, int mode);

// --- Functions for the VC MIPI Sensors --------------------------------------
int vc_sen_set_roi(struct vc_cam *cam);
struct v4l2_rect vc_sen_get_roi(struct vc_cam *cam);
int vc_sen_set_exposure(struct vc_cam *cam, int exposure);
int vc_sen_set_gain(struct vc_cam *cam, __u64 gain, bool unit_is_mdB);
int vc_sen_set_blacklevel(struct vc_cam *cam, __u32 blacklevel);
int vc_sen_start_stream(struct vc_cam *cam);
int vc_sen_stop_stream(struct vc_cam *cam);
int vc_sen_set_hmax(struct vc_cam *cam);
int vc_sen_write_vmax(struct vc_ctrl *ctrl, __u32 vmax);
int vc_mod_reset_module(struct vc_cam *cam, __u8 mode);



#endif // _VC_MIPI_CORE_H
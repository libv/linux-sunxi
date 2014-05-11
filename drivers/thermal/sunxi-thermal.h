#ifndef _SUNXI_THERMAL_H
#define _SUNXI_THERMAL_H

#define ACTIVE_INTERVAL		(500)
#define IDLE_INTERVAL		(10000)
#define MAX_TRIP_COUNT		(8)

/**
 * struct freq_clip_table
 * @freq_clip_max: maximum frequency allowed for this cooling state.
 * @temp_level: Temperature level at which the temperature clipping will
 *	happen.
 * @mask_val: cpumask of the allowed cpu's where the clipping will take place.
 *
 * This structure is required to be filled and passed to the
 * cpufreq_cooling_unregister function.
 */
struct freq_clip_table {
	unsigned int freq_clip_max;
	unsigned int freq_clip_min;
	unsigned int temp_level;
	const struct cpumask *mask_val;
};

struct	thermal_trip_point_conf {
	int trip_val[MAX_TRIP_COUNT];
	int trip_count;
};

struct	thermal_cooling_conf {
	struct freq_clip_table freq_data[MAX_TRIP_COUNT];
	int freq_clip_count;
};

struct thermal_sensor_conf {
	char name[THERMAL_NAME_LENGTH];
	int (*read_temperature)(int index);
	struct thermal_trip_point_conf *trip_data;
	struct thermal_cooling_conf *cooling_data;
	void *private_data;
};

struct sunxi_thermal_zone {
	int id;
	char name[THERMAL_NAME_LENGTH];
	struct thermal_sensor_conf *sunxi_ths_sensor_conf;
	enum thermal_device_mode mode;
	struct thermal_zone_device *therm_dev;
	bool bind;
};

extern int sunxi_ths_register_thermal(struct sunxi_thermal_zone *ths_zone);
extern void sunxi_ths_unregister_thermal(struct sunxi_thermal_zone *ths_zone);

#endif /* _SUNXI_THERMAL_H */

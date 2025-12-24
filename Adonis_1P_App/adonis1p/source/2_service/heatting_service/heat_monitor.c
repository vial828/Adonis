#include "heat_monitor.h"
#include "data_base_info.h"
#include "system_status.h"


typedef struct {
    float x;      // Estimated value
    float p;      // Estimation error covariance
    float q;      // Process noise covariance
    float r;      // Measurement noise covariance
    float k;      // Kalman gain
} KalmanFilter_t;

typedef struct
{
	int16_t sp;         
	int16_t pv;       
	int16_t dciv;
	int16_t dcov;
	int16_t dcoa;
	float dcow;
	float docw_kal;
	float htres;
	int16_t coldt;
	int16_t usbt;
	float power_J;
	uint32_t time; //ms
}HeatInfo_t;

#if 0
//For session log (149.345@208 - 1100.3@3150) from 300 to 3150, print each VALUE[(i+1)*50] - VALUE[i*50], total 57 values
static const float base_delta_J_per_5s[57] = 
{
	17.359, 17.298, 17.237, 17.177, 17.117, 17.057, 16.997, 16.937, 16.878, 16.819, 
	16.760, 16.702, 16.643, 16.584, 16.526, 16.469, 16.411, 16.353, 16.296, 16.239, 
	16.182, 16.125, 16.069, 16.012, 15.957, 15.900, 15.845, 15.789, 15.734, 15.679, 
	15.624, 15.569, 15.515, 15.460, 15.406, 15.352, 15.298, 15.244, 15.191, 15.138, 
	15.085, 15.032, 14.980, 14.927, 14.875, 14.822, 14.771, 14.719, 14.667, 14.616, 
	14.564, 14.514, 14.463, 14.412, 14.362, 14.311, 14.271
};
#endif
//For session log (149.345@208 - 1050@3150) from 300 to 3150, print each VALUE[(i+1)*50] - VALUE[i*50], total 57 values
static const float base_delta_J_per_5s[57] = 
{
	15.700, 15.674, 15.647, 15.620, 15.594, 15.568, 15.541, 15.515, 15.489, 15.462, 
	15.436, 15.410, 15.384, 15.358, 15.331, 15.305, 15.279, 15.253, 15.227, 15.202, 
	15.176, 15.150, 15.124, 15.099, 15.072, 15.047, 15.022, 14.996, 14.971, 14.945, 
	14.920, 14.895, 14.869, 14.844, 14.819, 14.794, 14.768, 14.744, 14.718, 14.694, 
	14.669, 14.644, 14.619, 14.594, 14.569, 14.544, 14.520, 14.495, 14.471, 14.446, 
	14.422, 14.397, 14.373, 14.348, 14.324, 14.299, 14.275
};
//For session log from 300 to 2550, print each VALUE[(i+1)*50] - VALUE[i*50], total 45 values
static const float boost_delta_J_per_5s[45] =
{
	18.791, 17.878, 16.488, 15.759, 15.377, 15.095, 15.501, 15.649, 15.096, 14.818, 
	14.989, 15.371, 15.454, 15.093, 14.907, 14.683, 14.914, 15.199, 14.994, 14.499, 
	14.166, 14.005, 14.083, 14.312, 14.214, 13.900, 13.501, 13.263, 13.470, 13.614, 
	13.508, 13.185, 13.111, 13.178, 13.503, 13.661, 13.499, 13.245, 13.086, 13.064, 
	13.339, 13.543, 13.462, 13.301, 13.187
};
	
/* captured real temperature v.s. energy data during ramp up 10s period*/
static const float rampup_t_e_table[213][2] = {
	{25, 0}, {29, 1.209}, {30, 6.967}, {31, 8.785}, {32, 9.542}, {33, 10.299}, {34, 10.905}, {35, 11.511}, {36, 12.419}, {37, 13.328},
	{38, 14.085}, {39, 14.843}, {40, 15.480}, {41, 16.118}, {42, 16.675}, {43, 17.233}, {44, 17.531}, {45, 17.828}, {46, 18.126},{47, 18.796},
	{48, 19.465}, {49, 20.023}, {50, 20.580}, {51, 20.878}, {52, 21.176}, {53, 21.474}, {54, 21.920}, {55, 22.365}, {56, 22.811}, {57, 23.109},
	{58, 23.406}, {59, 23.704}, {60, 24.373}, {61, 25.043}, {62, 25.601}, {63, 26.158}, {64, 26.536}, {65, 26.913}, {66, 27.291}, {67, 27.897},
	{68, 28.503}, {69, 29.109}, {70, 29.514}, {71, 29.918}, {72, 30.323}, {73, 31.233}, {74, 32.142}, {75, 32.546}, {76, 32.951}, {77, 33.355},
	{78, 33.860}, {79, 34.366}, {80, 34.871}, {81, 35.249}, {82, 35.628}, {83, 36.007}, {84, 36.385}, {85, 36.890}, {86, 37.395}, {87, 37.900},
	{88, 38.506}, {89, 39.111}, {90, 39.717}, {91, 40.121}, {92, 40.526}, {93, 40.930}, {94, 41.249}, {95, 41.568}, {96, 41.887}, {97, 42.206},
	{98, 42.876}, {99, 43.545}, {100, 43.843}, {101, 44.140}, {102, 44.438}, {103, 44.810}, {104, 45.181}, {105, 45.553}, {106, 45.832}, {107, 46.111},
	{108, 46.389}, {109, 46.668}, {110, 47.227}, {111, 47.785}, {112, 48.157}, {113, 48.529}, {114, 48.901}, {115, 49.460}, {116, 50.018}, {117, 50.241},
	{118, 50.465}, {119, 50.688}, {120, 50.911}, {121, 51.357}, {122, 51.803}, {123, 52.249}, {124, 52.928}, {125, 53.606}, {126, 54.111}, {127, 54.616},
	{128, 55.121}, {129, 55.627}, {130, 56.133}, {131, 56.639}, {132, 57.144}, {133, 57.648}, {134, 58.153}, {135, 58.911}, {136, 59.669}, {137, 60.073},
	{138, 60.477}, {139, 60.881}, {140, 61.488}, {141, 62.094}, {142, 62.701}, {143, 63.206}, {144, 63.712}, {145, 64.217}, {146, 64.975}, {147, 65.734},
	{148, 66.037}, {149, 66.340}, {150, 66.644}, {151, 66.947}, {152, 67.474}, {153, 68.001}, {154, 68.528}, {155, 69.085}, {156, 69.642}, {157, 69.940},
	{158, 70.238}, {159, 70.536}, {160, 71.206}, {161, 71.875}, {162, 72.097}, {163, 72.320}, {164, 72.543}, {165, 72.765}, {166, 73.436}, {167, 74.107},
	{168, 74.663}, {169, 75.219}, {170, 75.665}, {171, 76.112}, {172, 76.559}, {173, 77.006}, {174, 77.453}, {175, 78.569}, {176, 78.919}, {177, 79.270},
	{178, 79.620}, {179, 81.440}, {180, 82.046}, {181, 82.652}, {182, 83.157}, {183, 83.661}, {184, 84.166}, {185, 85.077}, {186, 85.988}, {187, 86.594},
	{188, 87.201}, {189, 88.111}, {190, 89.020}, {191, 89.779}, {192, 90.537}, {193, 90.941}, {194, 91.345}, {195, 91.749}, {196, 92.657}, {197, 93.566},
	{198, 93.851}, {199, 94.136}, {200, 94.421}, {201, 94.970}, {202, 95.519}, {203, 96.434}, {204, 96.678}, {205, 96.922}, {206, 97.166}, {207, 98.264},
	{208, 98.630}, {209, 98.997}, {210, 100.096}, {211, 100.462}, {212, 100.828}, {213, 101.740}, {214, 102.290}, {215, 102.839}, {216, 103.566}, {217, 104.455},
	{218, 105.320}, {219, 105.742}, {220, 106.164}, {221, 107.146}, {222, 107.783}, {223, 108.552}, {224, 109.306}, {225, 110.036}, {226, 110.744}, {227, 111.430},
	{228, 112.092}, {229, 113.358}, {230, 113.962}, {231, 114.429}, {232, 115.652}, {233, 117.076}, {234, 118.457}, {235, 119.857}, {236, 121.257}, {237, 122.757},
	{238, 124.176}, {239, 125.557}, {240, 126.957}
};
	
static KalmanFilter_t kf;
static HeatInfo_t heat;
static float rampup_energy = 0; //Energy to be output during the current ramp up.
static float average_p = 0;
static uint16_t full_power_t = 0;
uint8_t delta_t = 0; 
static uint8_t target_delta_t = 0;
static uint16_t delta_t_change_cycle = 0;
static float shc_thres[][2] = 
{
	{160000,1.1},
	{60000,	1.2},
	{0,		1.3}
};
static float current_shc_thres = 1.4;
static uint8_t low_shc_count = 0;
static uint8_t high_shc_count = 0;

static float power_to_keep_temp = 0;

typedef struct{
	uint8_t count; // how many numbers of the current window already passed.
	float J;	// accumulated power till to last cycle of the window scrolling. 
} window_t;

static window_t w[3] = {0};

static void kalman_init( float init_x, float init_p, float q, float r) 
{
    kf.x = init_x;
    kf.p = init_p;
    kf.q = q;
    kf.r = r;
}

static float kalman_update(float z) {

    // Prediction update
    kf.p += kf.q;

    // Measurement update
    kf.k = kf.p / (kf.p + kf.r);
    kf.x += kf.k * (z - kf.x);
    kf.p *= (1 - kf.k);

    return kf.x;

}

static void heat_info_update(HEATER* heater)
{
    SysStatus_u tempSysStatus = get_system_status();
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();

    if (tempSysStatus == HEATTING_BOOST || tempSysStatus == HEATTING_STANDARD || tempSysStatus == HEATTING_CLEAN) 
	{
        heat.pv = (int16_t)((heater->CurrDetectTemp - (float)p_fdb_b_info->fdb_b1_t.tempAdjB/100.0f)/p_fdb_b_info->fdb_b1_t.tempAdjK);
        heat.sp = (int16_t)(heater->CurrTargetTemp);
        heat.dcov = (int16_t)(heater->SetVotage * 1000);
        heat.dcoa = (int16_t)(heater->DetectCurrent * 1000);
        heat.power_J = heater->Heating_J;
    } else {
        heat.dcov = 0;
		heat.dcoa = 0;
        heat.power_J = 0;
    }
    heat.dcow = heater->CurrPowerVal;
	heat.docw_kal = kalman_update(heat.dcow);
	heat.time = heater->HeatingTime;
}

float set_shc_thres( float val )
{
	return(shc_thres[0][1] = val);
}

float get_shc_thres( void )
{
	return(shc_thres[0][1]);
}

float set_delta_t( uint8_t val )
{
	return(delta_t = val);
}

uint8_t get_delta_t(     void )
{
	return delta_t;
}

float set_power( uint8_t val )
{
	//power_to_keep_temp = val >= 18 ? 18 : val;
	return(power_to_keep_temp);
}

float get_power( void )
{
	return power_to_keep_temp;
}

static uint8_t search_rampup_table(uint16_t temperature)
{
	for( uint8_t i = 0; i < sizeof(rampup_t_e_table)/sizeof(rampup_t_e_table[0]); i++ )
	{
		if( temperature == rampup_t_e_table[i][0] )
			return i;
	}
	return 0;
}

void heat_monitor_init(void)
{
	kalman_init(14, 1.000, 0.001, 1.000);
	memset(w, 0, sizeof(w));
	rampup_energy = 0;
	average_p = 0;
	full_power_t = 0;
	delta_t = 0;
	target_delta_t = 0;
	low_shc_count = 0;
	high_shc_count = 0;
	power_to_keep_temp = 0;
	delta_t_change_cycle = 0;
	current_shc_thres = 1.4;
}

// callback each 20ms
void heat_monitor(HEATER* heater)
{
    FDB_area_b1_u* p_fdb_b_info = get_fdb_b_info_handle();
	uint16_t pv = (int16_t)((heater->CurrDetectTemp - (float)p_fdb_b_info->fdb_b1_t.tempAdjB/100.0f)/p_fdb_b_info->fdb_b1_t.tempAdjK);

	float power = 0;
	float shc = 0;

	if( heater->HeatingTime < 10000 )	
	{
		int16_t* ini_p = get_ini_val_info_handle();
		
#ifdef RAMPUP_ENERGY_TABLE		
		//Algorithm i: control the ramp up using rampup search table.
		uint8_t start_temperature_idx = 0;

		if( rampup_energy == 0 )
		{
			start_temperature_idx = search_rampup_table(pv);
			
			//per the index calculate the average power output during rampup phase.
			rampup_energy = rampup_t_e_table[212][1] - rampup_t_e_table[start_temperature_idx][1];
			average_p = rampup_energy / 12.0 ;
			if(average_p * 2 >= ((float)ini_p[DB_FLT_TC_PWR_LIM]/100 + 1.0f) || average_p * 2  > (float)ini_p[DB_FLT_TC_PWR_LIM]/100)
			{
				full_power_t = 2 * rampup_energy * 1000 / (ini_p[DB_FLT_TC_PWR_LIM]/100) - 12000; // in ms
			}
			sm_log( SM_LOG_WARNING, "full_power_t = %d \r\n", full_power_t );
		}
		// output the rampup energy in liner from high to low, total energy output during the first 12s is allocated to each 20ms cycle
		if(ini_p[DB_FLT_TC_PWR_LIM] > 0) 
		{
			if( full_power_t > 0 )
			{
				if( heater->HeatingTime <= full_power_t )
					heater->SetPower = (float)ini_p[DB_FLT_TC_PWR_LIM]/100;
				else
					heater->SetPower = (float)ini_p[DB_FLT_TC_PWR_LIM]/100 * ( 1.0 - (float)(heater->HeatingTime - full_power_t)/ (12000 - full_power_t) ) ;	
			}
			else
			{
				heater->SetPower = average_p * 2 * (1 - (float)heater->HeatingTime / 12000.0 ); 
			}
		}
#else
		if( pv < heater->CurrTargetTemp )
		{
			heater->SetPower = (float)ini_p[DB_FLT_TC_PWR_LIM]/100;
		}
		else
		{
			heater->SetPower = 6.0 * (1 - heater->HeatingTime / 12000) + 2.0;
		}
#endif		
		
		if( pv > 240.0 )
			heater->SetPower = heater->SetPower > 2.5 ? 2.5 : heater->SetPower;

		shc = heater->SetPower * 100 / (pv - delta_t);
	}
	else if( heater->HeatingTime > 15000 )
	{	
		
		uint8_t is_exceeded = FALSE;

		//SHC detection
		shc = heater->SetPower * 100 / (pv - delta_t);
		//heater->Heating_J = shc;
		
		//Algorithm ii: check the real time energy curve with golden curve.
		if( heater->HeatingTime <= 30000 )
		{
			for( uint8_t i = 0; i < 3; i++ )
			{
				w[i].J = heater->Heating_J;
			}
		}
		else
		{
			const float* p_J_ref = (get_system_status() == HEATTING_BOOST)? boost_delta_J_per_5s : base_delta_J_per_5s;
			uint8_t size_of_J_ref = (get_system_status() == HEATTING_BOOST)? sizeof(boost_delta_J_per_5s)/sizeof(boost_delta_J_per_5s[0]) :
														sizeof(base_delta_J_per_5s)/sizeof(base_delta_J_per_5s[0]);
			
			for( uint8_t i = 0; i < 3; i++ )
			{
				if( heater->HeatingTime / 100 >= 300 + (w[i].count + 1) * (i+1) * 50 ) // window length = (i+1) * 10s
				{
					uint16_t current_idx = (w[i].count + 1) * (i+1) - 1; //from 0 to 56(base) or 44(boost)

					// get total golden delta energy inside a window
					float w_delta_ref = 0;
					//sm_log( SM_LOG_WARNING, "w_delta_ref of W%d = ", i );
					for( uint8_t j = 0; j <= i; j++ )
					{
						w_delta_ref += p_J_ref[current_idx - j];
						//if( j <  i )
						//	sm_log( SM_LOG_WARNING, "p_J_ref[%d] + ", current_idx - j );
						//else
						//	sm_log( SM_LOG_WARNING, "p_J_ref[%d]\r\n ", current_idx - j );
					}
					
					if( heater->Heating_J - w[i].J > w_delta_ref )
					{
						is_exceeded = TRUE;
						sm_log( SM_LOG_WARNING, "w[%d] %ds exceeded. %.3f > %.3f \r\n", i, (i+1) * 5, heater->Heating_J - w[i].J, w_delta_ref );
					}
					w[i].J = heater->Heating_J;
					w[i].count++;
				}
			}
		}
		
		if( is_exceeded )
		{
			target_delta_t += target_delta_t > 20? 0 : 5;
			delta_t_change_cycle = 400; // 8s 
			sm_log( SM_LOG_WARNING, "target_delta_t = %d \r\n", target_delta_t );
		}
		else 
		{
			//Algorithm iii: check the real time SHC
			//dynamic SHC update
			for( uint8_t i = 0; i < sizeof(shc_thres)/sizeof(shc_thres[0]); i++ )
			{
				if( heater->HeatingTime >= shc_thres[i][0] && current_shc_thres > shc_thres[i][1] )
				{
					current_shc_thres = shc_thres[i][1];
					sm_log( SM_LOG_WARNING, "current_shc_thres = %.3f \r\n", current_shc_thres );
					break;
				}
					
			}

			if( delta_t == (int16_t)(target_delta_t * p_fdb_b_info->fdb_b1_t.tempAdjK + (target_delta_t == 0? 0 : p_fdb_b_info->fdb_b1_t.tempAdjB/100.0f)) )
			{
				if( shc < current_shc_thres )
				{
					high_shc_count = 0;
					if( pv < heater->CurrTargetTemp + 2  ) //todo: add scenario when stick is fired and temp. ramp up even with power 0
					{
						if( ++low_shc_count > 200 ) // detect low shc for more than 4s
						{
							low_shc_count = 0;
							target_delta_t += target_delta_t > 20? 0 : 5;
							delta_t_change_cycle = 400; // 8s 
							sm_log( SM_LOG_WARNING, "t: %d shc %.3f. %d. %d t_delta %d up\r\n", heater->HeatingTime, shc, (uint16_t)heater->CurrTargetTemp, pv, target_delta_t);
							if( target_delta_t >= 30) //stop heat
							{
								//set_system_status(IDLE);
								//sm_log( SM_LOG_WARNING, "t: %d SHC low timeout stop heat!!\r\n", heat.time );
							}
						}
					}
				}
				else 
				{
					low_shc_count = 0;
					if( pv > heater->CurrTargetTemp - 3 ) // only when temp. is close to target temp. the SHC is more stable to detect
					{
						if( ++high_shc_count > 200 ) // detect high shc for more than 4s
						{
							high_shc_count = 0;
							if( target_delta_t >= 5 )
							{
								target_delta_t -= 5;
								delta_t_change_cycle = 400; // 8s
								sm_log( SM_LOG_WARNING, "t: %d shc %.3f. %d. %d t_delta %d down\r\n", heater->HeatingTime, shc, (uint16_t)heater->CurrTargetTemp, pv, target_delta_t);
							}
						}
					}
				}
			}
		}

		if( delta_t_change_cycle > 0 && delta_t !=  (int16_t)(target_delta_t * p_fdb_b_info->fdb_b1_t.tempAdjK + (target_delta_t == 0? 0 : p_fdb_b_info->fdb_b1_t.tempAdjB/100.0f)) )
		{
			//delta_t roll back to target value in step of 200ms, so roll back of a gap value 10 will need 2ses 
			if( delta_t_change_cycle % 10 == 0 )
			{
				delta_t += (int16_t)( target_delta_t * p_fdb_b_info->fdb_b1_t.tempAdjK + (target_delta_t == 0? 0 : p_fdb_b_info->fdb_b1_t.tempAdjB/100.0f) ) > delta_t ? 
				1 : ( (int16_t)(target_delta_t * p_fdb_b_info->fdb_b1_t.tempAdjK + (target_delta_t == 0? 0 : p_fdb_b_info->fdb_b1_t.tempAdjB/100.0f) ) == delta_t ? 0: -1 );
				if( delta_t ==  (int16_t)(target_delta_t * p_fdb_b_info->fdb_b1_t.tempAdjK + (target_delta_t == 0? 0 : p_fdb_b_info->fdb_b1_t.tempAdjB/100.0f) ) )
					sm_log( SM_LOG_WARNING, "delta_t = %d \r\n", delta_t);
			}
			delta_t_change_cycle--;
		}
	}
}



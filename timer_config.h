//TimeSpecs use 24hr time
TimeSpec on_times[] =  {TimeSpec(8, 0, 0)}; //Add TimeSpecs as desired
TimeSpec off_times[] = {TimeSpec(16, 0, 0)};
//array format: {TimeSpec(hr, min, sec), TimeSpec(hr, min,sec)}
//values of 8 and 9 cannot be inserted as 08 or 09 (leading 0 specifies base 8)
